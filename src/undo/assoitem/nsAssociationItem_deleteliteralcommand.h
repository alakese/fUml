#ifndef NSASSOCIATIONITEM_DELETELITERALCOMMAND_H
#define NSASSOCIATIONITEM_DELETELITERALCOMMAND_H

#include <QUndoCommand>

class LiteralItem;


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{

class DeleteLiteralCommand : public QUndoCommand
{
    LiteralItem *m_pItem;
    QString m_text;
    
public:
     DeleteLiteralCommand(LiteralItem *);
     void undo();
     void redo();
};

} // Namespace

#endif // NSASSOCIATIONITEM_DELETELITERALCOMMAND_H
