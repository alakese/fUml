#include "wordhighlighter.h"
#include <QStringList>

#include <QDebug>


WordHighlighter::WordHighlighter(QTextDocument *parent) : 
    QSyntaxHighlighter(parent)
{
    // TODO bunlar codewindowda da var - optimize
    listCommands << "create" << "delete" << "move" << "rename" << "resize";
    listItems << "class";
    listSubCommands << "position" << "size" << "to";
}

void WordHighlighter::highlightBlock(const QString &text)
{
    qDebug() << "highlightblock " << text;
    
    /* Store the text */
    QString currentLine(text);
    /* Remove the first char ">" */
    currentLine.remove(0, 1);
    /* Get the line (from uder) and split it into words */
    QStringList currentLineWords = currentLine.split(QRegExp("\\s"));

    /* For each entered word check if it is a special key-word */
    foreach (const QString &oneWord, currentLineWords)
    {
        ITEM_TYPE itemType = OTHERS;
        
        /* Find the type of the string */
        if (listCommands.contains(oneWord))
            itemType = COMMAND;
        else if (listItems.contains(oneWord))
            itemType = ITEM;
        else if (listSubCommands.contains(oneWord))
            itemType = SUB_COMMAND;
       
        /* Set the format */
        QTextCharFormat format;
        /* Find the index of the word matched exact */
        QRegExp regex = QRegExp(QString("\\b%1\\b").arg(oneWord));
        int index = regex.indexIn(currentLine);
        /* Add the ">" char to the index calculation */
        index = index + 1;
        /* Set the color of the word */
        switch(itemType)
        {
            case COMMAND:
                format.setForeground(Qt::darkRed);
                break;
            case ITEM:
                format.setForeground(Qt::darkBlue);
                break;
            case SUB_COMMAND:
                format.setForeground(Qt::darkGreen);
                break;
             case OTHERS:
                format.setForeground(Qt::black);
                break;
        }
        /* Set the format : from index to the length of the word */
        setFormat(index, oneWord.length(), format);
    }
}
