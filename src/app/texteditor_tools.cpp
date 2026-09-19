#include "texteditor_private.h"

void TextEditor::flashTabLabel(int tabIndex, QTabWidget *targetWidget) {
    QTabWidget *tw = targetWidget ? targetWidget : currentTabWidget();
    if (tabIndex < 0 || tabIndex >= tw->count()) return;
    // Briefly change the tab text colour to accent blue then fade back via timer
    QTabBar *bar = tw->tabBar();
    bar->setTabTextColor(tabIndex, QColor("#569cd6"));
    QTimer::singleShot(600, this, [bar, tabIndex]() {
        bar->setTabTextColor(tabIndex, QColor()); // reset to stylesheet default
    });
}

void TextEditor::flashStatusMessage(const QString &msg, const QColor &color, int ms) {
    statusBar()->showMessage(msg, ms);
    QString prev = statusLabel->styleSheet();
    statusLabel->setStyleSheet(
        QString("color: %1; font-weight: bold; background: #252526; "
                "padding: 1px 10px; border-radius: 8px; font-size: 11px; margin: 2px 2px;")
        .arg(color.name()));
    QTimer::singleShot(ms, this, [this, prev]() {
        statusLabel->setStyleSheet(prev);
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  Tools – open in disassembler / binary inspector
// ─────────────────────────────────────────────────────────────────────────────

void TextEditor::openInDisassembler(const QString &filePath) {
    if (filePath.isEmpty()) return;

    // Check if a disassembler tab for this file is already open
    const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                                 : QList<QTabWidget *>{tabWidget};
    for (QTabWidget *tw : panes) {
        for (int i = 0; i < tw->count(); ++i) {
            DisassemblerWidget *w = qobject_cast<DisassemblerWidget *>(tw->widget(i));
            if (w && w->getFilePath() == filePath) {
                activeTabWidget = tw;
                tw->setCurrentIndex(i);
                return;
            }
        }
    }

    hideWelcomeScreen();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto *dw = new DisassemblerWidget();
    dw->loadFile(filePath);

    QString label = "[ASM] " + strippedName(filePath);
    QTabWidget *tw = currentTabWidget();
    int idx = tw->addTab(dw, label);
    tw->setCurrentIndex(idx);
    activeTabWidget = tw;
    flashTabLabel(idx, tw);

    QApplication::restoreOverrideCursor();
    flashStatusMessage("Disassembling " + strippedName(filePath) + "…",
                       QColor("#4ec9b0"), 3000);
}

void TextEditor::openInBinaryInspector(const QString &filePath) {
    if (filePath.isEmpty()) return;

    // Check if an inspector tab for this file is already open
    const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                                 : QList<QTabWidget *>{tabWidget};
    for (QTabWidget *tw : panes) {
        for (int i = 0; i < tw->count(); ++i) {
            BinaryInspectorWidget *w =
                qobject_cast<BinaryInspectorWidget *>(tw->widget(i));
            if (w && w->getFilePath() == filePath) {
                activeTabWidget = tw;
                tw->setCurrentIndex(i);
                return;
            }
        }
    }

    hideWelcomeScreen();
    QApplication::setOverrideCursor(Qt::WaitCursor);

    auto *bw = new BinaryInspectorWidget();
    bw->loadFile(filePath);

    QString label = "[BIN] " + strippedName(filePath);
    QTabWidget *tw = currentTabWidget();
    int idx = tw->addTab(bw, label);
    tw->setCurrentIndex(idx);
    activeTabWidget = tw;
    flashTabLabel(idx, tw);

    QApplication::restoreOverrideCursor();
    flashStatusMessage("Inspected " + strippedName(filePath),
                       QColor("#4ec9b0"), 2500);
}

// ── Public slots called by menu actions ──────────────────────────────────────

void TextEditor::openDisassembler() {
    // Try to use the current tab's file; otherwise show a file picker
    QString path;
    CodeEditor *ed = currentEditor();
    if (ed && !ed->getFileName().isEmpty()) {
        path = ed->getFileName();
    } else {
        HexEditor *hex = qobject_cast<HexEditor *>(currentTabWidget()->currentWidget());
        if (hex)
            path = hex->property("fileName").toString();
    }

    if (path.isEmpty()) {
        path = QFileDialog::getOpenFileName(
            this, "Select Binary to Disassemble", "",
            "Executables & Libraries (*.exe *.dll *.so *.o *.out *.elf);;"
            "All Files (*)");
    }
    if (!path.isEmpty())
        openInDisassembler(path);
}

void TextEditor::openNeuralGraph() {
    QString folder = currentFolder.isEmpty() ? QDir::currentPath() : currentFolder;
    QString activeFile;
    if (CodeEditor *ed = currentEditor())
        activeFile = ed->getFileName();
    // If no folder is set but a file is open, use its directory
    if (currentFolder.isEmpty() && !activeFile.isEmpty())
        folder = QFileInfo(activeFile).absolutePath();
    CodeGraph *graph = new CodeGraph(folder, activeFile, this);
    graph->exec();
}

void TextEditor::openGhostReplay() {
    CodeEditor *ed = currentEditor();
    if (!ed) return;
    
    QDialog *replayDialog = new QDialog(this);
    replayDialog->setWindowTitle("Ghost Replay Mode: " + ed->getFileName());
    replayDialog->resize(800, 600);
    QVBoxLayout *layout = new QVBoxLayout(replayDialog);
    QPlainTextEdit *replayEditor = new QPlainTextEdit(replayDialog);
    replayEditor->setReadOnly(true);
    replayEditor->setStyleSheet(ed->styleSheet());
    layout->addWidget(replayEditor);
    
    QTimer *playbackTimer = new QTimer(replayDialog);
    int eventIndex = 0;
    connect(playbackTimer, &QTimer::timeout, replayDialog, [=]() mutable {
        if (eventIndex >= ed->ghostLog.size()) {
            playbackTimer->stop();
            return;
        }
        GhostEvent ev = ed->ghostLog[eventIndex];
        QTextCursor c(replayEditor->document());
        c.setPosition(ev.position);
        if (ev.charsRemoved > 0) {
            c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, ev.charsRemoved);
            c.removeSelectedText();
        }
        if (!ev.textAdded.isEmpty()) {
            c.insertText(ev.textAdded);
        }
        eventIndex++;
    });
    playbackTimer->start(150); 
    
    replayDialog->exec();
}

void TextEditor::openScratchpad() {
    // If already open, just switch to it
    const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                                 : QList<QTabWidget *>{tabWidget};
    for (QTabWidget *tw : panes) {
        for (int i = 0; i < tw->count(); ++i) {
            if (tw->tabText(i) == "📝 Scratchpad") {
                activeTabWidget = tw;
                tw->setCurrentIndex(i);
                return;
            }
        }
    }

    hideWelcomeScreen();

    if (!scratchpadEditor) {
        scratchpadEditor = new QPlainTextEdit();
        scratchpadEditor->setFont(QFont("Consolas", editorPrefs.fontSize));
        scratchpadEditor->setStyleSheet(
            "QPlainTextEdit { background:#1a1a2e; color:#e0e0e0; "
            "border:none; font-family:Consolas,monospace; }");
        scratchpadEditor->setPlaceholderText(
            "Scratchpad — jot anything here. Saved automatically.");

        // Load persisted content
        QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                       + "/jim_scratchpad.txt";
        QFile f(path);
        if (f.open(QIODevice::ReadOnly | QIODevice::Text))
            scratchpadEditor->setPlainText(f.readAll());

        // Auto-save on every change
        connect(scratchpadEditor, &QPlainTextEdit::textChanged, this, [this]() {
            if (editorPrefs.paranoiaMode) return;
            QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                           + "/jim_scratchpad.txt";
            QDir().mkpath(QFileInfo(path).absolutePath());
            QSaveFile f(path);
            if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
                f.write(scratchpadEditor->toPlainText().toUtf8());
                f.commit();
            }
        });
    }

    QTabWidget *tw = currentTabWidget();
    int idx = tw->addTab(scratchpadEditor, "📝 Scratchpad");
    tw->setCurrentIndex(idx);
    activeTabWidget = tw;
    scratchpadEditor->setFocus();
    QTimer::singleShot(0, this, &TextEditor::clampToScreen);
}

// ── v1.9 Feature Implementations ──────────────────────────────────────────────

void TextEditor::toggleFocusFade() {
    bool enabled = focusFadeAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setFocusFadeEnabled(enabled);
    flashStatusMessage(enabled ? "Focus Fade: ON" : "Focus Fade: OFF",
                       QColor("#61afef"), 2000);
}

void TextEditor::toggleImagePreview() {
    bool enabled = imagePreviewAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setImagePreviewEnabled(enabled);
    flashStatusMessage(enabled ? "Image Preview: ON (hover over image paths)" : "Image Preview: OFF",
                       QColor("#61afef"), 2000);
}

void TextEditor::trackKeystroke(int key, const QString &text) {
    if (editorPrefs.paranoiaMode) return;
    Q_UNUSED(text)
    sessionMetrics.keystrokes++;
    if (key == Qt::Key_Return || key == Qt::Key_Enter)
        sessionMetrics.linesWritten++;

    qint64 now = QDateTime::currentMSecsSinceEpoch();
    sessionMetrics.keystrokeTimestamps.append(now);
    // Prune timestamps older than 60 seconds
    qint64 cutoff = now - 60000;
    while (!sessionMetrics.keystrokeTimestamps.isEmpty() &&
           sessionMetrics.keystrokeTimestamps.first() < cutoff)
        sessionMetrics.keystrokeTimestamps.removeFirst();

    // WPM = (keystrokes in window / 5) / (window_minutes)
    int wpm = 0;
    if (sessionMetrics.keystrokeTimestamps.size() >= 2) {
        double windowSec =
            (now - sessionMetrics.keystrokeTimestamps.first()) / 1000.0;
        if (windowSec > 0)
            wpm = static_cast<int>((sessionMetrics.keystrokeTimestamps.size() / 5.0) /
                                   (windowSec / 60.0));
    }
    sessionMetrics.peakWpm = qMax(sessionMetrics.peakWpm, wpm);
}

void TextEditor::showSessionStats() {
    // Compute active time from session timer
    int totalSecs = sessionSecondsAccumulated;
    if (sessionTimer && sessionTimer->isActive()) {
        QDateTime now = QDateTime::currentDateTime();
        totalSecs += static_cast<int>(sessionStart.secsTo(now));
    }
    int hours   = totalSecs / 3600;
    int minutes = (totalSecs % 3600) / 60;
    int secs    = totalSecs % 60;
    QString timeStr = QString("%1h %2m %3s").arg(hours).arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));

    QDialog dlg(this);
    dlg.setWindowTitle("Session Statistics");
    dlg.setFixedSize(340, 280);
    dlg.setStyleSheet(
        "QDialog { background:#1e1e2e; color:#cdd6f4; }"
        "QLabel  { color:#cdd6f4; font-family:Consolas,monospace; }"
        "QPushButton { background:#313244; color:#cdd6f4; border:1px solid #45475a; "
        "              border-radius:6px; padding:6px 20px; font-family:Consolas; }"
        "QPushButton:hover { background:#45475a; }");

    auto *layout = new QVBoxLayout(&dlg);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(10);

    auto *title = new QLabel("  Session Stats", &dlg);
    title->setStyleSheet("font-size:15px; font-weight:bold; color:#89b4fa;");
    layout->addWidget(title);

    auto addRow = [&](const QString &icon, const QString &label, const QString &value) {
        auto *row = new QLabel(QString("%1  <span style='color:#a6e3a1'>%2</span>"
                                       "  <span style='color:#cdd6f4'>%3</span>").arg(icon, label, value), &dlg);
        row->setTextFormat(Qt::RichText);
        row->setStyleSheet("font-size:12px; padding:2px 0;");
        layout->addWidget(row);
    };

    addRow("⌨", "Keystrokes:",   QString::number(sessionMetrics.keystrokes));
    addRow("↵", "Lines written:", QString::number(sessionMetrics.linesWritten));
    addRow("📂", "Files opened:", QString::number(sessionMetrics.filesOpened));
    addRow("⏱", "Active time:",  timeStr);
    addRow("🚀", "Peak WPM:",    QString::number(sessionMetrics.peakWpm));

    layout->addStretch();

    auto *closeBtn = new QPushButton("Close", &dlg);
    connect(closeBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
    layout->addWidget(closeBtn, 0, Qt::AlignCenter);

    dlg.exec();
}

void TextEditor::openCommandPalette() {
    if (!commandPalette)
        commandPalette = new CommandPalette(this);

    // Collect every QAction from all menus recursively
    QList<QAction*> actions;
    std::function<void(QMenu*)> collect = [&](QMenu *menu) {
        for (QAction *act : menu->actions()) {
            if (act->isSeparator()) continue;
            if (act->menu()) { collect(act->menu()); continue; }
            if (!act->text().isEmpty())
                actions.append(act);
        }
    };
    for (QAction *act : customMenuBar->actions()) {
        if (act->menu()) collect(act->menu());
    }

    commandPalette->populate(actions);

    // Centre it below the menu bar
    QRect geo = geometry();
    int cx = geo.left() + (geo.width() - commandPalette->width()) / 2;
    int cy = geo.top() + 60;
    commandPalette->move(cx, cy);
    commandPalette->exec();
}

void TextEditor::toggleTodoPanel() {
    if (!todoPanel) {
        todoPanel = new TodoPanel(this);
        todoDock = new QDockWidget("TODO / FIXME", this);
        todoDock->setObjectName("todoDock");
        todoDock->setAllowedAreas(Qt::BottomDockWidgetArea | Qt::RightDockWidgetArea);
        todoDock->setWidget(todoPanel);
        todoDock->setStyleSheet(
            "QDockWidget { color:#cccccc; font-family:Consolas; font-size:11px; }"
            "QDockWidget::title { background:#252526; padding:4px 8px; "
            "                     border-bottom:1px solid #3c3c3c; }");
        addDockWidget(Qt::BottomDockWidgetArea, todoDock);

        connect(todoPanel, &TodoPanel::jumpRequested, this, &TextEditor::onTodoJump);
        connect(todoDock, &QDockWidget::visibilityChanged, this, [this](bool vis) {
            if (todoAct) todoAct->setChecked(vis);
            if (vis) todoPanel->scan(tabWidget);
        });
    }

    bool show = !todoDock->isVisible();
    todoDock->setVisible(show);
    if (todoAct) todoAct->setChecked(show);
    if (show) todoPanel->scan(tabWidget);
}

void TextEditor::onTodoJump(const QString &filePath, int line) {
    if (filePath.isEmpty()) {
        // Refresh button pressed
        if (todoPanel) todoPanel->scan(tabWidget);
        return;
    }
    // Find the tab with this file and jump to the line
    const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                                 : QList<QTabWidget *>{tabWidget};
    for (QTabWidget *tw : panes) {
        for (int i = 0; i < tw->count(); ++i) {
            CodeEditor *ed = qobject_cast<CodeEditor *>(tw->widget(i));
            if (ed && normalizedPath(ed->getFileName()) == normalizedPath(filePath)) {
                activeTabWidget = tw;
                tw->setCurrentIndex(i);
                QTextCursor cur = ed->textCursor();
                cur.movePosition(QTextCursor::Start);
                cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
                ed->setTextCursor(cur);
                ed->centerCursor();
                ed->setFocus();
                return;
            }
        }
    }
    // File not open — load it
    loadFile(filePath);
    QTimer::singleShot(100, this, [this, line]() {
        CodeEditor *ed = currentEditor();
        if (!ed) return;
        QTextCursor cur = ed->textCursor();
        cur.movePosition(QTextCursor::Start);
        cur.movePosition(QTextCursor::NextBlock, QTextCursor::MoveAnchor, line);
        ed->setTextCursor(cur);
        ed->centerCursor();
    });
}

void TextEditor::toggleGraveyard() {
    bool visible = !graveyardDock->isVisible();
    graveyardDock->setVisible(visible);
    if (graveyardAct) graveyardAct->setChecked(visible);
}

// ── Security Pack Implementations ───────────────────────────────────────────

void TextEditor::toggleParanoiaMode() {
    editorPrefs.paranoiaMode = paranoiaModeAct->isChecked();
    if (paranoiaLabel) {
        paranoiaLabel->setVisible(editorPrefs.paranoiaMode);
    }
    for (CodeEditor *ed : allEditors())
        ed->setParanoiaMode(editorPrefs.paranoiaMode);
    if (editorPrefs.paranoiaMode) {
        flashStatusMessage("PARANOIA MODE: ON — session leaves no trace", QColor("#e81123"), 3000);
    } else {
        flashStatusMessage("PARANOIA MODE: OFF", QColor("#569cd6"), 2000);
    }
}

void TextEditor::toggleVulnScan() {
    bool enabled = vulnScanAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setVulnScanEnabled(enabled);
    if (enabled) {
        flashStatusMessage("⚡ Vuln Scanner: ON — laser underlines active", QColor("#ff2828"), 3000);
    } else {
        flashStatusMessage("Vuln Scanner: OFF", QColor("#569cd6"), 2000);
    }
}

void TextEditor::toggleCRT() {
    bool enabled = crtAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setCRTEnabled(enabled);
    flashStatusMessage(enabled ? "CRT Effect: ON" : "CRT Effect: OFF",
                       QColor("#66fcf1"), 2000);
}

void TextEditor::onCodeBlockDeleted(const QString &code, const QString &source) {
    if (graveyardWidget)
        graveyardWidget->addSnippet(code, source);
    // Auto-show graveyard dock briefly
    if (graveyardDock && !graveyardDock->isVisible()) {
        graveyardDock->show();
        if (graveyardAct) graveyardAct->setChecked(true);
    }
}

void TextEditor::toggleKeyHeatmap() {
    if (!keyHeatmap) return;
    if (keyHeatmap->isVisible())
        keyHeatmap->hide();
    else
        keyHeatmap->show();
}

void TextEditor::toggleVimMode() {
    bool enabled = vimModeAct->isChecked();
    for (CodeEditor *ed : allEditors())
        ed->setVimEnabled(enabled);
    if (vimModeLabel)
        vimModeLabel->setVisible(enabled);
    flashStatusMessage(enabled ? "Vim Mode: ON  (Esc = Normal)" : "Vim Mode: OFF",
                       QColor("#c678dd"), 2500);
}

void TextEditor::updateAmbientTheme() {
    int hour = QTime::currentTime().hour();
    QColor tint;
    if      (hour >=  5 && hour <  8) tint = QColor(255, 160,  80, 22); // dawn — warm amber
    else if (hour >= 17 && hour < 20) tint = QColor(255, 100,  40, 28); // dusk — deep orange
    else if (hour >= 20 || hour <  5) tint = QColor( 40,  70, 200, 20); // night — cool blue
    // else: daytime — no tint (default theme bg)

    for (CodeEditor *ed : allEditors())
        ed->setAmbientBackground(tint);
}

void TextEditor::openBinaryInspector() {
    QString path;
    CodeEditor *ed = currentEditor();
    if (ed && !ed->getFileName().isEmpty()) {
        path = ed->getFileName();
    } else {
        HexEditor *hex = qobject_cast<HexEditor *>(currentTabWidget()->currentWidget());
        if (hex)
            path = hex->property("fileName").toString();
    }

    if (path.isEmpty()) {
        path = QFileDialog::getOpenFileName(
            this, "Select Binary to Inspect", "",
            "Executables & Libraries (*.exe *.dll *.so *.o *.out *.elf);;"
            "All Files (*)");
    }
    if (!path.isEmpty())
        openInBinaryInspector(path);
}

// ── File-tree context menu ────────────────────────────────────────────────────

void TextEditor::onFileTreeContextMenu(const QPoint &pos) {
    QModelIndex idx = fileTree->indexAt(pos);
    if (!idx.isValid()) return;

    QString filePath = fileSystemModel->filePath(idx);
    QFileInfo fi(filePath);
    if (!fi.isFile()) return;

    QMenu menu(this);
    menu.setStyleSheet(
        "QMenu { background:#2d2d30; color:#cccccc; border:1px solid #454545; "
        "        border-radius:6px; padding:4px; }"
        "QMenu::item { padding:6px 24px 6px 12px; border-radius:3px; }"
        "QMenu::item:selected { background:#094771; }"
        "QMenu::separator { height:1px; background:#3e3e42; margin:3px 8px; }");

    QAction *openAct       = menu.addAction("Open");
    QAction *openHexMenuAct   = menu.addAction("⬡  Open in Hex Editor");
    menu.addSeparator();
    QAction *disasmAct     = menu.addAction("⚙  Disassemble");
    QAction *inspectAct    = menu.addAction("🔍 Binary Inspector");
    menu.addSeparator();
    QAction *revealAct     = menu.addAction("Reveal in Explorer");
    QAction *deleteAct     = menu.addAction("🗑 Delete File");

    QAction *chosen = menu.exec(fileTree->viewport()->mapToGlobal(pos));
    if (!chosen) return;

    if (chosen == openAct) {
        loadFile(filePath);
    } else if (chosen == openHexMenuAct) {
        QFile f(filePath);
        if (f.open(QIODevice::ReadOnly)) {
            QByteArray bytes = f.readAll();
            HexEditor *hex = new HexEditor();
            hex->setData(bytes);
            hex->setProperty("fileName", filePath);
            connect(hex, &HexEditor::modificationChanged,
                    this, &TextEditor::documentWasModified);
            hideWelcomeScreen();
            QTabWidget *tw = currentTabWidget();
            int tabIdx = tw->addTab(hex, "[HEX] " + fi.fileName());
            tw->setCurrentIndex(tabIdx);
            activeTabWidget = tw;
            flashTabLabel(tabIdx, tw);
        }
    } else if (chosen == disasmAct) {
        openInDisassembler(filePath);
    } else if (chosen == inspectAct) {
        openInBinaryInspector(filePath);
    } else if (chosen == revealAct) {
        QDesktopServices::openUrl(
            QUrl::fromLocalFile(fi.absolutePath()));
    } else if (chosen == deleteAct) {
        QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete File",
            QString("Are you sure you want to permanently delete\n%1?").arg(fi.fileName()),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (!QFile::remove(filePath)) {
                QMessageBox::critical(this, "Error", "Could not delete the file.");
            }
        }
    }
}
