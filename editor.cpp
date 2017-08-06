
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

void Editor::cursorPositionChanged()
{
}

