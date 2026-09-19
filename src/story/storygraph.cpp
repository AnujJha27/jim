#include "storygraph.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QWheelEvent>
#include <QQueue>
#include <QSet>
#include <cmath>
#include <chrono>

// ── Color palette ─────────────────────────────────────────────────────────────
static QColor nodeColor(StoryGraphNode::NodeType t) {
    switch (t) {
        case StoryGraphNode::Start:       return QColor("#50fa7b"); // green
        case StoryGraphNode::DeadEnd:     return QColor("#ff5555"); // red
        case StoryGraphNode::Ending:      return QColor("#f1fa8c"); // yellow
        case StoryGraphNode::Unreachable: return QColor("#6272a4"); // muted blue
        default:                          return QColor("#8be9fd"); // cyan
    }
}

// ── StoryGraph ────────────────────────────────────────────────────────────────

StoryGraph::StoryGraph(QWidget *parent) : QWidget(parent)
{
    setStyleSheet("background-color: #0b0c10;");

    m_scene = new QGraphicsScene(this);
    m_scene->setBackgroundBrush(QColor("#0b0c10"));

    m_view = new QGraphicsView(m_scene, this);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setDragMode(QGraphicsView::ScrollHandDrag);
    m_view->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    m_view->setStyleSheet("border: none; background: transparent;");

    // Legend
    auto *legend = new QWidget(this);
    legend->setStyleSheet("background: transparent;");
    auto *legendLayout = new QHBoxLayout(legend);
    legendLayout->setContentsMargins(8, 4, 8, 4);
    legendLayout->setSpacing(16);

    auto addLegend = [&](const QString &label, const QColor &col) {
        auto *dot = new QLabel("●");
        dot->setStyleSheet(QString("color: %1; font-size: 14px;").arg(col.name()));
        auto *lbl = new QLabel(label);
        lbl->setStyleSheet("color: #6272a4; font-size: 11px; font-family: Consolas;");
        legendLayout->addWidget(dot);
        legendLayout->addWidget(lbl);
    };
    addLegend("Start",       QColor("#50fa7b"));
    addLegend("Normal",      QColor("#8be9fd"));
    addLegend("Dead End",    QColor("#ff5555"));
    addLegend("Ending",      QColor("#f1fa8c"));
    addLegend("Unreachable", QColor("#6272a4"));
    legendLayout->addStretch();

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(legend);
    layout->addWidget(m_view, 1);

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &StoryGraph::applyPhysics);
}

void StoryGraph::clearGraph()
{
    m_timer->stop();
    m_scene->clear();
    m_nodes.clear();
    m_edges.clear();
}

void StoryGraph::refresh(const QString &storyText)
{
    clearGraph();
    if (storyText.trimmed().isEmpty()) return;

    QVector<StoryPassage> passages = StoryParser::parse(storyText);
    buildGraph(passages);
    m_timer->start(16);
}

void StoryGraph::setCurrentPassage(const QString &name)
{
    for (auto it = m_nodes.begin(); it != m_nodes.end(); ++it)
        it.value()->setActive(it.key() == name);
}

void StoryGraph::buildGraph(const QVector<StoryPassage> &passages)
{
    auto seed = static_cast<unsigned>(
        std::chrono::steady_clock::now().time_since_epoch().count());
    srand(seed);

    // Determine node types
    QString startName = StoryParser::findStartPassage(passages);

    // Build reachability set
    QMap<QString, const StoryPassage*> byName;
    for (const auto &p : passages) byName[p.name] = &p;

    QSet<QString> reachable;
    QQueue<QString> q;
    if (!startName.isEmpty()) { q.enqueue(startName); reachable.insert(startName); }
    while (!q.isEmpty()) {
        QString cur = q.dequeue();
        if (!byName.contains(cur)) continue;
        for (const auto &c : byName[cur]->choices) {
            if (byName.contains(c.target) && !reachable.contains(c.target)) {
                reachable.insert(c.target);
                q.enqueue(c.target);
            }
        }
    }

    // Create nodes
    int count = 0;
    for (const auto &p : passages) {
        StoryGraphNode::NodeType type = StoryGraphNode::Normal;
        if (p.name == startName)
            type = StoryGraphNode::Start;
        else if (!reachable.contains(p.name))
            type = StoryGraphNode::Unreachable;
        else if (p.tags.contains("ending") || p.tags.contains("end"))
            type = StoryGraphNode::Ending;
        else if (p.choices.isEmpty())
            type = StoryGraphNode::DeadEnd;

        auto *node = new StoryGraphNode(p.name, type, p.choices.size());
        connect(node, &StoryGraphNode::clicked, this, &StoryGraph::passageClicked);
        m_scene->addItem(node);
        m_nodes[p.name] = node;

        if (p.name == startName) {
            node->setPos(0, 0);
        } else {
            double spread = qMax(200.0, qMin(600.0, count * 5.0));
            node->setPos((rand() % (int)(spread * 2)) - spread,
                         (rand() % (int)(spread * 2)) - spread);
        }
        count++;
    }

    // Create edges
    for (const auto &p : passages) {
        if (!m_nodes.contains(p.name)) continue;
        QSet<QString> seen;
        for (const auto &c : p.choices) {
            if (!m_nodes.contains(c.target)) continue;
            QString edgeKey = p.name + "->" + c.target;
            if (seen.contains(edgeKey)) continue;
            seen.insert(edgeKey);
            auto *edge = new StoryGraphEdge(m_nodes[p.name], m_nodes[c.target], c.text);
            m_scene->addItem(edge);
            m_edges.append(edge);
        }
    }

    if (!startName.isEmpty() && m_nodes.contains(startName))
        m_view->centerOn(m_nodes[startName]);
}

void StoryGraph::applyPhysics()
{
    QList<QString> keys = m_nodes.keys();
    int n = keys.size();

    for (int i = 0; i < n; ++i) {
        StoryGraphNode *n1 = m_nodes[keys[i]];
        QVector2D force(0, 0);
        for (int j = 0; j < n; ++j) {
            if (i == j) continue;
            StoryGraphNode *n2 = m_nodes[keys[j]];
            QVector2D d(n1->pos().x() - n2->pos().x(), n1->pos().y() - n2->pos().y());
            float dist2 = d.lengthSquared();
            if (dist2 > 0) { d.normalize(); force += d * float(kRepulsion / dist2); }
        }
        force += QVector2D(-n1->pos().x(), -n1->pos().y()) * 0.003f;
        n1->addForce(force);
    }

    for (StoryGraphEdge *edge : m_edges) {
        StoryGraphNode *a = edge->src(), *b = edge->dst();
        QVector2D d(b->pos().x() - a->pos().x(), b->pos().y() - a->pos().y());
        float len = d.length();
        if (len > 0) {
            float f = (len - float(kSpringRestLen)) * float(kSpringStiffness);
            d.normalize();
            a->addForce(d *  f);
            b->addForce(d * -f);
        }
    }

    for (StoryGraphNode *node : m_nodes) node->advancePosition();
    for (StoryGraphEdge *edge  : m_edges) edge->adjust();
}

// ── StoryGraphNode ────────────────────────────────────────────────────────────

StoryGraphNode::StoryGraphNode(const QString &name, NodeType type, int choiceCount)
    : m_name(name), m_type(type), m_choiceCount(choiceCount)
{
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setAcceptHoverEvents(true);
    setCacheMode(DeviceCoordinateCache);
    setZValue(1);
    float r = qBound(14.0f, 14.0f + choiceCount * 2.0f, 28.0f);
    setRect(-r, -r, r * 2, r * 2);
    setToolTip(name);
}

void StoryGraphNode::advancePosition()
{
    if (m_force.lengthSquared() < 0.001f && m_velocity.lengthSquared() < 0.001f) {
        m_force = QVector2D(0, 0); return;
    }
    m_velocity += m_force;
    m_velocity *= 0.85f;
    setPos(pos() + QPointF(m_velocity.x(), m_velocity.y()));
    m_force = QVector2D(0, 0);
}

void StoryGraphNode::setActive(bool a)
{
    m_active = a;
    update();
}

void StoryGraphNode::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsEllipseItem::mousePressEvent(event);
    emit clicked(m_name);
}

void StoryGraphNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor col = nodeColor(m_type);
    QRectF r = rect();

    if (m_active) {
        painter->setPen(QPen(QColor(255, 220, 50, 200), 3));
        painter->setBrush(Qt::NoBrush);
        painter->drawEllipse(r.adjusted(-5, -5, 5, 5));
        col = col.lighter(130);
    }

    // Glow
    QRadialGradient glow(r.center(), r.width() * 0.9);
    glow.setColorAt(0, QColor(col.red(), col.green(), col.blue(), 100));
    glow.setColorAt(1, QColor(col.red(), col.green(), col.blue(), 0));
    painter->setPen(Qt::NoPen);
    painter->setBrush(glow);
    painter->drawEllipse(r.adjusted(-6, -6, 6, 6));

    // Main dot
    painter->setBrush(col);
    painter->setPen(QPen(col.darker(150), 1));
    painter->drawEllipse(r);

    // Label
    painter->setPen(QColor("#cdd6f4"));
    QFont f = painter->font();
    f.setPointSize(m_active ? 9 : 8);
    if (m_active) f.setBold(true);
    painter->setFont(f);
    painter->drawText(QRectF(r.right() + 5, r.top() - 4, 200, r.height() + 8),
                      Qt::AlignLeft | Qt::AlignVCenter, m_name);
}

// ── StoryGraphEdge ────────────────────────────────────────────────────────────

StoryGraphEdge::StoryGraphEdge(StoryGraphNode *src, StoryGraphNode *dst, const QString &label)
    : m_src(src), m_dst(dst), m_label(label)
{
    setAcceptedMouseButtons(Qt::NoButton);
    setZValue(0);
    adjust();
}

void StoryGraphEdge::adjust()
{
    if (!m_src || !m_dst) return;
    setLine(QLineF(mapFromItem(m_src, 0, 0), mapFromItem(m_dst, 0, 0)));
}

void StoryGraphEdge::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_src || !m_dst) return;
    QLineF line(m_src->pos(), m_dst->pos());
    if (qFuzzyCompare(line.length(), 0.0)) return;

    // Arrow
    painter->setPen(QPen(QColor(98, 114, 164, 160), 1.5));
    painter->drawLine(line);

    // Arrowhead
    double angle = std::atan2(-line.dy(), line.dx());
    double arrowSize = 8.0;
    QPointF tip = line.p2();
    QPointF p1 = tip + QPointF(std::cos(angle + M_PI * 5/6) * arrowSize,
                               -std::sin(angle + M_PI * 5/6) * arrowSize);
    QPointF p2 = tip + QPointF(std::cos(angle - M_PI * 5/6) * arrowSize,
                               -std::sin(angle - M_PI * 5/6) * arrowSize);
    painter->setBrush(QColor(98, 114, 164, 160));
    painter->setPen(Qt::NoPen);
    painter->drawPolygon(QPolygonF() << tip << p1 << p2);

    // Edge label (truncated)
    if (!m_label.isEmpty() && line.length() > 60) {
        QPointF mid = (m_src->pos() + m_dst->pos()) / 2.0;
        painter->setPen(QColor(100, 120, 160, 200));
        QFont f = painter->font();
        f.setPointSize(7);
        painter->setFont(f);
        QString label = m_label.length() > 20 ? m_label.left(18) + "…" : m_label;
        painter->drawText(mid + QPointF(4, -4), label);
    }
}
