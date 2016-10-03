#ifndef NSCLASSITEM_DELETECOMMAND_H
#define NSCLASSITEM_DELETECOMMAND_H

#include <QUndoCommand>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
QT_END_NAMESPACE
class ClassItem;
class AssociationItem;
class Project;



/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{

/*!
 * \brief The DeleteCommand class
 */
class DeleteCommand : public QUndoCommand
{
    Project *m_pProject;
    ClassItem *m_pClassItem;
    QTreeWidgetItem *m_pTopLevelItem;
    QList<AssociationItem*> m_listAssociationItems;
    
public:
    DeleteCommand(Project *, ClassItem *, QTreeWidgetItem *, QUndoCommand *parent = 0);
    ~DeleteCommand();
    void undo();
    void redo();
};

} // Namespace

#endif // NSCLASSITEM_DELETECOMMAND_H
