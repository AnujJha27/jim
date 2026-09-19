#include "search_everywhere.h"

#include <QAction>
#include <QColor>
#include <QEvent>
#include <QFileInfo>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QVBoxLayout>

SearchEverywhere::SearchEverywhere(QWidget *parent) : QDialog(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setModal(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    QWidget *container = new QWidget();
    container->setObjectName("seContainer");
    container->setStyleSheet(
        "#seContainer {"
        "  background-color: #1a1a2e;"
        "  border: 1px solid #569cd6;"
        "  border-radius: 10px;"
        "}"
    );
    QVBoxLayout *cl = new QVBoxLayout(container);
    cl->setContentsMargins(10, 10, 10, 10);
    cl->setSpacing(6);

    searchBox = new QLineEdit();
    searchBox->setPlaceholderText("🔍  Search files, actions, recent...");
    searchBox->setStyleSheet(
        "QLineEdit {"
        "  background: #252540;"
        "  color: #cdd6f4;"
        "  border: none;"
        "  border-radius: 6px;"
        "  padding: 8px 12px;"
        "  font-size: 15px;"
        "  font-family: Consolas;"
        "}"
    );
    cl->addWidget(searchBox);

    resultList = new QListWidget();
    resultList->setStyleSheet(
        "QListWidget {"
        "  background: transparent;"
        "  color: #cdd6f4;"
        "  border: none;"
        "  font-family: Consolas;"
        "  font-size: 12px;"
        "}"
        "QListWidget::item { padding: 4px 8px; border-radius: 4px; }"
        "QListWidget::item:selected { background-color: #2d4a6e; color: #ffffff; }"
        "QListWidget::item:hover { background-color: #1e3a5a; }"
    );
    resultList->setMaximumHeight(300);
    cl->addWidget(resultList);

    layout->addWidget(container);
    setFixedWidth(580);

    connect(searchBox, &QLineEdit::textChanged, this, &SearchEverywhere::filter);
    connect(resultList, &QListWidget::itemDoubleClicked, this, &SearchEverywhere::runSelected);
    searchBox->installEventFilter(this);

    if (parent) {
        QPoint center = parent->geometry().center();
        move(center.x() - width() / 2, center.y() - height() / 2 - 100);
    }
}

void SearchEverywhere::populate(const QList<QAction*> &actions,
                                const QStringList &recentFiles,
                                const QStringList &openFiles,
                                const QList<Symbol> &symbols,
                                bool commandsOnlyMode) {
    allActions = actions;
    allRecent = recentFiles;
    allFiles = openFiles;
    allSymbols = symbols;
    commandsOnly = commandsOnlyMode;
    searchBox->clear();
    filter("");
    searchBox->setFocus();
}

void SearchEverywhere::filter(const QString &text) {
    resultList->clear();
    QString q = text.trimmed().toLower();

    if (!commandsOnly) for (const QString &f : allFiles) {
        QString bn = QFileInfo(f).fileName();
        if (q.isEmpty() || bn.toLower().contains(q) || f.toLower().contains(q)) {
            QListWidgetItem *item = new QListWidgetItem("📄  " + bn);
            item->setData(Qt::UserRole, "file:" + f);
            item->setToolTip(f);
            resultList->addItem(item);
        }
    }

    if (!commandsOnly) for (const QString &f : allRecent) {
        QString bn = QFileInfo(f).fileName();
        if (allFiles.contains(f)) {
            continue;
        }
        if (q.isEmpty() || bn.toLower().contains(q) || f.toLower().contains(q)) {
            QListWidgetItem *item = new QListWidgetItem("🕐  " + bn + "  [recent]");
            item->setData(Qt::UserRole, "file:" + f);
            item->setForeground(QColor("#888888"));
            resultList->addItem(item);
        }
    }

    if (!commandsOnly) for (const Symbol &symbol : allSymbols) {
        if (q.isEmpty() || symbol.name.toLower().contains(q) ||
            symbol.filePath.toLower().contains(q)) {
            auto *item = new QListWidgetItem("[symbol] " + symbol.name);
            item->setData(Qt::UserRole, "symbol:" + symbol.filePath + ":" + QString::number(symbol.line));
            item->setToolTip(symbol.filePath + ":" + QString::number(symbol.line + 1));
            resultList->addItem(item);
        }
    }

    for (QAction *act : allActions) {
        if (act->isSeparator() || act->text().isEmpty() || !act->isEnabled()) {
            continue;
        }
        QString name = act->text().remove('&');
        if (q.isEmpty() || name.toLower().contains(q)) {
            QListWidgetItem *item = new QListWidgetItem("⚡  " + name);
            item->setData(Qt::UserRole, "action:" + name);
            item->setForeground(QColor("#569cd6"));
            resultList->addItem(item);
        }
    }

    if (resultList->count() > 0) {
        resultList->setCurrentRow(0);
    }
}

void SearchEverywhere::runSelected() {
    QListWidgetItem *item = resultList->currentItem();
    if (!item) {
        return;
    }
    QString data = item->data(Qt::UserRole).toString();
    if (data.startsWith("file:")) {
        emit fileRequested(data.mid(5));
        accept();
    } else if (data.startsWith("symbol:")) {
        const QString encoded = data.mid(7);
        const int separator = encoded.lastIndexOf(':');
        if (separator > 0) {
            emit symbolRequested(encoded.left(separator), encoded.mid(separator + 1).toInt());
            accept();
        }
    } else if (data.startsWith("action:")) {
        QString name = data.mid(7);
        for (QAction *act : allActions) {
            if (act->text().remove('&') == name) {
                act->trigger();
                break;
            }
        }
        accept();
    }
}

void SearchEverywhere::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Escape) {
        reject();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        runSelected();
        return;
    }
    if (event->key() == Qt::Key_Down) {
        int next = qMin(resultList->currentRow() + 1, resultList->count() - 1);
        resultList->setCurrentRow(next);
        return;
    }
    if (event->key() == Qt::Key_Up) {
        int prev = qMax(resultList->currentRow() - 1, 0);
        resultList->setCurrentRow(prev);
        return;
    }
    QDialog::keyPressEvent(event);
}

bool SearchEverywhere::eventFilter(QObject *obj, QEvent *event) {
    if (obj == searchBox && event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Down || ke->key() == Qt::Key_Up ||
            ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter ||
            ke->key() == Qt::Key_Escape) {
            keyPressEvent(ke);
            return true;
        }
    }
    return QDialog::eventFilter(obj, event);
}
