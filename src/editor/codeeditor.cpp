#include "codeeditor.h"
#include "folding_area.h"
#include "line_number_area.h"
#include "minimap.h"
#include "overlays.h"
#include "vimmode.h"
#include <QPainter>
#include <QPainterPath>
#include <QPropertyAnimation>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QTextBlock>
#include <QFileInfo>
#include <QTimer>
#include <QDateTime>
#include <QApplication>
#include <QClipboard>
#include <QDir>
#include <QRegularExpression>
#include <QToolTip>
#include <QUrl>
#include <QLocale>
#include <QMimeData>
#include <QProcess>
#include <QColorDialog>
#include <cmath>
#include <algorithm>

// ============================================================
// CodeEditor Implementation
// ============================================================
CodeEditor::CodeEditor(QWidget *parent)
    : QPlainTextEdit(parent), smoothScrollEnabled(true), targetScrollValue(0),
      currentLanguage(Language::PlainText) {
  setPlaceholderText("Start typing code...");
  lineNumberArea = new LineNumberArea(this);
  foldingArea = new FoldingArea(this);
  miniMap = new MiniMap(this);
  miniMap->hide();

  // Overlays (sized in resizeEvent)
  crtOverlay = new CRTOverlay(viewport());
  crtOverlay->hide();
  laserOverlay = new LaserParticleOverlay(viewport());
  laserOverlay->hide();
  
  glitchOverlay = new GlitchOverlay(viewport());
  glitchOverlay->hide();

  scrollAnimation = new QPropertyAnimation(verticalScrollBar(), "value", this);
  scrollAnimation->setDuration(40);
  scrollAnimation->setEasingCurve(QEasingCurve::OutQuad);

  connect(this, &CodeEditor::blockCountChanged, this,
          &CodeEditor::updateLineNumberAreaWidth);
  connect(this, &CodeEditor::updateRequest, this,
          &CodeEditor::updateLineNumberArea);
  connect(this, &CodeEditor::cursorPositionChanged, this,
          &CodeEditor::highlightCurrentLine);

  QTimer *minimapUpdateTimer = new QTimer(this);
  minimapUpdateTimer->setSingleShot(true);
  minimapUpdateTimer->setInterval(50);
  connect(minimapUpdateTimer, &QTimer::timeout, this, [this]() {
    if (miniMap->isVisible())
      miniMap->update();
  });
  connect(this, &CodeEditor::updateRequest, this,
          [minimapUpdateTimer]() { minimapUpdateTimer->start(); });

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
  setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);
  viewport()->setMouseTracking(true);

  // Vuln Scanner initialization
  vulnScanner = new VulnScanner(this);
  vulnScanTimer = new QTimer(this);
  vulnScanTimer->setSingleShot(true);
  vulnScanTimer->setInterval(300);
  connect(vulnScanTimer, &QTimer::timeout, this, &CodeEditor::runVulnScan);

  // Ghost replay & graveyard tracking
  connect(document(), &QTextDocument::contentsChange, this,
          &CodeEditor::onDocumentContentsChange);
  paintCacheTimer = new QTimer(this);
  paintCacheTimer->setSingleShot(true);
  paintCacheTimer->setInterval(100);
  connect(paintCacheTimer, &QTimer::timeout, this, &CodeEditor::rebuildPaintCache);
  rebuildPaintCache();
  startRecordingGhost();

  // Vim mode
  vimMode = new VimMode(this);
  connect(vimMode, &VimMode::modeChanged, this, &CodeEditor::vimModeChanged);
  connect(vimMode, &VimMode::deleteLineRequested, this, &CodeEditor::deleteLine);

  // Kinetic scroll timer
  kineticTimer = new QTimer(this);
  kineticTimer->setInterval(16); // ~60fps
  connect(kineticTimer, &QTimer::timeout, this, [this]() {
      if (qAbs(kineticVelocity) < 0.5) { kineticTimer->stop(); kineticVelocity = 0; return; }
      QScrollBar *sb = verticalScrollBar();
      sb->setValue(sb->value() + static_cast<int>(kineticVelocity));
      kineticVelocity *= 0.88; // friction
  });

  gasMinimapEnabled = false;
  memTraceEnabled = false;
}

void CodeEditor::setLanguage(Language lang) { currentLanguage = lang; }

void CodeEditor::applyTheme(const ColorTheme &theme) {
  currentTheme = theme;
  QPalette p = palette();
  p.setColor(QPalette::Base, theme.background);
  p.setColor(QPalette::Text, theme.foreground);
  setPalette(p);

  QString style = QString("QPlainTextEdit { background-color: %1; color: %2; "
                          "selection-background-color: %3; border: none; }")
                      .arg(theme.background.name())
                      .arg(theme.foreground.name())
                      .arg(theme.selection.name());
  setStyleSheet(style);
  highlightCurrentLine();
}

int CodeEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
  return space;
}

void CodeEditor::updateLineNumberAreaWidth(int) {
  int rightMargin = miniMap->isVisible() ? miniMapWidth() : 0;
  setViewportMargins(lineNumberAreaWidth() + foldingAreaWidth(), 0, rightMargin,
                     0);
}

void CodeEditor::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy) {
    lineNumberArea->scroll(0, dy);
    foldingArea->scroll(0, dy);
  } else {
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    foldingArea->update(0, rect.y(), foldingArea->width(), rect.height());
  }
  if (rect.contains(viewport()->rect()))
    updateLineNumberAreaWidth(0);
}

void CodeEditor::setSearchSelections(const QList<QTextCursor> &selections) {
  searchSelections = selections;
  highlightCurrentLine();
}

void CodeEditor::resizeEvent(QResizeEvent *e) {
  QPlainTextEdit::resizeEvent(e);
  QRect cr = contentsRect();
  int lnw = lineNumberAreaWidth();
  int fw = foldingAreaWidth();
  lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lnw, cr.height()));
  foldingArea->setGeometry(QRect(cr.left() + lnw, cr.top(), fw, cr.height()));
  // Keep overlays covering the full viewport
  crtOverlay->setGeometry(viewport()->rect());
  laserOverlay->setGeometry(viewport()->rect());
  glitchOverlay->setGeometry(viewport()->rect());
  crtOverlay->raise();
  laserOverlay->raise();
  glitchOverlay->raise();

  if (miniMap->isVisible()) {
    miniMap->setGeometry(QRect(cr.right() - miniMapWidth(), cr.top(),
                               miniMapWidth(), cr.height()));
    setViewportMargins(lnw + fw, 0, miniMapWidth(), 0);
  } else {
    setViewportMargins(lnw + fw, 0, 0, 0);
  }
}

void CodeEditor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extraSelections;

  // Search highlights
  for (const QTextCursor &cursor : searchSelections) {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(QColor(62, 62, 66)); // Subtle secondary highlight
    selection.cursor = cursor;
    extraSelections.append(selection);
  }

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;
    QColor lineColor = currentTheme.currentLine.isValid()
                           ? currentTheme.currentLine
                           : QColor(Qt::yellow).lighter(160);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();

    // If the current cursor is within a search match, highlight it more prominently
    for (int i = 0; i < searchSelections.size(); ++i) {
        if (searchSelections[i].selectionStart() == selection.cursor.selectionStart() &&
            searchSelections[i].selectionEnd() == selection.cursor.selectionEnd()) {
            selection.format.setBackground(QColor(163, 115, 20, 150)); // Golden highlight for current match
            break;
        }
    }

    extraSelections.append(selection);
  }
  setExtraSelections(extraSelections);
}

void CodeEditor::paintEvent(QPaintEvent *e) {
  QPlainTextEdit::paintEvent(e);

  QPainter painter(viewport());
  painter.setRenderHint(QPainter::Antialiasing);

  if (!ghostText.isEmpty() && textCursor().selectionStart() == textCursor().position()) {
    const QString firstLine = ghostText.section('\n', 0, 0);
    const QRect cursor = cursorRect(textCursor());
    painter.setPen(QColor(140, 140, 140, 170));
    painter.drawText(cursor.right() + 1, cursor.top(),
                    fontMetrics().horizontalAdvance(firstLine) + 4,
                    cursor.height(), Qt::AlignLeft | Qt::AlignVCenter, firstLine);
  }

  QTextBlock block = firstVisibleBlock();
  QPointF offset = contentOffset();
  while (block.isValid()) {
    QRectF blockRect = blockBoundingGeometry(block).translated(offset);
    if (blockRect.top() > e->rect().bottom()) break;

    if (block.isVisible() && blockRect.bottom() >= e->rect().top()) {
      for (const ColorMark &mark : colorMarks.value(block.blockNumber())) {
        QColor color = mark.color;
        if (color.isValid()) {
          int swatchSize = 10;
          QTextCursor endCursor(block);
          endCursor.setPosition(block.position() + mark.offset + mark.length);
          QRect endRect = cursorRect(endCursor);
          QRect square(endRect.right() + 4, endRect.top() + (endRect.height() - swatchSize)/2, swatchSize, swatchSize);

          painter.setPen(Qt::NoPen);
          painter.setBrush(color);
          painter.drawRoundedRect(square, 2, 2);
          painter.setPen(QColor(100, 100, 100, 150));
          painter.drawRoundedRect(square, 2, 2);
        }
      }
    }
    block = block.next();
  }

  // ── Laser Vuln Underlines ─────────────────────────────────────────────────
  if (vulnScanEnabled && !vulnFindings.isEmpty()) {
      QTextBlock block = firstVisibleBlock();
      QPointF offset = contentOffset();
      
      painter.save();
      while (block.isValid()) {
          QRectF blockRect = blockBoundingGeometry(block).translated(offset);
          if (blockRect.top() > e->rect().bottom()) break;
          
          if (block.isVisible() && blockRect.bottom() >= e->rect().top()) {
              int bNum = block.blockNumber();
              for (const auto &finding : vulnFindings) {
                  if (finding.line == bNum) {
                      QTextCursor curStart(block);
                      curStart.setPosition(block.position() + finding.colStart);
                      QTextCursor curEnd(block);
                      curEnd.setPosition(block.position() + finding.colEnd);
                      
                      QRect rStart = cursorRect(curStart);
                      QRect rEnd = cursorRect(curEnd);
                      
                      int xStart = rStart.left();
                      int xEnd = rEnd.left() > xStart ? rEnd.left() : rStart.right() + 8; // ensure some width
                      int yLine = rStart.bottom() - 1;
                      
                      // Draw laser glow (semi-transparent thicker line)
                      painter.setPen(QPen(QColor(255, 40, 40, 80), 4));
                      painter.drawLine(xStart, yLine, xEnd, yLine);
                      
                      // Draw laser core (solid thinner line)
                      painter.setPen(QPen(QColor(255, 40, 40), 2));
                      painter.drawLine(xStart, yLine, xEnd, yLine);
                  }
              }
          }
          block = block.next();
      }
      painter.restore();
  }

  // ── Bracket Pair Colorization ─────────────────────────────────────────────
  {
    static const QColor bColors[3] = {
        QColor(229, 192, 123),  // warm gold   (depth 0)
        QColor(198, 120, 221),  // muted purple (depth 1)
        QColor( 97, 175, 239),  // soft blue    (depth 2)
    };
    const QString opens  = "({[";
    const QString closes = ")}]";

    // Scan backward up to 150 blocks to determine starting bracket depth
    int depth = 0;
    {
        QTextBlock b = firstVisibleBlock().previous();
        int scanned = 0;
        while (b.isValid() && scanned < 150) {
            const QString &t = b.text();
            for (int k = t.size() - 1; k >= 0; --k) {
                if (opens.contains(t[k]))  --depth;
                if (closes.contains(t[k])) ++depth;
            }
            b = b.previous();
            ++scanned;
        }
        if (depth < 0) depth = 0;
    }

    painter.setFont(font());
    QFontMetrics fm = fontMetrics();
    QTextBlock blk = firstVisibleBlock();
    QPointF off = contentOffset();
    while (blk.isValid()) {
        QRectF br = blockBoundingGeometry(blk).translated(off);
        if (br.top() > e->rect().bottom()) break;
        if (blk.isVisible() && br.bottom() >= e->rect().top()) {
            const QString &txt = blk.text();
            for (int k = 0; k < txt.size(); ++k) {
                QChar ch = txt[k];
                bool isOpen  = opens.contains(ch);
                bool isClose = closes.contains(ch);
                if (!isOpen && !isClose) continue;

                if (isClose && depth > 0) --depth;
                QColor col = bColors[depth % 3];
                if (isOpen) ++depth;

                QTextCursor tc(blk);
                tc.setPosition(blk.position() + k);
                QRect cr = cursorRect(tc);
                int cw = fm.horizontalAdvance(ch);
                painter.setPen(col);
                painter.drawText(cr.left(), cr.top(), cw, cr.height(),
                                 Qt::AlignLeft | Qt::AlignVCenter, QString(ch));
            }
        }
        blk = blk.next();
    }
  }

  if (!extraCursors.isEmpty()) {
      QColor cColor = currentTheme.foreground.isValid() ? currentTheme.foreground : Qt::black;
      painter.setPen(cColor);
      for (const QTextCursor &c : extraCursors) {
          QRect rect = cursorRect(c);
          painter.drawLine(rect.topLeft(), rect.bottomLeft());
          painter.drawLine(rect.topLeft() + QPoint(1,0), rect.bottomLeft() + QPoint(1,0));
      }
  }

  // Focus Fade: dim all lines except the current cursor block
  if (focusFadeEnabled) {
      int cursorBlockNum = textCursor().blockNumber();
      QTextBlock blk = firstVisibleBlock();
      QPointF off = contentOffset();
      painter.save();
      while (blk.isValid()) {
          QRectF br = blockBoundingGeometry(blk).translated(off);
          if (br.top() > viewport()->height()) break;
          if (blk.blockNumber() != cursorBlockNum && blk.isVisible()) {
              painter.fillRect(br, QColor(0, 0, 0, 110));
          }
          blk = blk.next();
      }
      painter.restore();
  }

  // ── Memory Pointer Trace Highlights ─────────────────────────────────────
  if (memTraceEnabled && !memTraceHighlightLines.isEmpty()) {
      painter.save();
      QTextBlock blk = firstVisibleBlock();
      QPointF off = contentOffset();
      while (blk.isValid()) {
          QRectF br = blockBoundingGeometry(blk).translated(off);
          if (br.top() > e->rect().bottom()) break;
          if (blk.isVisible() && memTraceHighlightLines.contains(blk.blockNumber())) {
              painter.fillRect(br, QColor(0, 200, 255, 30));
              painter.setPen(QPen(QColor(0, 200, 255, 120), 1, Qt::DashLine));
              painter.drawRect(br.adjusted(0,0,-1,-1));
          }
          blk = blk.next();
      }
      painter.restore();
  }

  // ── Sticky Scroll: draw scope context header at top ────────────────────
  if (stickyScrollEnabled) {
      static QRegularExpression scopeRe(
          R"(^\s*(class|struct|namespace|void|int|float|double|bool|auto|QString|QWidget|public:|private:|protected:|function|def|async|if|for|while|switch)\b.*[:{]\s*$)");
      QTextBlock top = firstVisibleBlock();
      // Walk backwards to find the enclosing scope line
      QTextBlock scope;
      QTextBlock b = top.previous();
      for (int n = 0; n < 100 && b.isValid(); ++n, b = b.previous()) {
          if (scopeRe.match(b.text()).hasMatch()) { scope = b; break; }
      }
      if (scope.isValid() && scope.blockNumber() != top.blockNumber()) {
          QString header = scope.text().trimmed();
          if (header.length() > 80) header = header.left(77) + "...";
          QFont hf = font();
          hf.setBold(false);
          painter.save();
          painter.setFont(hf);
          QFontMetrics hfm(hf);
          int headerH = hfm.height() + 4;
          QRect bgRect(0, 0, viewport()->width(), headerH);
          // Background pill
          QColor bgCol = currentTheme.background.isValid()
              ? currentTheme.background.darker(130) : QColor(25, 25, 35);
          bgCol.setAlpha(230);
          painter.fillRect(bgRect, bgCol);
          // Left accent line
          painter.fillRect(0, 0, 3, headerH, QColor(86, 156, 214));
          // Text
          painter.setPen(QColor(180, 200, 220, 220));
          painter.drawText(8, 0, viewport()->width() - 12, headerH,
                           Qt::AlignVCenter | Qt::AlignLeft, header);
          painter.restore();
      }
  }

  // ── Invisible Character Rendering ────────────────────────────────
  if (invisibleCharsEnabled) {
      painter.save();
      QFont dotFont = font();
      painter.setFont(dotFont);
      QFontMetrics ifm(dotFont);
      int charW = ifm.horizontalAdvance(' ');
      QTextBlock blk = firstVisibleBlock();
      QPointF off = contentOffset();
      while (blk.isValid()) {
          QRectF br = blockBoundingGeometry(blk).translated(off);
          if (br.top() > e->rect().bottom()) break;
          if (blk.isVisible() && br.bottom() >= e->rect().top()) {
              const QString &txt = blk.text();
              // Trailing spaces
              int trailStart = txt.length();
              while (trailStart > 0 && txt[trailStart-1] == ' ') --trailStart;
              for (int k = trailStart; k < txt.length(); ++k) {
                  QTextCursor tc(blk);
                  tc.setPosition(blk.position() + k);
                  QRect cr = cursorRect(tc);
                  painter.setPen(QColor(220, 80, 80, 160));
                  int cx = cr.left() + charW/2;
                  int cy = static_cast<int>(br.center().y());
                  painter.drawEllipse(QPoint(cx, cy), 2, 2);
              }
              // Tabs
              for (int k = 0; k < txt.length(); ++k) {
                  if (txt[k] != '\t') continue;
                  QTextCursor tc(blk);
                  tc.setPosition(blk.position() + k);
                  QRect cr = cursorRect(tc);
                  painter.setPen(QColor(100, 160, 220, 140));
                  int y = static_cast<int>(br.center().y());
                  painter.drawLine(cr.left()+2, y, cr.left()+8, y);
                  painter.drawLine(cr.left()+6, y-2, cr.left()+8, y);
                  painter.drawLine(cr.left()+6, y+2, cr.left()+8, y);
              }
          }
          blk = blk.next();
      }
      painter.restore();
  }

  // ── Git Blame Annotations ───────────────────────────────────────
  if (gitBlameEnabled && !blameCache.isEmpty()) {
      painter.save();
      QFont bf = font();
      bf.setPointSizeF(bf.pointSizeF() * 0.78);
      bf.setItalic(true);
      painter.setFont(bf);
      QFontMetrics bfm(bf);
      QTextBlock blk = firstVisibleBlock();
      QPointF off = contentOffset();
      while (blk.isValid()) {
          QRectF br = blockBoundingGeometry(blk).translated(off);
          if (br.top() > e->rect().bottom()) break;
          if (blk.isVisible() && br.bottom() >= e->rect().top()) {
              int lineNum = blk.blockNumber();
              if (blameCache.contains(lineNum)) {
                  QString ann = blameCache[lineNum];
                  // Draw right-aligned at the end of the viewport
                  int x = viewport()->width() - bfm.horizontalAdvance(ann) - 8;
                  int y = static_cast<int>(br.top());
                  painter.setPen(QColor(130, 130, 160, 140));
                  painter.drawText(x, y, bfm.horizontalAdvance(ann), static_cast<int>(br.height()),
                                   Qt::AlignVCenter, ann);
              }
          }
          blk = blk.next();
      }
      painter.restore();
  }
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea);
  painter.setRenderHint(QPainter::TextAntialiasing, false);
  QColor bgColor = currentTheme.lineNumberBg.isValid()
                       ? currentTheme.lineNumberBg
                       : QColor(240, 240, 240);
  QColor fgColor = currentTheme.lineNumberFg.isValid()
                       ? currentTheme.lineNumberFg
                       : Qt::gray;
  painter.fillRect(event->rect(), bgColor);
  painter.setPen(fgColor);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
      qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());
  int lineHeight = fontMetrics().height();
  int width = lineNumberArea->width() - 5;
  int currentLine = textCursor().blockNumber();

  // Pre-compute max heat for normalisation
  int maxHeat = 1;
  for (auto v : lineEditHeat) maxHeat = qMax(maxHeat, v);
  int stripX = lineNumberArea->width() - 3;

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      if (blockNumber == currentLine)
        painter.setPen(QColor(255, 255, 255));
      else
        painter.setPen(fgColor);
      painter.drawText(0, top, width, lineHeight, Qt::AlignRight,
                       QString::number(blockNumber + 1));

      // Edit heatmap strip (3 px, right edge of gutter)
      int heat = lineEditHeat.value(blockNumber, 0);
      if (heat > 0) {
          float ratio = qMin(1.0f, static_cast<float>(heat) / maxHeat);
          int r = static_cast<int>(30  + ratio * 225);
          int g = static_cast<int>(120 - ratio * 80);
          int b = static_cast<int>(200 - ratio * 180);
          painter.fillRect(stripX, top, 3, bottom - top, QColor(r, g, b, 180));
      }
    }
    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }

  // ── Reentrancy Bloodline ─────────────────────────────────────────────────
  if (vulnScanEnabled && !vulnFindings.isEmpty()) {
      painter.save();
      QVector<QPair<int,int>> reentrancyPairs;
      for (const auto &f : vulnFindings) {
          if (f.category == "REENTRANCY") {
              reentrancyPairs.append({f.line, f.stateLine});
          }
      }
      for (auto &pair : reentrancyPairs) {
          int callLine = pair.first;
          int stateLine = pair.second;
          if (stateLine < 0) continue;
          QTextBlock callBlock = document()->findBlockByNumber(callLine);
          QTextBlock stateBlock = document()->findBlockByNumber(stateLine);
          if (!callBlock.isValid() || !stateBlock.isValid()) continue;
          int callTop = qRound(blockBoundingGeometry(callBlock).translated(contentOffset()).top());
          int stateTop = qRound(blockBoundingGeometry(stateBlock).translated(contentOffset()).top());
          int lh = fontMetrics().height();
          int callY = callTop + lh/2;
          int stateY = stateTop + lh/2;
          int x = lineNumberArea->width() / 2;
          QLinearGradient grad(x, callY, x, stateY);
          grad.setColorAt(0.0, QColor(255, 30, 30, 200));
          grad.setColorAt(0.5, QColor(255, 0, 0, 255));
          grad.setColorAt(1.0, QColor(200, 0, 50, 200));
          painter.setPen(QPen(QBrush(grad), 3));
          painter.drawLine(x, callY, x, stateY);
          painter.setBrush(QColor(255, 30, 30, 220));
          painter.setPen(Qt::NoPen);
          painter.drawEllipse(QPoint(x, callY), 4, 4);
          painter.drawEllipse(QPoint(x, stateY), 4, 4);
      }
      painter.restore();
  }
}

// ============================================================
// v1.9 Feature Implementations
// ============================================================
void CodeEditor::setFocusFadeEnabled(bool enabled) {
    focusFadeEnabled = enabled;
    viewport()->update();
}

void CodeEditor::setImagePreviewEnabled(bool enabled) {
    imagePreviewEnabled = enabled;
    if (!enabled)
        QToolTip::hideText();
}

void CodeEditor::setParanoiaMode(bool enabled) {
    paranoiaMode = enabled;
}

void CodeEditor::setVulnScanEnabled(bool enabled) {
    vulnScanEnabled = enabled;
    if (enabled) {
        connect(document(), &QTextDocument::contentsChanged, vulnScanTimer, qOverload<>(&QTimer::start), Qt::UniqueConnection);
        vulnScanTimer->start();
    } else {
        disconnect(document(), &QTextDocument::contentsChanged, vulnScanTimer, qOverload<>(&QTimer::start));
        vulnScanTimer->stop();
        vulnFindings.clear();
        viewport()->update();
    }
}

void CodeEditor::runVulnScan() {
    if (!vulnScanEnabled) return;
    if (fileName.endsWith(".sol", Qt::CaseInsensitive))
        vulnFindings = vulnScanner->scanSolidity(toPlainText(), fileName);
    else
        vulnFindings = vulnScanner->scan(toPlainText(), currentLanguage, fileName);
    viewport()->update();
}

void CodeEditor::mouseMoveEvent(QMouseEvent *event) {
    bool tooltipShown = false;
    
    // ── Vuln Scanner Hover HUD ──────────────────────────────────────────────
    if (vulnScanEnabled && !vulnFindings.isEmpty()) {
        QTextCursor cur = cursorForPosition(event->pos());
        int line = cur.blockNumber();
        int col = cur.positionInBlock();
        
        for (const auto &finding : vulnFindings) {
            if (finding.line == line && col >= finding.colStart && col <= finding.colEnd) {
                QString titleColor = "#ff2828";
                if (finding.category == "SECRET") titleColor = "#ffb86c";
                else if (finding.category == "SOLIDITY") titleColor = "#bd93f9";
                
                QString tip = QString(
                    "<div style='background-color:#0d0d0d; border:1px solid %1; padding:6px; border-radius:4px; font-family:Consolas,monospace;'>"
                    "<div style='color:%1; font-weight:bold; font-size:12px; margin-bottom:4px;'>[%2]</div>"
                    "<div style='color:#cdd6f4; font-size:11px;'>%3</div>"
                    "</div>"
                ).arg(titleColor, finding.category, finding.description);
                
                QToolTip::showText(event->globalPosition().toPoint(), tip, this);
                tooltipShown = true;
                break;
            }
        }
    }
    
    // ── Image Preview Hover ─────────────────────────────────────────────────
    if (!tooltipShown && imagePreviewEnabled) {
        QTextCursor cur = cursorForPosition(event->pos());
        QString line = cur.block().text();
        int col = cur.positionInBlock();

        // Match string literals containing image file extensions
        static QRegularExpression imgRe(
            R"([\"'`]([^\"'`\n]*\.(png|jpg|jpeg|gif|bmp|svg|webp|ico))[\"'`]?)",
            QRegularExpression::CaseInsensitiveOption);

        QRegularExpressionMatchIterator it = imgRe.globalMatch(line);
        bool shown = false;
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            if (col >= m.capturedStart() && col <= m.capturedEnd()) {
                QString imgPath = m.captured(1);
                // Resolve relative to current file's directory
                if (!QFileInfo(imgPath).isAbsolute() && !fileName.isEmpty())
                    imgPath = QFileInfo(fileName).absoluteDir().filePath(imgPath);
                if (QFileInfo::exists(imgPath)) {
                    // Build a rich-text tooltip with a thumbnail
                    QString url = QUrl::fromLocalFile(imgPath).toString();
                    QString tip = QString("<img src='%1' style='max-width:240px;max-height:160px;'>").arg(url);
                    QToolTip::showText(event->globalPosition().toPoint(), tip, this);
                    shown = true;
                }
                break;
            }
        }
        if (shown) tooltipShown = true;
        }

        // ── EVM Opcode/Gas HUD (Solidity/Yul) ──────────────────────────────────────
        if (!tooltipShown && (currentLanguage == Language::Solidity || currentLanguage == Language::Yul)) {
            QTextCursor cur = cursorForPosition(event->pos());
            QString lineText = cur.block().text();
            int col = cur.positionInBlock();
            // Extract the word under cursor
            int wordStart = col;
            int wordEnd = col;
            while (wordStart > 0 && (lineText[wordStart-1].isLetterOrNumber() || lineText[wordStart-1] == '_')) --wordStart;
            while (wordEnd < lineText.size() && (lineText[wordEnd].isLetterOrNumber() || lineText[wordEnd] == '_')) ++wordEnd;
            QString word = lineText.mid(wordStart, wordEnd - wordStart).toUpper();

            // EVM opcode gas cost map
            static const QMap<QString,QPair<int,QString>> evmOps = {
                {"SLOAD",   {100,  "Load from storage (warm: 100, cold: 2100)"}},
                {"SSTORE",  {100,  "Write to storage (warm: 100, new: 20000, modify: 2900)"}},
                {"MLOAD",   {3,    "Load 32 bytes from memory"}},
                {"MSTORE",  {3,    "Write 32 bytes to memory"}},
                {"MSTORE8", {3,    "Write 1 byte to memory"}},
                {"KECCAK256",{30,  "Compute Keccak-256 hash (30 + 6/word)"}},
                {"SHA3",    {30,   "Compute Keccak-256 hash (alias, 30 + 6/word)"}},
                {"CALL",    {2600, "Message call (cold: 2600+, warm: 100+)"}},
                {"STATICCALL",{2600,"Static call (cold: 2600+)"}},
                {"DELEGATECALL",{2600,"Delegate call (cold: 2600+)"}},
                {"BALANCE", {2600, "Get account balance (cold: 2600, warm: 100)"}},
                {"EXTCODESIZE",{2600,"Get external code size (cold: 2600)"}},
                {"LOG0",    {375,  "Emit log with 0 topics (375 + 8/byte)"}},
                {"LOG1",    {750,  "Emit log with 1 topic"}},
                {"LOG2",    {1125, "Emit log with 2 topics"}},
                {"LOG3",    {1500, "Emit log with 3 topics"}},
                {"LOG4",    {1875, "Emit log with 4 topics"}},
                {"CREATE",  {32000,"Create new contract"}},
                {"CREATE2", {32000,"Create contract with deterministic address"}},
                {"SELFDESTRUCT",{5000,"Destroy contract and send ETH"}},
                {"JUMP",    {8,    "Unconditional jump"}},
                {"JUMPI",   {10,   "Conditional jump"}},
                {"ADD",     {3,    "Addition"}},
                {"MUL",     {5,    "Multiplication"}},
                {"SUB",     {3,    "Subtraction"}},
                {"DIV",     {5,    "Integer division"}},
                {"MOD",     {5,    "Modulo"}},
                {"EXP",     {50,   "Exponentiation (50 + 50/byte of exponent)"}},
            };
            QString lookupKey = word;
            if (evmOps.contains(lookupKey)) {
                auto [gas, desc] = evmOps[lookupKey];
                QString tip = QString(
                    "<div style='background:#0d0d1a;border:1px solid #00ff88;padding:8px;border-radius:4px;font-family:Consolas,monospace;'>"
                    "<div style='color:#00ff88;font-weight:bold;font-size:13px;'>\u26a1 %1</div>"
                    "<div style='color:#88ff88;font-size:11px;margin-top:2px;'>Gas: <b>~%2</b></div>"
                    "<div style='color:#aaaaaa;font-size:10px;margin-top:4px;'>%3</div>"
                    "</div>"
                ).arg(lookupKey).arg(gas).arg(desc);
                QToolTip::showText(event->globalPosition().toPoint(), tip, this);
                tooltipShown = true;
            }
        }

        // ── Decimal Expansion Tooltips ──────────────────────────────────────────
        if (!tooltipShown) {
            QTextCursor cur = cursorForPosition(event->pos());
            QString lineText = cur.block().text();
            int col = cur.positionInBlock();
            static QRegularExpression numRe(R"((\b\d[\d_]*(?:\*\*\d+|e\d+)\b|0x[0-9A-Fa-f]{5,})\b)");
            QRegularExpressionMatchIterator it = numRe.globalMatch(lineText);
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                if (col >= m.capturedStart() && col <= m.capturedEnd()) {
                    QString token = m.captured(0);
                    QString expanded;
                    static QRegularExpression expRe(R"((\d+)e(\d+))");
                    static QRegularExpression powRe(R"((\d+)\*\*(\d+))");
                    static QRegularExpression hexRe2(R"(0x([0-9A-Fa-f]+))");
                    QRegularExpressionMatch em;
                    if ((em = expRe.match(token)).hasMatch()) {
                        bool ok; quint64 base = em.captured(1).toULongLong(&ok);
                        int exp = em.captured(2).toInt();
                        if (ok && exp <= 18) {
                            quint64 val = base;
                            for (int i=0;i<exp;++i) val *= 10;
                            expanded = QLocale().toString(val);
                        } else if (ok) {
                            expanded = em.captured(1) + " \u00d7 10^" + em.captured(2);
                        }
                    } else if ((em = powRe.match(token)).hasMatch()) {
                        bool ok; quint64 base2 = em.captured(1).toULongLong(&ok);
                        int exp2 = em.captured(2).toInt();
                        if (ok && base2 == 2 && exp2 <= 63) {
                            quint64 val = (quint64)1 << exp2;
                            expanded = QLocale().toString(val);
                        } else if (ok && exp2 <= 18) {
                            quint64 val = base2;
                            for (int i=1;i<exp2;++i) val *= base2;
                            expanded = QLocale().toString(val);
                        } else {
                            expanded = em.captured(1) + "^" + em.captured(2) + " (too large)";
                        }
                    } else if ((em = hexRe2.match(token)).hasMatch()) {
                        bool ok; quint64 val = em.captured(1).toULongLong(&ok, 16);
                        if (ok) expanded = "= " + QLocale().toString(val) + " (decimal)";
                    }
                    if (!expanded.isEmpty()) {
                        QString tip = QString(
                            "<div style='background:#0d0d0d;border:1px solid #ffb86c;padding:6px;border-radius:4px;font-family:Consolas,monospace;'>"
                            "<div style='color:#ffb86c;font-weight:bold;'>\U0001f522 %1</div>"
                            "<div style='color:#f8f8f2;font-size:13px;margin-top:3px;'>%2</div>"
                            "</div>"
                        ).arg(token.toHtmlEscaped(), expanded);
                        QToolTip::showText(event->globalPosition().toPoint(), tip, this);
                        tooltipShown = true;
                    }
                    break;
                }
            }
        }

        // ── Cyber-Encoder Ring (Alt + hover) ────────────────────────────────────
        if (!tooltipShown && (event->modifiers() & Qt::AltModifier)) {
            QTextCursor cur = cursorForPosition(event->pos());
            QString lineText = cur.block().text();
            int col = cur.positionInBlock();
            int tStart = col, tEnd = col;
            if (col > 0 && col < lineText.size()) {
                int sStart = col, sEnd = col;
                for (int i = col-1; i >= 0; --i) {
                    if (lineText[i] == '"' || lineText[i] == '\'') { sStart = i+1; break; }
                    if (lineText[i].isSpace()) { sStart = i+1; break; }
                }
                for (int i = col; i < lineText.size(); ++i) {
                    if (lineText[i] == '"' || lineText[i] == '\'') { sEnd = i; break; }
                    if (lineText[i].isSpace()) { sEnd = i; break; }
                }
                tStart = sStart; tEnd = sEnd;
            }
            QString token = lineText.mid(tStart, tEnd - tStart).trimmed();
            if (token.length() >= 4) {
                QString decoded;
                QString method;
                // Try Base64
                QByteArray b64 = QByteArray::fromBase64(token.toUtf8(), QByteArray::AbortOnBase64DecodingErrors);
                if (!b64.isEmpty() && token.size() % 4 == 0) {
                    bool printable = true;
                    for (char c : b64) { if ((unsigned char)c < 32 && c != '\n' && c != '\r' && c != '\t') { printable = false; break; } }
                    if (printable) { decoded = QString::fromUtf8(b64).left(200); method = "Base64"; }
                }
                // Try Hex
                if (decoded.isEmpty()) {
                    QString hexStr = token;
                    if (hexStr.startsWith("0x", Qt::CaseInsensitive)) hexStr = hexStr.mid(2);
                    static QRegularExpression hexOnly("^[0-9A-Fa-f]+$");
                    if (hexOnly.match(hexStr).hasMatch() && hexStr.size() >= 8) {
                        QByteArray hexBytes = QByteArray::fromHex(hexStr.toUtf8());
                        bool printable = true;
                        for (char c : hexBytes) { if ((unsigned char)c < 32 && c != '\n') { printable = false; break; } }
                        if (printable) { decoded = QString::fromUtf8(hexBytes).left(200); method = "Hex"; }
                        else { decoded = hexBytes.toHex(' ').toUpper(); method = "Hex bytes"; }
                    }
                }
                // Try URL encoding
                if (decoded.isEmpty() && token.contains('%')) {
                    decoded = QUrl::fromPercentEncoding(token.toUtf8());
                    if (decoded != token) method = "URL-decoded";
                    else decoded.clear();
                }
                if (!decoded.isEmpty()) {
                    QString tip = QString(
                        "<div style='background:#0a0a1a;border:1px solid #ff79c6;padding:8px;border-radius:4px;font-family:Consolas,monospace;max-width:400px;'>"
                        "<div style='color:#ff79c6;font-weight:bold;'>\U0001f510 Cyber-Encoder Ring \u2014 %1</div>"
                        "<div style='color:#8be9fd;font-size:11px;margin-top:4px;word-break:break-all;'>%2</div>"
                        "</div>"
                    ).arg(method, decoded.toHtmlEscaped());
                    QToolTip::showText(event->globalPosition().toPoint(), tip, this);
                    tooltipShown = true;
                }
            }
        }

        if (!tooltipShown)
            QToolTip::hideText();

    QPlainTextEdit::mouseMoveEvent(event);
}

// ============================================================
// Code Folding
// ============================================================
bool CodeEditor::isFoldable(const QTextBlock &block) const {
  QString text = block.text().trimmed();
  if (text.isEmpty()) return false;
  if (text.endsWith('{')) return true;
  if (text.endsWith('(') && text.contains("class ")) return true;
  if (text.startsWith("function ")) return true;
  // Python/YAML/indent-based: ends with ':' but is not a continuation line
  // (exclude closing-bracket lines like "):" or "]:" from multi-line signatures)
  if (text.endsWith(':') && !text.startsWith(')') && !text.startsWith(']'))
    return true;
  return false;
}

bool CodeEditor::isFolded(const QTextBlock &block) const {
  QTextBlock next = block.next();
  return next.isValid() && !next.isVisible();
}

int CodeEditor::findMatchingBrace(const QTextBlock &block) const {
  QString text = block.text();
  int depth = 0;
  for (const QChar &c : text) {
    if (c == '{')
      depth++;
    if (c == '}')
      depth--;
  }
  if (depth <= 0)
    return block.blockNumber();

  QTextBlock b = block.next();
  while (b.isValid()) {
    QString t = b.text();
    for (const QChar &c : t) {
      if (c == '{')
        depth++;
      if (c == '}')
        depth--;
    }
    if (depth <= 0)
      return b.blockNumber();
    b = b.next();
  }
  return document()->blockCount() - 1;
}

int CodeEditor::indentLevel(const QTextBlock &block) const {
    int spaces = 0;
    for (QChar c : block.text()) {
        if (c == ' ')       spaces++;
        else if (c == '\t') spaces += 4;
        else break;
    }
    return spaces;
}

int CodeEditor::findIndentEnd(const QTextBlock &block) const {
    int base = indentLevel(block);
    int lastNonEmpty = block.blockNumber();
    QTextBlock b = block.next();
    while (b.isValid()) {
        if (!b.text().trimmed().isEmpty()) {
            if (indentLevel(b) <= base) break;
            lastNonEmpty = b.blockNumber();
        }
        b = b.next();
    }
    // No body found — return header's own block number; toggleFoldAt will bail early
    if (lastNonEmpty == block.blockNumber())
        return lastNonEmpty;

    // Extend past trailing blank lines so the fold swallows the gap between blocks
    int last = lastNonEmpty;
    b = document()->findBlockByNumber(lastNonEmpty + 1);
    while (b.isValid() && b.text().trimmed().isEmpty()) {
        last = b.blockNumber();
        b = b.next();
    }
    return last;
}

void CodeEditor::toggleFoldAt(int blockNumber) {
  QTextBlock block = document()->findBlockByNumber(blockNumber);
  if (!block.isValid() || !isFoldable(block))
    return;

  bool fold = !isFolded(block);
  // Brace blocks use brace matching; indent blocks (Python/YAML/etc) use indent end
  int endBlock = block.text().trimmed().endsWith('{')
      ? findMatchingBrace(block)
      : findIndentEnd(block);

  // Nothing to fold (no body found)
  if (endBlock <= blockNumber)
    return;

  QTextBlock b = block.next();
  while (b.isValid() && b.blockNumber() <= endBlock) {
    b.setVisible(!fold);
    b = b.next();
  }

  // markContentsDirty triggers QPlainTextDocumentLayout::documentChanged
  // which recalculates block heights (invisible blocks → 0 height) and
  // emits documentSizeChanged so the scroll bar updates correctly.
  document()->markContentsDirty(block.position(),
                                document()->characterCount() - block.position());
  updateLineNumberAreaWidth(0);
  viewport()->update();
  foldingArea->update();
}

void CodeEditor::foldingAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(foldingArea);
  QColor bgColor = currentTheme.lineNumberBg.isValid()
                       ? currentTheme.lineNumberBg
                       : QColor(240, 240, 240);
  painter.fillRect(event->rect(), bgColor);

  QTextBlock block = firstVisibleBlock();
  int top =
      qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top() &&
        isFoldable(block)) {
      int yCenter = top + (bottom - top) / 2;
      int xCenter = foldingAreaWidth() / 2;

      painter.setPen(QColor(180, 180, 180));
      painter.setBrush(Qt::NoBrush);

      if (isFolded(block)) {
        // Draw right-pointing triangle ▶
        QPolygon tri;
        tri << QPoint(xCenter - 3, yCenter - 4) << QPoint(xCenter + 4, yCenter)
            << QPoint(xCenter - 3, yCenter + 4);
        painter.setBrush(QColor(180, 180, 180));
        painter.drawPolygon(tri);
      } else {
        // Draw down-pointing triangle ▼
        QPolygon tri;
        tri << QPoint(xCenter - 4, yCenter - 3)
            << QPoint(xCenter + 4, yCenter - 3) << QPoint(xCenter, yCenter + 4);
        painter.setBrush(QColor(180, 180, 180));
        painter.drawPolygon(tri);
      }
    }
    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
  }
}

void CodeEditor::addExtraCursor(const QTextCursor &c) {
    extraCursors.append(c);
    viewport()->update();
}

void CodeEditor::clearExtraCursors() {
    extraCursors.clear();
    viewport()->update();
}

void CodeEditor::selectNextOccurrence() {
    QTextCursor mainC = textCursor();
    if (!mainC.hasSelection()) {
        mainC.select(QTextCursor::WordUnderCursor);
        setTextCursor(mainC);
        return;
    }
    QString text = mainC.selectedText();
    QTextCursor searchStart = extraCursors.isEmpty() ? mainC : extraCursors.last();
    QTextCursor nextC = document()->find(text, searchStart);
    if (!nextC.isNull()) {
        addExtraCursor(nextC);
    }
}

void CodeEditor::mousePressEvent(QMouseEvent *event) {
    // Ctrl+Click on hex color code → open color picker
    if ((event->modifiers() & Qt::ControlModifier) && event->button() == Qt::LeftButton) {
        QTextCursor cur = cursorForPosition(event->pos());
        QString line = cur.block().text();
        int col = cur.positionInBlock();
        static QRegularExpression hexRe("#([0-9A-Fa-f]{6}|[0-9A-Fa-f]{3})\\b");
        QRegularExpressionMatchIterator it = hexRe.globalMatch(line);
        while (it.hasNext()) {
            QRegularExpressionMatch m = it.next();
            if (col >= m.capturedStart() && col <= m.capturedEnd()) {
                QColor initial(m.captured(0));
                QColor chosen = QColorDialog::getColor(initial, this, "Pick Color",
                    QColorDialog::ShowAlphaChannel);
                if (chosen.isValid()) {
                    QTextCursor tc = cur;
                    tc.setPosition(cur.block().position() + m.capturedStart());
                    tc.setPosition(cur.block().position() + m.capturedEnd(), QTextCursor::KeepAnchor);
                    QString newHex = chosen.name().toUpper();
                    tc.insertText(newHex);
                }
                event->accept();
                return;
            }
        }
    }

    // ── Memory Pointer / Yul Tracer ─────────────────────────────────────────
    if (memTraceEnabled && (currentLanguage == Language::Solidity || currentLanguage == Language::Yul)) {
        QTextCursor cur2 = cursorForPosition(event->pos());
        QString lineText2 = cur2.block().text();
        int col2 = cur2.positionInBlock();
        int wS = col2, wE = col2;
        while (wS > 0 && (lineText2[wS-1].isLetterOrNumber() || lineText2[wS-1] == 'x')) --wS;
        while (wE < lineText2.size() && (lineText2[wE].isLetterOrNumber())) ++wE;
        QString token2 = lineText2.mid(wS, wE - wS);
        bool isMemPtr = false;
        quint64 targetOffset = 0;
        if (token2.startsWith("0x", Qt::CaseInsensitive)) {
            bool ok; targetOffset = token2.mid(2).toULongLong(&ok, 16);
            isMemPtr = ok;
        } else if (token2 == "64") {
            targetOffset = 0x40; isMemPtr = true;
        } else if (token2 == "96") {
            targetOffset = 0x60; isMemPtr = true;
        }
        if (isMemPtr) {
            QString offsetHex = QString("0x%1").arg(targetOffset, 0, 16);
            QVector<int> matchLines;
            QString docText = toPlainText();
            QStringList docLines = docText.split('\n');
            static QRegularExpression mloadRe(R"(\bmload\s*\()");
            static QRegularExpression mstoreRe(R"(\bmstore\s*\()");
            for (int li = 0; li < docLines.size(); ++li) {
                const QString &dl = docLines[li];
                if ((mloadRe.match(dl).hasMatch() || mstoreRe.match(dl).hasMatch()) &&
                    dl.contains(offsetHex, Qt::CaseInsensitive)) {
                    matchLines.append(li);
                }
            }
            if (!matchLines.isEmpty()) {
                highlightMemoryTraceLines(matchLines);
                QToolTip::showText(event->globalPosition().toPoint(),
                    QString("Memory tracer: %1 mload/mstore ops at %2").arg(matchLines.size()).arg(offsetHex),
                    this, QRect(), 2000);
            }
        }
    }

    if (event->modifiers() & Qt::AltModifier) {
        QTextCursor c = cursorForPosition(event->pos());
        addExtraCursor(c);
        return;
    }
    clearExtraCursors();
    QPlainTextEdit::mousePressEvent(event);
}

void CodeEditor::keyPressEvent(QKeyEvent *event) {
  // Always emit keyPressed for heatmap tracking (before any returns)
  emit keyPressed(event->key(), event->text());

  if (!ghostText.isEmpty()) {
      if (event->key() == Qt::Key_Escape) {
          clearGhostText();
          return;
      }
      if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier) {
          insertPlainText(ghostText);
          clearGhostText();
          return;
      }
      clearGhostText();
  }

  // Vim mode — intercepts keys when enabled
  if (vimMode && vimMode->handleKey(event, this))
      return;

  // Glitch Art Undo
  if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_Z) {
      triggerGlitchUndo();
      return;
  }

  if (event->key() == Qt::Key_Escape && !extraCursors.isEmpty()) {
      clearExtraCursors();
      return;
  }

  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    emit characterTyped();
    autoIndent();
    return;
  }

  if (event->key() == Qt::Key_Home) {
    if (event->modifiers() == Qt::NoModifier ||
        event->modifiers() == Qt::ShiftModifier) {
      smartHome();
      // To handle Shift+Home selection, we could enhance smartHome to take a
      // KeepAnchor flag, but simple smartHome on Home press is a good start.
      return;
    }
  }

  QString text = event->text();
  if (text.isEmpty()) {
    QPlainTextEdit::keyPressEvent(event);
    return;
  }

  emit characterTyped();

  if (!extraCursors.isEmpty()) {
      QTextCursor mainCursor = textCursor();
      mainCursor.beginEditBlock();
      int key = event->key();
      for (int i=0; i<extraCursors.size(); i++) {
          QTextCursor &c = extraCursors[i];
          if (key == Qt::Key_Backspace) c.deletePreviousChar();
          else if (key == Qt::Key_Delete) c.deleteChar();
          else if (text[0].isPrint()) c.insertText(text);
      }
      mainCursor.endEditBlock();
  }

  QTextCursor cursor = textCursor();
  QChar ch = text[0];

  if (ch == '(' || ch == '[' || ch == '{') {
    QChar closing = ch == '(' ? ')' : ch == '[' ? ']' : '}';
    cursor.beginEditBlock();
    cursor.insertText(QString(ch) + QString(closing));
    cursor.movePosition(QTextCursor::Left);
    cursor.endEditBlock();
    setTextCursor(cursor);
    return;
  }

  if (ch == '"' || ch == '\'') {
    QChar nextChar =
        cursor.atEnd() ? QChar() : document()->characterAt(cursor.position());
    if (nextChar == ch) {
      cursor.movePosition(QTextCursor::Right);
      setTextCursor(cursor);
      return;
    } else {
      cursor.beginEditBlock();
      cursor.insertText(QString(ch) + QString(ch));
      cursor.movePosition(QTextCursor::Left);
      cursor.endEditBlock();
      setTextCursor(cursor);
      return;
    }
  }

  if (ch == ')' || ch == ']' || ch == '}') {
    QChar nextChar =
        cursor.atEnd() ? QChar() : document()->characterAt(cursor.position());
    if (nextChar == ch) {
      cursor.movePosition(QTextCursor::Right);
      setTextCursor(cursor);
      return;
    }
  }

  // Graveyard: catch large selections being deleted/cut
  bool isCut    = (ch == QChar(24));  // Ctrl+X = ASCII 24
  bool isDel    = (event->key() == Qt::Key_Delete    && cursor.hasSelection());
  bool isBacksp = (event->key() == Qt::Key_Backspace && cursor.hasSelection());
  if (isCut || isDel || isBacksp) {
      if (cursor.hasSelection()) {
          QString sel = cursor.selectedText();
          int newlineCount = sel.count('\n') + sel.count(QChar::ParagraphSeparator);
          if (newlineCount >= 3) {
              emit codeBlockDeleted(
                  sel.replace(QChar::ParagraphSeparator, '\n'),
                  fileName.isEmpty() ? "untitled" : QFileInfo(fileName).fileName());
          }
      }
  }

  // Auto-close HTML/XML tags
  if (ch == '/') {
    QTextCursor c = textCursor();
    c.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
    if (c.selectedText() == "<") {
        QString textBefore = document()->toPlainText().left(textCursor().position() - 1);
        QStringList openTags;
        QRegularExpression tagRegex("<(/?)(\\w+)[^>]*>");
        QRegularExpressionMatchIterator i = tagRegex.globalMatch(textBefore);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            bool isClosing = !match.captured(1).isEmpty();
            QString tagName = match.captured(2);
            if (isClosing) {
                if (!openTags.isEmpty() && openTags.last() == tagName) {
                    openTags.removeLast();
                }
            } else {
                if (!match.captured(0).endsWith("/>") &&
                    tagName.toLower() != "br" && tagName.toLower() != "hr" &&
                    tagName.toLower() != "img" && tagName.toLower() != "meta" &&
                    tagName.toLower() != "link" && tagName.toLower() != "input") {
                    openTags.append(tagName);
                }
            }
        }
        if (!openTags.isEmpty()) {
            QString tagToClose = openTags.last();
            QPlainTextEdit::keyPressEvent(event);
            QTextCursor insertC = textCursor();
            insertC.insertText(tagToClose + ">");
            return;
        }
    }
  }

  QPlainTextEdit::keyPressEvent(event);
}

void CodeEditor::setGhostText(const QString &text) {
  ghostText = text;
  viewport()->update();
}

void CodeEditor::clearGhostText() {
  if (ghostText.isEmpty()) return;
  ghostText.clear();
  viewport()->update();
}

void CodeEditor::autoIndent() {
  QTextCursor cursor = textCursor();
  QString previousLine = cursor.block().text();
  int indent = 0;
  for (QChar c : previousLine) {
    if (c == ' ')
      indent++;
    else if (c == '\t')
      indent += 4;
    else
      break;
  }
  if (previousLine.trimmed().endsWith('{') ||
      previousLine.trimmed().endsWith(':'))
    indent += 4;
  cursor.insertText("\n" + QString(" ").repeated(indent));
  setTextCursor(cursor);
}

void CodeEditor::matchBrackets() {}

void CodeEditor::setCRTEnabled(bool enabled) {
    crtOverlay->setEnabled(enabled);
    if (enabled) {
        crtOverlay->setGeometry(viewport()->rect());
        crtOverlay->raise();
    }
}

void CodeEditor::setVimEnabled(bool enabled) {
    if (vimMode) vimMode->setEnabled(enabled);
}

bool CodeEditor::isVimEnabled() const {
    return vimMode && vimMode->isEnabled();
}

void CodeEditor::setAmbientBackground(QColor tint) {
    QColor bg = currentTheme.background.isValid() ? currentTheme.background : QColor(30, 30, 30);
    if (tint.isValid() && tint.alpha() > 0) {
        float a = tint.alphaF();
        bg = QColor(
            qBound(0, static_cast<int>(bg.red()   * (1 - a) + tint.red()   * a), 255),
            qBound(0, static_cast<int>(bg.green() * (1 - a) + tint.green() * a), 255),
            qBound(0, static_cast<int>(bg.blue()  * (1 - a) + tint.blue()  * a), 255));
    }
    // Update stylesheet (takes precedence over palette in Qt)
    QString style = QString("QPlainTextEdit { background-color: %1; color: %2; "
                            "selection-background-color: %3; border: none; }")
                        .arg(bg.name())
                        .arg(currentTheme.foreground.name())
                        .arg(currentTheme.selection.name());
    setStyleSheet(style);
    QPalette p = palette();
    p.setColor(QPalette::Base, bg);
    setPalette(p);
    viewport()->update();
}

void CodeEditor::triggerLaserEffect() {
    QTextCursor c = textCursor();
    QRect lineRect = cursorRect(c);
    laserOverlay->setGeometry(viewport()->rect());
    laserOverlay->show();
    laserOverlay->raise();
    laserOverlay->spawnSlash(lineRect.center().y());
}

void CodeEditor::triggerGlitchUndo() {
    QPixmap snapshot = viewport()->grab();
    glitchOverlay->setGeometry(viewport()->rect());
    glitchOverlay->show();
    glitchOverlay->raise();
    glitchOverlay->trigger(snapshot);
    QPlainTextEdit::undo();
}

// Ghost Replay implementation
void CodeEditor::startRecordingGhost() {
    ghostLog.clear();
    sessionStartTimeMs = QDateTime::currentMSecsSinceEpoch();
    ghostIsRecording = true;
}

void CodeEditor::logGhostEvent(int pos, int charsRemoved, const QString &textAdded) {
    if (paranoiaMode) return;
    GhostEvent event;
    event.timestampMs = QDateTime::currentMSecsSinceEpoch() - sessionStartTimeMs;
    event.position = pos;
    event.charsRemoved = charsRemoved;
    event.textAdded = textAdded;
    ghostLog.append(event);
}

void CodeEditor::onDocumentContentsChange(int position, int charsRemoved, int charsAdded) {
    Q_UNUSED(position);
    Q_UNUSED(charsRemoved);
    Q_UNUSED(charsAdded);
    if (paintCacheTimer) paintCacheTimer->start();
    if (!ghostIsRecording) return;

    if (sessionStartTimeMs == 0) {
        startRecordingGhost();
    }

    QString addedStr;
    if (charsAdded > 0) {
        QTextCursor c(document());
        c.setPosition(position);
        c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, charsAdded);
        addedStr = c.selectedText();
    }

    logGhostEvent(position, charsRemoved, addedStr);
}

void CodeEditor::rebuildPaintCache() {
    colorMarks.clear();
    static const QRegularExpression hexRegex(
        "#([0-9A-Fa-f]{3}|[0-9A-Fa-f]{6}|[0-9A-Fa-f]{8})\\b");
    for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
        const QString text = block.text();
        for (auto it = hexRegex.globalMatch(text); it.hasNext();) {
            const QRegularExpressionMatch match = it.next();
            const QColor color(match.captured(0));
            if (color.isValid())
                colorMarks[block.blockNumber()].append({match.capturedStart(),
                                                         match.capturedLength(), color});
        }
    }
}

void CodeEditor::miniMapPaintEvent(QPaintEvent *event) {
  QPainter painter(miniMap);
  painter.fillRect(event->rect(), QColor(10, 14, 10)); // darker bg for waterfall

  // ── Data Waterfall: Matrix-style digital rain ─────────────────────────────
  if (minimapWaterfallEnabled) {
      int cols = miniMap->width() / 6; // one raindrop column per 6px
      // Grow / init the drops vector as needed
      while (waterfallDrops.size() < cols)
          waterfallDrops.append(QRandomGenerator::global()->bounded(miniMap->height()));
      waterfallFrame++;
      const QChar digits[] = {
          '0','1','2','3','4','5','6','7','8','9',
          'A','B','C','D','E','F','X','Z','<','>','{','}'
      };
      QFont wfFont("Consolas", 6);
      painter.setFont(wfFont);
      for (int col = 0; col < cols && col < waterfallDrops.size(); ++col) {
          int x = col * 6;
          int dropY = waterfallDrops[col];
          // Trail
          for (int trail = 0; trail < 10; ++trail) {
              int ty = dropY - trail * 8;
              if (ty < 0) continue;
              int alpha = 180 - trail * 18;
              if (alpha <= 0) break;
              QColor c = (trail == 0) ? QColor(180, 255, 180, alpha)
                                      : QColor(0, 160 - trail * 12, 0, alpha);
              painter.setPen(c);
              int charIdx = (waterfallFrame + col * 7 + trail * 3) %
                            static_cast<int>(sizeof(digits) / sizeof(digits[0]));
              painter.drawText(x, ty, QString(digits[charIdx]));
          }
          // Advance drop
          if ((waterfallFrame + col * 3) % 2 == 0) {
              waterfallDrops[col] += 8;
              if (waterfallDrops[col] > miniMap->height() + 80)
                  waterfallDrops[col] = -(QRandomGenerator::global()->bounded(80));
          }
      }
  }

  int totalLines = document()->blockCount();
  if (totalLines == 0) return;
  int visibleLines = height() / fontMetrics().height();
  int startLine = (event->rect().top() * totalLines) / miniMap->height();
  int endLine = (event->rect().bottom() * totalLines) / miniMap->height() + 1;
  QTextBlock block = document()->findBlockByLineNumber(qMax(0, startLine));
  int blockNumber = block.blockNumber();
  // Code overview overlay — semi-transparent cyan lines over the waterfall
  painter.setPen(QColor(0, 255, 200, 80));
  while (block.isValid() && blockNumber <= endLine) {
    int y = (blockNumber * miniMap->height()) / totalLines;
    QString text = block.text().trimmed();
    if (!text.isEmpty()) {
      int lineWidth = qMin(text.length() * 2, miniMap->width() - 10);
      painter.drawLine(5, y, 5 + lineWidth, y);
    }
    block = block.next();
    blockNumber++;
  }
  int firstVisible = firstVisibleBlock().blockNumber();
  int viewportY = (firstVisible * miniMap->height()) / totalLines;
  int viewportHeight =
      qMax(10, (visibleLines * miniMap->height()) / totalLines);
  painter.fillRect(0, viewportY, miniMap->width(), viewportHeight,
                   QColor(0, 200, 100, 40));
  painter.setPen(QColor(0, 255, 120, 180));
  painter.drawRect(0, viewportY, miniMap->width() - 1, viewportHeight);

  // Gas Topographic Overlay
  if (gasMinimapEnabled) {
      static QRegularExpression highGasRe(
          R"(\b(sstore|SSTORE|sload|SLOAD|keccak256|KECCAK256|sha3|SHA3|delegatecall|DELEGATECALL|call\{|\.call\(|create|CREATE)\b)",
          QRegularExpression::CaseInsensitiveOption);
      static QRegularExpression loopRe(R"(\b(for|while|do)\s*\()");
      QString docText = document()->toPlainText();
      QStringList docLines2 = docText.split('\n');
      int numLines2 = docLines2.size();
      if (numLines2 == 0) return;
      float lineH2 = (float)height() / numLines2;
      for (int li = 0; li < numLines2; ++li) {
          const QString &dl = docLines2[li];
          QColor lineColor;
          if (highGasRe.match(dl).hasMatch()) {
              lineColor = QColor(255, 60, 30, 140);
          } else if (loopRe.match(dl).hasMatch()) {
              lineColor = QColor(255, 160, 0, 100);
          } else if (dl.contains("view") || dl.contains("pure")) {
              lineColor = QColor(30, 100, 255, 60);
          } else {
              continue;
          }
          float y2 = li * lineH2;
          painter.fillRect(QRectF(0, y2, width(), qMax(1.0f, lineH2)), lineColor);
      }
      painter.setPen(QColor(255,80,80,200));
      painter.drawText(2, height()-28, "\U0001f525 high gas");
      painter.setPen(QColor(30,130,255,200));
      painter.drawText(2, height()-14, "\U0001f9ca view/pure");
  }
}

void CodeEditor::wheelEvent(QWheelEvent *event) {
  // Kinetic scroll overrides the old animation when enabled
  if (kineticScrollEnabled) {
      int delta = event->angleDelta().y();
      kineticVelocity -= delta * 0.08;
      if (!kineticTimer->isActive()) kineticTimer->start();
      event->accept();
      return;
  }

  if (!smoothScrollEnabled) {
    QPlainTextEdit::wheelEvent(event);
    return;
  }
  int numDegrees = event->angleDelta().y() / 8;
  int numSteps = numDegrees / 8;

  if (numSteps == 0) {
    event->accept();
    return;
  }

  QScrollBar *scrollBar = verticalScrollBar();
  int currentValue = scrollBar->value();
  targetScrollValue =
      currentValue - (numSteps * 5); // 5 lines per step for faster scrolling

  targetScrollValue =
      qMax(scrollBar->minimum(), qMin(targetScrollValue, scrollBar->maximum()));
  if (scrollAnimation->state() == QAbstractAnimation::Running)
    scrollAnimation->stop();
  scrollAnimation->setStartValue(currentValue);
  scrollAnimation->setEndValue(targetScrollValue);
  scrollAnimation->start();
  event->accept();
}

void CodeEditor::enableSmoothScrolling(bool enable) {
  smoothScrollEnabled = enable;
}

void CodeEditor::duplicateLine() {
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();
  cursor.movePosition(QTextCursor::StartOfBlock);
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  QString textText = cursor.selectedText();
  cursor.movePosition(QTextCursor::EndOfBlock);
  cursor.insertText("\n" + textText);
  cursor.endEditBlock();
}

void CodeEditor::moveLineUp() {
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();
  if (cursor.blockNumber() == 0) {
    cursor.endEditBlock();
    return;
  }

  QTextBlock currentBlock = cursor.block();
  QTextBlock prevBlock = currentBlock.previous();

  // Select current block + trailing newline
  cursor.movePosition(QTextCursor::StartOfBlock);
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  if (!cursor.atEnd())
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  QString text = cursor.selectedText();
  cursor.removeSelectedText();

  cursor.setPosition(prevBlock.position());
  cursor.insertText(text);
  cursor.endEditBlock();
}

void CodeEditor::moveLineDown() {
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();
  if (cursor.blockNumber() == document()->blockCount() - 1) {
    cursor.endEditBlock();
    return;
  }

  QTextBlock currentBlock = cursor.block();
  QTextBlock nextBlock = currentBlock.next();

  // Select current block + trailing newline
  cursor.movePosition(QTextCursor::StartOfBlock);
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  if (!cursor.atEnd())
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  QString text = cursor.selectedText();
  cursor.removeSelectedText();

  cursor.setPosition(nextBlock.position());
  cursor.movePosition(QTextCursor::EndOfBlock);
  if (cursor.atEnd())
    cursor.insertText("\n" + text.trimmed());
  else {
    cursor.movePosition(QTextCursor::NextCharacter);
    cursor.insertText(text);
  }

  cursor.endEditBlock();
}

void CodeEditor::deleteLine() {
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();
  cursor.movePosition(QTextCursor::StartOfBlock);
  cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
  if (!cursor.atEnd())
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
  cursor.removeSelectedText();
  cursor.endEditBlock();
  // Kinetic laser effect on delete
  triggerLaserEffect();
}

void CodeEditor::toggleComment() {
  QTextCursor cursor = textCursor();
  cursor.beginEditBlock();

  QString prefix = "//";
  if (currentLanguage == Language::Python || currentLanguage == Language::YAML)
    prefix = "#";
  else if (currentLanguage == Language::HTML)
    prefix = "<!--";

  int startBlock = cursor.selectionStart();
  int endBlock = cursor.selectionEnd();
  QTextCursor iterCursor(document());
  iterCursor.setPosition(startBlock);
  int firstBlockNum = iterCursor.blockNumber();
  iterCursor.setPosition(endBlock);
  int lastBlockNum = iterCursor.blockNumber();

  // Check if commenting or uncommenting
  bool allCommented = true;
  for (int i = firstBlockNum; i <= lastBlockNum; ++i) {
    QTextBlock block = document()->findBlockByLineNumber(i);
    QString text = block.text().trimmed();
    if (!text.isEmpty() && !text.startsWith(prefix)) {
      allCommented = false;
      break;
    }
  }

  for (int i = firstBlockNum; i <= lastBlockNum; ++i) {
    QTextBlock block = document()->findBlockByLineNumber(i);
    QString text = block.text();
    if (text.trimmed().isEmpty())
      continue;

    iterCursor.setPosition(block.position());
    if (allCommented) {
      int idx = text.indexOf(prefix);
      if (idx != -1) {
        iterCursor.setPosition(block.position() + idx);
        iterCursor.movePosition(QTextCursor::NextCharacter,
                                QTextCursor::KeepAnchor, prefix.length());
        if (prefix == "<!--") {
          // special HTML uncommenting
          QString full = text;
          int endIdx = full.indexOf("-->", idx);
          if (endIdx != -1) {
            QTextCursor endCur(document());
            endCur.setPosition(block.position() + endIdx);
            endCur.movePosition(QTextCursor::NextCharacter,
                                QTextCursor::KeepAnchor, 3);
            endCur.removeSelectedText();
          }
        }
        iterCursor.removeSelectedText();
        // Remove trailing space if exists
        if (iterCursor.block().text().length() > iterCursor.positionInBlock() &&
            iterCursor.block().text().at(iterCursor.positionInBlock()) == ' ') {
          iterCursor.deleteChar();
        }
      }
    } else {
      // Find first non-whitespace
      int idx = 0;
      while (idx < text.length() && text.at(idx).isSpace())
        idx++;
      iterCursor.setPosition(block.position() + idx);
      if (prefix == "<!--") {
        iterCursor.insertText(prefix + " ");
        iterCursor.movePosition(QTextCursor::EndOfBlock);
        iterCursor.insertText(" -->");
      } else {
        iterCursor.insertText(prefix + " ");
      }
    }
  }
  cursor.endEditBlock();
}

void CodeEditor::smartHome() {
  QTextCursor cursor = textCursor();
  QTextBlock block = cursor.block();
  QString text = block.text();

  int firstNonSpace = 0;
  while (firstNonSpace < text.length() && text.at(firstNonSpace).isSpace()) {
    firstNonSpace++;
  }

  int currentPos = cursor.positionInBlock();
  if (currentPos == firstNonSpace) {
    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::MoveAnchor);
  } else {
    cursor.setPosition(block.position() + firstNonSpace,
                       QTextCursor::MoveAnchor);
  }
  setTextCursor(cursor);
}

// ── v0.8.0 QoL Feature Implementations ──────────────────────────────────────

void CodeEditor::setStickyScrollEnabled(bool enabled) {
    stickyScrollEnabled = enabled;
    viewport()->update();
}

void CodeEditor::setInvisibleCharsEnabled(bool enabled) {
    invisibleCharsEnabled = enabled;
    viewport()->update();
}

void CodeEditor::setGitBlameEnabled(bool enabled) {
    gitBlameEnabled = enabled;
    if (enabled && !fileName.isEmpty())
        fetchGitBlame();
    else
        blameCache.clear();
    viewport()->update();
}

void CodeEditor::fetchGitBlame() {
    if (fileName.isEmpty()) return;
    if (blameProcess) {
        blameProcess->kill();
        blameProcess->deleteLater();
    }
    blameProcess = new QProcess(this);
    blameProcess->setWorkingDirectory(QFileInfo(fileName).absolutePath());
    QStringList args;
    args << "blame" << "--porcelain" << QFileInfo(fileName).fileName();
    connect(blameProcess, &QProcess::finished, this, [this]() {
        blameCache.clear();
        QByteArray data = blameProcess->readAllStandardOutput();
        QStringList lines = QString::fromUtf8(data).split('\n');
        int currentLine = -1;
        QString author, timeAgo;
        for (const QString &l : lines) {
            // Porcelain format: hash SP orig-line SP final-line SP ...
            static QRegularExpression hashLine("^[0-9a-f]{40} \\d+ (\\d+)");
            QRegularExpressionMatch m = hashLine.match(l);
            if (m.hasMatch()) {
                currentLine = m.captured(1).toInt() - 1; // 0-based
                author.clear(); timeAgo.clear();
            } else if (l.startsWith("author ") && currentLine >= 0) {
                author = l.mid(7).trimmed();
                if (author.length() > 12) author = author.left(11) + ".";
            } else if (l.startsWith("author-time ") && currentLine >= 0) {
                qint64 ts = l.mid(12).trimmed().toLongLong();
                qint64 now = QDateTime::currentSecsSinceEpoch();
                qint64 diff = now - ts;
                if (diff < 3600)      timeAgo = QString::number(diff/60) + "m ago";
                else if (diff < 86400) timeAgo = QString::number(diff/3600) + "h ago";
                else if (diff < 86400*30) timeAgo = QString::number(diff/86400) + "d ago";
                else                  timeAgo = QString::number(diff/2592000) + "mo ago";
                if (!author.isEmpty() && !timeAgo.isEmpty())
                    blameCache[currentLine] = author + ", " + timeAgo;
            }
        }
        blameProcess->deleteLater();
        blameProcess = nullptr;
        viewport()->update();
    });
    blameProcess->start("git", args);
}

void CodeEditor::setKineticScrollEnabled(bool enabled) {
    kineticScrollEnabled = enabled;
    if (!enabled) { kineticTimer->stop(); kineticVelocity = 0; }
}

// Smart Paste (auto-indent) + Visual URL Paste (Markdown)
void CodeEditor::insertFromMimeData(const QMimeData *source) {
    // Visual URL paste: if clipboard has a URL and there's a selection → Markdown link
    if (source->hasUrls() || (source->hasText() &&
        (source->text().startsWith("http://") || source->text().startsWith("https://")))) {
        if (textCursor().hasSelection() &&
            (currentLanguage == Language::Markdown || currentLanguage == Language::PlainText)) {
            QString url = source->hasUrls() ? source->urls().first().toString() : source->text().trimmed();
            QString sel = textCursor().selectedText();
            textCursor().insertText("[" + sel + "](" + url + ")");
            return;
        }
    }

    // Smart paste: re-indent multi-line blocks to match current cursor indent
    if (source->hasText()) {
        QString pasted = source->text();
        if (pasted.contains('\n')) {
            // Determine current line's indent
            QTextCursor cur = textCursor();
            QString curLine = cur.block().text();
            int targetIndent = 0;
            for (QChar c : curLine) {
                if (c == ' ') targetIndent++;
                else if (c == '\t') targetIndent += 4;
                else break;
            }
            // Determine pasted block's base indent
            QStringList pastedLines = pasted.split('\n');
            int baseIndent = INT_MAX;
            for (const QString &pl : pastedLines) {
                if (pl.trimmed().isEmpty()) continue;
                int ind = 0;
                for (QChar c : pl) {
                    if (c == ' ') ind++;
                    else if (c == '\t') ind += 4;
                    else break;
                }
                baseIndent = qMin(baseIndent, ind);
            }
            if (baseIndent == INT_MAX) baseIndent = 0;
            // Re-indent
            QStringList result;
            for (int i = 0; i < pastedLines.size(); ++i) {
                const QString &pl = pastedLines[i];
                if (pl.trimmed().isEmpty() && i > 0 && i < pastedLines.size()-1) {
                    result << QString();
                    continue;
                }
                int lineIndent = 0;
                int charStart = 0;
                for (QChar c : pl) {
                    if (c == ' ') { lineIndent++; charStart++; }
                    else if (c == '\t') { lineIndent += 4; charStart++; }
                    else break;
                }
                int newIndent = (i == 0) ? targetIndent : targetIndent + (lineIndent - baseIndent);
                newIndent = qMax(0, newIndent);
                result << QString(' ').repeated(newIndent) + pl.mid(charStart);
            }
            cur.insertText(result.join('\n'));
            return;
        }
    }

    QPlainTextEdit::insertFromMimeData(source);
}

void CodeEditor::setGasMinimapEnabled(bool en) {
    gasMinimapEnabled = en;
    if (miniMap) miniMap->update();
}

void CodeEditor::setMemTraceEnabled(bool en) {
    memTraceEnabled = en;
    viewport()->update();
}

void CodeEditor::highlightMemoryTraceLines(const QVector<int> &lines) {
    memTraceHighlightLines = lines;
    viewport()->update();
}
