#include "audiomonitor.h"
#include <cmath>

#ifdef Q_OS_WIN
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>

// Manually define the GUIDs as constants to avoid linker issues without using initguid.h
static const GUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_LOCAL = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };
static const GUID KSDATAFORMAT_SUBTYPE_PCM_LOCAL        = { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71 } };

#ifndef WAVE_FORMAT_IEEE_FLOAT
#define WAVE_FORMAT_IEEE_FLOAT 0x0003
#endif

#else // Linux / macOS — Qt Multimedia loopback capture

#include <QAudioSource>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QIODevice>
#include <QEventLoop>
#include <QTimer>

// Processes raw PCM bytes into 64 amplitude bands and notifies the monitor.
class AudioCaptureIO : public QIODevice {
public:
    AudioCaptureIO(QVector<float>& levels, QMutex& mutex,
                   const QAudioFormat& fmt, AudioMonitor* monitor)
        : m_levels(levels), m_mutex(mutex), m_format(fmt), m_monitor(monitor) {}

    qint64 readData(char*, qint64) override { return 0; }

    qint64 writeData(const char* data, qint64 len) override {
        const int channels    = m_format.channelCount();
        const int bps         = m_format.bytesPerSample();
        const bool isFloat    = (m_format.sampleFormat() == QAudioFormat::Float);
        const int numFrames   = static_cast<int>(len) / (channels * bps);
        const int blockSize   = qMax(1, numFrames / 64);

        QMutexLocker locker(&m_mutex);
        for (int i = 0; i < 64; i++) {
            float sum = 0;
            int   cnt = 0;
            for (int j = 0; j < blockSize; j++) {
                int fi = i * blockSize + j;
                if (fi >= numFrames) break;
                int off = fi * channels; // use first channel
                float sample = 0;
                if (isFloat) {
                    sample = reinterpret_cast<const float*>(data)[off];
                } else if (bps == 2) {
                    sample = reinterpret_cast<const int16_t*>(data)[off] / 32768.0f;
                } else if (bps == 4) {
                    sample = reinterpret_cast<const int32_t*>(data)[off] / 2147483648.0f;
                }
                sum += sample * sample;
                cnt++;
            }
            float rms   = (cnt > 0) ? sqrtf(sum / cnt) : 0.0f;
            float level = rms * 5.0f;
            m_levels[i] = m_levels[i] * 0.7f + level * 0.3f;
            if (m_levels[i] > 1.0f) m_levels[i] = 1.0f;
        }
        // Emit cross-thread via queued connection
        QMetaObject::invokeMethod(m_monitor, "levelsUpdated", Qt::QueuedConnection);
        return len;
    }

private:
    QVector<float>& m_levels;
    QMutex&         m_mutex;
    QAudioFormat    m_format;
    AudioMonitor*   m_monitor;
};

#endif // Q_OS_WIN

AudioMonitor::AudioMonitor(QObject *parent) : QThread(parent) {
    m_levels.resize(64);
    m_levels.fill(0.0f);
}

AudioMonitor::~AudioMonitor() {
    stop();
    wait();
}

void AudioMonitor::stop() {
    m_running = false;
}

QVector<float> AudioMonitor::getLevels() {
    QMutexLocker locker(&m_mutex);
    return m_levels;
}

void AudioMonitor::run() {
    m_running = true;

#ifdef Q_OS_WIN
    CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    IMMDeviceEnumerator* pEnumerator = nullptr;
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pwfx = nullptr;

    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (void**)&pEnumerator);
    if (FAILED(hr)) {
        CoUninitialize();
        return;
    }

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    if (FAILED(hr)) {
        IMMDeviceCollection* pCollection = nullptr;
        hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pCollection);
        if (SUCCEEDED(hr)) {
            UINT count = 0;
            pCollection->GetCount(&count);
            if (count > 0)
                hr = pCollection->Item(0, &pDevice);
            pCollection->Release();
        }
    }

    if (FAILED(hr) || !pDevice) {
        pEnumerator->Release();
        CoUninitialize();
        return;
    }

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient);
    if (FAILED(hr)) {
        pDevice->Release(); pEnumerator->Release(); CoUninitialize(); return;
    }

    hr = pAudioClient->GetMixFormat(&pwfx);
    if (FAILED(hr)) {
        pAudioClient->Release(); pDevice->Release(); pEnumerator->Release(); CoUninitialize(); return;
    }

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED, AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, pwfx, nullptr);
    if (SUCCEEDED(hr)) {
        hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
        if (SUCCEEDED(hr)) {
            pAudioClient->Start();
            while (m_running) {
                msleep(16);
                UINT32 packetLength = 0;
                hr = pCaptureClient->GetNextPacketSize(&packetLength);
                while (packetLength != 0 && m_running) {
                    BYTE* pData; UINT32 numFramesAvailable; DWORD flags;
                    hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
                    if (FAILED(hr)) break;
                    if (flags & AUDCLNT_BUFFERFLAGS_SILENT) {
                        m_mutex.lock();
                        for (int i = 0; i < 64; i++) m_levels[i] *= 0.75f;
                        m_mutex.unlock();
                    } else {
                        int channels  = pwfx->nChannels;
                        int bits      = pwfx->wBitsPerSample;
                        int blockSize = qMax(1, (int)numFramesAvailable / 64);
                        m_mutex.lock();
                        for (int i = 0; i < 64; i++) {
                            float sum = 0; int count = 0;
                            for (int j = 0; j < blockSize; j++) {
                                int frameIdx = i * blockSize + j;
                                if (frameIdx >= (int)numFramesAvailable) break;
                                int sampleOffset = frameIdx * channels;
                                float sample = 0;
                                bool isFloat = (pwfx->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
                                               (pwfx->wFormatTag == WAVE_FORMAT_EXTENSIBLE && ((WAVEFORMATEXTENSIBLE*)pwfx)->SubFormat == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_LOCAL));
                                if (isFloat) {
                                    if (sampleOffset < (int)(numFramesAvailable * channels))
                                        sample = ((float*)pData)[sampleOffset];
                                } else if (bits == 16) {
                                    if (sampleOffset < (int)(numFramesAvailable * channels))
                                        sample = ((short*)pData)[sampleOffset] / 32768.0f;
                                } else if (bits == 32) {
                                    if (sampleOffset < (int)(numFramesAvailable * channels))
                                        sample = ((int*)pData)[sampleOffset] / 2147483648.0f;
                                } else if (bits == 24) {
                                    BYTE* pSample = pData + (sampleOffset * 3);
                                    if ((sampleOffset * 3 + 2) < (int)(numFramesAvailable * channels * 3)) {
                                        int val = (pSample[0] << 8) | (pSample[1] << 16) | (pSample[2] << 24);
                                        sample = (val / 256) / 8388608.0f;
                                    }
                                }
                                sum += sample * sample; count++;
                            }
                            float rms = (count > 0) ? sqrt(sum / count) : 0;
                            float level = rms * 5.0f;
                            m_levels[i] = m_levels[i] * 0.7f + level * 0.3f;
                            if (m_levels[i] > 1.0f) m_levels[i] = 1.0f;
                        }
                        m_mutex.unlock();
                    }
                    hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
                    if (FAILED(hr)) break;
                    hr = pCaptureClient->GetNextPacketSize(&packetLength);
                }
                emit levelsUpdated();
            }
            pAudioClient->Stop();
        }
    } else {
        while (m_running) {
            msleep(50);
            m_mutex.lock();
            for (int i = 0; i < 64; i++) m_levels[i] *= 0.8f;
            m_mutex.unlock();
            emit levelsUpdated();
        }
    }

    if (pwfx) CoTaskMemFree(pwfx);
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    if (pEnumerator) pEnumerator->Release();
    CoUninitialize();

#else
    // Linux / macOS: capture via Qt Multimedia.
    // DJ Mode is output visualization only. Never fall back to a microphone.
    QAudioDevice device;
    for (const QAudioDevice& dev : QMediaDevices::audioInputs()) {
        if (dev.description().contains("monitor", Qt::CaseInsensitive) ||
            QString::fromUtf8(dev.id()).contains("monitor", Qt::CaseInsensitive)) {
            device = dev;
            break;
        }
    }

    if (device.isNull()) {
        emit captureStatus("DJ Mode: no system-output monitor is available. "
                           "Enable a PulseAudio/PipeWire monitor source.");
        while (m_running)
            msleep(100);
        return;
    }

    emit captureStatus("DJ Mode: capturing " + device.description());

    QAudioFormat format;
    format.setSampleRate(44100);
    format.setChannelCount(2);
    format.setSampleFormat(QAudioFormat::Int16);
    if (!device.isFormatSupported(format))
        format = device.preferredFormat();

    QAudioSource* source = new QAudioSource(device, format);
    AudioCaptureIO* io   = new AudioCaptureIO(m_levels, m_mutex, format, this);
    io->open(QIODevice::WriteOnly);
    source->start(io);

    // Run an event loop so QAudioSource can deliver data callbacks,
    // and poll m_running to stop cleanly.
    QEventLoop loop;
    QTimer stopChecker;
    stopChecker.setInterval(50);
    QObject::connect(&stopChecker, &QTimer::timeout, [&]() {
        if (!m_running) loop.quit();
    });
    stopChecker.start();
    loop.exec();

    source->stop();
    delete io;
    delete source;
#endif
}
