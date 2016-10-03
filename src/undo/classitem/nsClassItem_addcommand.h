#ifndef NSCLASSITEM_ADDCOMMAND_H
#define NSCLASSITEM_ADDCOMMAND_H

#include <QUndoCommand>
#include <QPoint>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
QT_END_NAMESPACE
class ClassItem;
class Project;
class TreeWidget;
class MainWindow;
class AssociationItem;



/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{

/*!
 * \brief The AddCommand class
 */
class AddCommand : public QUndoCommand
{
    Project *m_pProject;
    QPoint m_position;
    ClassItem *m_pClassItem;
    QTreeWidgetItem *m_pTopLevelItem;
    TreeWidget *m_pProjectItems;
    MainWindow *m_pMainWindow;
    QList<AssociationItem*> m_listAssociationItems;
    
public:
    AddCommand(Project *, const QPoint &, ClassItem *, QTreeWidgetItem *, TreeWidget *, MainWindow *, QUndoCommand *parent = 0);
    ~AddCommand();
    void undo();
    void redo();
};

} // Namespace

#endif // NSCLASSITEM_ADDCOMMAND_H
