#include "nsClassItem_changefontcommand.h"
#include "../../graphics/items/classitem.h"


/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{
    ChangeFontCommand::ChangeFontCommand(ClassItem *pClassItem, QUndoCommand *parent)
        : QUndoCommand(parent)
    {
        m_pClassItem = pClassItem;
        m_newFont = pClassItem->font();
        m_oldFont = pClassItem->oldFont();
    }
    
    ChangeFontCommand::~ChangeFontCommand()
    {
    }
    
    void ChangeFontCommand::undo()
    {
        m_pClassItem->setFont(m_oldFont);
        
        /* Set the text for undo-view */
        setText(QObject::tr("Change font for %1 to %2").arg(m_pClassItem->className()).arg(m_pClassItem->font().toString()));
    }
    
    void ChangeFontCommand::redo()
    {
        m_pClassItem->setFont(m_newFont);
        
        /* Set the text for undo-view */
        setText(QObject::tr("Change font for %1 to %2").arg(m_pClassItem->className()).arg(m_pClassItem->font().toString()));
    }
    
    /*!
     * \brief This function will be used in mergeWith() to get the other-command's class item. */
    ClassItem *ChangeFontCommand::classItem() const
    {
        return m_pClassItem;
    }
    
    /*!
     * \brief Merges the commands. If the same font will be changed more than once, then the last font will be kept in this command.
     * No need for different commands, if the same item will be chenged. */
    bool ChangeFontCommand::mergeWith(const QUndoCommand *command)
    {
        /* Check, if the same type of commands */
        if (command->id() != ID)
            return false;
        
        /* Check, if not the same command s*/
        const ChangeFontCommand *newCmd = static_cast<const ChangeFontCommand*>(command);
        if (m_pClassItem->className() != newCmd->classItem()->className())
            return false;
        
        m_pClassItem->setFont(newCmd->font());
        
        /* Set the text for undo-view */
        setText(QObject::tr("Change font for %1 to %2").arg(m_pClassItem->className()).arg(newCmd->font().toString()));
        
        /* If this line reached, then it is ok */
        return true;
    }
    
    int ChangeFontCommand::id() const
    {
        return ID;
    }
    
    QFont ChangeFontCommand::font() const
    {
        return m_newFont;
    }
}

