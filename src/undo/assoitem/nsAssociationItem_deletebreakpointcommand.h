#ifndef NSASSOCIATIONITEM_DELETEBREAKPOINTCOMMAND_H
#define NSASSOCIATIONITEM_DELETEBREAKPOINTCOMMAND_H

#include <QUndoCommand>
#include <QPointF>

class AssociationItem;


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{

/*!
 * \brief The DeleteBreakpointCommand class
 */
class DeleteBreakpointCommand : public QUndoCommand
{
    AssociationItem *m_pItem;
    QPointF m_breakPoint;
    int m_index;
    
public:
     DeleteBreakpointCommand(AssociationItem *, const QPointF &, int);
     void undo();
     void redo();
};

}

#endif // NSASSOCIATIONITEM_DELETEBREAKPOINTCOMMAND_H
