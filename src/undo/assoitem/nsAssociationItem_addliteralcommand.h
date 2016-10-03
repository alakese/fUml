#ifndef NSASSOCIATIONITEM_ADDLITERAL_H
#define NSASSOCIATIONITEM_ADDLITERAL_H

#include <QUndoCommand>

class LiteralItem;


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{

class AddLiteralCommand : public QUndoCommand
{
    LiteralItem *m_pItem;
    QString m_text;
    
public:
     AddLiteralCommand(LiteralItem *, const QString &);
     void undo();
     void redo();
};

} // Namespace

#endif // NSASSOCIATIONITEM_ADDLITERAL_H
