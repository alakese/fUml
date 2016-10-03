#ifndef NSASSOCIATIONITEM_MOVECOMMAND_H
#define NSASSOCIATIONITEM_MOVECOMMAND_H

#include <QUndoCommand>

class AssociationItem;


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{

/*!
 * \class The MoveCommand class
 */
class MoveCommand : public QUndoCommand
{
    enum { ID = 4 };
    AssociationItem *m_item;
    QList<QPointF> m_pointsOld;
    QList<QPointF> m_pointsNew;
    
public:
    MoveCommand(AssociationItem *);
    ~MoveCommand();
    int id() const;
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *);
};

}

#endif // NSASSOCIATIONITEM_MOVECOMMAND_H
