#include <QtDebug>
#include <QFileInfo>
#include <QMimeData>
#include <QMimeDatabase>
#include <QPainter>

#include "editor.h"
#include "highlighter.h"
#include "mainwindow.h"
#include "preferences.h"
#include "utils.h"


Editor::Editor(MainWindow *parent) : QPlainTextEdit(parent), parent(parent)
{
    // This syncs with the MainWindow-owned settings, even though it's a different object.
    editorPreferences = new Preferences();

    showLineNumbers = editorPreferences->editorShowLineNumbers();
    lineNumberArea = new LineNumberArea(this);

    highlighter = new Highlighter(document());

    CHECKED_CONNECT(this, &Editor::blockCountChanged, this, &Editor::updateLineNumberAreaWidth);
    CHECKED_CONNECT(this, &QPlainTextEdit::updateRequest, this, &Editor::updateLineNumberArea);
    CHECKED_CONNECT(document(), &QTextDocument::modificationChanged, this, &QWidget::setWindowModified);
    CHECKED_CONNECT(this, &QPlainTextEdit::cursorPositionChanged, this, &Editor::cursorPositionChanged);
    CHECKED_CONNECT(this, &Editor::loadFile, parent, &MainWindow::fileOpenNoDialog);

    updateLineNumberAreaWidth(0);

    setAcceptDrops(true);
}

void Editor::xableLineNumbers(bool checked)
{
    showLineNumbers = checked;
    editorPreferences->setEditorShowLineNumbers(checked);
    emit blockCountChanged(0);
}

int Editor::lineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10) {
        max /= 10;
        digits++;
    }
    int space = 2 + (fontMetrics().width(QLatin1Char('9')) * (digits + 1));     // JG tweak

    return space;
}

void Editor::updateLineNumberAreaWidth(int /* newBlockCount */)
{
    if (showLineNumbers)
        setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
    else
        setViewportMargins(0, 0, 0, 0);
}

void Editor::updateLineNumberArea(const QRect &rect, int dy)
{
    if (showLineNumbers) {
        if (dy)
            lineNumberArea->scroll(0, dy);
        else
            lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

        if (rect.contains(viewport()->rect()))
            updateLineNumberAreaWidth(0);
    }
}

void Editor::resizeEvent(QResizeEvent *e)
{
    QPlainTextEdit::resizeEvent(e);

    int width = 0;
    if (showLineNumbers)
        width = lineNumberAreaWidth();
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), width, cr.height()));
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent *event)
{
    if (showLineNumbers) {
        QPainter painter(lineNumberArea);
        painter.fillRect(event->rect(), Qt::darkGray);

        QTextBlock block = firstVisibleBlock();
        int blockNumber = block.blockNumber();
        int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
        int bottom = top + (int) blockBoundingRect(block).height();

        while (block.isValid() && top <= event->rect().bottom()) {
            if (block.isVisible() && bottom >= event->rect().top()) {
                QString number = QString::number(blockNumber + 1);
                painter.setPen(Qt::white);
                int margin = fontMetrics().height() / 4;    // JG addition
                painter.drawText(0, top, lineNumberArea->width() - margin, fontMetrics().height(), Qt::AlignRight, number);
            }

            block = block.next();
            top = bottom;
            bottom = top + (int) blockBoundingRect(block).height();
            blockNumber++;
        }
    }
}

void Editor::dragEnterEvent(QDragEnterEvent *event)
{
    const QMimeData *md = event->mimeData();
    if (md->hasText()
            || md->hasFormat("text/plain")
            || md->hasFormat("audio/wav")
            || md->hasFormat("audio/x-aiff")) {
        event->acceptProposedAction();
    }
    if (md->hasUrls())
        event->acceptProposedAction();
}

// code drawn from https://wiki.qt.io/Drag_and_Drop_of_files
void Editor::dropEvent(QDropEvent *event)
{
    bool dropped = false;
    const QMimeData *md = event->mimeData();

    if (md->hasUrls()) {
        QList<QUrl> urlList = md->urls();
#ifdef NOTYET
        // could be useful for future tabbed multidoc editor (for .sco files)
        for (int i = 0; i < urlList.size() && i < 32; i++) {
            QString *path = urlList.at(i).toLocalFile();
        }
#else
        QString path = urlList.at(0).toLocalFile();
        QFileInfo fileInfo(path);
        QMimeDatabase db;                          // NB only for Qt5
        QMimeType type = db.mimeTypeForFile(path);
        if (type.name() == "text/plain" || path.endsWith(".sco")) {
            dropped = true;
            emit loadFile(path);
        }
        else if (type.name() == "audio/x-wav" || type.name() == "audio/x-aiff" || fileInfo.isDir()) {
            QString text = path;
            text.prepend("\"");
            text.append("\"");
            QTextCursor cursor = this->textCursor();
            cursor.insertText(text);
            dropped = true;
        }
#endif
    }
    else if (md->hasText()) {
        QTextCursor cursor = this->textCursor();
        cursor.insertText(event->mimeData()->text());
        dropped = true;
    }

    if (dropped) {
        // If we don't do the following three lines, the cursor will be frozen
        // and unblinking. Found this in a post by "baohaojun" (5/5/17):
        // http://www.qtcentre.org/archive/index.php/t-16935.html
        this->setReadOnly(true);
        QPlainTextEdit::dropEvent(event);
        this->setReadOnly(false);
    }
}

// Do this after setting a new font or size.
void Editor::setTabStopChars(int tabStopChars)
{
    //TODO: make this a preference
    QString spaces; // more accurate to measure a string of tabStop spaces, instead of one space
    for (int i = 0; i < tabStopChars; i++)
        spaces += " ";
    QFontMetrics metrics(font());
    setTabStopWidth(metrics.width(spaces));
}

void Editor::cursorPositionChanged()
{
}

