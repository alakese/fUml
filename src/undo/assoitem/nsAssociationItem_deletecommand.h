#ifndef NSASSOCIATIONITEM_DELETECOMMAND_H
#define NSASSOCIATIONITEM_DELETECOMMAND_H

#include <QUndoCommand>

class AssociationItem;
class GraphicsScene;

/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{

/*!
* \class The DeleteCommand class
*/
class DeleteCommand : public QUndoCommand
{
    GraphicsScene *m_scene;
    AssociationItem *m_item;
    
public:
    DeleteCommand(GraphicsScene *, AssociationItem *);
    void undo();
    void redo();
};

} // Namespace

#endif // NSASSOCIATIONITEM_DELETECOMMAND_H
