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

    bool find(Editor *);
    bool findNext(Editor *);
    bool findPrevious(Editor *);
    void useSelectionForFind(Editor *);
    void replace(Editor *);
    bool replaceAndFind(Editor *);
    void replaceAll(Editor *);

private:
    QString searchString() { return findStringEdit->text(); }
    void setSearchString(const QString &str) { findStringEdit->setText(str); }

    QDialogButtonBox *findCancelButtonBox;
    QLineEdit *findStringEdit;
    QLineEdit *replaceStringEdit;
    QCheckBox *findPreviousCheckBox;
    QCheckBox *caseSensitiveCheckBox;
    QCheckBox *wholeWordsCheckBox;
};

#endif // FINDDIALOG_H
