#ifndef NSCLASSITEM_MOVECOMMAND_H
#define NSCLASSITEM_MOVECOMMAND_H

#include <QUndoCommand>
#include <QPoint>

class ClassItem;



/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{

/*!
 * \brief The MoveCommand class
 */
class MoveCommand : public QUndoCommand
{
    enum { ID = 1 };
    ClassItem *m_pClassItem;
    QPoint m_ptOldPosition;
    QPoint m_ptNewPosition;
    
public:
    MoveCommand(ClassItem *, QUndoCommand *parent = 0);
    ~MoveCommand();
    int id() const;
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *);
    ClassItem *classItem() const;    
};

}

#endif // NSCLASSITEM_MOVECOMMAND_H
