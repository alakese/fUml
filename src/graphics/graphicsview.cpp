#include "graphicsview.h"
#include "items/classitem.h"
#include "../project/projectmanagement.h"
#include "../project/project.h"
#include <QMouseEvent>
#include <QGraphicsItem>


GraphicsView::GraphicsView(QWidget *parent)
    : QGraphicsView(parent)
{
    setMouseTracking(true);
    m_bAddingClassItem = false;
}

/*!
 * \brief This event sends the cursor position in view-coordinates.
 * PS : View and scene coordinates are same : Beginning with (0, 0) */
void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    if (m_bAddingClassItem)
    {
        QString defaultClassName = "ClassName";
        emit mouseViewPressed(defaultClassName, event->pos());
        m_bAddingClassItem = false;
    }
    
    QGraphicsView::mousePressEvent(event);
}

/*!
 * \brief This function will be called from the MainWindow before and after adding an class item. */
void GraphicsView::setAddingItem(bool value)
{
    m_bAddingClassItem = value;
    if (value)
    {
        /* Now an item will be added - cursor is cross */
        viewport()->setCursor(Qt::CrossCursor);
        /* Set the focus to graphview */
        this->setFocus();
    }
    else
    {
        /* Set cursor back */
        viewport()->setCursor(Qt::ArrowCursor);
        
        /* Set toolbar back */
        emit escapePressed();
        
        /* If there is an item added, and if not updated, the item will not be drawn complete, because the view setting 
         * :setViewportUpdateMode can not be set to FullViewportUpdate, else the movement of items with gridlines is very slow. */
        viewport()->update();
    }
}

/*!
 * \brief In this function : 
 * * Qt::Key_Delete removes an item from the scene and treewidget using signal classItemDeleted() */
void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Delete)
    {
        QList<QGraphicsItem *> items = scene()->selectedItems();
        foreach(QGraphicsItem *item, items)
        {
            if (ClassItem *ci = dynamic_cast<ClassItem*>(item))
            {
                /* Remove the item from the scene */
                ProjectManager::getInstance()->getActiveProject()->removeClassItem(ci->className());
                /* Tell treewidget to remove it */
                emit classItemDeleted(ci->className());
            }
        }
    }
    
    if(event->key() == Qt::Key_Escape)
    {
        /* Can reset the cursor */
        setAddingItem(false);
    }
}
