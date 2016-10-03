#include "nsAssociationItem_addbreakpointcommand.h"
#include "../../graphics/items/associationitem.h"


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{
    AddBreakpointCommand::AddBreakpointCommand(AssociationItem *item, const QPointF &breakPoint, int index)
        : m_pItem(item), m_breakPoint(breakPoint), m_index(index)
    {
        
    }
    
    void AddBreakpointCommand::undo()
    {
        m_pItem->removeBreakpoint(m_index);
        /* Set the text for undo-view */
        setText(QObject::tr("Breakpoint removed"));
    }
    
    void AddBreakpointCommand::redo()
    {
        m_pItem->insertBreakpoint(m_breakPoint, m_index);
        /* Set the text for undo-view */
        setText(QObject::tr("Adding breakpoint"));
    }
}
