#ifndef JIM_SEARCH_EVERYWHERE_H
#define JIM_SEARCH_EVERYWHERE_H

#include <QDialog>
#include <QStringList>

class QAction;
class QEvent;
class QKeyEvent;
class QLineEdit;
class QListWidget;

class SearchEverywhere : public QDialog {
    Q_OBJECT
public:
    struct Symbol {
        QString name;
        QString filePath;
        int line = 0;
    };

    explicit SearchEverywhere(QWidget *parent = nullptr);
    void populate(const QList<QAction*> &actions,
                  const QStringList &recentFiles,
                  const QStringList &openFiles,
                  const QList<Symbol> &symbols = {},
                  bool commandsOnly = false);

signals:
    void fileRequested(const QString &filePath);
    void symbolRequested(const QString &filePath, int line);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    QLineEdit *searchBox;
    QListWidget *resultList;
    QList<QAction*> allActions;
    QStringList allFiles;
    QStringList allRecent;
    QList<Symbol> allSymbols;
    bool commandsOnly = false;

    void filter(const QString &text);
    void runSelected();
};

#endif
