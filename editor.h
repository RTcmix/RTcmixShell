#ifndef EDITOR_H
#define EDITOR_H

#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QString;
class QWidget;
QT_END_NAMESPACE

class Editor : public QTextEdit
{
    Q_OBJECT

public:
    Editor(QWidget *parent = 0);

public slots:

private slots:
    void cursorPositionChanged();

private:

};

#endif // EDITOR_H
