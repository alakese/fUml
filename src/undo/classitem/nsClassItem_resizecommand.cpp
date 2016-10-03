#include "nsClassItem_resizecommand.h"
#include "../../graphics/graphicsscene.h"
#include "../../graphics/items/classitem.h"


/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{
    ResizeCommand::ResizeCommand(ClassItem *pClassItem, QUndoCommand *parent)     
        : QUndoCommand(parent)
    {
        m_pClassItem = pClassItem;
        m_oldWidth = pClassItem->oldWidth();
        m_oldHeightHeader = pClassItem->oldHeightHeader();
        m_oldHeightMembers = pClassItem->oldHeightMembers();    
        m_oldHeightFunctions = pClassItem->oldHeightFunctions();
        m_newWidth = pClassItem->rectHeader().width();
        m_newHeightHeader = pClassItem->rectHeader().height();
        m_newHeightMembers = pClassItem->rectMembers().height();    
        m_newHeightFunctions = pClassItem->rectFunctions().height();
    }
    
    ResizeCommand::~ResizeCommand()
    {
    }
    
    /*!
     * \brief This function will be used in mergeWith() to get the other-command's class item. */
    ClassItem *ResizeCommand::classItem() const
    {
        return m_pClassItem;
    }
    
    void ResizeCommand::undo()
    {
        /* Undo to old size */
        m_pClassItem->setRectHeader(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), m_oldWidth, m_oldHeightHeader));
        m_pClassItem->setRectMembers(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), m_oldWidth, m_oldHeightMembers));
        m_pClassItem->setRectFunctions(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), m_oldWidth, m_oldHeightFunctions));
        
        /* Tell asso-item's, that class item moved undo */
        emit m_pClassItem->itemResizing(m_pClassItem);
        
        /* Set the text for undo-view */
        setText(QObject::tr("Resize %1 to (w, h):(%2, %3)").arg(m_pClassItem->className()).arg(m_pClassItem->boundingRect().width())
                .arg(m_pClassItem->boundingRect().height()));
    }
    
    void ResizeCommand::redo()
    {
        /* Redo to new size */
        m_pClassItem->setRectHeader(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), m_newWidth, m_newHeightHeader));
        m_pClassItem->setRectMembers(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), m_newWidth, m_newHeightMembers));
        m_pClassItem->setRectFunctions(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), m_newWidth, m_newHeightFunctions));

        /* Tell asso-item's, that class item moved undo */
        emit m_pClassItem->itemResizing(m_pClassItem);
        
        /* Set the text for undo-view */
        setText(QObject::tr("Resize %1 to (w, h):(%2, %3)").arg(m_pClassItem->className()).arg(m_pClassItem->boundingRect().width())
                .arg(m_pClassItem->boundingRect().height()));
    }
    
    /*!
     * \brief Merges the commands. If the same item will be resizes more than once, then the last size will be kept in this command.
     * No need for different commands, if the same item will be resized. */
    bool ResizeCommand::mergeWith(const QUndoCommand *command)
    {
        /* Check, if the same type of commands */
        if (command->id() != ID)
            return false;
        
        /* Check, if not the same command s*/
        const ResizeCommand *newCmd = static_cast<const ResizeCommand*>(command);
        if (m_pClassItem->className() != newCmd->classItem()->className())
            return false;
        
        /* Update the last size-info */
        m_pClassItem->setRectHeader(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), 
                                          newCmd->classItem()->rectHeader().width(), newCmd->classItem()->rectHeader().height()));
        m_pClassItem->setRectMembers(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(), 
                                           newCmd->classItem()->rectHeader().width(), newCmd->classItem()->rectMembers().height()));
        m_pClassItem->setRectFunctions(QRect(m_pClassItem->rectHeader().x(), m_pClassItem->rectHeader().y(),
                                             newCmd->classItem()->rectHeader().width(), newCmd->classItem()->rectFunctions().height()));

        /* Tell asso-item's, that class item moved undo */
        emit m_pClassItem->itemResizing(m_pClassItem);
        
        /* Set the text for undo-view */
        setText(QObject::tr("Resize %1 to (w, h):(%2, %3)").arg(m_pClassItem->className()).arg(m_pClassItem->boundingRect().width())
                .arg(m_pClassItem->boundingRect().height()));
        
        /* If this line reached, then it is ok */
        return true;
    }
    
    int ResizeCommand::id() const
    {
        return ID;
    }
}
