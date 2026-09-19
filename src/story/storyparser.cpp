#include "storyparser.h"
#include <QRegularExpression>
#include <QStringList>
#include <QQueue>

// ── Parse ─────────────────────────────────────────────────────────────────────

QVector<StoryPassage> StoryParser::parse(const QString &text)
{
    QVector<StoryPassage> passages;
    QStringList lines = text.split('\n');

    // Passage header: :: Name  or  :: Name [tag1 tag2]
    static QRegularExpression headerRe(R"(^::\s*([^\[]+?)(?:\s*\[([^\]]*)\])?\s*$)");
    // Choice link: [[text->target]] or [[text -> target]] or [[target]]
    static QRegularExpression choiceRe(R"(\[\[(?:([^\]]+?)\s*->\s*)?([^\]]+?)\]\])");

    StoryPassage *current = nullptr;
    QStringList contentLines;

    auto flushCurrent = [&]() {
        if (!current) return;
        // Join content, parse choices
        QString content = contentLines.join('\n').trimmed();
        current->content = content;

        // Extract choices from content
        auto it = choiceRe.globalMatch(content);
        while (it.hasNext()) {
            auto m = it.next();
            StoryChoice c;
            c.text   = m.captured(1).trimmed(); // may be empty if [[target]] form
            c.target = m.captured(2).trimmed();
            if (c.text.isEmpty()) c.text = c.target;
            current->choices.append(c);
        }
        passages.append(*current);
        delete current;
        current = nullptr;
        contentLines.clear();
    };

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];
        auto m = headerRe.match(line);
        if (m.hasMatch()) {
            flushCurrent();
            current = new StoryPassage();
            current->name = m.captured(1).trimmed();
            current->lineNumber = i;
            if (!m.captured(2).isEmpty())
                current->tags = m.captured(2).split(' ', Qt::SkipEmptyParts);
        } else if (current) {
            contentLines.append(line);
        }
    }
    flushCurrent();

    return passages;
}

// ── Variables ─────────────────────────────────────────────────────────────────

QMap<QString, StoryVariable> StoryParser::extractVariables(const QString &text)
{
    QMap<QString, StoryVariable> vars;
    QStringList lines = text.split('\n');

    // $var = value  (set)
    static QRegularExpression setRe(R"(\$(\w+)\s*=\s*([^\n]+))");
    // $var used in expression (read)
    static QRegularExpression readRe(R"(\$(\w+))");

    for (int i = 0; i < lines.size(); ++i) {
        const QString &line = lines[i];

        auto setIt = setRe.globalMatch(line);
        while (setIt.hasNext()) {
            auto m = setIt.next();
            QString name = m.captured(1);
            if (!vars.contains(name)) {
                StoryVariable v;
                v.name = name;
                v.defaultValue = m.captured(2).trimmed();
                vars[name] = v;
            }
            vars[name].setLines.append(i);
        }

        auto readIt = readRe.globalMatch(line);
        while (readIt.hasNext()) {
            auto m = readIt.next();
            QString name = m.captured(1);
            if (!vars.contains(name)) {
                StoryVariable v;
                v.name = name;
                vars[name] = v;
            }
            if (!vars[name].setLines.contains(i))
                vars[name].readLines.append(i);
        }
    }
    return vars;
}

// ── Lint ──────────────────────────────────────────────────────────────────────

QVector<StoryLint> StoryParser::lint(const QVector<StoryPassage> &passages,
                                      const QMap<QString, StoryVariable> &vars)
{
    QVector<StoryLint> issues;

    // Build name -> passage map
    QMap<QString, const StoryPassage*> byName;
    for (const auto &p : passages)
        byName[p.name] = &p;

    // Find reachable passages via BFS from start
    QString start = findStartPassage(passages);
    QSet<QString> reachable;
    QQueue<QString> queue;
    if (!start.isEmpty()) {
        queue.enqueue(start);
        reachable.insert(start);
    }
    while (!queue.isEmpty()) {
        QString cur = queue.dequeue();
        if (!byName.contains(cur)) continue;
        for (const auto &choice : byName[cur]->choices) {
            if (!reachable.contains(choice.target)) {
                reachable.insert(choice.target);
                queue.enqueue(choice.target);
            }
        }
    }

    for (const auto &p : passages) {
        // Unreachable passage
        if (!reachable.contains(p.name) && p.name != start) {
            issues.append({StoryLint::Warning, p.lineNumber, p.name,
                           "Passage '" + p.name + "' is unreachable — no links point to it"});
        }

        // Dead end (no choices and not tagged as ending)
        bool isEnding = p.tags.contains("ending") || p.tags.contains("end");
        if (p.choices.isEmpty() && !isEnding) {
            issues.append({StoryLint::Warning, p.lineNumber, p.name,
                           "Passage '" + p.name + "' is a dead end — add [[choices]] or tag as [ending]"});
        }

        // Broken links (choice targets that don't exist)
        for (const auto &c : p.choices) {
            if (!byName.contains(c.target)) {
                issues.append({StoryLint::Error, p.lineNumber, p.name,
                               "Broken link: [[" + c.text + " -> " + c.target + "]] — passage '" + c.target + "' does not exist"});
            }
        }

        // Very long passage (reader fatigue)
        int words = p.content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
        if (words > 500) {
            issues.append({StoryLint::Warning, p.lineNumber, p.name,
                           "Passage '" + p.name + "' is very long (" + QString::number(words) + " words) — consider splitting"});
        }
    }

    // Undefined variables (read before set)
    for (auto it = vars.begin(); it != vars.end(); ++it) {
        const StoryVariable &v = it.value();
        if (v.setLines.isEmpty() && !v.readLines.isEmpty()) {
            issues.append({StoryLint::Warning, v.readLines.first(), "",
                           "Variable $" + v.name + " is read but never set"});
        }
    }

    return issues;
}

// ── Stats ─────────────────────────────────────────────────────────────────────

StoryParser::Stats StoryParser::computeStats(const QVector<StoryPassage> &passages)
{
    Stats s;
    s.passageCount = passages.size();

    QMap<QString, const StoryPassage*> byName;
    for (const auto &p : passages) {
        byName[p.name] = &p;
        int words = p.content.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
        s.wordCount += words;
        s.choiceCount += p.choices.size();
        if (p.choices.isEmpty() && !p.tags.contains("ending") && !p.tags.contains("end"))
            s.deadEndCount++;
    }

    // Unreachable count
    QString start = findStartPassage(passages);
    QSet<QString> reachable;
    QQueue<QString> queue;
    if (!start.isEmpty()) { queue.enqueue(start); reachable.insert(start); }
    while (!queue.isEmpty()) {
        QString cur = queue.dequeue();
        if (!byName.contains(cur)) continue;
        for (const auto &c : byName[cur]->choices) {
            if (byName.contains(c.target) && !reachable.contains(c.target)) {
                reachable.insert(c.target);
                queue.enqueue(c.target);
            }
        }
    }
    s.unreachableCount = passages.size() - reachable.size();

    if (s.passageCount > 0)
        s.avgChoicesPerPassage = double(s.choiceCount) / s.passageCount;

    // Estimated read time at 200 WPM
    s.estimatedReadTimeSecs = int((s.wordCount / 200.0) * 60);

    // Longest path (BFS with depth tracking, cap at 1000 to avoid infinite loops)
    if (!start.isEmpty()) {
        QQueue<QPair<QString,int>> bfsQ;
        bfsQ.enqueue({start, 1});
        QSet<QString> visited;
        visited.insert(start);
        int maxDepth = 1;
        int steps = 0;
        while (!bfsQ.isEmpty() && steps++ < 1000) {
            auto [cur, depth] = bfsQ.dequeue();
            maxDepth = qMax(maxDepth, depth);
            if (!byName.contains(cur)) continue;
            for (const auto &c : byName[cur]->choices) {
                if (!visited.contains(c.target)) {
                    visited.insert(c.target);
                    bfsQ.enqueue({c.target, depth + 1});
                }
            }
        }
        s.longestPathLength = maxDepth;
    }

    return s;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

QString StoryParser::findStartPassage(const QVector<StoryPassage> &passages)
{
    if (passages.isEmpty()) return {};
    for (const auto &p : passages)
        if (p.name.toLower() == "start") return p.name;
    return passages.first().name;
}
