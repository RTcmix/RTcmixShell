#ifndef EDITOR_H
#define EDITOR_H

#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
class QString;
class QWidget;
QT_END_NAMESPACE
class Highlighter;
class MainWindow;
class Preferences;

class Editor : public QPlainTextEdit
{
    Q_OBJECT

public:
    Editor(MainWindow *parent = 0);

    void setTabStopChars(int tabStopChars);
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    Highlighter *getHighlighter() { return highlighter; }

protected:
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
    void resizeEvent(QResizeEvent *event) override;

public slots:
    void xableLineNumbers(bool);

private slots:
    void cursorPositionChanged();
    void updateLineNumberAreaWidth(int newBlockCount);
    void updateLineNumberArea(const QRect &, int);

private:
    MainWindow *parent;
    Highlighter *highlighter;
    QWidget *lineNumberArea;
    bool showLineNumbers;

    Preferences *editorPreferences;

signals:
    void loadFile(QString &);
};

// Line number code swiped from Qt example (BSD license), with just a few tweaks applied.
class LineNumberArea : public QWidget
{
public:
    LineNumberArea(Editor *theEditor) : QWidget(theEditor) {
        editor = theEditor;
    }

    QSize sizeHint() const override {
        return QSize(editor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        editor->lineNumberAreaPaintEvent(event);
    }

private:
    Editor *editor;
};

#endif // EDITOR_H
