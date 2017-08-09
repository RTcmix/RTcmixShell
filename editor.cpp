#include <QtDebug>
#include <QFileInfo>
#include <QMimeData>
#include <QMimeDatabase>

#include "editor.h"
#include "highlighter.h"
#include "mainwindow.h"
#include "utils.h"

Editor::Editor(MainWindow *parent) : QTextEdit(parent), parent(parent)
{
    Highlighter *h = new Highlighter(document());
    Q_UNUSED(h);

    CHECKED_CONNECT(document(), &QTextDocument::modificationChanged, this, &QWidget::setWindowModified);
    CHECKED_CONNECT(this, &QTextEdit::cursorPositionChanged, this, &Editor::cursorPositionChanged);
    CHECKED_CONNECT(this, &Editor::loadFile, parent, &MainWindow::loadFile);

    setAcceptDrops(true);
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
        QTextEdit::dropEvent(event);
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

