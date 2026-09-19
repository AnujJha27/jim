#include "terminal_widget.h"

#include <QDir>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QVBoxLayout>

TerminalWidget::TerminalWidget(QWidget *parent)
    : QWidget(parent), process(nullptr) {
    setupUI();
}

TerminalWidget::~TerminalWidget() {
    if (process) {
        process->kill();
        process->waitForFinished(1000);
    }
}

void TerminalWidget::setupUI() {
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *header = new QWidget();
    QHBoxLayout *headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(12, 6, 12, 6);

    QLabel *termLabel = new QLabel("TERMINAL");
    termLabel->setStyleSheet("color: #cccccc; font-size: 11px; font-weight: 600; "
                             "letter-spacing: 1px;");
    headerLayout->addWidget(termLabel);
    headerLayout->addStretch();

    header->setStyleSheet("background-color: #252526; border-bottom: 1px solid "
                          "#3e3e42;");
    layout->addWidget(header);

    output = new QPlainTextEdit();
    output->setReadOnly(true);
    output->setFont(QFont("Consolas", 10));
    output->setStyleSheet(
        "QPlainTextEdit { background-color: #1e1e1e; color: #cccccc; border: "
        "none; padding: 8px; selection-background-color: #264f78; }");
    output->setMaximumBlockCount(5000);
    layout->addWidget(output);

    input = new QLineEdit();
    input->setFont(QFont("Consolas", 10));
    input->setStyleSheet(
        "QLineEdit { background-color: #1e1e1e; color: #cccccc; border: none; "
        "border-top: 1px solid #3e3e42; padding: 8px; } "
        "QLineEdit:focus { border-top: 1px solid #007acc; }");
    input->setPlaceholderText("Type command and press Enter...");
    connect(input, &QLineEdit::returnPressed, this,
            &TerminalWidget::executeCommand);
    layout->addWidget(input);

    currentDir = QDir::homePath();
    setStyleSheet("background-color: #1e1e1e;");
    startShell();
}

void TerminalWidget::setWorkingDirectory(const QString &dir) {
    QDir d(dir);
    if (d.exists()) {
        currentDir = d.absolutePath();
        if (process && process->state() == QProcess::Running) {
#ifdef Q_OS_WIN
            process->write(QString("cd /d \"%1\"\r\n").arg(currentDir).toLocal8Bit());
#else
            QString quoted = currentDir;
            quoted.replace("'", "'\\''");
            process->write(QString("cd -- '%1'\n").arg(quoted).toLocal8Bit());
#endif
        }
    }
}

void TerminalWidget::startShell() {
    if (process && process->state() != QProcess::NotRunning)
        return;

    process = new QProcess(this);
    process->setWorkingDirectory(currentDir);
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, &QProcess::readyRead, this, [this]() {
        output->appendPlainText(QString::fromLocal8Bit(process->readAll()));
    });
    connect(process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        if (process)
            appendOutput(process->errorString());
    });
    QProcess *shellProcess = process;
    connect(shellProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, shellProcess]() {
                if (process == shellProcess)
                    process = nullptr;
                shellProcess->deleteLater();
            });

#ifdef Q_OS_WIN
    process->start("cmd.exe");
#else
    const QString shellProgram = QProcessEnvironment::systemEnvironment().value("SHELL", "/bin/sh");
    process->start(shellProgram);
#endif
}

void TerminalWidget::executeCommand() {
    QString cmd = input->text().trimmed();
    if (cmd.isEmpty()) {
        return;
    }

    output->appendPlainText("> " + cmd);
    input->clear();

    if (cmd == "clear" || cmd == "cls") {
        output->clear();
        return;
    }

    startShell();
    if (!process || (!process->waitForStarted(1000) && process->state() != QProcess::Running)) {
        output->appendPlainText("Could not start shell");
        return;
    }
    process->write(cmd.toLocal8Bit());
    process->write("\n");
}

void TerminalWidget::appendOutput(const QString &text) {
    output->appendPlainText(text);
}
