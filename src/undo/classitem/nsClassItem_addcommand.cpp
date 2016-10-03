#include "nsClassItem_addcommand.h"
#include "../../gui/mainwindow/mainwindow.h"
#include "../../project/projectmanagement.h"
#include "../../project/project.h"
#include "../../graphics/graphicsscene.h"
#include "../../graphics/items/classitem.h"
#include "../../gui/mainwindow/treewidget.h"
#include "../../graphics/items/associationitem.h"
#include <QTreeWidgetItem>



namespace nsClassItem
{
    AddCommand::AddCommand(Project *project, const QPoint &pos, ClassItem *item, QTreeWidgetItem *topLevelItem, TreeWidget *projectItems, MainWindow *mainWindow, QUndoCommand *parent) 
        : QUndoCommand(parent)
    {
        m_pProject = project;
        m_position = pos;
        m_pClassItem = item;
        m_pTopLevelItem = topLevelItem;
        m_pProjectItems = projectItems;
        m_pMainWindow = mainWindow;
    }
    
    AddCommand::~AddCommand()
    {
    }
    
    /*!
     * \brief This undo-function removes the new added item. */
    void AddCommand::undo()
    {
        /* Remove the association items, bounded with this class item */
        foreach (AssociationItem *assoItem, m_listAssociationItems)
             m_pProject->scene()->removeItem(assoItem);
        /* Remove the item from the scene */
        m_pProject->removeClassItem(m_pClassItem->className());
        m_pProject->scene()->update();
        
        /* Remove item from the list.
           PS: Removing item like this -> m_pTopLevelItem->removeChild(m_pTreeItem);
           is not safe. For some reason, it won't be deleted everytime. 
           Below is the safier: */
        /* This is the project we are looking for, now find the class item to rename it */
        for (int index = 0; index < m_pTopLevelItem->childCount(); ++index)
        {
            /* If this tree item is the one to be deleted */
            if (m_pTopLevelItem->child(index)->text(0) == m_pClassItem->className())
            {
                /* Remove the child item from the list */
                QTreeWidgetItem *witem = m_pTopLevelItem->child(index);
                m_pTopLevelItem->removeChild(witem);
                /* And delete the pointer, otherwise there will be memory leak */
                delete witem;
                /* No more dangling pointer */
                witem = NULL;
            }
        }
    }
    
    /*!
     * \brief This redo-function adds a new class item. */
    void AddCommand::redo()
    {
        /* Add the item into scene */
        m_pProject->scene()->addItem(m_pClassItem);
        /* Add the association items */
        foreach (AssociationItem *assoItem, m_listAssociationItems)
            m_pProject->scene()->addItem(assoItem);
        m_pProject->scene()->update();
        
        /* Create new treeitem for tree widget */
        QTreeWidgetItem *witem = new QTreeWidgetItem(QStringList() << m_pClassItem->className());
        /* Add the item */
        m_pTopLevelItem->addChild(witem);
        /* Set the text for undo-view */
        setText(QObject::tr("Add %1").arg(m_pClassItem->className()));
    }
}
