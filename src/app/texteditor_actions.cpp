#include "texteditor_private.h"

void TextEditor::createActions() {
  newAct = new QAction("&New", this);
  newAct->setShortcuts(QKeySequence::New);
  connect(newAct, &QAction::triggered, this, &TextEditor::newFile);

  openAct = new QAction("&Open File...", this);
  openAct->setShortcuts(QKeySequence::Open);
  connect(openAct, &QAction::triggered, this, &TextEditor::openFile);

  openFolderAct = new QAction("Open &Folder...", this);
  openFolderAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
  connect(openFolderAct, &QAction::triggered, this, &TextEditor::openFolder);

  saveAct = new QAction("&Save", this);
  saveAct->setShortcuts(QKeySequence::Save);
  connect(saveAct, &QAction::triggered, this, &TextEditor::saveFile);

  saveAsAct = new QAction("Save &As...", this);
  saveAsAct->setShortcuts(QKeySequence::SaveAs);
  connect(saveAsAct, &QAction::triggered, this, &TextEditor::saveFileAs);

  closeTabAct = new QAction("&Close Tab", this);
  closeTabAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
  connect(closeTabAct, &QAction::triggered, this,
          [this]() { closeTab(currentTabWidget()->currentIndex(), currentTabWidget()); });

  exitAct = new QAction("E&xit", this);
  exitAct->setShortcuts(QKeySequence::Quit);
  connect(exitAct, &QAction::triggered, this, &QWidget::close);

  cutAct = new QAction("Cu&t", this);
  cutAct->setShortcuts(QKeySequence::Cut);
  connect(cutAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->cut();
  });

  copyAct = new QAction("&Copy", this);
  copyAct->setShortcuts(QKeySequence::Copy);
  connect(copyAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->copy();
  });

  pasteAct = new QAction("&Paste", this);
  pasteAct->setShortcuts(QKeySequence::Paste);
  connect(pasteAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->paste();
  });

  undoAct = new QAction("&Undo", this);
  undoAct->setShortcuts(QKeySequence::Undo);
  connect(undoAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->undo();
  });

  redoAct = new QAction("&Redo", this);
  redoAct->setShortcuts(QKeySequence::Redo);
  connect(redoAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->redo();
  });

  selectAllAct = new QAction("Select &All", this);
  selectAllAct->setShortcuts(QKeySequence::SelectAll);
  connect(selectAllAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->selectAll();
  });

  findAct = new QAction("&Find...", this);
  findAct->setShortcuts(QKeySequence::Find);
  connect(findAct, &QAction::triggered, this, &TextEditor::findText);

  findNextAct = new QAction("Find &Next", this);
  findNextAct->setShortcut(QKeySequence(Qt::Key_F3));
  connect(findNextAct, &QAction::triggered, this, &TextEditor::findNext);

  connect(findBar, &FindBar::textChanged, this, &TextEditor::onFindTextChanged);
  connect(findBar, &FindBar::optionsChanged, this, [this]() {
    onFindTextChanged(findBar->getSearchText());
  });
  connect(findBar, &FindBar::findNextRequested, this, &TextEditor::findNext);
  connect(findBar, &FindBar::findPreviousRequested, this, &TextEditor::findPrevious);
  connect(findBar, &FindBar::closeRequested, this, &TextEditor::closeFindBar);

  replaceAct = new QAction("&Replace...", this);
  replaceAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_H));
  connect(replaceAct, &QAction::triggered, this, &TextEditor::replaceText);

  goToLineAct = new QAction("&Go to Line...", this);
  goToLineAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_G));
  connect(goToLineAct, &QAction::triggered, this, &TextEditor::goToLine);

  // Line editing actions
  duplicateLineAct = new QAction("Duplicate Line", this);
  duplicateLineAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_D));
  connect(duplicateLineAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->duplicateLine();
  });

  QAction *selectNextAct = new QAction("Select Next Occurrence", this);
  selectNextAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_D));
  connect(selectNextAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->selectNextOccurrence();
  });

  moveLineUpAct = new QAction("Move Line Up", this);
  moveLineUpAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Up));
  connect(moveLineUpAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->moveLineUp();
  });

  moveLineDownAct = new QAction("Move Line Down", this);
  moveLineDownAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_Down));
  connect(moveLineDownAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->moveLineDown();
  });

  deleteLineAct = new QAction("Delete Line", this);
  deleteLineAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_K));
  connect(deleteLineAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->deleteLine();
  });

  toggleCommentAct = new QAction("Toggle Line Comment", this);
  toggleCommentAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash));
  connect(toggleCommentAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->toggleComment();
  });

  smartHomeAct = new QAction("Smart Home", this);
  // Home key handling is done in keyPressEvent directly so it auto-overrides
  // but we add it to the menu just in case.
  connect(smartHomeAct, &QAction::triggered, this, [this]() {
    if (currentEditor())
      currentEditor()->smartHome();
  });

  increaseFontAct = new QAction("Increase Font Size", this);
  increaseFontAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus));
  connect(increaseFontAct, &QAction::triggered, this,
          &TextEditor::increaseFontSize);

  decreaseFontAct = new QAction("Decrease Font Size", this);
  decreaseFontAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus));
  connect(decreaseFontAct, &QAction::triggered, this,
          &TextEditor::decreaseFontSize);

  selectFontAct = new QAction("Select Font...", this);
  connect(selectFontAct, &QAction::triggered, this, &TextEditor::selectFont);

  wordWrapAct = new QAction("Word Wrap", this);
  wordWrapAct->setCheckable(true);
  wordWrapAct->setChecked(editorPrefs.wordWrapEnabled);
  connect(wordWrapAct, &QAction::triggered, this, &TextEditor::toggleWordWrap);

  splitViewAct = new QAction("Split View", this);
  splitViewAct->setCheckable(true);
  splitViewAct->setChecked(editorPrefs.splitViewEnabled);
  splitViewAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Backslash));
  connect(splitViewAct, &QAction::triggered, this,
          &TextEditor::toggleSplitView);

  fileTreeAct = new QAction("Explorer", this);
  fileTreeAct->setCheckable(true);
  fileTreeAct->setChecked(true);
  fileTreeAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
  connect(fileTreeAct, &QAction::triggered, this, &TextEditor::toggleFileTree);

  miniMapAct = new QAction("Mini Map", this);
  miniMapAct->setCheckable(true);
  miniMapAct->setChecked(false);
  miniMapAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_M));
  connect(miniMapAct, &QAction::triggered, this, &TextEditor::toggleMiniMap);

  terminalAct = new QAction("Terminal", this);
  terminalAct->setCheckable(true);
  terminalAct->setChecked(false);
  terminalAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_QuoteLeft));
  connect(terminalAct, &QAction::triggered, this, &TextEditor::toggleTerminal);

  djModeAct = new QAction("DJ Mode", this);
  djModeAct->setIcon(QIcon::fromTheme("audio-card", QIcon::fromTheme("audio-volume-high")));
  djModeAct->setCheckable(true);
  djModeAct->setShortcut(QKeySequence("Ctrl+Shift+J"));
  connect(djModeAct, &QAction::triggered, this, &TextEditor::toggleDJMode);

  animationAct = new QAction("Cycle Animation", this);
  animationAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_A));
  connect(animationAct, &QAction::triggered, this, &TextEditor::cycleAnimation);

  toggleAnimationDockAct = new QAction("Toggle Animation Dock", this);
  toggleAnimationDockAct->setCheckable(true);
  toggleAnimationDockAct->setChecked(false);
  connect(toggleAnimationDockAct, &QAction::triggered, this, &TextEditor::toggleAnimationDock);

  themeAct = new QAction("Toggle Theme", this);
  themeAct->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
  connect(themeAct, &QAction::triggered, this, &TextEditor::changeTheme);

  zenModeAct = new QAction("\U0001F9D8 Zen Mode", this);
  zenModeAct->setShortcut(QKeySequence("Ctrl+Alt+Z"));
  zenModeAct->setCheckable(true);
  connect(zenModeAct, &QAction::triggered, this, &TextEditor::toggleZenMode);

  typingSoundAct = new QAction("Typing Sounds", this);
  typingSoundAct->setCheckable(true);
  typingSoundAct->setChecked(typingSoundEnabled);
  connect(typingSoundAct, &QAction::triggered, this, &TextEditor::toggleTypingSound);

  customizeColorsAct = new QAction("Customize Colors...", this);
  connect(customizeColorsAct, &QAction::triggered, this,
          &TextEditor::customizeColors);

  aboutAct = new QAction("&About", this);
  connect(aboutAct, &QAction::triggered, this, &TextEditor::showAbout);

  // ── Markdown preview action ───────────────────────────────────────────────
  markdownPreviewAct = new QAction("📄 Markdown Preview", this);
  markdownPreviewAct->setCheckable(true);
  markdownPreviewAct->setChecked(false);
  markdownPreviewAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_M));
  markdownPreviewAct->setStatusTip("Toggle split Markdown preview panel");
  connect(markdownPreviewAct, &QAction::toggled, this, &TextEditor::setMarkdownPreviewVisible);

  // ── Tools actions ────────────────────────────────────────────────────────
  disassembleAct = new QAction("&Disassemble File...", this);
  disassembleAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_D));
  disassembleAct->setStatusTip("Disassemble a binary using objdump");
  disassembleAct->setIcon(QIcon::fromTheme("applications-engineering",
                          QIcon::fromTheme("utilities-terminal")));
  connect(disassembleAct, &QAction::triggered, this, &TextEditor::openDisassembler);

  binaryInspectAct = new QAction("&Binary Inspector...", this);
  binaryInspectAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_I));
  binaryInspectAct->setStatusTip("Inspect ELF/PE headers, sections and imports");
  binaryInspectAct->setIcon(QIcon::fromTheme("system-search",
                             QIcon::fromTheme("document-properties")));
  connect(binaryInspectAct, &QAction::triggered, this, &TextEditor::openBinaryInspector);

  neuralGraphAct = new QAction("&Neural Code Graph...", this);
  neuralGraphAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_N));
  neuralGraphAct->setStatusTip("Visualize workspace includes as a 3D force-directed graph");
  neuralGraphAct->setIcon(QIcon::fromTheme("preferences-system-network",
                          QIcon::fromTheme("view-web-browser-dom")));
  connect(neuralGraphAct, &QAction::triggered, this, &TextEditor::openNeuralGraph);

  ghostReplayAct = new QAction("&Ghost Replay Mode", this);
  ghostReplayAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_G));
  ghostReplayAct->setStatusTip("Watch a replay of the current session's code edits");
  ghostReplayAct->setIcon(QIcon::fromTheme("media-playback-start"));
  connect(ghostReplayAct, &QAction::triggered, this, &TextEditor::openGhostReplay);

  // v1.7 Cyberpunk actions
  graveyardAct = new QAction("Deleted Code History", this);
  graveyardAct->setCheckable(true);
  graveyardAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_G));
  graveyardAct->setStatusTip("Toggle Code Graveyard panel (deleted code >3 lines)");
  connect(graveyardAct, &QAction::triggered, this, &TextEditor::toggleGraveyard);

  crtAct = new QAction("CRT &Post-Processing", this);
  crtAct->setCheckable(true);
  crtAct->setStatusTip("Toggle CRT scanlines, chromatic aberration, and phosphor bloom");
  connect(crtAct, &QAction::triggered, this, &TextEditor::toggleCRT);

  keyHeatmapAct = new QAction("Key &Heatmap", this);
  keyHeatmapAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_K));
  keyHeatmapAct->setStatusTip("Show keystroke frequency heatmap overlay (click to dismiss)");
  connect(keyHeatmapAct, &QAction::triggered, this, &TextEditor::toggleKeyHeatmap);

  vimModeAct = new QAction("&Vim Mode", this);
  vimModeAct->setCheckable(true);
  vimModeAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_V));
  vimModeAct->setStatusTip("Toggle Vim Normal/Insert modal editing");
  connect(vimModeAct, &QAction::triggered, this, &TextEditor::toggleVimMode);

  scratchpadAct = new QAction("&Scratchpad", this);
  scratchpadAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_S));
  scratchpadAct->setStatusTip("Open persistent scratchpad for mid-session notes");
  scratchpadAct->setIcon(QIcon::fromTheme("accessories-text-editor",
                         QIcon::fromTheme("document-new")));
  connect(scratchpadAct, &QAction::triggered, this, &TextEditor::openScratchpad);

  commandPaletteAct = new QAction("&Command Palette", this);
  commandPaletteAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
  commandPaletteAct->setStatusTip("Fuzzy-search all editor commands");
  connect(commandPaletteAct, &QAction::triggered, this, &TextEditor::openCommandPalette);

  todoAct = new QAction("TODO/FIXME &Panel", this);
  todoAct->setCheckable(true);
  todoAct->setStatusTip("Show panel listing all TODO, FIXME, HACK, NOTE, BUG tags in open files");
  connect(todoAct, &QAction::triggered, this, &TextEditor::toggleTodoPanel);

  // Security Pack
  paranoiaModeAct = new QAction("☣ Paranoia Mode", this);
  paranoiaModeAct->setCheckable(true);
  paranoiaModeAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_P));
  paranoiaModeAct->setStatusTip("Zero-trace mode: disables disk writes, history, and telemetry");
  connect(paranoiaModeAct, &QAction::triggered, this, &TextEditor::toggleParanoiaMode);

  vulnScanAct = new QAction("⚡ Vuln Scanner", this);
  vulnScanAct->setCheckable(true);
  vulnScanAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_X));
  vulnScanAct->setStatusTip("Enable native regex scanner for secrets and dangerous patterns");
  connect(vulnScanAct, &QAction::triggered, this, &TextEditor::toggleVulnScan);

  focusFadeAct = new QAction("Focus &Fade", this);
  focusFadeAct->setCheckable(true);
  focusFadeAct->setStatusTip("Dim all lines except the current line while editing");
  connect(focusFadeAct, &QAction::triggered, this, &TextEditor::toggleFocusFade);

  imagePreviewAct = new QAction("Image &Preview on Hover", this);
  imagePreviewAct->setCheckable(true);
  imagePreviewAct->setStatusTip("Show thumbnail tooltip when hovering over image paths in code");
  connect(imagePreviewAct, &QAction::triggered, this, &TextEditor::toggleImagePreview);

  sessionStatsAct = new QAction("Session &Statistics", this);
  sessionStatsAct->setStatusTip("View keystrokes, WPM, and activity stats for this session");
  connect(sessionStatsAct, &QAction::triggered, this, &TextEditor::showSessionStats);

  openHexAct = new QAction("Open in &Hex Editor", this);
  openHexAct->setIcon(QIcon::fromTheme("text-x-generic",
                      QIcon::fromTheme("document-open")));
  openHexAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_H));
  openHexAct->setStatusTip("Re-open the current file in the built-in hex editor");
  connect(openHexAct, &QAction::triggered, this, [this]() {
      CodeEditor *ed = currentEditor();
      if (ed && !ed->getFileName().isEmpty()) {
          QFile f(ed->getFileName());
          if (f.open(QIODevice::ReadOnly)) {
              QByteArray bytes = f.readAll();
              HexEditor *hex = new HexEditor();
              hex->setData(bytes);
              hex->setProperty("fileName", ed->getFileName());
              connect(hex, &HexEditor::modificationChanged,
                      this, &TextEditor::documentWasModified);
              QTabWidget *tw = currentTabWidget();
              int idx = tw->addTab(hex, "[HEX] " + strippedName(ed->getFileName()));
              tw->setCurrentIndex(idx);
              activeTabWidget = tw;
              flashTabLabel(idx, tw);
          }
      }
  });

  // ── v0.8.0 QoL Actions ─────────────────────────────────────────────
  switchHeaderSourceAct = new QAction("Switch Header/Source", this);
  switchHeaderSourceAct->setShortcut(QKeySequence(Qt::ALT | Qt::Key_O));
  switchHeaderSourceAct->setStatusTip("Toggle between .cpp and .h counterpart (Alt+O)");
  connect(switchHeaderSourceAct, &QAction::triggered, this, &TextEditor::switchHeaderSource);

  locateInTreeAct = new QAction("Locate File in Tree", this);
  locateInTreeAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_L));
  locateInTreeAct->setStatusTip("Reveal and highlight the current file in the file explorer");
  connect(locateInTreeAct, &QAction::triggered, this, &TextEditor::locateCurrentFileInTree);

  searchEverywhereAct = new QAction("Search Everywhere", this);
  searchEverywhereAct->setStatusTip("Search files, actions, and history (double-Shift)");
  connect(searchEverywhereAct, &QAction::triggered, this, &TextEditor::openSearchEverywhere);

  stickyScrollAct = new QAction("Sticky Scroll", this);
  stickyScrollAct->setCheckable(true);
  stickyScrollAct->setStatusTip("Pin the enclosing scope definition to the top of the viewport");
  connect(stickyScrollAct, &QAction::triggered, this, &TextEditor::toggleStickyScroll);

  invisibleCharsAct = new QAction("Show Invisible Characters", this);
  invisibleCharsAct->setCheckable(true);
  invisibleCharsAct->setStatusTip("Render trailing spaces and tabs as visible markers");
  connect(invisibleCharsAct, &QAction::triggered, this, &TextEditor::toggleInvisibleChars);

  gitBlameAct = new QAction("Git Blame Annotations", this);
  gitBlameAct->setCheckable(true);
  gitBlameAct->setStatusTip("Show inline git blame (author, time) at end of each line");
  connect(gitBlameAct, &QAction::triggered, this, &TextEditor::toggleGitBlame);

  autoSaveFocusAct = new QAction("Auto-Save on Focus Lost", this);
  autoSaveFocusAct->setCheckable(true);
  autoSaveFocusAct->setStatusTip("Automatically save file when editor loses window focus");
  connect(autoSaveFocusAct, &QAction::triggered, this, &TextEditor::toggleAutoSaveOnFocusLost);

  sendToScratchpadAct = new QAction("Send Selection to Scratchpad", this);
  sendToScratchpadAct->setStatusTip("Append selected code to Scratchpad with a timestamp");
  connect(sendToScratchpadAct, &QAction::triggered, this, &TextEditor::sendSelectionToScratchpad);

  // ── v0.9.0 Web3Sec Actions ──────────────────────────────────────────────
  godViewAct = new QAction("\U0001F441 God View Contract", this);
  godViewAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_G));
  godViewAct->setStatusTip("Collapse all function bodies to spec-mode (signatures + NatSpec only)");
  connect(godViewAct, &QAction::triggered, this, &TextEditor::triggerGodView);

  extractABIAct = new QAction("\U0001F4CB Extract ABI/Bytecode", this);
  extractABIAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_C));
  extractABIAct->setStatusTip("Run solc on current file and copy ABI to clipboard (Ctrl+Alt+C)");
  connect(extractABIAct, &QAction::triggered, this, &TextEditor::extractABI);

  resolveFourByteAct = new QAction("\U0001F50D Resolve 4-Byte Signature", this);
  resolveFourByteAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_F));
  resolveFourByteAct->setStatusTip("Look up selected 4-byte hex selector in 4byte.directory");
  connect(resolveFourByteAct, &QAction::triggered, this, &TextEditor::resolveFourByte);

  panicButtonAct = new QAction("\U0001F480 PANIC BUTTON", this);
  panicButtonAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::SHIFT | Qt::Key_Delete));
  panicButtonAct->setStatusTip("Best-effort local cleanup: close editor, clear scratchpad, wipe history");
  connect(panicButtonAct, &QAction::triggered, this, &TextEditor::triggerPanicButton);

  storageSlotVizAct = new QAction("\U0001F5C4 Storage Slot Visualizer", this);
  storageSlotVizAct->setStatusTip("Show Tetris-style 32-byte storage slot layout for .sol files");
  connect(storageSlotVizAct, &QAction::triggered, this, &TextEditor::showStorageSlotVisualizer);

  gasMiniMapAct = new QAction("\U0001F321 Gas Topographic Minimap", this);
  gasMiniMapAct->setCheckable(true);
  gasMiniMapAct->setStatusTip("Overlay gas consumption heatmap on minimap (red=expensive, blue=cheap)");
  connect(gasMiniMapAct, &QAction::triggered, this, &TextEditor::toggleGasMinimap);

  slitherOverlayAct = new QAction("\u2622 Slither/Foundry Overlays", this);
  slitherOverlayAct->setStatusTip("Run slither or forge test and overlay findings on editor");
  connect(slitherOverlayAct, &QAction::triggered, this, &TextEditor::toggleSlitherOverlay);

  onChainTracerAct = new QAction("\u26D3 On-Chain Trace Explorer", this);
  onChainTracerAct->setStatusTip("Paste a tx hash to replay execution trace against local .sol files");
  connect(onChainTracerAct, &QAction::triggered, this, &TextEditor::showOnChainTracer);

  proxyDiffAct = new QAction("\U0001F504 Proxy/Implementation Diff", this);
  proxyDiffAct->setStatusTip("Compare storage layouts of two .sol files for proxy collision detection");
  connect(proxyDiffAct, &QAction::triggered, this, &TextEditor::showProxyDiff);

  memTraceAct = new QAction("\U0001F9E0 Memory Pointer Tracer", this);
  memTraceAct->setCheckable(true);
  memTraceAct->setStatusTip("Enable: click on 0x40/memory offsets in Yul to highlight all mload/mstore ops");
  connect(memTraceAct, &QAction::triggered, this, [this](bool checked) {
      for (CodeEditor *ed : allEditors())
          ed->setMemTraceEnabled(checked);
  });
}

void TextEditor::createMenus() {
  fileMenu = customMenuBar->addMenu("&File");
  fileMenu->addAction(newAct);
  fileMenu->addAction(openAct);
  fileMenu->addAction(openFolderAct);
  recentFilesMenu = fileMenu->addMenu("Recent Files");
  fileMenu->addSeparator();
  fileMenu->addAction(saveAct);
  fileMenu->addAction(saveAsAct);
  fileMenu->addAction(closeTabAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  editMenu = customMenuBar->addMenu("&Edit");
  editMenu->addAction(undoAct);
  editMenu->addAction(redoAct);
  editMenu->addSeparator();
  editMenu->addAction(cutAct);
  editMenu->addAction(copyAct);
  editMenu->addAction(pasteAct);
  editMenu->addSeparator();
  editMenu->addAction(selectAllAct);

  searchMenu = customMenuBar->addMenu("&Search");
  searchMenu->addAction(findAct);
  searchMenu->addAction(findNextAct);
  searchMenu->addAction(replaceAct);
  searchMenu->addAction(goToLineAct);

  viewMenu = customMenuBar->addMenu("&View");
  viewMenu->addAction(fileTreeAct);
  viewMenu->addAction(miniMapAct);
  viewMenu->addAction(terminalAct);
  viewMenu->addAction(toggleAnimationDockAct);
  viewMenu->addAction(animationAct);
  viewMenu->addAction(splitViewAct);
  viewMenu->addSeparator();
  viewMenu->addAction(increaseFontAct);
  viewMenu->addAction(decreaseFontAct);
  viewMenu->addAction(selectFontAct);
  viewMenu->addSeparator();
  viewMenu->addAction(wordWrapAct);
  viewMenu->addSeparator();
  viewMenu->addAction(markdownPreviewAct);
  viewMenu->addAction(djModeAct);
  viewMenu->addSeparator();
  viewMenu->addAction(themeAct);
  viewMenu->addAction(customizeColorsAct);
  viewMenu->addSeparator();
  viewMenu->addAction(zenModeAct);
  viewMenu->addAction(typingSoundAct);
  viewMenu->addSeparator();
  viewMenu->addAction(crtAct);
  viewMenu->addAction(vimModeAct);
  viewMenu->addAction(keyHeatmapAct);
  viewMenu->addAction(graveyardAct);
  viewMenu->addSeparator();
  viewMenu->addAction(focusFadeAct);
  viewMenu->addAction(imagePreviewAct);
  viewMenu->addSeparator();
  viewMenu->addAction(stickyScrollAct);
  viewMenu->addAction(invisibleCharsAct);
  viewMenu->addAction(gitBlameAct);
  viewMenu->addAction(autoSaveFocusAct);

  toolsMenu = customMenuBar->addMenu("&Tools");
  toolsMenu->addAction(commandPaletteAct);
  toolsMenu->addAction(searchEverywhereAct);
  toolsMenu->addSeparator();
  toolsMenu->addAction(switchHeaderSourceAct);
  toolsMenu->addAction(locateInTreeAct);
  toolsMenu->addSeparator();
  toolsMenu->addAction(scratchpadAct);
  toolsMenu->addAction(sendToScratchpadAct);
  toolsMenu->addAction(sessionStatsAct);
  toolsMenu->addAction(todoAct);
  toolsMenu->addSeparator();
  toolsMenu->addAction(paranoiaModeAct);
  toolsMenu->addAction(vulnScanAct);
  toolsMenu->addSeparator();
  toolsMenu->addAction(openHexAct);
  toolsMenu->addAction(disassembleAct);
  toolsMenu->addAction(binaryInspectAct);
  toolsMenu->addAction(neuralGraphAct);
  toolsMenu->addAction(ghostReplayAct);
  toolsMenu->addSeparator();
  toolsMenu->setStyleSheet(
      "QMenu { background-color: #252526; color: #d4d4d4; border: 1px solid #3c3c3c; }"
      "QMenu::item:selected { background-color: #094771; }"
      "QMenu::separator { background: #3c3c3c; height: 1px; margin: 2px 8px; }");

  // ── v0.9.0 Web3Sec Menu ──────────────────────────────────────────────
  QMenu *web3Menu = customMenuBar->addMenu("&Web3Sec");
  web3Menu->addAction(godViewAct);
  web3Menu->addAction(extractABIAct);
  web3Menu->addAction(resolveFourByteAct);
  web3Menu->addSeparator();
  web3Menu->addAction(storageSlotVizAct);
  web3Menu->addAction(gasMiniMapAct);
  web3Menu->addAction(memTraceAct);
  web3Menu->addSeparator();
  web3Menu->addAction(slitherOverlayAct);
  web3Menu->addAction(onChainTracerAct);
  web3Menu->addAction(proxyDiffAct);
  web3Menu->addSeparator();
  web3Menu->addAction(panicButtonAct);
  web3Menu->setStyleSheet(
      "QMenu { background-color: #1a0a0a; color: #ff6666; border: 1px solid #ff3333; }"
      "QMenu::item:selected { background-color: #3a0000; }"
      "QMenu::separator { background: #ff3333; height: 1px; margin: 2px 8px; }");

  helpMenu = customMenuBar->addMenu("&Help");
  helpMenu->addAction(aboutAct);

  pluginsMenu = customMenuBar->addMenu("&Plugins");
  aiSettingsAct = new QAction("AI Autocomplete Settings...", this);
  connect(aiSettingsAct, &QAction::triggered, this, &TextEditor::showAISettings);
  pluginsMenu->addAction(aiSettingsAct);

  aiToggleAct = new QAction("Enable AI Autocomplete", this);
  aiToggleAct->setCheckable(true);
  connect(aiToggleAct, &QAction::toggled, this, &TextEditor::toggleAIAutocomplete);
  pluginsMenu->addAction(aiToggleAct);

  // ── Narrative Engine Menu ─────────────────────────────────────────────────
  QMenu *narrativeMenu = customMenuBar->addMenu("&Narrative");

  storyGraphAct = new QAction("✦ Story Graph", this);
  storyGraphAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_N));
  storyGraphAct->setCheckable(true);
  connect(storyGraphAct, &QAction::triggered, this, &TextEditor::toggleStoryGraph);
  narrativeMenu->addAction(storyGraphAct);

  storyPlaytestAct = new QAction("▶ Playtest Story", this);
  storyPlaytestAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_Y));
  storyPlaytestAct->setCheckable(true);
  connect(storyPlaytestAct, &QAction::triggered, this, &TextEditor::toggleStoryPlaytest);
  narrativeMenu->addAction(storyPlaytestAct);

  narrativeMenu->addSeparator();

  storyExportAct = new QAction("Export Story...", this);
  storyExportAct->setShortcut(QKeySequence(Qt::CTRL | Qt::ALT | Qt::Key_E));
  connect(storyExportAct, &QAction::triggered, this, &TextEditor::exportStory);
  narrativeMenu->addAction(storyExportAct);

  validateShortcuts();
  updateRecentFilesMenu();
}

void TextEditor::validateShortcuts() {
  shortcutRegistry.clear();
  for (QAction *action : findChildren<QAction *>()) {
    QList<QKeySequence> valid;
    for (const QKeySequence &shortcut : action->shortcuts()) {
      const QString key = shortcut.toString(QKeySequence::PortableText);
      if (key.isEmpty() || shortcutRegistry.contains(key)) {
        if (!key.isEmpty())
          qWarning() << "Shortcut collision:" << key << "for" << action->text()
                     << "already used by" << shortcutRegistry.value(key)->text();
        continue;
      }
      shortcutRegistry.insert(key, action);
      valid.append(shortcut);
    }
    action->setShortcuts(valid);
  }
}

void TextEditor::showAISettings() {
    AISettingsDialog dialog(this);
    QSettings settings("TextEditor", "Settings");
    dialog.setSettings(settings.value("ai/baseUrl").toString(),
                       settings.value("ai/apiKey").toString(),
                       settings.value("ai/model").toString(),
                       aiAutocomplete->isEnabled());
    
    if (dialog.exec() == QDialog::Accepted) {
        settings.setValue("ai/baseUrl", dialog.getBaseUrl());
        settings.setValue("ai/apiKey", dialog.getApiKey());
        settings.setValue("ai/model", dialog.getModel());
        
        aiAutocomplete->setProvider(dialog.getBaseUrl(), dialog.getApiKey(), dialog.getModel());
        toggleAIAutocomplete(dialog.isEnabled());
        aiToggleAct->setChecked(dialog.isEnabled());
    }
}

void TextEditor::toggleAIAutocomplete(bool enabled) {
    aiAutocomplete->setEnabled(enabled);
    if (!enabled)
        for (CodeEditor *editor : allEditors()) editor->clearGhostText();
}

void TextEditor::onAISuggestion(QPlainTextEdit *editor, int position, int revision,
                                const QString &suggestion) {
    auto *codeEditor = qobject_cast<CodeEditor *>(editor);
    if (!codeEditor || suggestion.isEmpty() || codeEditor != currentEditor()) return;
    if (codeEditor->textCursor().position() != position ||
        codeEditor->document()->revision() != revision) return;
    codeEditor->setGhostText(suggestion);
}

void TextEditor::clampToScreen() {
    QRect avail = QGuiApplication::primaryScreen()->availableGeometry();
    QRect cur   = geometry();
    // Clamp size first so the window actually fits
    int w = qMin(cur.width(),  avail.width());
    int h = qMin(cur.height(), avail.height());
    // Then clamp position so the bottom/right edges don't escape
    int x = qBound(avail.left(), cur.left(), avail.right()  - w);
    int y = qBound(avail.top(),  cur.top(),  avail.bottom() - h);
    if (w != cur.width() || h != cur.height() || x != cur.left() || y != cur.top())
        setGeometry(x, y, w, h);
}

void TextEditor::createStatusBar() {
  static const char *kPill =
      "background: #252526; color: #cccccc; padding: 1px 10px; "
      "border-radius: 8px; font-size: 11px; margin: 2px 2px;";

  languageLabel = new QLabel("Plain Text");
  languageLabel->setStyleSheet(
      "background: #005f99; color: #e8f4fd; padding: 1px 10px; "
      "border-radius: 8px; font-size: 11px; margin: 2px 2px;");
  statusBar()->addPermanentWidget(languageLabel);

  sessionTimeLabel = new QLabel("⏱ 0m", this);
  sessionTimeLabel->setStyleSheet(
      "background: #252526; color: #777777; padding: 1px 10px; "
      "border-radius: 8px; font-size: 11px; margin: 2px 2px;");
  statusBar()->addPermanentWidget(sessionTimeLabel);

  statusLabel = new QLabel("Ln 1, Col 1");
  statusLabel->setStyleSheet(kPill);
  statusBar()->addPermanentWidget(statusLabel);

  vimModeLabel = new QLabel("  NORMAL  ");
  vimModeLabel->setStyleSheet(
      "color: #282c34; background-color: #c678dd; padding: 1px 8px; "
      "font-family: Consolas; font-weight: bold; font-size: 10px; "
      "border-radius: 8px; margin: 2px 2px;");
  vimModeLabel->setVisible(false);
  statusBar()->addPermanentWidget(vimModeLabel);

  paranoiaLabel = new QLabel("  ☣ PARANOIA  ");
  paranoiaLabel->setStyleSheet(
      "color: #ffffff; background-color: #e81123; padding: 1px 8px; "
      "font-family: Consolas; font-weight: bold; font-size: 10px; "
      "border-radius: 8px; margin: 2px 2px;");
  paranoiaLabel->setVisible(false);
  statusBar()->addPermanentWidget(paranoiaLabel);

  statusBar()->showMessage("Ready");

  // Lift the bar with a subtle upward shadow
  auto *shadow = new QGraphicsDropShadowEffect(statusBar());
  shadow->setBlurRadius(10);
  shadow->setColor(QColor(0, 0, 0, 130));
  shadow->setOffset(0, -3);
  statusBar()->setGraphicsEffect(shadow);
}
