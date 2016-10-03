#ifndef NSASSOCIATIONITEM_ADDBREAKPOINTCOMMAND_H
#define NSASSOCIATIONITEM_ADDBREAKPOINTCOMMAND_H

#include <QUndoCommand>
#include <QPointF>

class AssociationItem;


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{

/*!
 * \brief The AddBreakpointCommand class
 */
class AddBreakpointCommand : public QUndoCommand
{
    AssociationItem *m_pItem;
    QPointF m_breakPoint;
    int m_index;
    
public:
     AddBreakpointCommand(AssociationItem *, const QPointF &, int);
     void undo();
     void redo();
};

} // Namespace

#endif // NSASSOCIATIONITEM_ADDBREAKPOINTCOMMAND_H
