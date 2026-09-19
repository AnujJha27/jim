#include "texteditor_private.h"

void TextEditor::applyModernStyle() {
  QString style = R"(
        QMainWindow {
            background-color: #1e1e1e;
            border: none;
        }
        QWidget {
            background-color: #1e1e1e;
            color: #cccccc;
            font-family: 'Segoe UI', 'Roboto', sans-serif;
        }
        QMenuBar {
            background-color: #323233;
            color: #cccccc;
            border: none;
            border-bottom: 1px solid #1e1e1e;
            padding: 0px;
            font-size: 13px;
        }
        QMenuBar::item {
            padding: 8px 14px;
            background: transparent;
            border-radius: 0px;
        }
        QMenuBar::item:selected {
            background-color: #505050;
        }
        QMenuBar::item:pressed {
            background-color: #094771;
        }
        QMenu {
            background-color: #2d2d30;
            color: #cccccc;
            border: 1px solid #454545;
            border-radius: 8px;
            padding: 6px;
            font-size: 13px;
        }
        QMenu::item {
            padding: 8px 28px 8px 16px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background-color: #094771;
        }
        QMenu::separator {
            height: 1px;
            background-color: #3e3e42;
            margin: 4px 10px;
        }
        QTabWidget::pane {
            border: none;
            background-color: #1e1e1e;
            top: -1px;
        }
        QTabBar {
            background-color: #252526;
        }
        QTabBar::tab {
            background-color: #2d2d30;
            color: #969696;
            padding: 10px 20px;
            border: none;
            border-right: 1px solid #252526;
            min-width: 100px;
            font-size: 13px;
        }
        QTabBar::tab:selected {
            background-color: #1e1e1e;
            color: #ffffff;
            border-top: 2px solid #007acc;
        }
        QTabBar::tab:hover:!selected {
            background-color: #2a2d2e;
            color: #cccccc;
        }
        QTabBar::close-button {
            subcontrol-position: right;
            margin: 4px;
            padding: 4px;
            border-radius: 3px;
            background-color: transparent;
            width: 16px;
            height: 16px;
        }
        QTabBar::close-button:hover {
            background-color: #e81123;
        }
        QStatusBar {
            background-color: #141414;
            color: #9e9e9e;
            border: none;
            border-top: 1px solid #2a2a2a;
            padding: 2px 4px;
            font-size: 11px;
        }
        QStatusBar QLabel {
            background-color: transparent;
            color: #cccccc;
            font-size: 11px;
        }
        QDockWidget {
            color: #cccccc;
            border: none;
            font-size: 12px;
        }
        QDockWidget::title {
            background-color: #252526;
            padding: 8px 12px;
            border: none;
            text-align: left;
            font-size: 11px;
            font-weight: 600;
            letter-spacing: 1px;
        }
        QDockWidget::close-button, QDockWidget::float-button {
            background-color: transparent;
            border: none;
            padding: 2px;
        }
        QDockWidget::close-button:hover, QDockWidget::float-button:hover {
            background-color: #3e3e42;
        }
        QTreeView {
            background-color: #252526;
            color: #cccccc;
            border: none;
            outline: none;
            show-decoration-selected: 1;
            font-size: 13px;
        }
        QTreeView::item {
            padding: 5px 4px;
            border: none;
        }
        QTreeView::item:hover {
            background-color: #2a2d2e;
        }
        QTreeView::item:selected {
            background-color: #094771;
            color: #ffffff;
        }
        QHeaderView::section {
            background-color: #252526;
            color: #cccccc;
            padding: 6px;
            border: none;
            border-bottom: 1px solid #3e3e42;
            font-size: 12px;
        }
        QScrollBar:vertical {
            background-color: transparent;
            width: 14px;
            margin: 0;
            border: none;
        }
        QScrollBar::handle:vertical {
            background-color: rgba(121, 121, 121, 0.4);
            min-height: 30px;
            border-radius: 7px;
            margin: 2px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: rgba(121, 121, 121, 0.7);
        }
        QScrollBar::handle:vertical:pressed {
            background-color: rgba(121, 121, 121, 0.9);
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
            background: none;
        }
        QScrollBar:horizontal {
            background-color: transparent;
            height: 14px;
            margin: 0;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background-color: rgba(121, 121, 121, 0.4);
            min-width: 30px;
            border-radius: 7px;
            margin: 2px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: rgba(121, 121, 121, 0.7);
        }
        QScrollBar::handle:horizontal:pressed {
            background-color: rgba(121, 121, 121, 0.9);
        }
        QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {
            width: 0px;
        }
        QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
            background: none;
        }
        QPlainTextEdit {
            background-color: #1e1e1e;
            color: #d4d4d4;
            border: none;
            selection-background-color: #264f78;
            selection-color: #ffffff;
            font-family: 'Consolas', 'Fira Code', monospace;
        }
        QSplitter::handle {
            background-color: #3e3e42;
            width: 6px;
            height: 6px;
        }
        QSplitter::handle:hover {
            background-color: #007acc;
        }
        QSplitter::handle:vertical {
            height: 6px;
            margin: 0px;
        }
        QSplitter::handle:horizontal {
            width: 6px;
            margin: 0px;
        }
        QInputDialog {
            background-color: #2d2d30;
        }
        QMessageBox {
            background-color: #2d2d30;
        }
    )";
  setStyleSheet(style);
}

// ── v0.8.0 QoL Implementations ───────────────────────────────────────────────

void TextEditor::changeEvent(QEvent *e) {
    QMainWindow::changeEvent(e);
    if (e->type() == QEvent::ActivationChange && !isActiveWindow()) {
        // Auto-save on focus lost
        if (editorPrefs.autoSaveFocusEnabled) {
            const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                                         : QList<QTabWidget *>{tabWidget};
            for (QTabWidget *tw : panes) {
                for (int i = 0; i < tw->count(); ++i) {
                    CodeEditor *ed = qobject_cast<CodeEditor *>(tw->widget(i));
                    if (ed && ed->isModified() && !ed->getFileName().isEmpty())
                        saveFileToPath(ed->getFileName(), ed);
                }
            }
        }
    }
}

bool TextEditor::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::FocusIn) {
        for (QWidget *widget = qobject_cast<QWidget *>(obj); widget; widget = widget->parentWidget()) {
            if (widget == tabWidget || widget == tabWidget2) {
                activeTabWidget = qobject_cast<QTabWidget *>(widget);
                break;
            }
        }
    }
    // Double-Shift detection for Search Everywhere
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *ke = static_cast<QKeyEvent*>(event);
        if (ke->key() == Qt::Key_Shift && !ke->isAutoRepeat()) {
            qint64 now = QDateTime::currentMSecsSinceEpoch();
            if (now - searchState.lastShiftPressMs < 400 &&
                (!searchEverywhere || !searchEverywhere->isVisible()))
                openSearchEverywhere();
            searchState.lastShiftPressMs = now;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void TextEditor::switchHeaderSource() {
    CodeEditor *ed = currentEditor();
    if (!ed || ed->getFileName().isEmpty()) return;
    QFileInfo fi(ed->getFileName());
    QString ext = fi.suffix().toLower();
    QString companion;
    if (ext == "cpp" || ext == "cxx" || ext == "cc")
        companion = fi.absolutePath() + "/" + fi.completeBaseName() + ".h";
    else if (ext == "h" || ext == "hpp" || ext == "hxx")
        companion = fi.absolutePath() + "/" + fi.completeBaseName() + ".cpp";
    else {
        flashStatusMessage("No header/source counterpart for this file type", QColor("#ffb86c"), 2000);
        return;
    }
    if (QFileInfo::exists(companion)) {
        loadFile(companion);
    } else {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Create Counterpart",
            QString("'%1' doesn't exist. Create it?").arg(QFileInfo(companion).fileName()),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            QFile f(companion);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream s(&f);
                if (companion.endsWith(".h")) {
                    QString guard = QFileInfo(companion).completeBaseName().toUpper() + "_H";
                    s << "#ifndef " << guard << "\n#define " << guard << "\n\n\n#endif // " << guard << "\n";
                } else {
                    s << "#include \"" << fi.completeBaseName() << ".h\"\n\n";
                }
            }
            loadFile(companion);
        }
    }
}

void TextEditor::locateCurrentFileInTree() {
    CodeEditor *ed = currentEditor();
    if (!ed || ed->getFileName().isEmpty()) return;
    // Ensure file tree is visible
    if (!fileTreeDock->isVisible()) {
        fileTreeDock->setVisible(true);
        if (fileTreeAct) fileTreeAct->setChecked(true);
    }
    QModelIndex idx = fileSystemModel->index(ed->getFileName());
    if (idx.isValid()) {
        fileTree->scrollTo(idx, QAbstractItemView::PositionAtCenter);
        fileTree->setCurrentIndex(idx);
        flashStatusMessage("📁 Located: " + QFileInfo(ed->getFileName()).fileName(),
                           QColor("#569cd6"), 2000);
    }
}

void TextEditor::openSearchEverywhere() {
    if (searchEverywhere && searchEverywhere->isVisible())
        return;
    if (!searchEverywhere) {
        searchEverywhere = new SearchEverywhere(this);
        connect(searchEverywhere, &SearchEverywhere::fileRequested,
                this, &TextEditor::loadFile);
    }
    // Gather all actions
    QList<QAction*> allActs;
    allActs << menuBar()->actions();
    // Gather open files
    QStringList openFiles;
    const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                                 : QList<QTabWidget *>{tabWidget};
    for (QTabWidget *tw : panes) {
        for (int i = 0; i < tw->count(); ++i) {
            CodeEditor *ed = qobject_cast<CodeEditor *>(tw->widget(i));
            if (ed && !ed->getFileName().isEmpty())
                openFiles << ed->getFileName();
        }
    }
    searchEverywhere->populate(allActs, recentFiles, openFiles);
    searchEverywhere->exec();
}

void TextEditor::toggleStickyScroll() {
    editorPrefs.stickyScrollEnabled = stickyScrollAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setStickyScrollEnabled(editorPrefs.stickyScrollEnabled);
}

void TextEditor::toggleInvisibleChars() {
    editorPrefs.invisibleCharsEnabled = invisibleCharsAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setInvisibleCharsEnabled(editorPrefs.invisibleCharsEnabled);
}

void TextEditor::toggleGitBlame() {
    editorPrefs.gitBlameEnabled = gitBlameAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setGitBlameEnabled(editorPrefs.gitBlameEnabled);
}

void TextEditor::toggleAutoSaveOnFocusLost() {
    editorPrefs.autoSaveFocusEnabled = autoSaveFocusAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setAutoSaveOnFocusLost(editorPrefs.autoSaveFocusEnabled);
    if (editorPrefs.autoSaveFocusEnabled)
        flashStatusMessage("Auto-Save on Focus Lost: ON", QColor("#4ec9b0"), 2000);
    else
        flashStatusMessage("Auto-Save on Focus Lost: OFF", QColor("#569cd6"), 2000);
}

void TextEditor::sendSelectionToScratchpad() {
    CodeEditor *ed = currentEditor();
    if (!ed || !ed->textCursor().hasSelection()) {
        flashStatusMessage("No selection to send", QColor("#ffb86c"), 1500);
        return;
    }
    QString sel = ed->textCursor().selectedText()
                      .replace(QChar::ParagraphSeparator, '\n');
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                   + "/jim_scratchpad.txt";
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile f(path);
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream s(&f);
        s << "\n\n--- " << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
          << " [from " << (ed->getFileName().isEmpty() ? "untitled" : QFileInfo(ed->getFileName()).fileName())
          << "] ---\n";
        s << sel << "\n";
        f.close();
        flashStatusMessage("✓ Sent to Scratchpad", QColor("#4ec9b0"), 2000);
    }
}

void TextEditor::applyPaneDimming() {
    // Dim the non-active pane in split view
    if (!editorPrefs.splitViewEnabled || !tabWidget2) return;
    QTabWidget *activePane = currentTabWidget();
    bool pane1Active = activePane == tabWidget;
    // Apply semi-transparent overlay on the inactive pane container
    // We use GraphicsOpacityEffect on the entire inactive QTabWidget
    QTabWidget *active = pane1Active ? tabWidget : tabWidget2;
    QTabWidget *inactive = pane1Active ? tabWidget2 : tabWidget;
    Q_UNUSED(active);
    inactive->setWindowOpacity(0.65);
}

void TextEditor::propagateV080Settings(CodeEditor *ed) {
    if (!ed) return;
    ed->setStickyScrollEnabled(editorPrefs.stickyScrollEnabled);
    ed->setInvisibleCharsEnabled(editorPrefs.invisibleCharsEnabled);
    ed->setGitBlameEnabled(editorPrefs.gitBlameEnabled);
    ed->setAutoSaveOnFocusLost(editorPrefs.autoSaveFocusEnabled);
}

// ============================================================
// v0.9.0 — Web3Sec & Solidity Pack
// ============================================================

void TextEditor::propagateV090Settings(CodeEditor *ed) {
    if (!ed) return;
    ed->setGasMinimapEnabled(gasMiniMapEnabled);
}

void TextEditor::triggerGodView() {
    CodeEditor *ed = currentEditor();
    if (!ed) return;

    Language lang = ed->getLanguage();

    QTextBlock block = ed->document()->begin();
    int collapsed = 0;
    while (block.isValid()) {
        if (ed->isFoldable(block) && !ed->isFolded(block)) {
            QString text = block.text().trimmed();
            bool isFunctionBody = text.contains("function ") ||
                                  text.endsWith('{') ||
                                  text.contains(") {") ||
                                  text.contains(") public") ||
                                  text.contains(") private") ||
                                  text.contains(") external") ||
                                  text.contains(") internal");
            if (isFunctionBody || lang == Language::Solidity || lang == Language::CPP) {
                ed->toggleFoldAt(block.blockNumber());
                collapsed++;
            }
        }
        block = block.next();
    }

    if (collapsed == 0) {
        QTextBlock b = ed->document()->begin();
        while (b.isValid()) {
            if (ed->isFolded(b)) ed->toggleFoldAt(b.blockNumber());
            b = b.next();
        }
        flashStatusMessage("God View: expanded all blocks");
    } else {
        flashStatusMessage(QString("God View: collapsed %1 function bodies").arg(collapsed));
    }
}

void TextEditor::extractABI() {
    CodeEditor *ed = currentEditor();
    if (!ed) {
        flashStatusMessage("No active editor");
        return;
    }
    QString path = ed->getFileName();
    if (path.isEmpty() || !path.endsWith(".sol", Qt::CaseInsensitive)) {
        flashStatusMessage("Open a .sol file first");
        return;
    }

    QProcess *proc = new QProcess(this);
    proc->setWorkingDirectory(QFileInfo(path).absolutePath());

    QStringList solcCandidates = {"solc", "solc-0.8", "/usr/local/bin/solc", "/usr/bin/solc"};
    QString solcPath;
    for (const QString &c : solcCandidates) {
        QProcess test;
        test.start(c, {"--version"});
        test.waitForFinished(1000);
        if (test.exitCode() == 0) { solcPath = c; break; }
    }

    if (solcPath.isEmpty()) {
        flashStatusMessage("solc not found — install Solidity compiler");
        proc->deleteLater();
        return;
    }

    flashStatusMessage("⚡ Running solc...");

    connect(proc, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
    this, [this, proc, path](int exitCode, QProcess::ExitStatus) {
        QByteArray out = proc->readAllStandardOutput();
        QByteArray err = proc->readAllStandardError();
        proc->deleteLater();

        if (exitCode != 0) {
            QMessageBox::warning(this, "solc Error",
                QString("solc failed:\n%1").arg(QString::fromUtf8(err).left(1000)));
            return;
        }

        QApplication::clipboard()->setText(QString::fromUtf8(out));

        QPalette p = palette();
        QPalette orig = p;
        p.setColor(QPalette::Window, QColor(0, 80, 0));
        setPalette(p);
        QTimer::singleShot(300, this, [this, orig]() { setPalette(orig); });

        flashStatusMessage(QString("✓ ABI copied to clipboard (%1 bytes)").arg(out.size()));

        QDialog *dlg = new QDialog(this);
        dlg->setWindowTitle("ABI / Bytecode — " + QFileInfo(path).fileName());
        dlg->resize(700, 400);
        auto *layout = new QVBoxLayout(dlg);
        auto *te = new QPlainTextEdit(dlg);
        te->setPlainText(QString::fromUtf8(out));
        te->setStyleSheet("background:#0d0d0d;color:#00ff88;font-family:Consolas;font-size:11px;");
        te->setReadOnly(true);
        layout->addWidget(te);
        auto *closeBtn = new QPushButton("Close", dlg);
        connect(closeBtn, &QPushButton::clicked, dlg, &QDialog::accept);
        layout->addWidget(closeBtn);
        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    });

    proc->start(solcPath, {"--abi", "--bin", "--optimize", path});
}

void TextEditor::resolveFourByte() {
    CodeEditor *ed = currentEditor();
    QString selectedText;
    if (ed) {
        QTextCursor cur = ed->textCursor();
        selectedText = cur.selectedText().trimmed();
    }

    if (selectedText.isEmpty() || selectedText.length() > 10) {
        bool ok;
        selectedText = QInputDialog::getText(this, "4-Byte Signature Resolver",
            "Enter 4-byte hex selector (e.g. 0xa9059cbb or a9059cbb):",
            QLineEdit::Normal, selectedText.left(10), &ok);
        if (!ok || selectedText.isEmpty()) return;
    }

    QString hex = selectedText;
    if (hex.startsWith("0x", Qt::CaseInsensitive)) hex = hex.mid(2);
    hex = hex.toLower().left(8);

    if (hex.length() != 8) {
        flashStatusMessage("Invalid selector — need exactly 4 bytes (8 hex chars)");
        return;
    }

    flashStatusMessage(QString("\U0001F50D Looking up 0x%1 in 4byte.directory...").arg(hex));

    QUrl url(QString("https://www.4byte.directory/api/v1/signatures/?hex_signature=0x%1").arg(hex));
    QNetworkRequest req(url);
    req.setHeader(QNetworkRequest::UserAgentHeader, "Jim-Editor/0.9.0");

    QNetworkReply *reply = networkManager->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, hex, ed]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            flashStatusMessage("Network error: " + reply->errorString());
            return;
        }
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();
        QJsonArray results = obj["results"].toArray();

        if (results.isEmpty()) {
            flashStatusMessage(QString("0x%1 — not found in 4byte.directory").arg(hex));
            return;
        }

        QStringList sigs;
        for (const QJsonValue &v : results)
            sigs << v.toObject()["text_signature"].toString();

        QString best = sigs.first();
        flashStatusMessage(QString("0x%1 \u2192 %2").arg(hex, best));

        QDialog *dlg = new QDialog(this);
        dlg->setWindowTitle(QString("4-Byte: 0x%1").arg(hex));
        dlg->setFixedSize(480, 200);
        dlg->setStyleSheet("background:#0a0a1a;color:#00ffff;border:2px solid #00ffff;");
        auto *layout = new QVBoxLayout(dlg);
        auto *title = new QLabel(QString("<div style='color:#00ff88;font-size:16px;font-weight:bold;'>0x%1</div>").arg(hex), dlg);
        title->setTextFormat(Qt::RichText);
        layout->addWidget(title);
        for (const QString &sig : sigs) {
            auto *lbl = new QLabel(sig, dlg);
            lbl->setStyleSheet("color:#ffffff;font-family:Consolas;font-size:13px;padding:2px;");
            lbl->setTextInteractionFlags(Qt::TextSelectableByMouse);
            layout->addWidget(lbl);
        }
        auto *copyBtn = new QPushButton("Copy: " + best, dlg);
        copyBtn->setStyleSheet("background:#003333;color:#00ffcc;border:1px solid #00ffcc;padding:4px;");
        connect(copyBtn, &QPushButton::clicked, [best, dlg]() {
            QApplication::clipboard()->setText(best);
            dlg->accept();
        });
        layout->addWidget(copyBtn);

        if (ed) {
            QTextCursor cur = ed->textCursor();
            if (cur.hasSelection()) {
                auto *replaceBtn = new QPushButton("Replace selection with: " + best, dlg);
                replaceBtn->setStyleSheet("background:#003300;color:#00ff88;border:1px solid #00ff88;padding:4px;");
                connect(replaceBtn, &QPushButton::clicked, [ed, best, dlg]() {
                    ed->textCursor().insertText(best);
                    dlg->accept();
                });
                layout->addWidget(replaceBtn);
            }
        }

        dlg->setAttribute(Qt::WA_DeleteOnClose);
        dlg->show();
    });
}

void TextEditor::triggerPanicButton() {
    QMessageBox::StandardButton reply = QMessageBox::warning(this,
        "\u26A0 PANIC BUTTON — CONFIRM",
        "This will:\n"
        "\u2022 Immediately close Jim\n"
        "\u2022 Overwrite scratchpad with zeroes (unrecoverable)\n"
        "\u2022 Clear all recent files history\n"
        "\u2022 Clear editor settings\n\n"
        "Are you absolutely sure?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    if (reply != QMessageBox::Yes) return;

    QString scratchPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/jim_scratchpad.txt";
    QFile scratchFile(scratchPath);
    if (scratchFile.exists()) {
        qint64 sz = scratchFile.size();
        if (sz > 0 && scratchFile.open(QIODevice::WriteOnly)) {
            QByteArray zeroes(sz, '\0');
            for (int pass = 0; pass < 3; ++pass) {
                scratchFile.seek(0);
                scratchFile.write(zeroes);
                scratchFile.flush();
                if (pass == 1) {
                    zeroes.fill('\xFF');
                } else if (pass == 2) {
                    for (char &c : zeroes) c = (char)(QRandomGenerator::global()->bounded(256));
                }
            }
            scratchFile.close();
        }
        scratchFile.remove();
    }

    recentFiles.clear();
    QSettings settings("TextEditor", "Settings");
    settings.remove("recentFiles");
    settings.remove("geometry");
    settings.remove("windowState");
    settings.sync();

    QApplication::quit();
}

void TextEditor::showStorageSlotVisualizer() {
    CodeEditor *ed = currentEditor();
    if (!ed) { flashStatusMessage("No active editor"); return; }

    QString code = ed->toPlainText();
    Language lang = ed->getLanguage();

    bool isSolidity = lang == Language::Solidity ||
                      ed->getFileName().endsWith(".sol", Qt::CaseInsensitive) ||
                      code.contains("pragma solidity") || code.contains("contract ");

    if (!isSolidity) {
        flashStatusMessage("Storage Slot Visualizer: open a .sol file first");
        return;
    }

    if (!slotVisualizerDock) {
        slotVisualizerWidget = new StorageSlotVisualizerWidget(this);
        slotVisualizerDock = new QDockWidget("\U0001F5C4 Storage Slots", this);
        slotVisualizerDock->setWidget(slotVisualizerWidget);
        slotVisualizerDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
        addDockWidget(Qt::RightDockWidgetArea, slotVisualizerDock);
        slotVisualizerDock->setStyleSheet(
            "QDockWidget { background:#1a0030; color:#cc88ff; }"
            "QDockWidget::title { background:#2a0050; padding:4px; }");

        connect(slotVisualizerWidget, &StorageSlotVisualizerWidget::jumpToVariable,
        this, [this](const QString &varName) {
            CodeEditor *activeEd = currentEditor();
            if (!activeEd) return;
            QTextDocument *doc = activeEd->document();
            QTextCursor found = doc->find(varName);
            if (!found.isNull()) {
                activeEd->setTextCursor(found);
                activeEd->centerCursor();
            }
        });
    }

    slotVisualizerWidget->analyzeCode(code);
    slotVisualizerDock->show();
    slotVisualizerDock->raise();

    auto vars = SolidityAnalyzer::parseStorageSlots(code);
    flashStatusMessage(QString("Storage Slots: %1 variables in %2 slots analyzed")
        .arg(vars.size())
        .arg(vars.isEmpty() ? 0 : vars.last().slot + 1));
}

void TextEditor::toggleGasMinimap() {
    gasMiniMapEnabled = !gasMiniMapEnabled;
    if (gasMiniMapAct) gasMiniMapAct->setChecked(gasMiniMapEnabled);

    for (CodeEditor *ed : allEditors())
        ed->setGasMinimapEnabled(gasMiniMapEnabled);
    flashStatusMessage(gasMiniMapEnabled ? "\U0001F321 Gas Minimap ON" : "Gas Minimap OFF");
}

void TextEditor::toggleSlitherOverlay() {
    CodeEditor *ed = currentEditor();
    if (!ed) { flashStatusMessage("No active editor"); return; }

    QString path = ed->getFileName();
    if (path.isEmpty()) { flashStatusMessage("Save file first"); return; }

    bool hasSlither = false, hasForge = false;
    {
        QProcess test;
        test.start("slither", {"--version"});
        test.waitForFinished(1500);
        hasSlither = (test.exitCode() == 0);
    }
    {
        QProcess test;
        test.start("forge", {"--version"});
        test.waitForFinished(1500);
        hasForge = (test.exitCode() == 0);
    }

    if (!hasSlither && !hasForge) {
        QMessageBox::information(this, "Slither/Foundry Overlays",
            "Neither slither nor forge was found in PATH.\n\n"
            "Install with:\n  pip install slither-analyzer\n  curl -L https://foundry.paradigm.xyz | bash\n\nfoundryup");
        return;
    }

    QDialog *toolDialog = new QDialog(this);
    toolDialog->setWindowTitle("Run Analysis Tool");
    toolDialog->setStyleSheet("background:#0d0d0d;color:#d4d4d4;");
    auto *layout = new QVBoxLayout(toolDialog);
    layout->addWidget(new QLabel(QString("File: %1").arg(QFileInfo(path).fileName()), toolDialog));

    QComboBox *toolCombo = new QComboBox(toolDialog);
    if (hasSlither) toolCombo->addItem("slither (static analysis)");
    if (hasForge) toolCombo->addItem("forge test (unit tests)");
    layout->addWidget(toolCombo);

    auto *runBtn = new QPushButton("\u25B6 Run", toolDialog);
    auto *cancelBtn = new QPushButton("Cancel", toolDialog);
    auto *btns = new QHBoxLayout();
    btns->addWidget(runBtn);
    btns->addWidget(cancelBtn);
    layout->addLayout(btns);

    connect(cancelBtn, &QPushButton::clicked, toolDialog, &QDialog::reject);
    connect(runBtn, &QPushButton::clicked, toolDialog, [=]() {
        toolDialog->accept();
        QString tool = toolCombo->currentText().contains("slither") ? "slither" : "forge";

        QProcess *proc = new QProcess(this);
        proc->setWorkingDirectory(QFileInfo(path).absolutePath());

        QStringList args;
        QString outFile = QDir::temp().filePath("jim_analysis.json");
        if (tool == "slither") {
            args << path << "--json" << outFile;
        } else {
            args << "test" << "--json-summary";
        }

        flashStatusMessage(QString("\u2699 Running %1...").arg(tool));

        connect(proc, QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
        this, [this, proc, outFile, tool, ed](int, QProcess::ExitStatus) {
            proc->deleteLater();

            QVector<VulnScanner::Finding> findings;

            QFile jf(outFile);
            if (jf.open(QIODevice::ReadOnly)) {
                QByteArray jsonData = jf.readAll();
                jf.close();
                QJsonDocument jdoc = QJsonDocument::fromJson(jsonData);
                if (!jdoc.isNull()) {
                    QJsonObject root = jdoc.object();
                    QJsonArray detectors = root["results"].toObject()["detectors"].toArray();
                    for (const QJsonValue &det : detectors) {
                        QJsonObject d = det.toObject();
                        QString check = d["check"].toString();
                        QString impact = d["impact"].toString();
                        QJsonArray elements = d["elements"].toArray();
                        for (const QJsonValue &el : elements) {
                            QJsonObject src = el.toObject()["source_mapping"].toObject();
                            int line = src["lines"].toArray().first().toInt() - 1;
                            VulnScanner::Finding f;
                            f.line = line;
                            f.colStart = 0;
                            f.colEnd = 5;
                            f.category = "SLITHER";
                            f.description = check + " [" + impact + "] " + d["description"].toString().left(100);
                            findings.append(f);
                        }
                    }
                }
            }

            QByteArray stdErr = proc->readAllStandardError();
            if (findings.isEmpty() && !stdErr.isEmpty()) {
                static QRegularExpression lineRe(R"(\((\d+)\))");
                QStringList errLines = QString::fromUtf8(stdErr).split('\n');
                for (const QString &el : errLines) {
                    QRegularExpressionMatch m = lineRe.match(el);
                    if (m.hasMatch()) {
                        VulnScanner::Finding f;
                        f.line = m.captured(1).toInt() - 1;
                        f.colStart = 0; f.colEnd = 5;
                        f.category = "SLITHER";
                        f.description = el.left(120);
                        findings.append(f);
                    }
                }
            }

            if (findings.isEmpty()) {
                flashStatusMessage(QString("\u2713 %1: No issues found").arg(tool));
            } else {
                ed->vulnFindings.append(findings);
                ed->viewport()->update();
                ed->update();
                flashStatusMessage(QString("\u2622 %1: %2 findings overlaid on editor").arg(tool).arg(findings.size()));
            }
        });

        proc->start(tool, args);
    });

    toolDialog->exec();
    toolDialog->deleteLater();
}

void TextEditor::showOnChainTracer() {
    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("\u26D3 On-Chain Trace Explorer");
    dlg->resize(800, 600);
    dlg->setStyleSheet("background:#080818;color:#00ffaa;");

    auto *layout = new QVBoxLayout(dlg);

    auto *header = new QLabel(
        "<div style='color:#00ffaa;font-size:16px;font-weight:bold;'>\u26D3 Time-Travel On-Chain Trace Explorer</div>"
        "<div style='color:#888;font-size:11px;'>Paste a mainnet tx hash to replay execution against local .sol files</div>",
        dlg);
    header->setTextFormat(Qt::RichText);
    layout->addWidget(header);

    auto *rpcRow = new QHBoxLayout();
    rpcRow->addWidget(new QLabel("RPC Endpoint:", dlg));
    auto *rpcInput = new QLineEdit("https://eth-mainnet.g.alchemy.com/v2/YOUR_KEY", dlg);
    rpcInput->setStyleSheet("background:#0a0a20;color:#aaffaa;border:1px solid #004400;padding:4px;font-family:Consolas;");
    rpcRow->addWidget(rpcInput, 1);
    layout->addLayout(rpcRow);

    auto *txRow = new QHBoxLayout();
    txRow->addWidget(new QLabel("Tx Hash:", dlg));
    auto *txInput = new QLineEdit(dlg);
    txInput->setPlaceholderText("0x...");
    txInput->setStyleSheet("background:#0a0a20;color:#aaffaa;border:1px solid #004400;padding:4px;font-family:Consolas;");
    auto *fetchBtn = new QPushButton("\u25B6 Fetch Trace", dlg);
    fetchBtn->setStyleSheet("background:#003300;color:#00ff88;border:1px solid #00ff88;padding:6px 12px;");
    txRow->addWidget(txInput, 1);
    txRow->addWidget(fetchBtn);
    layout->addLayout(txRow);

    auto *traceOutput = new QPlainTextEdit(dlg);
    traceOutput->setReadOnly(true);
    traceOutput->setPlaceholderText("Trace will appear here...");
    traceOutput->setStyleSheet("background:#050510;color:#00cc88;font-family:Consolas,monospace;font-size:11px;border:1px solid #003333;");
    layout->addWidget(traceOutput, 1);

    auto *statusLbl = new QLabel("Ready", dlg);
    statusLbl->setStyleSheet("color:#666;font-size:10px;");
    layout->addWidget(statusLbl);

    auto *closeBtnRow = new QHBoxLayout();
    auto *closeBtn2 = new QPushButton("Close", dlg);
    closeBtn2->setStyleSheet("background:#1a0000;color:#ff6666;border:1px solid #ff3333;padding:4px 12px;");
    connect(closeBtn2, &QPushButton::clicked, dlg, &QDialog::accept);
    closeBtnRow->addStretch();
    closeBtnRow->addWidget(closeBtn2);
    layout->addLayout(closeBtnRow);

    connect(fetchBtn, &QPushButton::clicked, dlg, [=]() {
        QString txHash = txInput->text().trimmed();
        QString rpcUrl = rpcInput->text().trimmed();

        if (txHash.isEmpty() || rpcUrl.isEmpty()) {
            statusLbl->setText("Enter RPC URL and tx hash");
            return;
        }

        statusLbl->setText("Fetching trace...");
        fetchBtn->setEnabled(false);
        traceOutput->clear();

        QJsonObject rpcCall;
        rpcCall["jsonrpc"] = "2.0";
        rpcCall["method"] = "debug_traceTransaction";
        rpcCall["id"] = 1;
        QJsonArray params;
        params.append(txHash);
        QJsonObject opts;
        opts["disableStorage"] = false;
        opts["disableMemory"] = true;
        opts["disableStack"] = false;
        params.append(opts);
        rpcCall["params"] = params;

        QNetworkRequest req{QUrl(rpcUrl)};
        req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        req.setHeader(QNetworkRequest::UserAgentHeader, "Jim-Editor/0.9.0");

        QNetworkReply *reply = networkManager->post(req, QJsonDocument(rpcCall).toJson());

        connect(reply, &QNetworkReply::finished, dlg, [=]() {
            reply->deleteLater();
            fetchBtn->setEnabled(true);

            if (reply->error() != QNetworkReply::NoError) {
                statusLbl->setText("Error: " + reply->errorString());
                return;
            }

            QByteArray data = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(data);
            QJsonObject obj = doc.object();

            if (obj.contains("error")) {
                statusLbl->setText("RPC Error: " + obj["error"].toObject()["message"].toString());
                traceOutput->setPlainText(QString::fromUtf8(data).left(2000));
                return;
            }

            QJsonObject result = obj["result"].toObject();
            QJsonArray structLogs = result["structLogs"].toArray();

            if (structLogs.isEmpty()) {
                statusLbl->setText("No trace data (check tx hash and RPC endpoint)");
                traceOutput->setPlainText(QString::fromUtf8(data).left(1000));
                return;
            }

            QString traceText;
            traceText += QString("Transaction Trace: %1\n").arg(txHash);
            traceText += QString("Gas Used: %1\n").arg(result["gas"].toInt());
            traceText += QString("Steps: %1\n\n").arg(structLogs.size());
            traceText += QString("%-6s  %-16s  %-8s  %s\n").arg("PC", "OPCODE", "GAS", "STACK TOP");
            traceText += QString("-").repeated(70) + "\n";

            int maxSteps = qMin(structLogs.size(), 500);
            for (int i = 0; i < maxSteps; ++i) {
                QJsonObject step = structLogs[i].toObject();
                int pc = step["pc"].toInt();
                QString op = step["op"].toString();
                int gas = step["gas"].toInt();
                QJsonArray stack = step["stack"].toArray();
                QString stackTop = stack.isEmpty() ? "" : stack.last().toString().right(16);
                traceText += QString("%-6d  %-16s  %-8d  %s\n").arg(pc).arg(op).arg(gas).arg(stackTop);
            }
            if (structLogs.size() > maxSteps)
                traceText += QString("... (%1 more steps)\n").arg(structLogs.size() - maxSteps);

            traceOutput->setPlainText(traceText);
            statusLbl->setText(QString("\u2713 Loaded %1 execution steps").arg(structLogs.size()));
        });
    });

    dlg->exec();
    dlg->deleteLater();
}

void TextEditor::showProxyDiff() {
    QStringList files = QFileDialog::getOpenFileNames(this,
        "Select Two .sol Files for Proxy/Implementation Diff",
        currentFolder, "Solidity (*.sol);;All Files (*)");

    if (files.size() < 2) {
        CodeEditor *ed = currentEditor();
        if (ed && ed->getFileName().endsWith(".sol")) {
            files.clear();
            files << ed->getFileName();
            QString second = QFileDialog::getOpenFileName(this,
                "Select Implementation .sol to compare against", currentFolder, "Solidity (*.sol)");
            if (second.isEmpty()) return;
            files << second;
        } else {
            flashStatusMessage("Select two .sol files to compare");
            return;
        }
    }

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("\U0001F504 Proxy/Implementation Storage Diff");
    dlg->resize(1000, 600);
    dlg->setStyleSheet("background:#0a0a0a;color:#d4d4d4;");

    auto *mainLayout = new QVBoxLayout(dlg);

    auto *header = new QLabel(
        "<b style='color:#00aaff;'>Proxy/Implementation Storage Slot Comparison</b><br>"
        "<span style='color:#888;font-size:10px;'>Mismatches indicate storage collision risk</span>",
        dlg);
    header->setTextFormat(Qt::RichText);
    mainLayout->addWidget(header);

    QStringList contents;
    QStringList names;
    for (const QString &f : files.mid(0, 2)) {
        QFile file(f);
        if (file.open(QIODevice::ReadOnly))
            contents << QString::fromUtf8(file.readAll());
        else
            contents << "";
        names << QFileInfo(f).fileName();
    }

    auto slots1 = SolidityAnalyzer::parseStorageSlots(contents[0]);
    auto slots2 = SolidityAnalyzer::parseStorageSlots(contents[1]);

    auto *table = new QTableWidget(dlg);
    int maxSlots = qMax(slots1.size(), slots2.size());
    table->setRowCount(maxSlots);
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({names[0] + " Name", names[0] + " Type/Slot",
                                       "\u26A1",
                                       names[1] + " Name", names[1] + " Type/Slot"});
    table->setStyleSheet(
        "QTableWidget { background:#0d0d0d; color:#d4d4d4; gridline-color:#333; }"
        "QHeaderView::section { background:#1a1a1a; color:#aaaaaa; border:1px solid #333; }"
        "QTableWidget::item { padding:4px; }");
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();

    for (int i = 0; i < maxSlots; ++i) {
        bool hasCollision = false;
        if (i < slots1.size() && i < slots2.size()) {
            hasCollision = (slots1[i].slot != slots2[i].slot ||
                           slots1[i].byteSize != slots2[i].byteSize);
        } else {
            hasCollision = true;
        }

        QString v1Name = i < slots1.size() ? slots1[i].name : "";
        QString v1Type = i < slots1.size() ?
            QString("%1 (slot %2)").arg(slots1[i].typeName).arg(slots1[i].slot) : "";
        QString v2Name = i < slots2.size() ? slots2[i].name : "";
        QString v2Type = i < slots2.size() ?
            QString("%1 (slot %2)").arg(slots2[i].typeName).arg(slots2[i].slot) : "";

        table->setItem(i, 0, new QTableWidgetItem(v1Name));
        table->setItem(i, 1, new QTableWidgetItem(v1Type));
        auto *flagItem = new QTableWidgetItem(hasCollision ? "\u26A0 COLLISION" : "\u2713 OK");
        flagItem->setForeground(hasCollision ? QColor(255, 80, 80) : QColor(80, 255, 80));
        table->setItem(i, 2, flagItem);
        table->setItem(i, 3, new QTableWidgetItem(v2Name));
        table->setItem(i, 4, new QTableWidgetItem(v2Type));

        if (hasCollision) {
            for (int c = 0; c < 5; ++c) {
                if (table->item(i, c))
                    table->item(i, c)->setBackground(QColor(60, 10, 10));
            }
        }
    }

    mainLayout->addWidget(table, 1);

    int collisions = 0;
    for (int i = 0; i < maxSlots; ++i) {
        bool coll = false;
        if (i < slots1.size() && i < slots2.size())
            coll = (slots1[i].slot != slots2[i].slot || slots1[i].byteSize != slots2[i].byteSize);
        else coll = true;
        if (coll) ++collisions;
    }

    auto *summary = new QLabel(collisions > 0 ?
        QString("<b style='color:#ff4444;'>\u26A0 WARNING: %1 storage collision(s) detected!</b>").arg(collisions) :
        "<b style='color:#44ff44;'>\u2713 No storage collisions detected</b>", dlg);
    summary->setTextFormat(Qt::RichText);
    mainLayout->addWidget(summary);

    auto *closeBtn3 = new QPushButton("Close", dlg);
    closeBtn3->setStyleSheet("background:#1a0000;color:#ff6666;border:1px solid #ff3333;padding:4px 12px;");
    connect(closeBtn3, &QPushButton::clicked, dlg, &QDialog::accept);
    mainLayout->addWidget(closeBtn3);

    dlg->exec();
    dlg->deleteLater();
}

// ============================================================
// Narrative Engine
// ============================================================

void TextEditor::toggleStoryGraph() {
    if (!storyGraphDock) return;
    bool show = !storyGraphDock->isVisible();
    storyGraphDock->setVisible(show);
    if (storyGraphAct) storyGraphAct->setChecked(show);
    if (show) refreshStoryPanels();
}

void TextEditor::toggleStoryPlaytest() {
    if (!storyPlaytestDock) return;
    bool show = !storyPlaytestDock->isVisible();
    storyPlaytestDock->setVisible(show);
    if (storyPlaytestAct) storyPlaytestAct->setChecked(show);
    if (show) refreshStoryPanels();
}

void TextEditor::refreshStoryPanels() {
    CodeEditor *ed = currentEditor();
    if (!ed) return;
    if (ed->getLanguage() != Language::Story) return;

    QString text = ed->toPlainText();
    if (storyGraph && storyGraphDock->isVisible())
        storyGraph->refresh(text);
    if (storyPlaytest && storyPlaytestDock->isVisible())
        storyPlaytest->loadStory(text);
}

void TextEditor::onStoryPassageClicked(const QString &passageName) {
    // Jump editor cursor to the passage header
    CodeEditor *ed = currentEditor();
    if (!ed) return;

    QVector<StoryPassage> passages = StoryParser::parse(ed->toPlainText());
    for (const auto &p : passages) {
        if (p.name == passageName) {
            QTextBlock block = ed->document()->findBlockByNumber(p.lineNumber);
            if (block.isValid()) {
                QTextCursor cursor(block);
                ed->setTextCursor(cursor);
                ed->centerCursor();
                ed->setFocus();
            }
            break;
        }
    }
}

void TextEditor::exportStory() {
    CodeEditor *ed = currentEditor();
    if (!ed || ed->getLanguage() != Language::Story) {
        flashStatusMessage("Open a .story file to export");
        return;
    }

    QStringList formats = {"HTML (self-contained)", "JSON (game engine)", "Ink (.ink)", "Markdown"};
    bool ok;
    QString choice = QInputDialog::getItem(this, "Export Story", "Export format:", formats, 0, false, &ok);
    if (!ok) return;

    QVector<StoryPassage> passages = StoryParser::parse(ed->toPlainText());
    QString title = QFileInfo(ed->getFileName()).baseName();
    if (title.isEmpty()) title = "Story";

    QString content, ext, filter;
    if (choice.startsWith("HTML")) {
        content = StoryExporter::toHTML(passages, title);
        ext = ".html"; filter = "HTML Files (*.html)";
    } else if (choice.startsWith("JSON")) {
        content = StoryExporter::toJSON(passages);
        ext = ".json"; filter = "JSON Files (*.json)";
    } else if (choice.startsWith("Ink")) {
        content = StoryExporter::toInk(passages);
        ext = ".ink"; filter = "Ink Files (*.ink)";
    } else {
        content = StoryExporter::toMarkdown(passages);
        ext = ".md"; filter = "Markdown Files (*.md)";
    }

    QString defaultPath = QFileInfo(ed->getFileName()).dir().absoluteFilePath(title + ext);
    QString savePath = QFileDialog::getSaveFileName(this, "Export Story", defaultPath, filter);
    if (savePath.isEmpty()) return;

    QFile f(savePath);
    if (f.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream(&f) << content;
        flashStatusMessage("Exported to " + QFileInfo(savePath).fileName(), QColor("#50fa7b"));
    } else {
        flashStatusMessage("Export failed: " + f.errorString(), QColor("#ff5555"));
    }
}

void TextEditor::openCodebaseCityscape() {
    if (currentFolder.isEmpty()) {
        QMessageBox::warning(this, "Codebase Cityscape", "Please open a folder first to visualize the codebase.");
        return;
    }
    CityscapeWidget *city = new CityscapeWidget();
    city->loadDirectory(currentFolder);
    city->setWindowTitle("Jim — Codebase Cityscape");
    city->resize(1024, 768);
    city->setAttribute(Qt::WA_DeleteOnClose);
    city->show();
}
