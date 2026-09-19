#include "aiautocomplete.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QTextCursor>
#include <QDebug>

AIAutocomplete::AIAutocomplete(QObject *parent)
    : QObject(parent), m_enabled(false)
{
    m_networkManager = new QNetworkAccessManager(this);
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    m_debounceTimer->setInterval(400);

    connect(m_debounceTimer, &QTimer::timeout, this, &AIAutocomplete::fetchAutocomplete);
}

QList<AIAutocomplete::Provider> AIAutocomplete::getProviders()
{
    return {
        {"Groq", "https://api.groq.com/openai/v1", "llama3-8b-8192", true},
        {"OpenRouter", "https://openrouter.ai/api/v1", "google/gemini-pro-1.5-exp", true},
        {"Together AI", "https://api.together.xyz/v1", "codellama/CodeLlama-13b-Instruct-hf", true},
        {"OpenAI", "https://api.openai.com/v1", "gpt-3.5-turbo", false},
        {"Local (llama-server)", "http://localhost:8080", "default", true}
    };
}

void AIAutocomplete::setProvider(const QString &baseUrl, const QString &apiKey, const QString &model)
{
    m_baseUrl = baseUrl;
    m_apiKey = apiKey;
    m_model = model;
}

void AIAutocomplete::trigger(QPlainTextEdit *editor)
{
    if (!m_enabled || m_baseUrl.isEmpty()) return;
    
    m_currentEditor = editor;
    if (m_reply) {
        m_reply->abort();
        m_reply = nullptr;
    }
    m_debounceTimer->start();
}

void AIAutocomplete::fetchAutocomplete()
{
    if (!m_currentEditor) return;

    QTextCursor cursor = m_currentEditor->textCursor();
    QString fullText = m_currentEditor->toPlainText();
    int pos = cursor.position();
    m_requestPosition = pos;
    m_requestRevision = m_currentEditor->document()->revision();

    // Context windows
    QString prefix = fullText.left(pos).right(2000);
    QString suffix = fullText.mid(pos).left(1000);

    QUrl url(m_baseUrl + "/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_apiKey.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_apiKey).toUtf8());
    }

    QJsonObject json;
    json["model"] = m_model;
    
    QJsonArray messages;
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = "You are a code autocomplete assistant. Provide only the text that should follow the given prefix to complete the code intelligently. Do not explain anything.";
    messages.append(systemMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = QString("Complete this code.\n\nPREFIX:\n%1\n\nSUFFIX:\n%2").arg(prefix, suffix);
    messages.append(userMsg);

    json["messages"] = messages;
    json["max_tokens"] = 50;
    json["temperature"] = 0.2;
    json["stream"] = false;

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(json).toJson());
    m_reply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        onReplyFinished(reply);
    });
}

void AIAutocomplete::onReplyFinished(QNetworkReply *reply)
{
    if (m_reply != reply) {
        reply->deleteLater();
        return;
    }
    m_reply = nullptr;
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject root = doc.object();
        
        QJsonArray choices = root["choices"].toArray();
        if (!choices.isEmpty()) {
            QString content = choices[0].toObject()["message"].toObject()["content"].toString();
            emit suggestionReady(m_currentEditor, m_requestPosition, m_requestRevision,
                                 content.trimmed());
        }
    } else {
        emit errorOccurred(reply->errorString());
    }
    reply->deleteLater();
}

void AIAutocomplete::onTimeout()
{
    // Placeholder
}
