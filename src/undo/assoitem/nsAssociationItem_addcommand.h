#ifndef ADDASSOCIATIONCOMMAND_H
#define ADDASSOCIATIONCOMMAND_H

#include <QUndoCommand>

class AssociationItem;
class GraphicsScene;

/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{

/*!
* \class The AddCommand class
*/
class AddCommand : public QUndoCommand
{
    GraphicsScene *m_scene;
    AssociationItem *m_item;
    
public:
    AddCommand(GraphicsScene *, AssociationItem *);
    void undo();
    void redo();
};

} // Namespace

#endif // ADDASSOCIATIONCOMMAND_H
