#pragma once
#include <QThread>
#include <QVector>
#include <QMutex>

class QString;

class AudioMonitor : public QThread {
    Q_OBJECT
public:
    explicit AudioMonitor(QObject *parent = nullptr);
    ~AudioMonitor() override;
    
    void stop();
    QVector<float> getLevels();

signals:
    void levelsUpdated();
    void captureStatus(const QString &status);

protected:
    void run() override;

private:
    bool m_running = true;
    QVector<float> m_levels;
    QMutex m_mutex;
};
