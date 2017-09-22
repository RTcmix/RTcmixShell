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
#ifdef NOTYET
    gridLayout->addWidget(replaceLabel, 1, 0);
    gridLayout->addWidget(replaceStringEdit, 1, 1, 1, 3);
    gridLayout->addWidget(findPreviousCheckBox, 2, 1);
    gridLayout->addWidget(caseSensitiveCheckBox, 2, 2);
    gridLayout->addWidget(wholeWordsCheckBox, 2, 3);
#else
    gridLayout->addWidget(findPreviousCheckBox, 1, 1);
    gridLayout->addWidget(caseSensitiveCheckBox, 1, 2);
    gridLayout->addWidget(wholeWordsCheckBox, 1, 3);
#endif

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(findCancelButtonBox);
    setLayout(mainLayout);
    this->layout()->setSizeConstraint(QLayout::SetFixedSize);

    setWindowTitle(tr("Find"));
}

void FindDialog::find(Editor *curEditor)
{
    QTextDocument::FindFlags flags = 0;
    flags.setFlag(QTextDocument::FindBackward, findPreviousCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindCaseSensitively, caseSensitiveCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindWholeWords, wholeWordsCheckBox->isChecked());
    curEditor->find(searchString(), flags);
}

void FindDialog::findNext(Editor *curEditor)
{
    QTextDocument::FindFlags flags = 0;
    flags.setFlag(QTextDocument::FindCaseSensitively, caseSensitiveCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindWholeWords, wholeWordsCheckBox->isChecked());
    curEditor->find(searchString(), flags);
}

void FindDialog::findPrevious(Editor *curEditor)
{
    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    flags.setFlag(QTextDocument::FindCaseSensitively, caseSensitiveCheckBox->isChecked());
    flags.setFlag(QTextDocument::FindWholeWords, wholeWordsCheckBox->isChecked());
    curEditor->find(searchString(), flags);
}
