#include <windows.h>
#include <audioclient.h>
#include <ksmedia.h>
#include <mmdeviceapi.h>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <vector>

namespace {

void report(const char *message, HRESULT result = S_OK) {
    if (FAILED(result)) {
        std::fprintf(stderr, "%s (HRESULT 0x%08lx)\n", message,
                     static_cast<unsigned long>(result));
    } else {
        std::fprintf(stderr, "%s\n", message);
    }
}

float sampleAsFloat(const BYTE *data, int index, int bits, bool isFloat) {
    if (isFloat && bits == 32)
        return reinterpret_cast<const float *>(data)[index];
    if (bits == 16)
        return reinterpret_cast<const int16_t *>(data)[index] / 32768.0f;
    if (bits == 32)
        return reinterpret_cast<const int32_t *>(data)[index] / 2147483648.0f;
    if (bits == 24) {
        const BYTE *sample = data + index * 3;
        const int32_t value = (static_cast<int32_t>(sample[0]) |
                               (static_cast<int32_t>(sample[1]) << 8) |
                               (static_cast<int32_t>(sample[2]) << 16));
        return ((value & 0x800000) ? (value | ~0xFFFFFF) : value) / 8388608.0f;
    }
    return 0.0f;
}

int16_t toPcm16(float sample) {
    sample = std::clamp(sample, -1.0f, 1.0f);
    return static_cast<int16_t>(sample * 32767.0f);
}

} // namespace

int main() {
    if (FAILED(CoInitializeEx(nullptr, COINIT_MULTITHREADED))) {
        report("CoInitializeEx failed");
        return 1;
    }

    IMMDeviceEnumerator *enumerator = nullptr;
    IMMDevice *device = nullptr;
    IAudioClient *audioClient = nullptr;
    IAudioCaptureClient *captureClient = nullptr;
    WAVEFORMATEX *format = nullptr;
    int exitCode = 1;

    do {
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL,
                                      __uuidof(IMMDeviceEnumerator),
                                      reinterpret_cast<void **>(&enumerator));
        if (FAILED(hr)) { report("Could not create audio device enumerator", hr); break; }

        hr = enumerator->GetDefaultAudioEndpoint(eRender, eConsole, &device);
        if (FAILED(hr)) { report("Could not find the default output device", hr); break; }

        hr = device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr,
                              reinterpret_cast<void **>(&audioClient));
        if (FAILED(hr)) { report("Could not activate the output device", hr); break; }

        hr = audioClient->GetMixFormat(&format);
        if (FAILED(hr)) { report("Could not read the output format", hr); break; }

        hr = audioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
                                     AUDCLNT_STREAMFLAGS_LOOPBACK, 0, 0, format, nullptr);
        if (FAILED(hr)) { report("Could not initialize WASAPI loopback", hr); break; }

        hr = audioClient->GetService(__uuidof(IAudioCaptureClient),
                                     reinterpret_cast<void **>(&captureClient));
        if (FAILED(hr)) { report("Could not open WASAPI capture", hr); break; }

        const bool isFloat = format->wFormatTag == WAVE_FORMAT_IEEE_FLOAT ||
            (format->wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
             reinterpret_cast<WAVEFORMATEXTENSIBLE *>(format)->SubFormat ==
                 KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
        const int channels = std::max(1, static_cast<int>(format->nChannels));
        const int bits = format->wBitsPerSample;

        hr = audioClient->Start();
        if (FAILED(hr)) { report("Could not start WASAPI loopback", hr); break; }

        while (true) {
            Sleep(10);
            UINT32 packetFrames = 0;
            hr = captureClient->GetNextPacketSize(&packetFrames);
            if (FAILED(hr)) break;

            while (packetFrames > 0) {
                BYTE *data = nullptr;
                UINT32 frames = 0;
                DWORD flags = 0;
                hr = captureClient->GetBuffer(&data, &frames, &flags, nullptr, nullptr);
                if (FAILED(hr)) break;

                std::vector<int16_t> output(static_cast<size_t>(frames) * 2);
                if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                    for (UINT32 frame = 0; frame < frames; ++frame) {
                        const int offset = static_cast<int>(frame) * channels;
                        const float left = sampleAsFloat(data, offset, bits, isFloat);
                        const float right = sampleAsFloat(data, offset + (channels > 1 ? 1 : 0), bits, isFloat);
                        output[frame * 2] = toPcm16(left);
                        output[frame * 2 + 1] = toPcm16(right);
                    }
                }

                std::fwrite(output.data(), sizeof(int16_t), output.size(), stdout);
                std::fflush(stdout);
                captureClient->ReleaseBuffer(frames);
                hr = captureClient->GetNextPacketSize(&packetFrames);
                if (FAILED(hr)) break;
            }
            if (FAILED(hr)) break;
        }

        audioClient->Stop();
        exitCode = 0;
    } while (false);

    if (format) CoTaskMemFree(format);
    if (captureClient) captureClient->Release();
    if (audioClient) audioClient->Release();
    if (device) device->Release();
    if (enumerator) enumerator->Release();
    CoUninitialize();
    return exitCode;
}
