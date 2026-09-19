#include "texteditor_private.h"

void TextEditor::newFile() {
  hideWelcomeScreen();
  CodeEditor *editor = new CodeEditor();
  SyntaxHighlighter *highlighter = new SyntaxHighlighter(editor->document());
  highlighters[editor] = highlighter;
  QFont font(editorPrefs.editorFontFamily, editorPrefs.fontSize);
  font.setStyleStrategy(QFont::PreferDefault);
  editor->setFont(font);
  editor->setLineWrapMode(editorPrefs.wordWrapEnabled ? QPlainTextEdit::WidgetWidth
                                          : QPlainTextEdit::NoWrap);
  applyThemeToEditor(editor, highlighter);
  connect(editor->document(), &QTextDocument::modificationChanged, this,
          &TextEditor::documentWasModified);
  connect(editor, &QPlainTextEdit::cursorPositionChanged, this,
          &TextEditor::updateStatusBar);
  connect(editor, &QPlainTextEdit::cursorPositionChanged, this,
          &TextEditor::updateBreadcrumb);
  connect(editor, &QPlainTextEdit::textChanged, this, [this, editor]() {
      aiAutocomplete->trigger(editor);
  });
  if (typingSoundEnabled && typingSound) {
      connect(editor, &CodeEditor::characterTyped, typingSound, &QSoundEffect::play);
  }
  connect(editor, &CodeEditor::codeBlockDeleted, this, &TextEditor::onCodeBlockDeleted);
  if (hudWidget)
      connect(editor, &CodeEditor::characterTyped, hudWidget, &HUDWidget::addKeystroke);
  // Keystroke heatmap
  if (keyHeatmap)
      connect(editor, &CodeEditor::keyPressed, keyHeatmap, [this](int key, const QString &text) {
          keyHeatmap->recordKey(text, key);
      });
  // Vim mode status label
  connect(editor, &CodeEditor::vimModeChanged, this, [this](const QString &mode) {
      if (vimModeLabel) {
          if (mode.isEmpty()) { vimModeLabel->hide(); return; }
          vimModeLabel->setText("  " + mode + "  ");
          bool isNormal = (mode == "NORMAL");
          vimModeLabel->setStyleSheet(QString(
              "color: #282c34; background-color: %1; padding: 1px 8px; "
              "font-family: Consolas; font-weight: bold; font-size: 10px;")
              .arg(isNormal ? "#c678dd" : "#98c379"));
          vimModeLabel->show();
      }
  });
  if (crtAct && crtAct->isChecked())
      editor->setCRTEnabled(true);
  if (vimModeAct && vimModeAct->isChecked())
      editor->setVimEnabled(true);
  if (paranoiaModeAct && paranoiaModeAct->isChecked())
      editor->setParanoiaMode(true);
  if (vulnScanAct && vulnScanAct->isChecked())
      editor->setVulnScanEnabled(true);
  if (focusFadeAct && focusFadeAct->isChecked())
      editor->setFocusFadeEnabled(true);
  if (imagePreviewAct && imagePreviewAct->isChecked())
      editor->setImagePreviewEnabled(true);
  propagateV080Settings(editor);
  propagateV090Settings(editor);
  connect(editor, &CodeEditor::keyPressed, this, &TextEditor::trackKeystroke);
  QTabWidget *tw = currentTabWidget();
  int index = tw->addTab(editor, "Untitled");
  tw->setCurrentIndex(index);
  activeTabWidget = tw;
  editor->setFocus();
  QTimer::singleShot(0, this, &TextEditor::clampToScreen);
}

void TextEditor::openFile() {
  QString fileName = QFileDialog::getOpenFileName(
      this, "Open File", "",
      "All Files (*);;Text Files (*.txt);;Story Files (*.story *.tw *.twee);;C++ Files (*.cpp *.h);;Python Files "
      "(*.py);;JavaScript (*.js *.ts);;Rust (*.rs);;Go (*.go)");
  if (!fileName.isEmpty()) {
    loadFile(fileName);
  }
}

void TextEditor::openRecentFile() {
  QAction *action = qobject_cast<QAction *>(sender());
  if (action)
    loadFile(action->data().toString());
}

bool TextEditor::saveFile() {
  QTabWidget *tw = currentTabWidget();
  QWidget *document = tw ? tw->currentWidget() : nullptr;
  CodeEditor *editor = currentEditor();
  if (!editor) {
    HexEditor *hexEditor = qobject_cast<HexEditor *>(document);
    if (hexEditor) {
      QString fileName = hexEditor->property("fileName").toString();
      if (fileName.isEmpty())
        return saveFileAs();
      return saveFileToPath(fileName, document);
    }
    return false;
  }
  if (editor->getFileName().isEmpty())
    return saveFileAs();
  else
    return saveFileToPath(editor->getFileName(), document);
}

bool TextEditor::saveFileAs() {
  CodeEditor *editor = currentEditor();
  QTabWidget *tw = currentTabWidget();
  QWidget *document = tw ? tw->currentWidget() : nullptr;
  HexEditor *hexEditor = qobject_cast<HexEditor *>(document);
  if (!editor && !hexEditor)
    return false;

  QString fileName =
      QFileDialog::getSaveFileName(this, "Save File", "",
                                   "All Files (*);;Text Files (*.txt);;Story Files (*.story *.tw *.twee);;C++ "
                                   "Files (*.cpp *.h);;Python Files (*.py)");
  if (fileName.isEmpty())
    return false;
  return saveFileToPath(fileName, document);
}

void TextEditor::closeTab(int index, QTabWidget *targetWidget) {
  QTabWidget *tw = targetWidget ? targetWidget : currentTabWidget();
  if (!tw || index < 0 || index >= tw->count())
    return;
  QWidget *page = tw->widget(index);
  if (page == welcomeWidget) {
    tw->removeTab(index);
    return;
  }
  if (maybeSave(index, tw)) {
    CodeEditor *editor = qobject_cast<CodeEditor *>(page);
    if (editor) {
      unwatchFile(editor->getFileName());
      highlighters.remove(editor);
    }
    tw->removeTab(index);
    page->deleteLater();
    if (tw == tabWidget2 && tw->count() == 0 && editorPrefs.splitViewEnabled) {
      editorPrefs.splitViewEnabled = false;
      splitViewAct->setChecked(false);
      tabWidget2->hide();
      activeTabWidget = tabWidget;
    }
    updateSplitEmptyState();
    if (tw == tabWidget && tw->count() == 0)
      showWelcomeScreen();
  }
}

void TextEditor::tabChanged(int) {
  if (auto *source = qobject_cast<QTabWidget *>(sender()))
    activeTabWidget = source;
  QTabWidget *tw = currentTabWidget();
  if (tabWidget2) {
    tabWidget->setProperty("activePane", tw == tabWidget);
    tabWidget2->setProperty("activePane", tw == tabWidget2);
    tabWidget->style()->unpolish(tabWidget);
    tabWidget->style()->polish(tabWidget);
    tabWidget2->style()->unpolish(tabWidget2);
    tabWidget2->style()->polish(tabWidget2);
    updateSplitEmptyState();
    applyPaneDimming();
  }
  updateStatusBar();
  updateBreadcrumb();

  // Keep markdown preview in sync when switching tabs
  if (markdownPreview && markdownPreviewAct->isChecked()) {
      CodeEditor *ed = currentEditor();
      if (ed && ed != markdownEditor)
          connectMarkdownPreview(ed);
      // Trigger a refresh (debounced)
      if (markdownTimer)
          markdownTimer->start();
  }

  CodeEditor *editor = currentEditor();
  HexEditor *hexEditor = qobject_cast<HexEditor *>(tw ? tw->currentWidget() : nullptr);
  if (editor) {
    QString title = "Jim";
    if (!editor->getFileName().isEmpty())
      title = strippedName(editor->getFileName()) + " - " + title;
    if (editor->isModified())
      title = "*" + title;
    setWindowTitle(title);

    // Update tab text with asterisk if modified
    int currentIdx = tw->currentIndex();
    QString tabText = strippedName(editor->getFileName());
    if (tabText.isEmpty())
      tabText = "Untitled";
    if (editor->isModified())
      tabText = "*" + tabText;
    tw->setTabText(currentIdx, tabText);

    // Update language label
    Language lang = editor->getLanguage();
    QStringList langNames = {"Plain Text", "C++",    "Python",   "JavaScript",
                             "HTML",       "CSS",    "Rust",     "Go",
                             "JSON",       "YAML",   "Markdown", "Solidity",
                             "Yul",        "Story ✦"};
    int langIdx = static_cast<int>(lang);
    languageLabel->setText(langIdx < langNames.size() ? langNames[langIdx] : "Plain Text");
  } else if (hexEditor) {
    QString fileName = hexEditor->property("fileName").toString();
    QString title = "Jim";
    if (!fileName.isEmpty())
      title = strippedName(fileName) + " - " + title;
    if (hexEditor->isModified())
      title = "*" + title;
    setWindowTitle(title);

    // Update tab text with asterisk
    int currentIdx = tw->currentIndex();
    QString tabText = "[HEX] " + strippedName(fileName);
    if (hexEditor->isModified())
      tabText = "*" + tabText;
    tw->setTabText(currentIdx, tabText);

    languageLabel->setText("Binary (Hex)");
  }
  updateContextualActions();
}

void TextEditor::findText() {
    CodeEditor *editor = currentEditor();
    if (!editor) return;
    
    QString selected = editor->textCursor().selectedText();
    if (selected.isEmpty()) selected = searchState.lastText;
    
    findBar->showAndFocus(selected);
    onFindTextChanged(findBar->getSearchText());
}

void TextEditor::onFindTextChanged(const QString &text) {
    searchState.lastText = text;
    searchState.matches.clear();
    searchState.currentIndex = -1;
    
    CodeEditor *editor = currentEditor();
    if (!editor || text.isEmpty()) {
        updateSearchHighlights();
        findBar->setMatchCount(0, 0);
        return;
    }

    QString content = editor->toPlainText();
    QString expression = findBar->isRegex() ? text : QRegularExpression::escape(text);
    if (findBar->isWholeWord())
        expression = "(?<![\\w])(?:" + expression + ")(?![\\w])";
    QRegularExpression::PatternOptions options = QRegularExpression::NoPatternOption;
    if (!findBar->isCaseSensitive())
        options |= QRegularExpression::CaseInsensitiveOption;
    QRegularExpression re(expression, options);
    if (!re.isValid()) {
        updateSearchHighlights();
        findBar->setMatchCount(0, 0);
        return;
    }
    QRegularExpressionMatchIterator i = re.globalMatch(content);
    const QTextCursor selection = editor->textCursor();
    const int selectionStart = findBar->isSelectionOnly() && selection.hasSelection()
        ? selection.selectionStart() : 0;
    const int selectionEnd = findBar->isSelectionOnly() && selection.hasSelection()
        ? selection.selectionEnd() : content.size();

    int currentPos = editor->textCursor().position();
    
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.capturedStart() < selectionStart || match.capturedEnd() > selectionEnd)
            continue;
        QTextCursor cursor(editor->document());
        cursor.setPosition(match.capturedStart());
        cursor.setPosition(match.capturedEnd(), QTextCursor::KeepAnchor);
        searchState.matches.append(cursor);
        
        if (searchState.currentIndex == -1 && match.capturedStart() >= currentPos) {
            searchState.currentIndex = searchState.matches.size() - 1;
        }
    }

    if (searchState.currentIndex == -1 && !searchState.matches.isEmpty()) {
        searchState.currentIndex = 0;
    }

    updateSearchHighlights();
    findBar->setMatchCount(searchState.currentIndex + 1, searchState.matches.size());
    
    if (searchState.currentIndex != -1) {
        editor->setTextCursor(searchState.matches[searchState.currentIndex]);
        editor->ensureCursorVisible();
    }
}

void TextEditor::findNext() {
    if (searchState.matches.isEmpty()) return;
    searchState.currentIndex =
        (searchState.currentIndex + 1) % searchState.matches.size();
    CodeEditor *editor = currentEditor();
    if (editor) {
        editor->setTextCursor(searchState.matches[searchState.currentIndex]);
        editor->ensureCursorVisible();
        updateSearchHighlights();
        findBar->setMatchCount(searchState.currentIndex + 1, searchState.matches.size());
    }
}

void TextEditor::findPrevious() {
    if (searchState.matches.isEmpty()) return;
    searchState.currentIndex =
        (searchState.currentIndex - 1 + searchState.matches.size()) %
        searchState.matches.size();
    CodeEditor *editor = currentEditor();
    if (editor) {
        editor->setTextCursor(searchState.matches[searchState.currentIndex]);
        editor->ensureCursorVisible();
        updateSearchHighlights();
        findBar->setMatchCount(searchState.currentIndex + 1, searchState.matches.size());
    }
}

void TextEditor::closeFindBar() {
    findBar->hide();
    searchState.matches.clear();
    searchState.currentIndex = -1;
    updateSearchHighlights();
    if (CodeEditor *editor = currentEditor()) {
        editor->setFocus();
    }
}

void TextEditor::updateSearchHighlights() {
    CodeEditor *editor = currentEditor();
    if (editor) {
        editor->setSearchSelections(searchState.matches);
    }
}

void TextEditor::replaceText() {
  CodeEditor *editor = currentEditor();
  if (!editor)
    return;
  bool ok;
  QString findStr = QInputDialog::getText(
      this, "Replace", "Find what:", QLineEdit::Normal, searchState.lastText, &ok);
  if (!ok || findStr.isEmpty())
    return;
  QString replaceStr = QInputDialog::getText(
      this, "Replace", "Replace with:", QLineEdit::Normal, "", &ok);
  if (!ok)
    return;
  searchState.lastText = findStr;
  QString content = editor->toPlainText();
  QString expression = findBar->isRegex() ? findStr : QRegularExpression::escape(findStr);
  if (findBar->isWholeWord())
    expression = "(?<![\\w])(?:" + expression + ")(?![\\w])";
  QRegularExpression re(expression,
                         findBar->isCaseSensitive() ? QRegularExpression::NoPatternOption
                                                     : QRegularExpression::CaseInsensitiveOption);
  if (!re.isValid()) return;
  if (findBar->isSelectionOnly() && editor->textCursor().hasSelection()) {
    QTextCursor selection = editor->textCursor();
    QString selected = selection.selectedText();
    selected.replace(re, replaceStr);
    selection.insertText(selected);
  } else {
    content.replace(re, replaceStr);
    editor->setPlainText(content);
  }
}

void TextEditor::goToLine() {
  CodeEditor *editor = currentEditor();
  if (!editor)
    return;
  bool ok;
  int line = QInputDialog::getInt(this, "Go to Line", "Line number:", 1, 1,
                                  editor->document()->blockCount(), 1, &ok);
  if (ok) {
    QTextCursor cursor(editor->document()->findBlockByLineNumber(line - 1));
    editor->setTextCursor(cursor);
    editor->centerCursor();
  }
}

void TextEditor::documentWasModified() {
  QTabWidget *tw = currentTabWidget();
  tabChanged(tw ? tw->currentIndex() : -1);
  // Refresh story panels live as the user types
  CodeEditor *ed = currentEditor();
  if (ed && ed->getLanguage() == Language::Story) {
      if ((storyGraphDock && storyGraphDock->isVisible()) ||
          (storyPlaytestDock && storyPlaytestDock->isVisible()))
          refreshStoryPanels();
  }
}

void TextEditor::updateStatusBar() {
  CodeEditor *editor = currentEditor();
  if (editor) {
    QTextCursor cursor = editor->textCursor();
    statusLabel->setText(QString("Ln %1, Col %2")
                             .arg(cursor.blockNumber() + 1)
                             .arg(cursor.columnNumber() + 1));
  }
}

void TextEditor::increaseFontSize() {
  editorPrefs.fontSize++;
  for (CodeEditor *editor : allEditors()) {
    QFont font = editor->font();
    font.setPointSize(editorPrefs.fontSize);
    editor->setFont(font);
  }
}

void TextEditor::decreaseFontSize() {
  if (editorPrefs.fontSize > 6) {
    editorPrefs.fontSize--;
    for (CodeEditor *editor : allEditors()) {
      QFont font = editor->font();
      font.setPointSize(editorPrefs.fontSize);
      editor->setFont(font);
    }
  }
}

void TextEditor::selectFont() {
  // Build font list: featured ligature fonts first, then remaining monospace
  static const QStringList kFeatured = {
      "JetBrains Mono", "Fira Code", "Cascadia Code", "Cascadia Mono",
      "Iosevka", "Iosevka Term", "Victor Mono", "Hack", "Inconsolata",
      "Source Code Pro", "Consolas", "Courier New"
  };

  QStringList allFamilies = QFontDatabase::families();
  QStringList fontList;
  for (const QString &f : kFeatured)
      if (allFamilies.contains(f))
          fontList << f;
  for (const QString &f : allFamilies)
      if (QFontDatabase::isFixedPitch(f) && !fontList.contains(f))
          fontList << f;

  QDialog dlg(this);
  dlg.setWindowTitle("Select Editor Font");
  dlg.setMinimumWidth(420);
  dlg.setStyleSheet(
      "QDialog { background: #1e1e1e; color: #cccccc; }"
      "QLabel  { color: #cccccc; }"
      "QComboBox { background: #252526; color: #cccccc; border: 1px solid #3e3e42; "
      "            border-radius: 4px; padding: 4px 8px; font-size: 12px; }"
      "QComboBox::drop-down { border: none; }"
      "QComboBox QAbstractItemView { background: #252526; color: #cccccc; "
      "                              selection-background-color: #094771; }"
      "QDialogButtonBox QPushButton { background: #313244; color: #cccccc; "
      "  border: 1px solid #45475a; border-radius: 6px; padding: 5px 18px; }"
      "QDialogButtonBox QPushButton:hover { background: #45475a; }");

  auto *layout = new QVBoxLayout(&dlg);
  layout->setSpacing(10);

  auto *descLabel = new QLabel("Choose a monospace font. Ligature fonts (e.g. Fira Code, JetBrains Mono)\nrender <code>=>  !=  >=  -></code> as single glyphs when installed.");
  descLabel->setTextFormat(Qt::RichText);
  descLabel->setStyleSheet("color: #9e9e9e; font-size: 11px;");
  layout->addWidget(descLabel);

  auto *combo = new QComboBox;
  combo->addItems(fontList);
  int cur = fontList.indexOf(editorPrefs.editorFontFamily);
  if (cur >= 0) combo->setCurrentIndex(cur);
  layout->addWidget(combo);

  auto *previewLabel = new QLabel("fn => x != y && result >= 0 -> done");
  previewLabel->setFont(QFont(editorPrefs.editorFontFamily, 13));
  previewLabel->setStyleSheet(
      "padding: 10px 12px; background: #0d0d0d; color: #9cdcfe; "
      "border-radius: 6px; font-size: 13px;");
  layout->addWidget(previewLabel);

  connect(combo, &QComboBox::currentTextChanged, previewLabel, [previewLabel](const QString &f) {
      QFont pf(f, 13);
      pf.setStyleStrategy(QFont::PreferDefault);
      previewLabel->setFont(pf);
  });

  auto *noteLabel = new QLabel("Tip: Install Fira Code or JetBrains Mono for ligature support.");
  noteLabel->setStyleSheet("color: #555; font-size: 10px;");
  layout->addWidget(noteLabel);

  auto *buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  layout->addWidget(buttons);
  connect(buttons, &QDialogButtonBox::accepted, &dlg, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dlg, &QDialog::reject);

  if (dlg.exec() != QDialog::Accepted)
      return;

  editorPrefs.editorFontFamily = combo->currentText();
  for (CodeEditor *editor : allEditors()) {
      QFont f(editorPrefs.editorFontFamily, editorPrefs.fontSize);
      f.setStyleStrategy(QFont::PreferDefault);
      editor->setFont(f);
  }
  flashStatusMessage(QString("Font: %1").arg(editorPrefs.editorFontFamily), QColor("#98c379"));
}

void TextEditor::toggleWordWrap() {
  editorPrefs.wordWrapEnabled = !editorPrefs.wordWrapEnabled;
  wordWrapAct->setChecked(editorPrefs.wordWrapEnabled);
  for (CodeEditor *editor : allEditors())
    editor->setLineWrapMode(editorPrefs.wordWrapEnabled ? QPlainTextEdit::WidgetWidth
                                            : QPlainTextEdit::NoWrap);
}

void TextEditor::animateTerminalShow() {
  terminalWidget->show();
  if (!terminalAnim) {
    terminalAnim = new QVariantAnimation(this);
    terminalAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(terminalAnim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &val) {
              QList<int> sizes = verticalSplitter->sizes();
              int total = sizes[0] + sizes[1];
              int newBottom = val.toInt();
              sizes[1] = newBottom;
              sizes[0] = qMax(50, total - newBottom);
              verticalSplitter->setSizes(sizes);
            });
  }
  if (terminalAnim->state() == QAbstractAnimation::Running)
    terminalAnim->stop();

  QList<int> sizes = verticalSplitter->sizes();
  int current = sizes.value(1, 0);
  terminalAnim->setDuration(260);
  terminalAnim->setStartValue(current);
  terminalAnim->setEndValue(terminalTargetH);
  terminalAnim->start();
}

void TextEditor::animateTerminalHide() {
  if (!terminalAnim) {
    terminalAnim = new QVariantAnimation(this);
    terminalAnim->setEasingCurve(QEasingCurve::OutCubic);
    connect(terminalAnim, &QVariantAnimation::valueChanged, this,
            [this](const QVariant &val) {
              QList<int> sizes = verticalSplitter->sizes();
              int total = sizes[0] + sizes[1];
              int newBottom = val.toInt();
              sizes[1] = newBottom;
              sizes[0] = qMax(50, total - newBottom);
              verticalSplitter->setSizes(sizes);
            });
  }
  if (terminalAnim->state() == QAbstractAnimation::Running)
    terminalAnim->stop();

  // Remember the current height before collapsing
  QList<int> sizes = verticalSplitter->sizes();
  if (sizes.value(1, 0) > 10)
    terminalTargetH = sizes[1];

  terminalAnim->setDuration(220);
  terminalAnim->setStartValue(sizes.value(1, 0));
  terminalAnim->setEndValue(0);
  connect(terminalAnim, &QVariantAnimation::finished, this, [this]() {
    terminalWidget->hide();
    disconnect(terminalAnim, &QVariantAnimation::finished, this, nullptr);
  });
  terminalAnim->start();
}

void TextEditor::toggleTerminal() {
  bool currentlyVisible = terminalWidget->isVisible() &&
                          verticalSplitter->sizes().value(1, 0) > 5;
  if (currentlyVisible) {
    animateTerminalHide();
    terminalAct->setChecked(false);
  } else {
    animateTerminalShow();
    terminalAct->setChecked(true);
  }
}

void TextEditor::cycleAnimation() {
  animationWidget->cycleAnimation();
  
  AnimationWidget::AnimationType currentType = animationWidget->getCurrentType();
  if (currentType == AnimationWidget::None) {
    animationDock->hide();
    toggleAnimationDockAct->setChecked(false);
  } else {
    animationDock->show();
    toggleAnimationDockAct->setChecked(true);
  }
  
  QString animName;
  switch (currentType) {
    case AnimationWidget::None: animName = "None"; break;
    case AnimationWidget::Matrix: animName = "Matrix"; break;
    case AnimationWidget::Particles: animName = "Particles"; break;
    case AnimationWidget::Waves: animName = "Waves"; break;
    case AnimationWidget::Pulse: animName = "Pulse"; break;
    case AnimationWidget::Starfield: animName = "Starfield"; break;
    case AnimationWidget::Rain: animName = "Rain"; break;
    case AnimationWidget::Snow: animName = "Snow"; break;
    case AnimationWidget::Fire: animName = "Fire"; break;
    case AnimationWidget::DJMode: animName = "DJ Mode"; break;
  }
  statusBar()->showMessage("Animation: " + animName, 2000);
}

void TextEditor::toggleAnimationDock() {
    if (animationDock->isVisible()) {
        animationDock->hide();
        animationWidget->setAnimationType(AnimationWidget::None);
    } else {
        animationDock->show();
        if (animationWidget->getCurrentType() == AnimationWidget::None) {
            animationWidget->cycleAnimation(); // Start with first animation
        }
    }
    toggleAnimationDockAct->setChecked(animationDock->isVisible());
}

void TextEditor::toggleZenMode() {
    zenModeActive = !zenModeActive;
    if (zenModeAct) zenModeAct->setChecked(zenModeActive);
    
    auto applyZen = [&](QWidget *w, bool hide) {
        if (w) w->setVisible(!hide);
    };

    applyZen(menuBar(), zenModeActive);
    applyZen(statusBar(), zenModeActive);
    applyZen(breadcrumbBar, zenModeActive);
    
    if (zenModeActive) {
        if (terminalWidget->isVisible()) terminalWidget->hide();
        if (animationDock->isVisible()) animationDock->hide();
        if (fileTreeDock->isVisible()) fileTreeDock->hide();
        // Don't hide DJ visualizer dock - keep it visible in fullscreen
        setWindowState(windowState() | Qt::WindowFullScreen);
    } else {
        setWindowState(windowState() & ~Qt::WindowFullScreen);
    }
}

void TextEditor::toggleTypingSound() {
    typingSoundEnabled = !typingSoundEnabled;
    if (typingSoundAct) typingSoundAct->setChecked(typingSoundEnabled);
    
    if (typingSoundEnabled && !typingSound) {
        typingSound = new QSoundEffect(this);
        QString tempPath = QDir::tempPath() + "/jim_click.wav";
        if (!QFile::exists(tempPath)) {
            QFile f(tempPath);
            if (f.open(QIODevice::WriteOnly)) {
                QByteArray wav = QByteArray::fromHex(
                    "524946463A00000057415645666D74201000000001000100112B0000112B0000010008006461746116000000809a80b380bf80b3809a807f8065804c8040804c8065807f809a80b380bf80b3809a807f8065804c804080"
                );
                f.write(wav);
                f.close();
            }
        }
        typingSound->setSource(QUrl::fromLocalFile(tempPath));
        typingSound->setVolume(0.5f);
        
        for (CodeEditor *ed : allEditors())
            connect(ed, &CodeEditor::characterTyped, typingSound, &QSoundEffect::play);
    } else if (!typingSoundEnabled && typingSound) {
        for (CodeEditor *ed : allEditors())
            disconnect(ed, &CodeEditor::characterTyped, typingSound, &QSoundEffect::play);
    }
}

void TextEditor::toggleDJMode() {
    bool active = djModeAct->isChecked();
    if (active) {
        if (!audioMonitor) {
            audioMonitor = new AudioMonitor(this);
            connect(audioMonitor, &AudioMonitor::captureStatus, this,
                    [this](const QString &status) {
                        statusBar()->showMessage(status, 6000);
                    });
        }
        
        // Cap height before adding the dock so Qt can't grow past the screen
        setMaximumHeight(QGuiApplication::primaryScreen()->availableGeometry().height());

        // Create and show the DJ visualizer dock
        if (!djVisualizerWidget) {
            djVisualizerWidget = new DJVisualizerWidget(this);
            djVisualizerDock = new QDockWidget("🎵 DJ Mode Visualizer", this);
            djVisualizerDock->setObjectName("djVisualizerDock");
            djVisualizerDock->setWidget(djVisualizerWidget);
            djVisualizerDock->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
            djVisualizerDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
            // Make the dock more persistent in fullscreen
            djVisualizerDock->setProperty("fullscreen", true);
            addDockWidget(Qt::BottomDockWidgetArea, djVisualizerDock);
            
            // Connect audio updates to the visualizer
            connect(audioMonitor, &AudioMonitor::levelsUpdated,
                    djVisualizerWidget->visualizerWidget, qOverload<>(&QWidget::update));
        }
        djVisualizerWidget->setAudioMonitor(audioMonitor);
        djVisualizerDock->setVisible(true);
        djVisualizerDock->raise();
        QTimer::singleShot(0, this, &TextEditor::clampToScreen);

        // Start only after the visualizer is ready so status and repaint signals are connected.
        if (!audioMonitor->isRunning()) {
            audioMonitor->start();
        }

    } else {
        if (audioMonitor) {
            audioMonitor->stop();
        }
        if (djVisualizerDock) {
            djVisualizerDock->setVisible(false);
        }
        setMaximumHeight(QWIDGETSIZE_MAX); // release the screen-height cap
        // Reset the dock animation widget to Matrix mode
        animationWidget->setAnimationType(AnimationWidget::Matrix);
    }
}

void TextEditor::showAbout() {
  QMessageBox::about(this, "About Jim",
                     "Jim - Lightweight Code Editor\n\n"
                     "Features:\n"
                     "  Syntax highlighting (11 languages)\n"
                     "  Code folding & Breadcrumb navigation\n"
                     "  Integrated terminal\n"
                     "  Multiple tabs & Split view\n"
                     "  Find & Replace\n"
                     "  Auto-indentation & bracket pairing\n"
                     "  Theme switching & File watcher\n"
                     "  Mini map & Welcome screen");
}

void TextEditor::closeEvent(QCloseEvent *event) {
  const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                               : QList<QTabWidget *>{tabWidget};
  for (QTabWidget *tw : panes) {
    for (int i = 0; i < tw->count(); ++i) {
      if (tw->widget(i) == welcomeWidget)
        continue;
      if (!maybeSave(i, tw)) {
        event->ignore();
        return;
      }
    }
  }
  
  // Save session time
  QSettings settings("TextEditor", "Settings");
  int secs = sessionSecondsAccumulated;
  if (sessionStart.isValid())
    secs += static_cast<int>(sessionStart.secsTo(QDateTime::currentDateTime()));
  settings.setValue("sessionDate", sessionDateString);
  settings.setValue("sessionSeconds", secs);
  
  writeSettings();
  event->accept();
}

void TextEditor::readSettings() {
  QSettings settings("TextEditor", "Settings");
  recentFiles = settings.value("recentFiles").toStringList();
  sessionSecondsAccumulated = settings.value("sessionSeconds", 0).toInt();
  editorPrefs.fontSize = settings.value("fontSize", 11).toInt();
  editorPrefs.editorFontFamily = settings.value("editorFont", "Consolas").toString();
  editorPrefs.wordWrapEnabled = settings.value("wordWrap", false).toBool();
  editorPrefs.stickyScrollEnabled = settings.value("stickyScroll", false).toBool();
  editorPrefs.invisibleCharsEnabled = settings.value("invisibleChars", false).toBool();
  editorPrefs.gitBlameEnabled = settings.value("gitBlame", false).toBool();
  editorPrefs.autoSaveFocusEnabled = settings.value("autoSaveFocus", false).toBool();
  const bool hasThemePreference = settings.value("themePreferenceVersion", 0).toInt() >= 1;
  editorPrefs.currentThemeIndex = qBound(
      0, hasThemePreference ? settings.value("theme", 1).toInt() : 1,
      qMax(0, editorPrefs.themes.size() - 1));
  const bool restoreSplitView = settings.value("splitView", false).toBool();
  editorPrefs.splitViewEnabled = false;
  if (settings.contains("geometry")) restoreGeometry(settings.value("geometry").toByteArray());
  if (settings.contains("windowState")) restoreState(settings.value("windowState").toByteArray());
  if (settings.contains("editorSplitter"))
    mainSplitter->setSizes(settings.value("editorSplitter").value<QList<int>>());
  if (settings.contains("verticalSplitter"))
    verticalSplitter->setSizes(settings.value("verticalSplitter").value<QList<int>>());
  wordWrapAct->setChecked(editorPrefs.wordWrapEnabled);
  stickyScrollAct->setChecked(editorPrefs.stickyScrollEnabled);
  invisibleCharsAct->setChecked(editorPrefs.invisibleCharsEnabled);
  gitBlameAct->setChecked(editorPrefs.gitBlameEnabled);
  autoSaveFocusAct->setChecked(editorPrefs.autoSaveFocusEnabled);
  splitViewAct->setChecked(false);
  markdownPreviewAct->setChecked(settings.value("markdownPreview", false).toBool());
  applyThemeToAllEditors();
  if (restoreSplitView)
    QTimer::singleShot(0, this, &TextEditor::toggleSplitView);
  if (markdownPreviewAct->isChecked())
    QTimer::singleShot(0, this, [this]() { setMarkdownPreviewVisible(true); });
}

void TextEditor::writeSettings() {
  if (editorPrefs.paranoiaMode) return;
  QSettings settings("TextEditor", "Settings");
  settings.setValue("recentFiles", recentFiles);
  settings.setValue("fontSize", editorPrefs.fontSize);
  settings.setValue("editorFont", editorPrefs.editorFontFamily);
  settings.setValue("wordWrap", editorPrefs.wordWrapEnabled);
  settings.setValue("stickyScroll", editorPrefs.stickyScrollEnabled);
  settings.setValue("invisibleChars", editorPrefs.invisibleCharsEnabled);
  settings.setValue("gitBlame", editorPrefs.gitBlameEnabled);
  settings.setValue("autoSaveFocus", editorPrefs.autoSaveFocusEnabled);
  settings.setValue("theme", editorPrefs.currentThemeIndex);
  settings.setValue("themePreferenceVersion", 1);
  settings.setValue("splitView", editorPrefs.splitViewEnabled);
  settings.setValue("markdownPreview", markdownPreviewAct && markdownPreviewAct->isChecked());
  settings.setValue("geometry", saveGeometry());
  settings.setValue("windowState", saveState());
  settings.setValue("editorSplitter", QVariant::fromValue(mainSplitter->sizes()));
  settings.setValue("verticalSplitter", QVariant::fromValue(verticalSplitter->sizes()));
}

bool TextEditor::maybeSave(int tabIndex, QTabWidget *targetWidget) {
  QTabWidget *tw = targetWidget ? targetWidget : tabWidget;
  if (!tw || tabIndex < 0 || tabIndex >= tw->count())
    return true;
  QWidget *widget = tw->widget(tabIndex);
  CodeEditor *editor = qobject_cast<CodeEditor *>(widget);
  HexEditor *hexEditor = qobject_cast<HexEditor *>(widget);

  bool modified = false;
  if (editor)
    modified = editor->isModified();
  else if (hexEditor)
    modified = hexEditor->isModified();

  if (!modified)
    return true;

  activeTabWidget = tw;
  tw->setCurrentIndex(tabIndex);
  widget->setFocus();
  const QMessageBox::StandardButton ret = QMessageBox::warning(
      this, "Jim",
      "The document has been modified.\nDo you want to save your changes?",
      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
  switch (ret) {
  case QMessageBox::Save:
    if (editor && editor->getFileName().isEmpty())
      return saveFileAs();
    if (editor)
      return saveFileToPath(editor->getFileName(), editor);
    if (hexEditor && hexEditor->property("fileName").toString().isEmpty())
      return saveFileAs();
    if (hexEditor)
      return saveFileToPath(hexEditor->property("fileName").toString(), hexEditor);
    return false;
  case QMessageBox::Cancel:
    return false;
  default:
    break;
  }
  return true;
}

void TextEditor::loadFile(const QString &fileName) {
  const QString path = normalizedPath(fileName);
  QTabWidget *existingPane = nullptr;
  QWidget *existing = findOpenDocument(path, &existingPane);
  if (existing) {
    activeTabWidget = existingPane;
    existingPane->setCurrentIndex(existingPane->indexOf(existing));
    existing->setFocus();
    return;
  }

  QFile file(path);
  if (!file.open(QFile::ReadOnly)) {
    QMessageBox::warning(this, "Jim",
                         QString("Cannot read file %1:\n%2.")
                             .arg(path)
                             .arg(file.errorString()));
    return;
  }

  // Check if file is binary
  QByteArray fileData = file.readAll();
  file.close();

  QStringConverter::Encoding encoding = QStringDecoder::Utf8;
  if (fileData.startsWith(QByteArray::fromHex("FFFE0000")))
    encoding = QStringDecoder::Utf32LE;
  else if (fileData.startsWith(QByteArray::fromHex("0000FEFF")))
    encoding = QStringDecoder::Utf32BE;
  else if (fileData.startsWith(QByteArray::fromHex("FFFE")))
    encoding = QStringDecoder::Utf16LE;
  else if (fileData.startsWith(QByteArray::fromHex("FEFF")))
    encoding = QStringDecoder::Utf16BE;

  QStringDecoder decoder(encoding);
  QString decodedText = decoder(fileData);
  bool isBinary = decoder.hasError();
  if (!decodedText.isEmpty() && decodedText.front() == QChar::ByteOrderMark)
    decodedText.remove(0, 1);

  hideWelcomeScreen();
  QApplication::setOverrideCursor(Qt::WaitCursor);

  Language lang = Language::PlainText; // Default language

  if (isBinary) {
    // Open in hex editor
    HexEditor *hexEditor = new HexEditor();
    hexEditor->setData(fileData);
    hexEditor->setProperty("fileName", path);

    connect(hexEditor, &HexEditor::modificationChanged, this,
            &TextEditor::documentWasModified);

    int index = currentTabWidget()->addTab(hexEditor, "[HEX] " + strippedName(path));
    currentTabWidget()->setCurrentIndex(index);
  } else {
    // Open in text editor
    CodeEditor *editor = new CodeEditor();
    editor->setPlainText(decodedText);
    editor->setFileName(path);
    editor->document()->setModified(false);

    // Auto-detect language
    lang = detectLanguage(path);
    editor->setLanguage(lang);

    SyntaxHighlighter *highlighter = new SyntaxHighlighter(editor->document());
    highlighter->setLanguage(lang);
    highlighters[editor] = highlighter;

    QFont font(editorPrefs.editorFontFamily, editorPrefs.fontSize);
    font.setStyleStrategy(QFont::PreferDefault);
    editor->setFont(font);
    editor->setLineWrapMode(editorPrefs.wordWrapEnabled ? QPlainTextEdit::WidgetWidth
                                            : QPlainTextEdit::NoWrap);
    applyThemeToEditor(editor, highlighter);

    connect(editor->document(), &QTextDocument::modificationChanged, this,
            &TextEditor::documentWasModified);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this,
            &TextEditor::updateStatusBar);
    connect(editor, &QPlainTextEdit::cursorPositionChanged, this,
            &TextEditor::updateBreadcrumb);
    connect(editor, &CodeEditor::codeBlockDeleted, this,
            &TextEditor::onCodeBlockDeleted);
    if (keyHeatmap)
        connect(editor, &CodeEditor::keyPressed, keyHeatmap, [this](int key, const QString &text) {
          keyHeatmap->recordKey(text, key);
      });
    connect(editor, &CodeEditor::vimModeChanged, this, [this](const QString &mode) {
        if (vimModeLabel) {
            if (mode.isEmpty()) { vimModeLabel->hide(); return; }
            vimModeLabel->setText("  " + mode + "  ");
            bool isNormal = (mode == "NORMAL");
            vimModeLabel->setStyleSheet(QString(
                "color: #282c34; background-color: %1; padding: 1px 8px; "
                "font-family: Consolas; font-weight: bold; font-size: 10px;")
                .arg(isNormal ? "#c678dd" : "#98c379"));
            vimModeLabel->show();
        }
    });
    // Apply CRT if currently enabled
    if (crtAct && crtAct->isChecked())
        editor->setCRTEnabled(true);
    if (vimModeAct && vimModeAct->isChecked())
        editor->setVimEnabled(true);
    if (paranoiaModeAct && paranoiaModeAct->isChecked())
        editor->setParanoiaMode(true);
    if (vulnScanAct && vulnScanAct->isChecked())
        editor->setVulnScanEnabled(true);
    if (focusFadeAct && focusFadeAct->isChecked())
        editor->setFocusFadeEnabled(true);
    if (imagePreviewAct && imagePreviewAct->isChecked())
        editor->setImagePreviewEnabled(true);
    propagateV080Settings(editor);
    propagateV090Settings(editor);
    connect(editor, &CodeEditor::keyPressed, this, &TextEditor::trackKeystroke);
    if (!editorPrefs.paranoiaMode) ++sessionMetrics.filesOpened;

    // Apply ambient tint immediately so the new editor matches others
    updateAmbientTheme();

    int index = currentTabWidget()->addTab(editor, strippedName(path));
    currentTabWidget()->setCurrentIndex(index);

    watchFile(path);
  }

  QApplication::restoreOverrideCursor();
  updateRecentFiles(path);

  // Update language label
  if (isBinary) {
    languageLabel->setText("Binary (Hex)");
  } else {
    QStringList langNames = {"Plain Text", "C++",  "Python",  "JavaScript",
                             "HTML",       "CSS",  "Rust",    "Go",
                             "JSON",       "YAML", "Markdown", "Solidity", "Yul", "Story ✦"};
    int idx = static_cast<int>(lang);
    languageLabel->setText(idx < langNames.size() ? langNames[idx] : "Plain Text");
  }

  statusBar()->showMessage("File loaded", 2000);
  QTimer::singleShot(0, this, &TextEditor::clampToScreen);
}

bool TextEditor::saveFileToPath(const QString &fileName, QWidget *document) {
  const QString path = normalizedPath(fileName);
  QWidget *target = document;
  if (!target) {
    QTabWidget *tw = currentTabWidget();
    target = tw ? tw->currentWidget() : nullptr;
  }
  if (!target || path.isEmpty())
    return false;

  QGuiApplication::setOverrideCursor(Qt::WaitCursor);

  // Temporarily unwatch to prevent false "modified externally" alert
  QString oldPath;
  if (auto *editor = qobject_cast<CodeEditor *>(target))
    oldPath = editor->getFileName();
  else if (auto *hexEditor = qobject_cast<HexEditor *>(target))
    oldPath = hexEditor->property("fileName").toString();
  unwatchFile(oldPath.isEmpty() ? path : oldPath);

  QSaveFile file(path);
  if (file.open(QFile::WriteOnly)) {
    CodeEditor *editor = qobject_cast<CodeEditor *>(target);
    HexEditor *hexEditor = qobject_cast<HexEditor *>(target);
    if (editor) {
      QTextStream out(&file);
      out << editor->toPlainText();
      if (!file.commit()) {
        QGuiApplication::restoreOverrideCursor();
        watchFile(oldPath.isEmpty() ? path : oldPath); // Re-watch on failure
        QMessageBox::warning(this, "Jim",
                             QString("Cannot write file %1:\n%2.")
                                 .arg(path)
                                 .arg(file.errorString()));
        return false;
      }
    } else if (hexEditor) {
      file.write(hexEditor->data());
      if (!file.commit()) {
        QGuiApplication::restoreOverrideCursor();
        watchFile(oldPath.isEmpty() ? path : oldPath); // Re-watch on failure
        QMessageBox::warning(this, "Jim",
                             QString("Cannot write file %1:\n%2.")
                                 .arg(path)
                                 .arg(file.errorString()));
        return false;
      }
      hexEditor->setModified(false);
      hexEditor->setProperty("fileName", path);
    }
  } else {
    QGuiApplication::restoreOverrideCursor();
    watchFile(oldPath.isEmpty() ? path : oldPath); // Re-watch on failure
    QMessageBox::warning(this, "Jim",
                         QString("Cannot write file %1:\n%2.")
                             .arg(path)
                             .arg(file.errorString()));
    return false;
  }
  QApplication::restoreOverrideCursor();

  // Re-watch after successful save
  watchFile(path);

  setCurrentFile(path, target);
  updateRecentFiles(path);
  statusBar()->showMessage("File saved", 2000);
  return true;
}

void TextEditor::setCurrentFile(const QString &fileName, QWidget *document) {
  QWidget *target = document;
  if (!target) {
    QTabWidget *tw = currentTabWidget();
    target = tw ? tw->currentWidget() : nullptr;
  }
  CodeEditor *editor = qobject_cast<CodeEditor *>(target);
  if (!editor)
    return;
  unwatchFile(editor->getFileName());
  editor->setFileName(fileName);
  editor->document()->setModified(false);
  // Re-detect language
  Language lang = detectLanguage(fileName);
  editor->setLanguage(lang);
  SyntaxHighlighter *hl = highlighters.value(editor);
  if (hl) {
    hl->setLanguage(lang);
  }
  QString shownName = strippedName(fileName);
  for (QWidget *parent = editor->parentWidget(); parent; parent = parent->parentWidget()) {
    if (auto *tw = qobject_cast<QTabWidget *>(parent)) {
      tw->setTabText(tw->indexOf(editor), shownName);
      break;
    }
  }
  setWindowTitle(shownName + " - Jim");
  watchFile(fileName);
}

QString TextEditor::strippedName(const QString &fullFileName) {
  return QFileInfo(fullFileName).fileName();
}

void TextEditor::updateRecentFiles(const QString &fileName) {
  if (editorPrefs.paranoiaMode) return;
  recentFiles.removeAll(fileName);
  recentFiles.prepend(fileName);
  while (recentFiles.size() > 10)
    recentFiles.removeLast();
  updateRecentFilesMenu();
}

void TextEditor::updateRecentFilesMenu() {
  recentFilesMenu->clear();
  for (const QString &file : recentFiles) {
    QAction *action = new QAction(strippedName(file), this);
    action->setData(file);
    action->setStatusTip(file);
    connect(action, &QAction::triggered, this, &TextEditor::openRecentFile);
    recentFilesMenu->addAction(action);
  }
  if (recentFiles.isEmpty()) {
    QAction *noFilesAction = new QAction("No recent files", this);
    noFilesAction->setEnabled(false);
    recentFilesMenu->addAction(noFilesAction);
  }
}

QTabWidget *TextEditor::currentTabWidget() const {
  if (activeTabWidget == tabWidget2 && tabWidget2)
    return tabWidget2;
  return tabWidget;
}

QList<CodeEditor *> TextEditor::allEditors() const {
  QList<CodeEditor *> editors;
  const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                               : QList<QTabWidget *>{tabWidget};
  for (QTabWidget *tw : panes) {
    for (int i = 0; i < tw->count(); ++i) {
      if (auto *editor = qobject_cast<CodeEditor *>(tw->widget(i)))
        editors.append(editor);
    }
  }
  return editors;
}

QString TextEditor::normalizedPath(const QString &path) const {
  QFileInfo info(path);
  const QString canonical = info.canonicalFilePath();
  return QDir::cleanPath(canonical.isEmpty() ? info.absoluteFilePath() : canonical);
}

QWidget *TextEditor::findOpenDocument(const QString &path, QTabWidget **pane) const {
  const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                               : QList<QTabWidget *>{tabWidget};
  for (QTabWidget *tw : panes) {
    for (int i = 0; i < tw->count(); ++i) {
      QWidget *document = tw->widget(i);
      QString documentPath;
      if (auto *editor = qobject_cast<CodeEditor *>(document))
        documentPath = editor->getFileName();
      else if (auto *hexEditor = qobject_cast<HexEditor *>(document))
        documentPath = hexEditor->property("fileName").toString();
      if (!documentPath.isEmpty() && normalizedPath(documentPath) == path) {
        if (pane)
          *pane = tw;
        return document;
      }
    }
  }
  return nullptr;
}

CodeEditor *TextEditor::currentEditor() {
  QTabWidget *tw = currentTabWidget();
  return qobject_cast<CodeEditor *>(tw ? tw->currentWidget() : nullptr);
}
SyntaxHighlighter *TextEditor::currentHighlighter() {
  CodeEditor *editor = currentEditor();
  return editor ? highlighters.value(editor) : nullptr;
}

void TextEditor::toggleSplitView() {
  editorPrefs.splitViewEnabled = !editorPrefs.splitViewEnabled;
  splitViewAct->setChecked(editorPrefs.splitViewEnabled);
  if (editorPrefs.splitViewEnabled) {
    if (!tabWidget2) {
      tabWidget2 = new DraggableTabWidget();
      tabWidget2->setTabsClosable(true);
      tabWidget2->setMovable(true);
      
      connect(tabWidget2, &QTabWidget::tabCloseRequested, this, [this](int index) {
          closeTab(index, tabWidget2);
      });
      connect(tabWidget2, &QTabWidget::currentChanged, this, &TextEditor::tabChanged);
      tabWidget2->installEventFilter(this);
      splitEmptyLabel = new QLabel("Drag a tab here", tabWidget2);
      splitEmptyLabel->setAlignment(Qt::AlignCenter);
      splitEmptyLabel->setStyleSheet("color:#777; font-size:13px; background:transparent;");
      splitEmptyLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
      
      mainSplitter->addWidget(tabWidget2);
    }
    tabWidget2->show();
    updateSplitEmptyState();
  } else {
    if (tabWidget2) {
      tabWidget2->hide();
      if (activeTabWidget == tabWidget2)
        activeTabWidget = tabWidget;
      if (splitEmptyLabel) splitEmptyLabel->hide();
    }
  }
}

void TextEditor::changeTheme() {
  editorPrefs.currentThemeIndex =
      (editorPrefs.currentThemeIndex + 1) % editorPrefs.themes.size();
  applyThemeToAllEditors();
  statusBar()->showMessage(
      QString("Theme: %1").arg(editorPrefs.themes[editorPrefs.currentThemeIndex].name), 2000);
}

void TextEditor::initializeThemes() {
  ColorTheme light;
  light.name = "Light";
  light.background = QColor(255, 255, 255);
  light.foreground = QColor(0, 0, 0);
  light.lineNumberBg = QColor(240, 240, 240);
  light.lineNumberFg = QColor(128, 128, 128);
  light.currentLine = QColor(255, 255, 200);
  light.selection = QColor(0, 120, 215);
  light.keyword = QColor(0, 0, 255);
  light.string = QColor(0, 128, 0);
  light.comment = QColor(128, 128, 128);
  light.number = QColor(128, 0, 128);
  light.function = QColor(255, 140, 0);
  editorPrefs.themes.append(light);

  ColorTheme dark;
  dark.name = "Dark";
  dark.background = QColor(30, 30, 30);
  dark.foreground = QColor(220, 220, 220);
  dark.lineNumberBg = QColor(40, 40, 40);
  dark.lineNumberFg = QColor(128, 128, 128);
  dark.currentLine = QColor(50, 50, 50);
  dark.selection = QColor(0, 120, 215);
  dark.keyword = QColor(86, 156, 214);
  dark.string = QColor(206, 145, 120);
  dark.comment = QColor(106, 153, 85);
  dark.number = QColor(181, 206, 168);
  dark.function = QColor(220, 220, 170);
  editorPrefs.themes.append(dark);

  ColorTheme monokai;
  monokai.name = "Monokai";
  monokai.background = QColor(39, 40, 34);
  monokai.foreground = QColor(248, 248, 242);
  monokai.lineNumberBg = QColor(49, 50, 44);
  monokai.lineNumberFg = QColor(144, 144, 138);
  monokai.currentLine = QColor(62, 63, 55);
  monokai.selection = QColor(73, 72, 62);
  monokai.keyword = QColor(249, 38, 114);
  monokai.string = QColor(230, 219, 116);
  monokai.comment = QColor(117, 113, 94);
  monokai.number = QColor(174, 129, 255);
  monokai.function = QColor(166, 226, 46);
  editorPrefs.themes.append(monokai);

  ColorTheme noir;
  noir.name = "Noir Edition";
  noir.background = QColor("#0a0a0a");
  noir.foreground = QColor("#c0c0c0");
  noir.selection = QColor("#333333");
  noir.lineNumberBg = QColor("#111111");
  noir.lineNumberFg = QColor("#555555");
  noir.currentLine = QColor("#1a1a1a");
  noir.keyword = QColor("#ff4444");
  noir.string = QColor("#999999");
  noir.comment = QColor("#666666");
  noir.number = QColor("#bbbbbb");
  noir.function = QColor("#e0e0e0");
  editorPrefs.themes.append(noir);

  editorPrefs.currentThemeIndex = 1; // Default to dark
}

void TextEditor::applyThemeToEditor(CodeEditor *editor,
                                    SyntaxHighlighter *highlighter) {
  if (!editor)
    return;
  const ColorTheme &theme = editorPrefs.themes[editorPrefs.currentThemeIndex];
  editor->applyTheme(theme);
  if (highlighter)
    highlighter->applyTheme(theme);
}

void TextEditor::applyThemeToAllEditors() {
  for (CodeEditor *editor : allEditors())
    applyThemeToEditor(editor, highlighters.value(editor));
  applyModernStyle();
}
