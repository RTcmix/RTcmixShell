#include <QtWidgets>
#include <QFlags>
#include "editor.h"
#include "finddialog.h"
#include "utils.h"

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
{
    QPushButton *findButton = new QPushButton(tr("&Find"));
    findButton->setAutoDefault(true);
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    cancelButton->setDefault(false);
    findCancelButtonBox = new QDialogButtonBox();
    findCancelButtonBox->addButton(cancelButton, QDialogButtonBox::RejectRole);
    findCancelButtonBox->addButton(findButton, QDialogButtonBox::AcceptRole);
    CHECKED_CONNECT(findCancelButtonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    CHECKED_CONNECT(findCancelButtonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QLabel *findLabel = new QLabel(tr("Search for:"));
    QLabel *replaceLabel = new QLabel(tr("Replace with:"));
    findStringEdit = new QLineEdit;
    findStringEdit->setMinimumWidth(380);
    replaceStringEdit = new QLineEdit;
    findPreviousCheckBox = new QCheckBox(tr("Find Previous"));
    caseSensitiveCheckBox = new QCheckBox(tr("Case Sensitive"));
    wholeWordsCheckBox = new QCheckBox(tr("Whole Words Only"));

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(findLabel, 0, 0);
    gridLayout->addWidget(findStringEdit, 0, 1, 1, 3);
    gridLayout->addWidget(replaceLabel, 1, 0);
    gridLayout->addWidget(replaceStringEdit, 1, 1, 1, 3);
    gridLayout->addWidget(findPreviousCheckBox, 2, 1);
    gridLayout->addWidget(caseSensitiveCheckBox, 2, 2);
    gridLayout->addWidget(wholeWordsCheckBox, 2, 3);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(findCancelButtonBox);
    setLayout(mainLayout);
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    setWindowTitle(tr("Find"));
}

bool FindDialog::find(Editor *curEditor)
{
    QTextDocument::FindFlags flags;
    flags.setFlag(QTextDocument::FindBackward, findPreviousCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindCaseSensitively, caseSensitiveCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindWholeWords, wholeWordsCheckBox->isChecked());
    return curEditor->find(searchString(), flags);
}

bool FindDialog::findNext(Editor *curEditor)
{
//FIXME: should this affect state of Find Previous checkbox?
    QTextDocument::FindFlags flags;
    flags.setFlag(QTextDocument::FindCaseSensitively, caseSensitiveCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindWholeWords, wholeWordsCheckBox->isChecked());
    return curEditor->find(searchString(), flags);
}

bool FindDialog::findPrevious(Editor *curEditor)
{
//FIXME: should this affect state of Find Previous checkbox?
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    flags.setFlag(QTextDocument::FindCaseSensitively, caseSensitiveCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindWholeWords, wholeWordsCheckBox->isChecked());
    return curEditor->find(searchString(), flags);
}

void FindDialog::useSelectionForFind(Editor *curEditor)
{
    QString selStr = curEditor->textCursor().selectedText();
    setSearchString(selStr);
}

void FindDialog::replace(Editor *curEditor)
{
    QString selStr = curEditor->textCursor().selectedText();
    Qt::CaseSensitivity cs = (caseSensitiveCheckBox->isChecked() ? Qt::CaseSensitive : Qt::CaseInsensitive);
//FIXME: does not take into account whole words option
    if (QString::compare(selStr, searchString(), cs) == 0)
        curEditor->textCursor().insertText(replaceStringEdit->text());
}

bool FindDialog::replaceAndFind(Editor *curEditor)
{
    replace(curEditor);
    return find(curEditor);
}

void FindDialog::replaceAll(Editor *curEditor)
{
    curEditor->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);
    bool found = find(curEditor);
    while (found)
        found = replaceAndFind(curEditor);
}
