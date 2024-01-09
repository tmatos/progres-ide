#include "veriloghighlighter.h"

VerilogHighlighter::VerilogHighlighter(QTextDocument *doc)
    : QSyntaxHighlighter(doc)
{
    Rule r;

    QTextCharFormat keywordFmt;
    keywordFmt.setFontWeight(QFont::Bold);
    keywordFmt.setForeground(Qt::darkBlue);

    QRegularExpression keyworRegex(QStringLiteral("\\bmodule\\b|\\bendmodule\\b|\\bparameter\\b|") +
                                   QStringLiteral("\\binput\\b|\\boutput\\b|\\binout\\b|") +
                                   QStringLiteral("\\bwire\\b|\\breg\\b|\\bassign\\b|\\balways\\b|") +
                                   QStringLiteral("\\bbegin|\\bend\\b|\\bif|\\belse\\b|") +
                                   QStringLiteral("\\bor\\b|\\band\\b|\\bcase\\b|\\bendcase\\b") );

    r.pattern = keyworRegex;
    r.format = keywordFmt;
    rulesList.append(r);

    QTextCharFormat singleLineCommentFmt;
    singleLineCommentFmt.setForeground(Qt::gray);

    r.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    r.format = singleLineCommentFmt;
    rulesList.append(r);

    // TODO ...
}

void VerilogHighlighter::highlightBlock(const QString &text)
{
    for (const Rule &r : qAsConst(rulesList))
    {
        QRegularExpressionMatchIterator i = r.pattern.globalMatch(text);
        while (i.hasNext())
        {
            QRegularExpressionMatch match = i.next();
            setFormat(match.capturedStart(), match.capturedLength(), r.format);
        }
    }
}
