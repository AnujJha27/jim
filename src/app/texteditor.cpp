#include "texteditor_private.h"

Language TextEditor::detectLanguage(const QString &fileName) {
  QString ext = QFileInfo(fileName).suffix().toLower();
  if (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "c" || ext == "h" ||
      ext == "hpp" || ext == "hxx")
    return Language::CPP;
  if (ext == "py" || ext == "pyw" || ext == "pyi")
    return Language::Python;
  if (ext == "js" || ext == "jsx" || ext == "mjs" || ext == "ts" ||
      ext == "tsx")
    return Language::JavaScript;
  if (ext == "html" || ext == "htm" || ext == "xml" || ext == "svg")
    return Language::HTML;
  if (ext == "css" || ext == "scss" || ext == "sass" || ext == "less")
    return Language::CSS;
  if (ext == "rs")
    return Language::Rust;
  if (ext == "go")
    return Language::Go;
  if (ext == "json" || ext == "jsonc")
    return Language::JSON;
  if (ext == "yaml" || ext == "yml")
    return Language::YAML;
  if (ext == "md" || ext == "markdown" || ext == "mkd")
    return Language::Markdown;
  if (ext == "sol")
    return Language::Solidity;
  if (ext == "yul")
    return Language::Yul;
  if (ext == "story" || ext == "tw" || ext == "twee")
    return Language::Story;
  return Language::PlainText;
}

// DJVisualizerWidget implementation → animationwidget.cpp

// CodeEditor implementation → codeeditor.cpp

// App widget implementations live in dedicated src/app/* files

// AnimationWidget implementation → animationwidget.cpp

// GraveyardWidget / CRTOverlay / LaserParticleOverlay / HUDWidget implementation → overlays.cpp


// ============================================================
// TextEditor Implementation (Main Window)
// ============================================================
TextEditor::TextEditor(QWidget *parent)
    : QMainWindow(parent) {
  fileWatcher = new QFileSystemWatcher(this);
  connect(fileWatcher, &QFileSystemWatcher::fileChanged, this,
          &TextEditor::onFileChangedExternally);

  // Frameless window with custom title bar
  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint |
                 Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                 Qt::WindowCloseButtonHint);

  setupUI();
  activeTabWidget = tabWidget;
  networkManager = new QNetworkAccessManager(this);
  sessionStart = QDateTime::currentDateTime();
  sessionDateString = sessionStart.date().toString(Qt::ISODate);
  sessionTimer = new QTimer(this);
  sessionTimer->start(1000);
  initializeThemes();
  createActions();
  createMenus();
  createStatusBar();
  applyModernStyle();
  readSettings();
  setWindowTitle("Jim");
  resize(1200, 800);
  showWelcomeScreen();
  // Install event filter for double-Shift Search Everywhere detection
  qApp->installEventFilter(this);
}

TextEditor::~TextEditor() { writeSettings(); }

void TextEditor::setupUI() {
  // Main layout container (since QMainWindow needs a central widget)
  QWidget *mainContainer = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(mainContainer);
  mainLayout->setContentsMargins(1, 1, 1, 1); // 1px border for frameless
  mainLayout->setSpacing(0);

  titleBar = new TitleBar(this);
  mainLayout->addWidget(titleBar);

  // Explicitly place a custom menu bar inside our custom layout so it renders
  // below the title bar
  customMenuBar = new QMenuBar(mainContainer);
  customMenuBar->setStyleSheet(
      "QMenuBar { background-color: transparent; color: #cccccc; } "
      "QMenuBar::item:selected { background-color: #3e3e42; }");
  mainLayout->addWidget(customMenuBar);

  // Vertical splitter: top = editor area, bottom = terminal
  verticalSplitter = new QSplitter(Qt::Vertical, mainContainer);
  verticalSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  verticalSplitter->setOpaqueResize(false); // Show live preview while dragging

  // Container for breadcrumb + editor
  QWidget *editorContainer = new QWidget();
  editorContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  QVBoxLayout *editorLayout = new QVBoxLayout(editorContainer);
  editorLayout->setContentsMargins(0, 0, 0, 0);
  editorLayout->setSpacing(0);

  // Breadcrumb bar
  breadcrumbBar = new BreadcrumbBar();
  editorLayout->addWidget(breadcrumbBar);

  // Find bar (overlay/top of editor)
  findBar = new FindBar(this);
  editorLayout->addWidget(findBar);

  // Main horizontal splitter for editor tabs
  mainSplitter = new QSplitter(Qt::Horizontal);
  tabWidget = new DraggableTabWidget();
  tabWidget->setTabsClosable(true);
  tabWidget->setMovable(true);
  tabWidget->setDocumentMode(true);
  mainSplitter->addWidget(tabWidget);
  tabWidget2 = nullptr;

  connect(tabWidget, &QTabWidget::tabCloseRequested, this,
          [this](int index) { closeTab(index, tabWidget); });
  connect(tabWidget, &QTabWidget::currentChanged, this,
          &TextEditor::tabChanged);
  
  aiAutocomplete = new AIAutocomplete(this);
  connect(aiAutocomplete, &AIAutocomplete::suggestionReady, this, &TextEditor::onAISuggestion);
  
  editorLayout->addWidget(mainSplitter);
  verticalSplitter->addWidget(editorContainer);

  // Welcome widget (stacked on top of editor)
  welcomeWidget = new WelcomeWidget();
  connect(welcomeWidget, &WelcomeWidget::openFileRequested, this,
          &TextEditor::openFile);
  connect(welcomeWidget, &WelcomeWidget::openFolderRequested, this,
          &TextEditor::openFolder);
  connect(welcomeWidget, &WelcomeWidget::recentFileClicked, this,
          [this](const QString &path) { loadFile(path); });

  // Terminal widget (hidden by default)
  terminalWidget = new TerminalWidget();
  terminalWidget->setMinimumHeight(100);
  terminalWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  terminalWidget->hide();
  verticalSplitter->addWidget(terminalWidget);
  verticalSplitter->setStretchFactor(0, 3);
  verticalSplitter->setStretchFactor(1, 1);
  verticalSplitter->setHandleWidth(6);
  verticalSplitter->setChildrenCollapsible(false);
  
  // Set initial sizes for the splitter (70% editor, 30% terminal)
  QList<int> sizes;
  sizes << 700 << 300;
  verticalSplitter->setSizes(sizes);

  mainLayout->addWidget(verticalSplitter, 1); // stretch to fill
  
  setCentralWidget(mainContainer);

  // Animation widget as a dockable pane
  animationDock = new QDockWidget("Animation", this);
  animationDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  animationWidget = new AnimationWidget(animationDock);
  animationDock->setWidget(animationWidget);
  animationDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
  animationDock->setMinimumWidth(300); // Increased initial width
  addDockWidget(Qt::RightDockWidgetArea, animationDock);
  animationDock->hide();

  // File tree dock
  fileTreeDock = new QDockWidget("Explorer", this);
  fileTreeDock->setFeatures(QDockWidget::DockWidgetMovable |
                            QDockWidget::DockWidgetClosable);

  fileTreeContainer = new QStackedWidget();

  // 0: Empty state
  emptyTreeWidget = new QWidget();
  QVBoxLayout *emptyLayout = new QVBoxLayout(emptyTreeWidget);
  emptyLayout->setAlignment(Qt::AlignCenter);
  QLabel *emptyDesc = new QLabel("You have not yet opened a folder.");
  emptyDesc->setStyleSheet("color: #cccccc; font-size: 13px;");
  emptyDesc->setWordWrap(true);
  emptyDesc->setAlignment(Qt::AlignCenter);
  QPushButton *openFolderBtn = new QPushButton("Open Folder");
  openFolderBtn->setStyleSheet(
      "QPushButton { background-color: #0e639c; color: white; border: none; "
      "padding: 6px 12px; border-radius: 2px; } QPushButton:hover { "
      "background-color: #1177bb; }");
  connect(openFolderBtn, &QPushButton::clicked, this, &TextEditor::openFolder);
  emptyLayout->addWidget(emptyDesc);
  emptyLayout->addWidget(openFolderBtn);
  fileTreeContainer->addWidget(emptyTreeWidget);

  // 1: File tree state
  fileTree = new QTreeView();
  fileSystemModel = new QFileSystemModel();
  fileSystemModel->setFilter(QDir::AllDirs | QDir::Files |
                             QDir::NoDotAndDotDot);
  fileTree->setModel(fileSystemModel);
  fileTree->setColumnWidth(0, 250);
  fileTree->setHeaderHidden(false);
  fileTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
  fileTree->setAnimated(true);
  fileTree->setIndentation(20);
  fileTree->setSortingEnabled(true);
  for (int i = 1; i < fileSystemModel->columnCount(); ++i)
    fileTree->hideColumn(i);
  fileTree->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(fileTree, &QTreeView::doubleClicked, this,
          &TextEditor::onFileTreeDoubleClicked);
  connect(fileTree, &QTreeView::customContextMenuRequested, this,
          &TextEditor::onFileTreeContextMenu);
  fileTreeContainer->addWidget(fileTree);

  // Set default state to Empty (0)
  fileTreeContainer->setCurrentIndex(0);

  fileTreeDock->setWidget(fileTreeContainer);
  addDockWidget(Qt::LeftDockWidgetArea, fileTreeDock);

  // Code Graveyard dock
  graveyardDock = new QDockWidget("Deleted Code", this);
  graveyardDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  graveyardWidget = new GraveyardWidget(graveyardDock);
  graveyardDock->setWidget(graveyardWidget);
  graveyardDock->setMinimumWidth(240);
  graveyardDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
  addDockWidget(Qt::RightDockWidgetArea, graveyardDock);
  graveyardDock->hide();
  connect(graveyardWidget, &GraveyardWidget::resurrectRequested,
          this, [this](const QString &code) {
              CodeEditor *ed = currentEditor();
              if (ed) {
                  QTextCursor c = ed->textCursor();
                  c.insertText(code);
                  ed->setTextCursor(c);
                  flashStatusMessage("Pasted from deleted code history", QColor("#4ec9b0"), 2500);
              }
          });

  // ── Narrative Engine docks ────────────────────────────────────────────────
  storyGraph = new StoryGraph(this);
  storyGraphDock = new QDockWidget("Story Graph ✦", this);
  storyGraphDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
  storyGraphDock->setWidget(storyGraph);
  storyGraphDock->setMinimumWidth(320);
  storyGraphDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::RightDockWidgetArea, storyGraphDock);
  storyGraphDock->hide();
  connect(storyGraph, &StoryGraph::passageClicked, this, &TextEditor::onStoryPassageClicked);

  storyPlaytest = new StoryPlaytest(this);
  storyPlaytestDock = new QDockWidget("Playtest ▶", this);
  storyPlaytestDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
  storyPlaytestDock->setWidget(storyPlaytest);
  storyPlaytestDock->setMinimumWidth(360);
  storyPlaytestDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
  addDockWidget(Qt::RightDockWidgetArea, storyPlaytestDock);
  storyPlaytestDock->hide();
  connect(storyPlaytest, &StoryPlaytest::passageChanged, this, [this](const QString &name) {
      if (storyGraph) storyGraph->setCurrentPassage(name);
  });

  // Cybernetic HUD widget in status bar
  hudWidget = new HUDWidget(statusBar());
  statusBar()->addPermanentWidget(hudWidget);

  // Keystroke heatmap overlay (child of main window, centred on show)
  keyHeatmap = new KeyHeatmapOverlay(this);
  keyHeatmap->hide();

  // Ambient time-of-day theme tinting — fires immediately then every 10 min
  ambientTimer = new QTimer(this);
  ambientTimer->setInterval(10 * 60 * 1000);
  connect(ambientTimer, &QTimer::timeout, this, &TextEditor::updateAmbientTheme);
  ambientTimer->start();
  // Initial application after a short delay (editors not yet created)
  QTimer::singleShot(500, this, &TextEditor::updateAmbientTheme);
}

void TextEditor::showWelcomeScreen() {
  welcomeWidget->setRecentFiles(recentFiles);
  int idx = tabWidget->indexOf(welcomeWidget);
  if (idx == -1)
    idx = tabWidget->addTab(welcomeWidget, "Welcome");
  tabWidget->setCurrentIndex(idx);

  // Fade in
  if (!welcomeOpacity) {
    welcomeOpacity = new QGraphicsOpacityEffect(welcomeWidget);
    welcomeWidget->setGraphicsEffect(welcomeOpacity);
  }
  welcomeOpacity->setOpacity(0.0);
  auto *anim = new QPropertyAnimation(welcomeOpacity, "opacity", this);
  anim->setDuration(350);
  anim->setStartValue(0.0);
  anim->setEndValue(1.0);
  anim->setEasingCurve(QEasingCurve::OutCubic);
  anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void TextEditor::hideWelcomeScreen() {
  for (int i = 0; i < tabWidget->count(); ++i) {
    if (tabWidget->widget(i) == welcomeWidget) {
      // Quick fade out then remove
      if (!welcomeOpacity) {
        welcomeOpacity = new QGraphicsOpacityEffect(welcomeWidget);
        welcomeWidget->setGraphicsEffect(welcomeOpacity);
      }
      auto *anim = new QPropertyAnimation(welcomeOpacity, "opacity", this);
      anim->setDuration(180);
      anim->setStartValue(1.0);
      anim->setEndValue(0.0);
      anim->setEasingCurve(QEasingCurve::InCubic);
      int capturedIndex = i;
      connect(anim, &QPropertyAnimation::finished, this, [this, capturedIndex]() {
        // Re-check the index in case tabs changed during animation
        for (int j = 0; j < tabWidget->count(); ++j) {
          if (tabWidget->widget(j) == welcomeWidget) {
            tabWidget->removeTab(j);
            break;
          }
        }
        Q_UNUSED(capturedIndex)
      });
      anim->start(QAbstractAnimation::DeleteWhenStopped);
      return;
    }
  }
}

void TextEditor::watchFile(const QString &filePath) {
  if (editorPrefs.paranoiaMode) return;
  if (!filePath.isEmpty() && QFileInfo::exists(filePath))
    fileWatcher->addPath(filePath);
}

void TextEditor::unwatchFile(const QString &filePath) {
  if (!filePath.isEmpty() && fileWatcher->files().contains(filePath))
    fileWatcher->removePath(filePath);
}

void TextEditor::onFileChangedExternally(const QString &path) {
  // Find the editor with this file
  const QList<QTabWidget *> panes = tabWidget2 ? QList<QTabWidget *>{tabWidget, tabWidget2}
                                               : QList<QTabWidget *>{tabWidget};
  for (QTabWidget *tw : panes) {
    for (int i = 0; i < tw->count(); ++i) {
      CodeEditor *editor = qobject_cast<CodeEditor *>(tw->widget(i));
      if (!editor || normalizedPath(editor->getFileName()) != normalizedPath(path))
        continue;
      QMessageBox::StandardButton reply = QMessageBox::question(
          this, "File Changed",
          QString("The file '%1' has been modified externally.\nDo you want to "
                  "reload it?")
              .arg(QFileInfo(path).fileName()),
          QMessageBox::Yes | QMessageBox::No);
      if (reply == QMessageBox::Yes) {
        QFile file(path);
        if (file.open(QFile::ReadOnly | QFile::Text)) {
          int cursorPos = editor->textCursor().position();
          editor->setPlainText(QTextStream(&file).readAll());
          editor->document()->setModified(false);
          QTextCursor cursor = editor->textCursor();
          cursor.setPosition(
              qMin(cursorPos, editor->document()->characterCount() - 1));
          editor->setTextCursor(cursor);
        }
      }
      // Re-watch (Qt removes paths after change signal)
      watchFile(path);
      return;
    }
  }
}

QString TextEditor::detectCurrentSymbol(CodeEditor *editor) {
  if (!editor)
    return "";
  QTextBlock block = editor->textCursor().block();
  
  // For C++, walk up to find the closest scopes
  Language lang = editor->getLanguage();
  
  QRegularExpression funcRe;
  QRegularExpression classRe("(?:class|struct|enum|interface|trait|impl|namespace)\\s+([A-Za-z_][A-Za-z0-9_]*)");

  if (lang == Language::CPP) {
      // Improved C++ regex: captures return type (optional), scope (optional), and function name
      // Handles: void Class::Func(), int* ptr(), std::vector<int> some_func(), etc.
      // We make the trailing brace optional to handle "brace on next line"
      funcRe = QRegularExpression("(?xi)"
                                  "(?: [A-Za-z_][A-Za-z0-9_<>:\\*&\\s]* \\s+ )?" // Return type (optional)
                                  " ( [A-Za-z_][A-Za-z0-9_\\s]* :: )? "           // Class/Namespace scope (optional)
                                  " ( [A-Za-z_][A-Za-z0-9_]* ) "                  // Function name
                                  " \\s* \\( [^\\)]* \\) "                       // Parameters
                                  " \\s* (?: const )? \\s* (?: [{;]|$) ");        // End of signature (brace, semi, or EOL)
  } else if (lang == Language::Python) {
      funcRe = QRegularExpression("^\\s*def\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\(");
      classRe = QRegularExpression("^\\s*class\\s+([A-Za-z_][A-Za-z0-9_]*)");
  } else {
      funcRe = QRegularExpression("(?:fn|func|def|function)\\s+([A-Za-z_][A-Za-z0-9_]*)\\s*\\(");
  }

  // Keywords to exclude in C++
  static const QStringList cppKeywords = {"if", "while", "for", "switch", "catch", "else", "foreach"};

  while (block.isValid()) {
    QString text = block.text().trimmed();
    if (text.isEmpty()) {
        block = block.previous();
        continue;
    }

    QRegularExpressionMatch m;
    
    if (lang == Language::CPP) {
        m = funcRe.match(text);
        if (m.hasMatch()) {
            QString name = m.captured(2);
            // Verify it's not a control flow keyword
            if (!cppKeywords.contains(name)) {
                QString scope = m.captured(1);
                return (scope.isEmpty() ? "" : scope) + name + "()";
            }
        }
    } else {
        m = funcRe.match(text);
        if (m.hasMatch()) return m.captured(1) + "()";
    }
    
    m = classRe.match(text);
    if (m.hasMatch()) return m.captured(1);
    
    block = block.previous();
  }
  return "";
}

void TextEditor::updateBreadcrumb() {
  CodeEditor *editor = currentEditor();
  if (!editor) {
    breadcrumbBar->updatePath("", "");
    return;
  }
  QString symbol = detectCurrentSymbol(editor);
  breadcrumbBar->updatePath(editor->getFileName(), symbol);
}
