#include "nsAssociationItem_deletebreakpointcommand.h"
#include "../../graphics/items/associationitem.h"


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{
    DeleteBreakpointCommand::DeleteBreakpointCommand(AssociationItem *item, const QPointF &breakPoint, int index)
        : m_pItem(item), m_breakPoint(breakPoint), m_index(index)
    {
        
    }
    
    void DeleteBreakpointCommand::undo()
    {
        m_pItem->insertBreakpoint(m_breakPoint, m_index);
        /* Set the text for undo-view */
        setText(QObject::tr("Adding breakpoint"));
    }
    
    void DeleteBreakpointCommand::redo()
    {
        m_pItem->removeBreakpoint(m_index);
        /* Set the text for undo-view */
        setText(QObject::tr("Breakpoint removed"));
    }
}
