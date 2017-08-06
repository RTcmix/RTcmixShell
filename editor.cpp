
#include "editor.h"
#include "highlighter.h"
#include "utils.h"

Editor::Editor(QWidget *parent) : QTextEdit(parent)
{
    Highlighter *h = new Highlighter(document());
    Q_UNUSED(h);

    CHECKED_CONNECT(document(), &QTextDocument::modificationChanged, this, &QWidget::setWindowModified);
    CHECKED_CONNECT(this, &QTextEdit::cursorPositionChanged, this, &Editor::cursorPositionChanged);

    viewport()->setAcceptDrops(true);
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

