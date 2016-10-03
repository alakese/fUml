#include "nsClassItem_movecommand.h"
#include "../../graphics/items/classitem.h"


/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{
    MoveCommand::MoveCommand(ClassItem *pClassItem, QUndoCommand *parent)     
        : QUndoCommand(parent)
    {
        m_pClassItem = pClassItem;
        m_ptOldPosition = pClassItem->oldPosition().toPoint();
        m_ptNewPosition = pClassItem->pos().toPoint();
    }
    
    MoveCommand::~MoveCommand()
    {
    }
    
    /*!
     * \brief This function will be used in mergeWith() to get the other-command's class item. */
    ClassItem *MoveCommand::classItem() const
    {
        return m_pClassItem;
    }
    
    void MoveCommand::undo()
    {
        /* Undo to old position */
        m_pClassItem->setPos(m_ptOldPosition);
        /* Tell asso-item's, that class item moved undo */
        emit m_pClassItem->itemMoving(m_pClassItem);
        /* Set the text for undo-view */
        setText(QObject::tr("Move %1 to (%2, %3)").arg(m_pClassItem->className()).arg(m_pClassItem->pos().x()).arg(m_pClassItem->pos().y()));
    }
    
    void MoveCommand::redo()
    {
        /* Redo to new position */
        m_pClassItem->setPos(m_ptNewPosition);
        /* Tell asso-item's, that class item moved redo */
        emit m_pClassItem->itemMoving(m_pClassItem);
        /* Set the text for undo-view */
        setText(QObject::tr("Move %1 to (%2, %3)").arg(m_pClassItem->className()).arg(m_pClassItem->pos().x()).arg(m_pClassItem->pos().y()));
    }
    
    /*!
     * \brief Merges the commands. If the same item will be moved more than once, then the last position will be kept in this command.
     * No need for different commands, if the same item will be moved. */
    bool MoveCommand::mergeWith(const QUndoCommand *command)
    {
        /* Check, if the same type of commands */
        if (command->id() != ID)
            return false;
        
        /* Check, if not the same command s*/
        const MoveCommand *newCmd = static_cast<const MoveCommand*>(command);
        if (m_pClassItem->className() != newCmd->classItem()->className())
            return false;
        
        /* Update the last set-position */
        m_pClassItem->setPos(newCmd->classItem()->pos());
        
        /* Set the text for undo-view */
        setText(QObject::tr("Move %1 to (%2, %3)").arg(m_pClassItem->className()).arg(m_pClassItem->pos().x()).arg(m_pClassItem->pos().y()));
        
        /* If this line reached, then it is ok */
        return true;
    }
    
    /*!
     * \brief This function returns the ID of this command.. Is neccessary for commands which has mergeWith() - Property/Function. 
     * A command ID is used in command compression. It must be an integer unique to this command's class, or -1 if the command doesn't support compression.
     * If the command supports compression this function must be overridden in the derived class to return the correct ID. The base implementation returns -1.
     * QUndoStack::push() will only try to merge two commands if they have the same ID, and the ID is not -1. See documentation. */
    int MoveCommand::id() const
    {
        return ID;
    }
}
