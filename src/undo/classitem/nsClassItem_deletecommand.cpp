#include "nsClassItem_deletecommand.h"
#include "../../project/projectmanagement.h"
#include "../../project/project.h"
#include "../../graphics/graphicsscene.h"
#include "../../graphics/items/classitem.h"
#include "../../graphics/items/associationitem.h"
#include <QTreeWidgetItem>

#include <QDebug>



/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{
    DeleteCommand::DeleteCommand(Project *project, ClassItem *item, QTreeWidgetItem *topLevelItem, QUndoCommand *parent)
        : QUndoCommand(parent)
    {
        m_pProject = project;
        m_pClassItem = item;
        m_pTopLevelItem = topLevelItem;
       
        /* Add the association items, bounded with this class item */
        QList<QGraphicsItem*> allItems = m_pProject->scene()->items();
        QListIterator<QGraphicsItem*> iter(allItems);
        while (iter.hasNext())
        if (AssociationItem *assoItem = dynamic_cast<AssociationItem*>(iter.next()))
            if (assoItem->associatedWithThisClass(m_pClassItem))
               m_listAssociationItems.append(assoItem);
    }
    
    DeleteCommand::~DeleteCommand()
    {
        // TODO here disconnect all signals?    
    }
    
    /*!
     * \brief This undo-function adds the deleted item again */
    void DeleteCommand::undo()
    {
        /* Add the class item into scene */
        m_pProject->scene()->addItem(m_pClassItem);
        
        /* Add the association items */
        foreach (AssociationItem *assoItem, m_listAssociationItems)
        {        
            /* First connect signals again */
            QObject::connect(m_pProject->scene(), &GraphicsScene::mousePressed, assoItem, &AssociationItem::mousePressed);
            QObject::connect(m_pProject->scene(), &GraphicsScene::mouseReleased, assoItem, &AssociationItem::mouseReleased);
            QObject::connect(m_pProject->scene(), &GraphicsScene::mouseMoved, assoItem, &AssociationItem::mouseMoved);
            QObject::connect(assoItem, &AssociationItem::deleteMe, m_pProject->scene(), &GraphicsScene::deleteAssoItem);
            m_pProject->scene()->addItem(assoItem);
        }
        m_pProject->scene()->update();
       
//        /* Remove the class item from the delete-list, to not to remove it anymore, if save pressed. */
//        ProjectManager::getInstance()->getActiveProject()->removeFromDeleteList(m_pClassItem);
        /* Create new treeitem for tree widget */
        QTreeWidgetItem *witem = new QTreeWidgetItem(QStringList() << m_pClassItem->className());    
        /* Add the item to tree widget */
        m_pTopLevelItem->addChild(witem);
    }
    
    /*!
     * \brief This redo-function deletes an item.  */
    void DeleteCommand::redo()
    {
       /* Remove the association items, bounded with this class item */
       foreach (AssociationItem *assoItem, m_listAssociationItems)
       {   
            /* First disconnect signals, or else scene tries to send signals */
            QObject::disconnect(m_pProject->scene(), &GraphicsScene::mousePressed, assoItem, &AssociationItem::mousePressed);
            QObject::disconnect(m_pProject->scene(), &GraphicsScene::mouseReleased, assoItem, &AssociationItem::mouseReleased);
            QObject::disconnect(m_pProject->scene(), &GraphicsScene::mouseMoved, assoItem, &AssociationItem::mouseMoved);
            QObject::disconnect(assoItem, &AssociationItem::deleteMe, m_pProject->scene(), &GraphicsScene::deleteAssoItem);
            m_pProject->scene()->removeItem(assoItem);
       }

       /* Remove the item from the scene */
       m_pProject->removeClassItem(m_pClassItem->className());
       
       m_pProject->scene()->update();
       
       /* Remove item from the list.
          PS: Removing item like this ->  m_pTopLevelItem->removeChild(m_pTreeItem);
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
     
       /* Set the text for undo-view */
       setText(QObject::tr("Delete %1").arg(m_pClassItem->className()));
    }
}
