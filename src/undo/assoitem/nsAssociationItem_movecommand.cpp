#include "nsAssociationItem_movecommand.h"
#include "../../graphics/items/associationitem.h"

#include <QDebug>



/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{
    MoveCommand::MoveCommand(AssociationItem *item) : m_item(item)
    {
        m_pointsOld = item->oldPoints();
        m_pointsNew = item->points();
    }
    
    MoveCommand::~MoveCommand()
    {
        
    }
    
    int MoveCommand::id() const
    {
        return ID;
    }
    
    void MoveCommand::undo()
    {
        /* Redo to new position */
        m_item->setPoints(m_pointsOld);
        /* Set the text for undo-view */
        setText(QObject::tr("Association moved back"));
    }
    
    void MoveCommand::redo()
    {
        /* Redo to new position */
        m_item->setPoints(m_pointsNew);
        /* Set the text for undo-view */
        setText(QObject::tr("Association moved"));
    }
    
    bool MoveCommand::mergeWith(const QUndoCommand *command)
    {
        /* Check, if the same type of commands */
        if (command->id() != ID)
            return false;
        
        /* Update the last set-position */
        m_pointsNew = m_item->points();
        m_item->setPoints(m_pointsNew);
        
        /* Set the text for undo-view */
        setText(QObject::tr("Association moved again"));
        
        /* If this line reached, then it is ok */
        return true;
    }
}
