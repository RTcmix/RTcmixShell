#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <QLineEdit>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QDialogButtonBox;
class QString;
QT_END_NAMESPACE
class Editor;

class FindDialog : public QDialog
{
    Q_OBJECT

public:
    FindDialog(QWidget *parent = 0);
    void find(Editor *);
    void findNext(Editor *);
    void findPrevious(Editor *);
    void setSearchString(const QString &str) { findStringEdit->setText(str); }

private:
    QString searchString() { return findStringEdit->text(); }

    QDialogButtonBox *findCancelButtonBox;
    QLineEdit *findStringEdit;
    QLineEdit *replaceStringEdit;
    QCheckBox *findPreviousCheckBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordsCheckBox;
};

#endif // FINDDIALOG_H
