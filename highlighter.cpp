/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtDebug>

#include "highlighter.h"
#include "preferences.h"

Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    // This syncs with the MainWindow-owned settings, even though it's a different object.
    syntaxHighlightingPreferences = new Preferences();

    HighlightingRule rule;

    // 1. reserved key words
    keywordFormat.setForeground(syntaxHighlightingPreferences->editorReservedColor());
//    keywordFormat.setFontWeight(QFont::Bold);
    QStringList keywordPatterns;
    keywordPatterns << "\\bfloat\\b" << "\\bif\\b" << "\\belse\\b" << "\\return\\b" << "\\bstring\\b"
                    << "\\bfor\\b" << "\\bwhile\\b" << "\\binclude\\b" << "\\bhandle\\b" << "\\blist\\b"
                    << "\\btrue\\b" << "\\bfalse\\b" << "\\bTRUE\\b" << "\\bFALSE\\b";
    foreach (const QString &pattern, keywordPatterns) {
        rule.type = ReservedRule;
        rule.format = keywordFormat;
        rule.pattern = QRegularExpression(pattern);
        highlightingRules.append(rule);
    }

    // 2. numbers
    numberFormat.setForeground(syntaxHighlightingPreferences->editorNumberColor());
    rule.type = NumberRule;
    rule.format = numberFormat;
    rule.pattern = QRegularExpression("\\.?\\b\\d+\\.?\\d?\\b\\.?");
    highlightingRules.append(rule);

    // 3. double-quoted strings
    quotationFormat.setForeground(syntaxHighlightingPreferences->editorStringColor());
    rule.type = StringRule;
    rule.format = quotationFormat;
    rule.pattern = QRegularExpression("\".*\"");
    highlightingRules.append(rule);

    // 4. functions, including instruments (i.e., anything except keywords that are followed by '(')
//    functionFormat.setFontItalic(true);
    functionFormat.setForeground(syntaxHighlightingPreferences->editorFunctionColor());
    rule.type = FunctionRule;
    rule.format = functionFormat;
    rule.pattern = QRegularExpression("\\b[A-Za-z0-9_]+(?=\\()");
    highlightingRules.append(rule);

    // 5. RTcmix commands that don't function in this environment, currently:
    //    rtsetparams, rtoutput, load
    unusedFuncsFormat.setForeground(syntaxHighlightingPreferences->editorUnusedColor());
    rule.type = UnusedRule;
    rule.format = unusedFuncsFormat;
    rule.pattern = QRegularExpression("\\s*rtsetparams\\s*\\(.*\\).*");
    highlightingRules.append(rule);
//    qDebug() << "rule.pattern" << rule.pattern;
    rule.pattern = QRegularExpression("\\s*rtoutput\\s*\\(.*\\).*");
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression("\\s*load\\s*\\(.*\\).*");
    highlightingRules.append(rule);

    // 6. C++-style comments
    singleLineCommentFormat.setForeground(syntaxHighlightingPreferences->editorCommentColor());
    rule.type = CommentRule;
    rule.format = singleLineCommentFormat;
    rule.pattern = QRegularExpression("//[^\n]*");
    highlightingRules.append(rule);

    // 7. shell-style comments (beginning with '#')
    hashLineCommentFormat.setForeground(syntaxHighlightingPreferences->editorCommentColor());
    rule.type = CommentRule;
    rule.format = hashLineCommentFormat;
    // FIXME: avoid matching pitch letter format, e.g., "G#4" -- still doesn't work yet
    rule.pattern = QRegularExpression("[^ABCDEFG]*#[^\n]*");
    highlightingRules.append(rule);

    // 8. C-style multiline comments
    multiLineCommentFormat.setForeground(syntaxHighlightingPreferences->editorCommentColor());
    commentStartExpression = QRegularExpression("/\\*");
    commentEndExpression = QRegularExpression("\\*/");

    rulesActive = syntaxHighlightingPreferences->editorDoSyntaxHighlighting();
}

void Highlighter::setRuleColor(SyntaxHighlighterRule ruleType, QColor color)
{
//    qDebug() << "setRuleColor:  ruleType:" << ruleType << "color:" << color;
    bool rehighlight = false;
    int size = highlightingRules.size();
    for (int i = 0; i < size; i++) {
        if (highlightingRules[i].type == ruleType) {
            QTextCharFormat newFormat;
            newFormat.setForeground(color);
            highlightingRules[i].format = newFormat;
            rehighlight = true;
//            qDebug() << "   type:" << highlightingRules[i].type << " foreground:" << highlightingRules[i].format;
        }
    }
    if (rehighlight)
        this->rehighlight();
}

void Highlighter::highlightBlock(const QString &text)
{
    if (rulesActive) {
        foreach (const HighlightingRule &rule, highlightingRules) {
            QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
            while (matchIterator.hasNext()) {
                QRegularExpressionMatch match = matchIterator.next();
                setFormat(match.capturedStart(), match.capturedLength(), rule.format);
            }
        }
        setCurrentBlockState(0);

        int startIndex = 0;
        if (previousBlockState() != 1)
            startIndex = text.indexOf(commentStartExpression);

        while (startIndex >= 0) {
            QRegularExpressionMatch match = commentEndExpression.match(text, startIndex);
            int endIndex = match.capturedStart();
            int commentLength = 0;
            if (endIndex == -1) {
                setCurrentBlockState(1);
                commentLength = text.length() - startIndex;
            } else {
                commentLength = endIndex - startIndex
                                + match.capturedLength();
            }
            setFormat(startIndex, commentLength, multiLineCommentFormat);
            startIndex = text.indexOf(commentStartExpression, startIndex + commentLength);
        }
    }
}

void Highlighter::dumpRules()
{
    qDebug("\n******* printing highlighting rules *******");
    QVectorIterator<HighlightingRule> it(highlightingRules);
    while (it.hasNext()) {
        HighlightingRule h = it.next();
        qDebug() << "type:" << h.type << "pattern:" << h.pattern << "\n   foreground:" << h.format.foreground();
    }
}
