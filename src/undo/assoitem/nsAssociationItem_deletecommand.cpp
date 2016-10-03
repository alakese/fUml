#include "nsAssociationItem_deletecommand.h"
#include "../../graphics/items/associationitem.h"
#include "../../graphics/graphicsscene.h"
#include "../../graphics/items/classitem.h"

#include <QDebug>


namespace nsAssociationItem
{

    DeleteCommand::DeleteCommand(GraphicsScene *scene, AssociationItem *item)
        : m_scene(scene), m_item(item)
    {
        
    }
    
    /*!
     * \brief This undo-function removes the new added item. */
    void DeleteCommand::undo()
    {
        /* First connect signals again */
        QObject::connect(m_scene, &GraphicsScene::mousePressed, m_item, &AssociationItem::mousePressed);
        QObject::connect(m_scene, &GraphicsScene::mouseReleased, m_item, &AssociationItem::mouseReleased);
        QObject::connect(m_scene, &GraphicsScene::mouseMoved, m_item, &AssociationItem::mouseMoved);
        QObject::connect(m_item, &AssociationItem::deleteMe, m_scene, &GraphicsScene::deleteAssoItem);
        /* Add the item */
        m_scene->addItem(m_item);
        /* Call update */
        m_scene->update();
    }
    
    /*!
     * \brief This redo-function adds a new class item. */
    void DeleteCommand::redo()
    {
        //m_scene->deleteAssoItem(m_item);
        /* First disconnect signals, or else scene tries to send signals */
        QObject::disconnect(m_scene, &GraphicsScene::mousePressed, m_item, &AssociationItem::mousePressed);
        QObject::disconnect(m_scene, &GraphicsScene::mouseReleased, m_item, &AssociationItem::mouseReleased);
        QObject::disconnect(m_scene, &GraphicsScene::mouseMoved, m_item, &AssociationItem::mouseMoved);
        QObject::disconnect(m_item, &AssociationItem::deleteMe, m_scene, &GraphicsScene::deleteAssoItem);
        /* Remove */
        m_scene->removeItem(m_item);
        
        /* Set the text for undo-view */
        setText(QObject::tr("Deleting association"));
        /* Re-draw */
        m_scene->update();
    }

}
