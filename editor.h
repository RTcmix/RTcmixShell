#ifndef EDITOR_H
#define EDITOR_H

#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QString;
class QWidget;
QT_END_NAMESPACE
class MainWindow;

class Editor : public QTextEdit
{
    Q_OBJECT

public:
    Editor(MainWindow *parent = 0);
    void setTabStopChars(int tabStopChars);

public slots:

protected:
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

private slots:
    void cursorPositionChanged();

private:
    MainWindow *parent;

signals:
    void loadFile(QString &);
};

#endif // EDITOR_H
