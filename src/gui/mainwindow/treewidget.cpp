#include "treewidget.h"
#include "mainwindow.h"
#include "../../graphics/items/classitem.h"
#include "../../gui/dialogs/headerinfodlg.h"
#include "../../project/projectmanagement.h"
#include "../../project/project.h"
#include "../../undo/classitem/nsClassItem_deletecommand.h"
#include <QMenu>
#include <QMouseEvent>

#include <QDebug>

/*
    TODO Show list item disabled, if the project is not selected.
    TODO Show the item as selected too, when the user selects the item in the scene?
*/
TreeWidget::TreeWidget(QWidget *parent) : QTreeWidget(parent)
{
    actionDeleteItem = new QAction(tr("Delete item"), this);
    actionRenameItem = new QAction(tr("Rename item"), this);
    actionShowOccurrence = new QAction(tr("Show occurence"), this);
    
    connect(actionDeleteItem, &QAction::triggered, this, &TreeWidget::deleteItem);
    connect(actionRenameItem, &QAction::triggered, this, &TreeWidget::renameItem);
    connect(actionShowOccurrence, &QAction::triggered, this, &TreeWidget::showOccurence);
    connect(this, &TreeWidget::currentItemChanged, this, &TreeWidget::checkForProjectChange);
}

void TreeWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->matches(QKeySequence::Delete))
        deleteItem();
}

void TreeWidget::contextMenuEvent(QContextMenuEvent *event)
{
    Q_UNUSED(event);
    
    /* No recursive search, we want only project names */
    QList<QTreeWidgetItem*> items = findItems(currentItem()->text(COLUMN_ZERO), Qt::MatchExactly);
    /* There must be only one item, if found - each project unique name */
    if (items.count() == 1)
        return;
    
    QMenu menu;
    menu.addAction(actionRenameItem);
    menu.addAction(actionDeleteItem);
    menu.addAction(actionShowOccurrence);
    menu.exec(QCursor::pos());
}

/*!
 * \brief This functions sets the name of the active project to its parameter, if a new project has been created. */
void TreeWidget::setActiveProject(const QString &strName)
{
    m_strActiveProjectName = strName;
}

/*!
 * \brief This function puts a star near a project name, if there is a change in it. */
void TreeWidget::setProjectDirty(const QString &projectName)
{
    QList<QTreeWidgetItem*> items;
    /* No recursive search, we want only project names */
    items = findItems(projectName, Qt::MatchExactly);
    /* There must be only one item, if found - each project unique name */
    if (items.count() == 1)
    {
        items.at(0)->setText(COLUMN_ZERO, QString("%1[*]").arg(projectName));
    }
    else
    {
        /* Set or de-set the star */
        items = findItems(QString("%1[*]").arg(projectName), Qt::MatchExactly);
        if (items.count() == 1)
            items.at(0)->setText(COLUMN_ZERO, projectName);
    }
}

/*!
 * \brief This function changes an item's name after the signal ClassItem::classNameChanged.
 * When the user changes the item name on the scene via double click or context menu, the name
 * must be updated in tree widget to.
 * PS : if only its project is active (this will be ensured by the system).
 * See TreeWidget for more information. */
void TreeWidget::changeItemName(const QString &oldName, const QString &newName)
{
    /* Get active project : PS we are sure that only the items can be renamed, if their project is active */
    QString projectName = ProjectManager::getInstance()->getActiveProjectName();
    
    /* For each projects in the tree list */
    for (int index = 0; index < invisibleRootItem()->childCount(); ++index)
    {
        /* Get project info */
        QTreeWidgetItem * project = invisibleRootItem()->child(index);
        /* Remove always the star, in order to compare them */
        if (project->text(COLUMN_ZERO).remove("[*]") == projectName.remove("[*]"))
            /* This is the project we were looking for, now find the class item to rename it */
            for (int subIndex = 0; subIndex < project->childCount(); ++subIndex)
                /* If this tree item has the old name */
                if (project->child(subIndex)->text(COLUMN_ZERO) == oldName)
                    /* Then change it */
                    project->child(subIndex)->setText(COLUMN_ZERO, newName);
    }
}

/*!
 * \brief This function removes an item from the treewidget, if the user removes the item from the scene first.
 * Scene (class Project) will inform treewidget. */
void TreeWidget::removeItem(const QString &name)
{
    QString projectName = ProjectManager::getInstance()->getActiveProjectName();
    /* Get the project index */
    for (int index = 0; index < invisibleRootItem()->childCount(); ++index)
    {
        QTreeWidgetItem *project = invisibleRootItem()->child(index);
        /* Remove always the star, in order to compare them */
        if (project->text(COLUMN_ZERO).remove("[*]") == projectName.remove("[*]"))
        {
            /* This is the project we are looking for, now find the class item to rename it */
            for (int subIndex = 0; subIndex < project->childCount(); ++subIndex)
            {
                /* If this tree item is the one to be deleted */
                if (project->child(subIndex)->text(COLUMN_ZERO) == name)
                {
                    //topLevelItem(index)->removeChild(project->child(subIndex));
                    /* Delete it */
                    ClassItem *item = ProjectManager::getInstance()->getActiveProject()->getClassItem(name);
                    /* Create the DeleteCommand object */
                    nsClassItem::DeleteCommand *delCmd = new nsClassItem::DeleteCommand(ProjectManager::getInstance()->getActiveProject(), 
                                                                                        item, topLevelItem(index));
                    /* Push it to undo-command */
                    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(delCmd);
                    /* Found, deleted and finish */
                    return;
                }
            }
        }
    }
}

/*!
 * \brief This function deletes an item from the treewidget and the scene, if the user removes the item from treewidget. */
void TreeWidget::deleteItem()
{
    /* User can delete an selected item, if its project is active */
    QString projectNameInTree = currentItem()->parent()->text(COLUMN_ZERO);
    QString activeProjectName = ProjectManager::getInstance()->getActiveProjectName();
    if (projectNameInTree.remove("[*]") != activeProjectName.remove("[*]"))
    {
        qDebug() << "WARNING : Item can only be deleted, only if its project is active!";
        return;
    }
    
    /* Remove the item via undo-framework */
    QString classItem = currentItem()->text(COLUMN_ZERO);
    removeItem(classItem);
}

/*!
 * \brief This function renames an item's name, if the given name is acceptable. Then informs the scene
 * to update the name in the view too. For other direction (renaming in scene) see changeItemName() */
void TreeWidget::renameItem()
{
    /* This is the old name, if the item can be renamed */
    QString oldName = currentItem()->text(COLUMN_ZERO);
    
    /* User can rename a selected item, if its project is active */
    QString projectNameInTree = currentItem()->parent()->text(COLUMN_ZERO);
    QString activeProjectName = ProjectManager::getInstance()->getActiveProjectName();
    if (projectNameInTree.remove("[*]") != activeProjectName.remove("[*]"))
    {
        qDebug() << "WARNING : Item can only be renamed, if its project is active!";
        return;
    }
    
    /* Now the user can rename the item, get the old name */
    HeaderInfoDlg dlg(oldName);
    if (dlg.exec() == QDialog::Accepted) 
    {
        /* Get the new name */
        QString newName = dlg.className();
        /* Change the name in tree-list */
        emit itemRenamed(oldName, newName);
    }
}

/*!
 * \brief This function shows where is the item in the scene. */
void TreeWidget::showOccurence()
{
    /* User can select an item in the scene, if its project is active */
    QString projectNameInTree = currentItem()->parent()->text(COLUMN_ZERO);
    QString activeProjectName = ProjectManager::getInstance()->getActiveProjectName();
    if (projectNameInTree.remove("[*]") != activeProjectName.remove("[*]"))
    {
        qDebug() << "WARNING : Item can only be deleted, only if its project is active!";
        return;
    }
    
    /* Select the item in the scene */
    ProjectManager::getInstance()->getActiveProject()->selectItem(currentItem()->text(COLUMN_ZERO));
}

/*!
 * \brief This function monitors the project list for the active project item. */
void TreeWidget::checkForProjectChange()
{
    QString projName = currentItem()->text(COLUMN_ZERO);
    if (!ProjectManager::getInstance()->isThisAProjectName(projName))
        return;

    /* Compare to last active name */
    if (projName != m_strActiveProjectName)
    {
        /* Now this item is the active project */
        m_strActiveProjectName = projName;
        /* Now we know project has changed : send a signal for ProjectManagement to update the active project pointer */
        emit activeProjectChanged(projName);
    }
}
