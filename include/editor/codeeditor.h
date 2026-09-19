#ifndef JIM_CODEEDITOR_H
#define JIM_CODEEDITOR_H

#include <QMap>
#include <QPlainTextEdit>
#include <QTextCursor>
#include <QVector>

#include "syntaxhighlighter.h"
#include "vulnscanner.h"

class CRTOverlay;
class FoldingArea;
class GlitchOverlay;
class LaserParticleOverlay;
class LineNumberArea;
class MiniMap;
class QKeyEvent;
class QMimeData;
class QMouseEvent;
class QPaintEvent;
class QProcess;
class QPropertyAnimation;
class QRect;
class QResizeEvent;
class QTimer;
class QWheelEvent;
class VimMode;

struct GhostEvent {
    long long timestampMs;
    int position;
    int charsRemoved;
    QString textAdded;
};

class CodeEditor : public QPlainTextEdit {
    Q_OBJECT

public:
    CodeEditor(QWidget *parent = nullptr);
    void enableSmoothScrolling(bool enable);

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    void setFileName(const QString &name) { fileName = name; }
    QString getFileName() const { return fileName; }
    bool isModified() const { return document()->isModified(); }
    void applyTheme(const ColorTheme &theme);
    void miniMapPaintEvent(QPaintEvent *event);
    int miniMapWidth() const { return 120; }
    MiniMap *getMiniMap() const { return miniMap; }

    void foldingAreaPaintEvent(QPaintEvent *event);
    int foldingAreaWidth() const { return 16; }
    void toggleFoldAt(int blockNumber);
    bool isFoldable(const QTextBlock &block) const;
    bool isFolded(const QTextBlock &block) const;
    int findMatchingBrace(const QTextBlock &block) const;
    int findIndentEnd(const QTextBlock &block) const;
    int indentLevel(const QTextBlock &block) const;

    void setLanguage(Language lang);
    Language getLanguage() const { return currentLanguage; }

    void setSearchSelections(const QList<QTextCursor> &selections);
    void setGhostText(const QString &text);
    void clearGhostText();
    void addExtraCursor(const QTextCursor &c);
    void clearExtraCursors();
    void selectNextOccurrence();

    QVector<GhostEvent> ghostLog;
    long long sessionStartTimeMs = 0;
    bool ghostIsRecording = false;
    void startRecordingGhost();
    void logGhostEvent(int pos, int charsRemoved, const QString &textAdded);

    void triggerLaserEffect();
    void triggerGlitchUndo();
    void setCRTEnabled(bool enabled);

    bool minimapWaterfallEnabled = true;
    void setAmbientBackground(QColor tint);

    void setVimEnabled(bool enabled);
    bool isVimEnabled() const;

    void setParanoiaMode(bool enabled);
    void setVulnScanEnabled(bool enabled);
    void setFocusFadeEnabled(bool enabled);
    void setImagePreviewEnabled(bool enabled);

    void setStickyScrollEnabled(bool enabled);
    void setInvisibleCharsEnabled(bool enabled);
    void setGitBlameEnabled(bool enabled);
    void fetchGitBlame();
    void setKineticScrollEnabled(bool enabled);
    void setAutoSaveOnFocusLost(bool enabled) { autoSaveOnFocusLost = enabled; }
    bool getAutoSaveOnFocusLost() const { return autoSaveOnFocusLost; }
    void insertFromMimeData(const QMimeData *source) override;

    void setGasMinimapEnabled(bool en);
    void setMemTraceEnabled(bool en);
    void highlightMemoryTraceLines(const QVector<int> &lines);
    QVector<VulnScanner::Finding> vulnFindings;

signals:
    void characterTyped();
    void codeBlockDeleted(const QString &code, const QString &source);
    void keyPressed(int key, const QString &text);
    void vimModeChanged(const QString &modeName);

public slots:
    void duplicateLine();
    void moveLineUp();
    void moveLineDown();
    void deleteLine();
    void toggleComment();
    void smartHome();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);
    void onDocumentContentsChange(int position, int charsRemoved, int charsAdded);
    void rebuildPaintCache();
    void runVulnScan();

    friend class FoldingArea;

private:
    LineNumberArea *lineNumberArea;
    FoldingArea *foldingArea;
    MiniMap *miniMap;
    CRTOverlay *crtOverlay;
    LaserParticleOverlay *laserOverlay;
    GlitchOverlay *glitchOverlay;
    QString fileName;
    ColorTheme currentTheme;
    bool smoothScrollEnabled;
    QPropertyAnimation *scrollAnimation;
    int targetScrollValue;
    Language currentLanguage;
    QList<QTextCursor> searchSelections;
    QList<QTextCursor> extraCursors;
    QVector<int> waterfallDrops;
    int waterfallFrame = 0;
    QMap<int, int> lineEditHeat;
    VimMode *vimMode = nullptr;
    bool paranoiaMode = false;
    bool vulnScanEnabled = false;
    VulnScanner *vulnScanner = nullptr;
    QTimer *vulnScanTimer = nullptr;
    bool focusFadeEnabled = false;
    bool imagePreviewEnabled = false;
    void autoIndent();
    void matchBrackets();

    bool stickyScrollEnabled = false;
    bool invisibleCharsEnabled = false;
    bool gitBlameEnabled = false;
    bool kineticScrollEnabled = false;
    bool autoSaveOnFocusLost = false;
    QMap<int, QString> blameCache;
    QProcess *blameProcess = nullptr;
    double kineticVelocity = 0.0;
    QTimer *kineticTimer = nullptr;

    bool gasMinimapEnabled;
    bool memTraceEnabled;
    QVector<int> memTraceHighlightLines;
    QString ghostText;
    struct ColorMark { int offset; int length; QColor color; };
    QMap<int, QVector<ColorMark>> colorMarks;
    QTimer *paintCacheTimer = nullptr;
};

#endif
