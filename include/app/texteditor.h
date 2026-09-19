#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include <QColor>
#include <QDateTime>
#include <QList>
#include <QMainWindow>
#include <QMap>
#include <QKeySequence>
#include <QStringList>
#include <QTextCursor>
#include <QVector>

#include "codeeditor.h"
#include "syntaxhighlighter.h"
#include "vulnscanner.h"
#include "solidityanalyzer.h"
#include "storageslotvisualizer.h"
#include "storyparser.h"
#include "storygraph.h"
#include "storyplaytest.h"
#include "storyexporter.h"

class QAction;
class AudioMonitor;
class AIAutocomplete;
class AnimationWidget;
class BinaryInspectorWidget;
class BreadcrumbBar;
class DisassemblerWidget;
class DJVisualizerWidget;
class DJVisualizerWindow;
class QFileSystemModel;
class QFileSystemWatcher;
class FindBar;
class GraveyardWidget;
class HUDWidget;
class KeyHeatmapOverlay;
class QLabel;
class MarkdownPreviewWidget;
class QModelIndex;
class QCloseEvent;
class QDockWidget;
class QEvent;
class QGraphicsColorizeEffect;
class QGraphicsOpacityEffect;
class QKeyEvent;
class QMenu;
class QMenuBar;
class QNetworkAccessManager;
class QSoundEffect;
class QSplitter;
class QStackedWidget;
class QTabWidget;
class QTimer;
class QTreeView;
class QVariantAnimation;
class SearchEverywhere;
class TerminalWidget;
class TitleBar;
class TodoPanel;
class WelcomeWidget;

// Language enum is defined in syntaxhighlighter.h (included above)
// ColorTheme struct is defined in syntaxhighlighter.h (included above)

class TextEditor : public QMainWindow {
    Q_OBJECT

public:
    TextEditor(QWidget *parent = nullptr);
    ~TextEditor();
    
    void openFilePath(const QString &filePath);
    void openFolderPath(const QString &folderPath);
    
    QAction *djModeAct = nullptr; // Make public for DJVisualizerWindow access
    void toggleDJMode(); // Make public for DJVisualizerWindow access

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void newFile();
    void openFile();
    void openRecentFile();
    void openFolder();
    bool saveFile();
    bool saveFileAs();
    bool maybeSave(int tabIndex, QTabWidget *targetWidget = nullptr);
    void closeTab(int index, QTabWidget *targetWidget = nullptr);
    void tabChanged(int index);
    void findText();
    void findNext();
    void findPrevious();
    void onFindTextChanged(const QString &text);
    void closeFindBar();
    void replaceText();
    void goToLine();
    void documentWasModified();
    void updateStatusBar();
    void increaseFontSize();
    void decreaseFontSize();
    void selectFont();
    void toggleWordWrap();
    void toggleSplitView();
    void toggleFileTree();
    void toggleMiniMap();
    void toggleTerminal();
    void cycleAnimation();
    void toggleAnimationDock();
    void toggleZenMode();
    void toggleTypingSound();
    void changeTheme();
    void customizeColors();
    void showAbout();
    void onFileTreeDoubleClicked(const QModelIndex &index);
    void onFileTreeContextMenu(const QPoint &pos);
    void setMarkdownPreviewVisible(bool visible);
    void updateMarkdownPreview();
    void onFileChangedExternally(const QString &path);
    void updateBreadcrumb();
    void showAISettings();

    // Narrative Engine
    void toggleStoryGraph();
    void toggleStoryPlaytest();
    void exportStory();
    void onStoryPassageClicked(const QString &passageName);
    void refreshStoryPanels();
    void toggleAIAutocomplete(bool enabled);
    void onAISuggestion(QPlainTextEdit *editor, int position, int revision,
                        const QString &suggestion);
    // Tools
    void openDisassembler();
    void openBinaryInspector();
    void openNeuralGraph();
    void openGhostReplay();
    void openScratchpad();
    void openCodebaseCityscape();
    
    // Security Pack
    void toggleParanoiaMode();
    void toggleVulnScan();

    // v0.8.0 QoL slots
    void switchHeaderSource();
    void locateCurrentFileInTree();
    void openSearchEverywhere();
    void toggleStickyScroll();
    void toggleInvisibleChars();
    void toggleGitBlame();
    void toggleAutoSaveOnFocusLost();
    void sendSelectionToScratchpad();

    // Web3Sec Pack (v0.8.1)
    void triggerGodView();
    void extractABI();
    void resolveFourByte();
    void triggerPanicButton();

    // v0.9.0 Web3Sec
    void showStorageSlotVisualizer();
    void toggleGasMinimap();
    void toggleSlitherOverlay();
    void showOnChainTracer();
    void showProxyDiff();

private:
    struct SearchState {
        QString lastText;
        QList<QTextCursor> matches;
        int currentIndex = -1;
        qint64 lastShiftPressMs = 0;
    };

    struct EditorPreferences {
        bool wordWrapEnabled = false;
        bool splitViewEnabled = false;
        bool paranoiaMode = false;
        int fontSize = 11;
        QString editorFontFamily = "Consolas";
        int currentThemeIndex = 0;
        QVector<ColorTheme> themes;
        bool stickyScrollEnabled = false;
        bool invisibleCharsEnabled = false;
        bool gitBlameEnabled = false;
        bool autoSaveFocusEnabled = false;
    };

    struct SessionMetrics {
        int keystrokes = 0;
        int linesWritten = 0;
        int filesOpened = 0;
        int peakWpm = 0;
        QVector<qint64> keystrokeTimestamps;
    };

    void createActions();
    void validateShortcuts();
    void createMenus();
    void createStatusBar();
    void clampToScreen();
    void readSettings();
    void writeSettings();
    void loadFile(const QString &fileName);
    bool saveFileToPath(const QString &fileName, QWidget *document = nullptr);
    void setCurrentFile(const QString &fileName, QWidget *document = nullptr);
    QString strippedName(const QString &fullFileName);
    void updateRecentFiles(const QString &fileName);
    void updateRecentFilesMenu();
    QTabWidget *currentTabWidget() const;
    QList<CodeEditor *> allEditors() const;
    CodeEditor* currentEditor();
    QString normalizedPath(const QString &path) const;
    QWidget *findOpenDocument(const QString &path, QTabWidget **pane = nullptr) const;
    SyntaxHighlighter* currentHighlighter();
    void initializeThemes();
    void applyThemeToEditor(CodeEditor *editor, SyntaxHighlighter *highlighter);
    void applyThemeToAllEditors();
    void setupUI();
    void applyModernStyle();
    void showWelcomeScreen();
    void hideWelcomeScreen();
    void watchFile(const QString &filePath);
    void unwatchFile(const QString &filePath);
    static Language detectLanguage(const QString &fileName);
    QString detectCurrentSymbol(CodeEditor *editor);
    void updateSearchHighlights();

    // Binary tools – open a file path in the respective viewer tab
    void openInDisassembler   (const QString &filePath);
    void openInBinaryInspector(const QString &filePath);

    // Animated panel helpers
    void animateTerminalShow();
    void animateTerminalHide();
    void flashTabLabel(int tabIndex, QTabWidget *targetWidget = nullptr);
    void flashStatusMessage(const QString &msg, const QColor &color = QColor("#4ec9b0"), int ms = 2500);

    // Markdown preview helpers
    void connectMarkdownPreview(CodeEditor *editor);
    void disconnectMarkdownPreview();

    // Core window chrome and main layout
    TitleBar *titleBar = nullptr;
    QMenuBar *customMenuBar = nullptr;
    QSplitter *mainSplitter = nullptr;
    QSplitter *verticalSplitter = nullptr;
    QTabWidget *tabWidget = nullptr;
    QTabWidget *tabWidget2 = nullptr;
    QTabWidget *activeTabWidget = nullptr;
    QLabel *splitEmptyLabel = nullptr;
    WelcomeWidget *welcomeWidget = nullptr;
    BreadcrumbBar *breadcrumbBar = nullptr;
    FindBar *findBar = nullptr;
    TerminalWidget *terminalWidget = nullptr;

    // Editor/project state
    QFileSystemWatcher *fileWatcher = nullptr;
    QFileSystemModel *fileSystemModel = nullptr;
    QTreeView *fileTree = nullptr;
    QString currentFolder;
    QStringList recentFiles;
    QMap<CodeEditor*, SyntaxHighlighter*> highlighters;
    QMap<QString, QAction*> shortcutRegistry;
    SearchState searchState;

    // Global editor preferences/state
    EditorPreferences editorPrefs;

    // Search/navigation and QoL state
    SearchEverywhere *searchEverywhere = nullptr;
    QGraphicsColorizeEffect *pane2DimEffect = nullptr;

    // Status/session state
    QLabel *statusLabel = nullptr;
    QLabel *languageLabel = nullptr;
    QLabel *paranoiaLabel = nullptr;
    QLabel *sessionTimeLabel = nullptr;
    QLabel *vimModeLabel = nullptr;
    QTimer *sessionTimer = nullptr;
    QDateTime sessionStart;
    int sessionSecondsAccumulated = 0;
    QString sessionDateString;
    bool zenModeActive = false;

    // Core side panels and docks
    QDockWidget *fileTreeDock = nullptr;
    QStackedWidget *fileTreeContainer = nullptr;
    QWidget *emptyTreeWidget = nullptr;
    AnimationWidget *animationWidget = nullptr;
    QDockWidget *animationDock = nullptr;
    DJVisualizerWidget *djVisualizerWidget = nullptr;
    QDockWidget *djVisualizerDock = nullptr;

    // Markdown preview
    MarkdownPreviewWidget *markdownPreview = nullptr;
    QTimer *markdownTimer = nullptr;
    CodeEditor *markdownEditor = nullptr;

    // Tooling/services
    AIAutocomplete *aiAutocomplete = nullptr;
    QNetworkAccessManager *networkManager = nullptr;
    AudioMonitor *audioMonitor = nullptr;
    QSoundEffect *typingSound = nullptr;
    bool typingSoundEnabled = false;

    // Window/panel animation state
    QVariantAnimation *terminalAnim = nullptr;
    int terminalTargetH = 250;
    QGraphicsOpacityEffect *welcomeOpacity = nullptr;

    // Narrative engine
    StoryGraph *storyGraph = nullptr;
    StoryPlaytest *storyPlaytest = nullptr;
    QDockWidget *storyGraphDock = nullptr;
    QDockWidget *storyPlaytestDock = nullptr;

    // Web3 / analysis panels
    StorageSlotVisualizerWidget *slotVisualizerWidget = nullptr;
    QDockWidget *slotVisualizerDock = nullptr;
    bool gasMiniMapEnabled = false;

    // Cyberpunk / ambient features
    GraveyardWidget *graveyardWidget = nullptr;
    QDockWidget *graveyardDock = nullptr;
    HUDWidget *hudWidget = nullptr;
    KeyHeatmapOverlay *keyHeatmap = nullptr;
    QTimer *ambientTimer = nullptr;

    // Scratchpad / command UI
    QPlainTextEdit *scratchpadEditor = nullptr;
    TodoPanel *todoPanel = nullptr;
    QDockWidget *todoDock = nullptr;

    // Menu objects
    QMenu *fileMenu = nullptr;
    QMenu *markdownMenu = nullptr;
    QMenu *editMenu = nullptr;
    QMenu *searchMenu = nullptr;
    QMenu *viewMenu = nullptr;
    QMenu *toolsMenu = nullptr;
    QMenu *pluginsMenu = nullptr;
    QMenu *helpMenu = nullptr;
    QMenu *recentFilesMenu = nullptr;

    // File actions
    QAction *newAct = nullptr;
    QAction *openAct = nullptr;
    QAction *openFolderAct = nullptr;
    QAction *saveAct = nullptr;
    QAction *saveAsAct = nullptr;
    QAction *closeAllAct = nullptr;
    QAction *closeOthersAct = nullptr;
    QAction *closeTabAct = nullptr;
    QAction *exitAct = nullptr;

    // Edit actions
    QAction *undoAct = nullptr;
    QAction *redoAct = nullptr;
    QAction *cutAct = nullptr;
    QAction *copyAct = nullptr;
    QAction *pasteAct = nullptr;
    QAction *selectAllAct = nullptr;
    QAction *findAct = nullptr;
    QAction *findNextAct = nullptr;
    QAction *replaceAct = nullptr;
    QAction *goToLineAct = nullptr;
    QAction *duplicateLineAct = nullptr;
    QAction *moveLineUpAct = nullptr;
    QAction *moveLineDownAct = nullptr;
    QAction *deleteLineAct = nullptr;
    QAction *toggleCommentAct = nullptr;
    QAction *smartHomeAct = nullptr;

    // View / appearance actions
    QAction *wordWrapAct = nullptr;
    QAction *increaseFontAct = nullptr;
    QAction *decreaseFontAct = nullptr;
    QAction *selectFontAct = nullptr;
    QAction *splitViewAct = nullptr;
    QAction *fileTreeAct = nullptr;
    QAction *miniMapAct = nullptr;
    QAction *terminalAct = nullptr;
    QAction *animationAct = nullptr;
    QAction *toggleAnimationDockAct = nullptr;
    QAction *themeAct = nullptr;
    QAction *customizeColorsAct = nullptr;
    QAction *markdownPreviewAct = nullptr;
    QAction *zenModeAct = nullptr;
    QAction *typingSoundAct = nullptr;
    QAction *graveyardAct = nullptr;
    QAction *crtAct = nullptr;
    QAction *keyHeatmapAct = nullptr;
    QAction *vimModeAct = nullptr;
    QAction *focusFadeAct = nullptr;
    QAction *imagePreviewAct = nullptr;
    QAction *stickyScrollAct = nullptr;
    QAction *invisibleCharsAct = nullptr;
    QAction *gitBlameAct = nullptr;
    QAction *autoSaveFocusAct = nullptr;

    // Tools / workflow actions
    QAction *aiSettingsAct = nullptr;
    QAction *aiToggleAct = nullptr;
    QAction *aboutAct = nullptr;
    QAction *disassembleAct = nullptr;
    QAction *binaryInspectAct = nullptr;
    QAction *openHexAct = nullptr;
    QAction *neuralGraphAct = nullptr;
    QAction *ghostReplayAct = nullptr;
    QAction *codebaseCityscapeAct = nullptr;
    QAction *scratchpadAct = nullptr;
    QAction *commandPaletteAct = nullptr;
    QAction *todoAct = nullptr;
    QAction *sessionStatsAct = nullptr;
    QAction *switchHeaderSourceAct = nullptr;
    QAction *locateInTreeAct = nullptr;
    QAction *searchEverywhereAct = nullptr;
    QAction *sendToScratchpadAct = nullptr;

    // Narrative actions
    QAction *storyGraphAct = nullptr;
    QAction *storyPlaytestAct = nullptr;
    QAction *storyExportAct = nullptr;

    // Security / Web3 actions
    QAction *paranoiaModeAct = nullptr;
    QAction *vulnScanAct = nullptr;
    QAction *godViewAct = nullptr;
    QAction *extractABIAct = nullptr;
    QAction *resolveFourByteAct = nullptr;
    QAction *panicButtonAct = nullptr;
    QAction *storageSlotVizAct = nullptr;
    QAction *gasMiniMapAct = nullptr;
    QAction *slitherOverlayAct = nullptr;
    QAction *onChainTracerAct = nullptr;
    QAction *proxyDiffAct = nullptr;
    QAction *memTraceAct = nullptr;

    // Session metrics
    SessionMetrics sessionMetrics;

    // Internal feature helpers
    void openCommandPalette();
    void toggleTodoPanel();
    void onTodoJump(const QString &filePath, int line);
    void trackKeystroke(int key, const QString &text);
    void toggleGraveyard();
    void toggleCRT();
    void onCodeBlockDeleted(const QString &code, const QString &source);
    void toggleKeyHeatmap();
    void toggleVimMode();
    void updateAmbientTheme();
    void toggleFocusFade();
    void toggleImagePreview();
    void showSessionStats();
    void applyPaneDimming();
    void updateContextualActions();
    void updateSplitEmptyState();
    void propagateV080Settings(CodeEditor *ed);
    void propagateV090Settings(CodeEditor *ed);
    void changeEvent(QEvent *e) override;
    bool eventFilter(QObject *obj, QEvent *event) override;
};

#endif
