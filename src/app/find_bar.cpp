#include "find_bar.h"

#include <QHBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

FindBar::FindBar(QWidget *parent) : QWidget(parent) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 2, 10, 2);
    layout->setSpacing(10);

    findInput = new QLineEdit(this);
    findInput->setPlaceholderText("Find...");
    findInput->setStyleSheet("QLineEdit { background-color: #3c3c3c; color: "
                             "#cccccc; border: 1px solid #555555; padding: 2px "
                             "5px; border-radius: 2px; }");
    layout->addWidget(findInput);

    regexBox = new QCheckBox(".*", this);
    caseBox = new QCheckBox("Aa", this);
    wordBox = new QCheckBox("Word", this);
    selectionBox = new QCheckBox("Sel", this);
    regexBox->setToolTip("Regular expression");
    caseBox->setToolTip("Case sensitive");
    wordBox->setToolTip("Whole word");
    selectionBox->setToolTip("Search in selection");
    for (QCheckBox *box : {regexBox, caseBox, wordBox, selectionBox}) {
        box->setStyleSheet("QCheckBox { color:#aaa; font-size:10px; }");
        layout->addWidget(box);
        connect(box, &QCheckBox::toggled, this, &FindBar::optionsChanged);
    }

    matchLabel = new QLabel("0/0", this);
    matchLabel->setStyleSheet("color: #999999; font-size: 11px;");
    layout->addWidget(matchLabel);

    prevBtn = new QPushButton("\xE2\x86\x91", this);
    nextBtn = new QPushButton("\xE2\x86\x93", this);
    closeBtn = new QPushButton("\xE2\x9C\x95", this);

    QString btnStyle = "QPushButton { background-color: transparent; color: "
                       "#cccccc; border: none; padding: 2px 8px; font-size: "
                       "14px; } QPushButton:hover { background-color: #4a4a4a; "
                       "border-radius: 2px; }";
    prevBtn->setStyleSheet(btnStyle);
    nextBtn->setStyleSheet(btnStyle);
    closeBtn->setStyleSheet(
        btnStyle +
        " QPushButton:hover { background-color: #e81123; color: white; }");

    layout->addWidget(prevBtn);
    layout->addWidget(nextBtn);
    layout->addWidget(closeBtn);

    setFixedHeight(34);
    setStyleSheet("QWidget { background-color: #2d2d2d; border-bottom: 1px solid "
                  "#3e3e42; border-left: 1px solid #3e3e42; }");

    connect(findInput, &QLineEdit::textChanged, this, &FindBar::textChanged);
    connect(findInput, &QLineEdit::returnPressed, this,
            [this]() { emit findNextRequested(findInput->text()); });
    connect(prevBtn, &QPushButton::clicked, this,
            [this]() { emit findPreviousRequested(findInput->text()); });
    connect(nextBtn, &QPushButton::clicked, this,
            [this]() { emit findNextRequested(findInput->text()); });
    connect(closeBtn, &QPushButton::clicked, this, &FindBar::closeRequested);

    hide();
}

void FindBar::showAndFocus(const QString &text) {
    if (!text.isEmpty()) {
        findInput->setText(text);
    }
    show();
    findInput->setFocus();
    findInput->selectAll();
}

void FindBar::setMatchCount(int current, int total) {
    if (total == 0) {
        matchLabel->setText("No results");
    } else {
        matchLabel->setText(QString("%1/%2").arg(current).arg(total));
    }
}

QString FindBar::getSearchText() const {
    return findInput->text();
}

bool FindBar::isRegex() const { return regexBox->isChecked(); }
bool FindBar::isCaseSensitive() const { return caseBox->isChecked(); }
bool FindBar::isWholeWord() const { return wordBox->isChecked(); }
bool FindBar::isSelectionOnly() const { return selectionBox->isChecked(); }
