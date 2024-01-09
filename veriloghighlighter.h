#ifndef VERILOGHIGHLIGHTER_H
#define VERILOGHIGHLIGHTER_H

#include <QObject>
#include <QWidget>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QRegularExpression>
#include <QTextCharFormat>

class VerilogHighlighter : public QSyntaxHighlighter
{

public:
    VerilogHighlighter(QTextDocument *doc = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    struct Rule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QList<Rule> rulesList;
};

#endif // VERILOGHIGHLIGHTER_H
