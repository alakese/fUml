#ifndef WORDHIGHLIGHTER_H
#define WORDHIGHLIGHTER_H

#include <QSyntaxHighlighter>

/*!
 * \class WordHighlighter
 * \brief The keywords will be highlighted. They will be shown in colors. 
*/
class WordHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT
    
public:
    explicit WordHighlighter(QTextDocument *parent = 0);

protected:
    void highlightBlock(const QString &text);
    
private:
    enum ITEM_TYPE { COMMAND = 1, ITEM, SUB_COMMAND, OTHERS };
    QStringList listCommands;
    QStringList listItems;
    QStringList listSubCommands;
};

#endif // WORDHIGHLIGHTER_H
