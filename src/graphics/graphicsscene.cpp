#include "graphicsscene.h"
#include "items/classitem.h"
#include "items/associationitem.h"
#include "../gui/mainwindow/mainwindow.h"
#include "../undo/assoitem/nsAssociationItem_addcommand.h"
#include "../undo/assoitem/nsAssociationItem_deletecommand.h"
#include "../project/project.h"
#include "../project/projectmanagement.h"
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsLineItem>
#include <QGraphicsItem>
#include <QList>

#include <QDebug>

//TODO load
// TODO test db
// TODO delete a class item +/* asso item
// TODO code entry
// TODO point selection not strong + add breakpoint, position it on anywhere, try to move the point on the class item
// TODO toolbar selection and escape : for example select classitem then asso
// TODO options for application : e.g. snap grid etc
// TODO snap grid for classitem length and asso item movement
// TODO 1 font uzunliugunu ekle 2 role ile mul arasini ac 3 literalleri hareket ettit +  reset ekle 4 font ekle + degistir
// TODO diagonalde asso secilince ve lbutt basiliyken mouse hareket ettirilince literaller yeniden hesaplaniyor hesaplama sadece move olursa

GraphicsScene::GraphicsScene(QObject *parent) :
    QGraphicsScene(parent)
{
    m_assoItemTmp = NULL;
    isAssociationTriggered = false;
    isAssociationCreated = false;
}

/*!
 * \brief If the user cancels creating an association by pressing the escape, the shown line(s) will be removed.
 * The asso item will be inserted in to the scene first, to draw it while moving the mouse. If in this moment 
 * the user presses the escape button, then the asso-item must be deleted.
 * Alternative : could be painting the item without adding it. */
void GraphicsScene::addItemCanceled()
{
    /* Toolbar is pressed and the item not created fully */
    if (isAssociationTriggered && !isAssociationCreated)
        if (m_assoItemTmp != NULL)
        {
            removeItem(m_assoItemTmp);
            delete m_assoItemTmp;
            m_assoItemTmp = NULL;
            isAssociationCreated = false;
            isAssociationTriggered = false;
        
            /* Asso-Item drawn, now the items may be moved again */
            QList<QGraphicsItem*> items = this->items();
            QListIterator<QGraphicsItem*> i(items);
            while (i.hasNext())
                if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
                    item->setMoveResize(true);
        }
}

/*!
 * \brief If the user presses the toolbar icon in mainwindow, the scene must know this, in order to begin drawing the association item. */
void GraphicsScene::addItemTriggered()
{
    isAssociationTriggered = true;
    
    /* Asso-Item is being drawing, the class items may not move at this point */
    QList<QGraphicsItem*> items = this->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            item->setMoveResize(false);
}

/*!
 * \brief This function deletes the asso-item, which sends its kill signal to scene. */
void GraphicsScene::deleteAssoItem(AssociationItem *item)
{
    /* Create the DeleteCommand object */
    nsAssociationItem::DeleteCommand *delCmd = new nsAssociationItem::DeleteCommand(this, item);
    /* Push it to undo-command */
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(delCmd);
    
//    /* First disconnect signals, or else scene tries to send signals */
//    disconnect(this, &GraphicsScene::mousePressed, item, &AssociationItem::mousePressed);
//    disconnect(this, &GraphicsScene::mouseReleased, item, &AssociationItem::mouseReleased);
//    disconnect(this, &GraphicsScene::mouseMoved, item, &AssociationItem::mouseMoved);
//    disconnect(item, &AssociationItem::deleteMe, this, &GraphicsScene::deleteAssoItem);
//    /* Remove */
//    removeItem(item);
//    /* Re-draw */
//    update();
}

/*!
 * \brief This function draws the association between two class items. */
void GraphicsScene::drawAssociationConnection()
{
    /* Now connecting two items : calculate the beginning and end point by calculating the two class items position */
    QPointF fromPoint(m_begPoint);
    QPointF toPoint(m_endPoint);
    
    ClassItem *item1 = m_assoItemTmp->getItemBegin();
    ClassItem *item2 = m_assoItemTmp->getItemEnd();

    /* First four cases : items are in diagonal position to each other */
    if (item1->getTopLeftPos().x() > item2->getBottomRightPos().x() && item1->getTopLeftPos().y() > item2->getBottomRightPos().y())
    {
        /* item2 TopLeft to item1*/
        fromPoint = item1->getTopLeftPos();
        toPoint = item2->getBottomRightPos();
    }
    else if (item1->getTopRightPos().x() < item2->getBottomLeftPos().x() && item1->getTopRightPos().y() > item2->getBottomLeftPos().y())
    {
        /* item2 TopRight to item1*/
        fromPoint = item1->getTopRightPos();
        toPoint = item2->getBottomLeftPos();
    }
    else if (item1->getBottomRightPos().x() < item2->getTopLeftPos().x() && item1->getBottomRightPos().y() < item2->getTopLeftPos().y())
    {
        /* item2 BottomRight to item1 */
        fromPoint = item1->getBottomRightPos();
        toPoint = item2->getTopLeftPos();
    }
    else if (item1->getBottomLeftPos().x() > item2->getTopRightPos().x() && item1->getBottomLeftPos().y() < item2->getTopRightPos().y())
    {
        /* item2 BottomLeft to item1 */
        fromPoint = item1->getBottomLeftPos();
        toPoint = item2->getTopRightPos();
    }
    /* Now decide if the class items are horizontal or vertical to each other */
    else if (item1->getTopLeftPos().x() > item2->getBottomRightPos().x())
    {
        /* Item1 is on the left side */
        toPoint.setX(item2->pos().x()+item2->boundingRect().width());
        toPoint.setY(fromPoint.y());
    }
    else if (item1->getBottomRightPos().x() < item2->getTopLeftPos().x())
    {
        /* Item1 is on the right side */
        toPoint.setX(item2->pos().x());
        toPoint.setY(fromPoint.y());
    }
    else if (item1->getBottomLeftPos().y() < item2->getTopLeftPos().y())
    {
        /* Item1 is above item2 */
        toPoint.setX(fromPoint.x());
        toPoint.setY(item2->pos().y());
    }
    else if (item1->getTopLeftPos().y() > item2->getBottomLeftPos().y())
    {
        /* Item1 is below item2 */
        toPoint.setX(fromPoint.x());
        toPoint.setY(item2->pos().y() + item2->boundingRect().height());
    }
    
    /* Now save the points */
    m_assoItemTmp->setFirstPoint(fromPoint);
    m_assoItemTmp->setLastPoint(toPoint);
    /* Calculate the new positions of the literals, if there are any */
    m_assoItemTmp->moveLiterals();
    
    /* update */
    update();
    m_assoItemTmp->update();
    
    /* Asso-Item drawn, now the items may be moved again */
    QList<QGraphicsItem*> items = this->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            item->setMoveResize(true);
    
    /* Tell that item is placed successfully */
    m_assoItemTmp->itemCreatedFully();
}

/*!
 * \brief This function draws the association line from a class item to a given mouse point
 * * originPoint where the line starts
 * * toPoint target point; to mouse pointer */
void GraphicsScene::drawAssociationMoving(const QPointF &toPoint)
{
    /* From class item*/
    ClassItem *item = m_assoItemTmp->getItemBegin();
    /* Define points for readability */
    QPointF topLeft(item->pos().x(), item->pos().y());
    QPointF bottomLeft(item->pos().x(), item->pos().y()+item->boundingRect().height());
    QPointF topRight(item->pos().x()+item->boundingRect().width(), item->pos().y());
    QPointF bottomRight(item->pos().x()+item->boundingRect().width(), item->pos().y()+item->boundingRect().height());
    
    /* Origin point is the begining point of the line. This is on the border-line of the item.
     * Depends on, where the mouse pointer is.
     * PS : Initializing with scene pos => if the mouse pointer moves over the item, there will be one point, not a line */
    QPointF originPoint(toPoint);
    
    if (toPoint.y() >= topLeft.y() && toPoint.y() <= bottomRight.y())
    {
        /* If mouse pointer moves on the right side of the item */
        if (toPoint.x() >= bottomRight.x())
            originPoint.setX(bottomRight.x());
        /* or if mouse pointer moves on the left side of the item */
        else if (toPoint.x() <= topLeft.x())
            originPoint.setX(topLeft.x());

        originPoint.setY(toPoint.y());
    }
    else if (toPoint.x() >= topLeft.x() && toPoint.x() <= bottomRight.x())
    {
        /* If mouse pointer moves above the item */
        if (toPoint.y() <= topLeft.y())
            originPoint.setY(topLeft.y());
        /* or if mouse pointer moves below the item */
        else if (toPoint.y() >= bottomRight.y())
            originPoint.setY(bottomRight.y());
        
        originPoint.setX(toPoint.x());
    }
    /* At this point mouse pointer is on one of the corner */
    else if (toPoint.x() < topLeft.x() && toPoint.y() < topLeft.y())
        originPoint = topLeft;
    else if (toPoint.x() > topRight.x() && toPoint.y() < topRight.y())
        originPoint = topRight;
    else if (toPoint.x() > bottomRight.x() && toPoint.y() > bottomRight.y())
        originPoint = bottomRight;
    else if (toPoint.x() < bottomLeft.x() && toPoint.y() > bottomLeft.y())
        originPoint = bottomLeft;
    
    m_assoItemTmp->setFirstPoint(originPoint);
    m_assoItemTmp->setLastPoint(toPoint);
    
    m_begPoint = originPoint;
    m_endPoint = toPoint;
    
    update();
    m_assoItemTmp->update();
}

void GraphicsScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isAssociationTriggered && isAssociationCreated)
        drawAssociationMoving(event->scenePos());
    
    /* Tell asso-item, that items moved.
     * So the literals or the asso-item self shall move. */
    emit mouseMoved(event->scenePos());
    
    QGraphicsScene::mouseMoveEvent(event);
}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    /* Send to all asso-items, that mouse in scene pressed */
    emit mousePressed(event->scenePos(), event->buttons());

    /* First the item must be selected */
    QGraphicsScene::mousePressEvent(event);
    
    /* Then create an item if conditions met : toolbar icon pressed */
    if (isAssociationTriggered)
    {
        /* Asso item not created yet */
        if (!isAssociationCreated)
        {
            /* Get the selected class item : INFO item must be first selected */
            if (!(ClassItem*)selectedItems().isEmpty()) // TODO only one item selectable here
            {
                m_assoItemTmp = new AssociationItem();
                
                /* Add two points for the beginning and the end of the asso-item. 
                 * These points will be updated whenever the class item will be moved */
                m_assoItemTmp->addPoint(QPointF());
                m_assoItemTmp->addPointID(-1);
                m_assoItemTmp->addPoint(QPointF());
                m_assoItemTmp->addPointID(-1);
                
                nsAssociationItem::AddCommand *addCommand = new nsAssociationItem::AddCommand(this, m_assoItemTmp);
                ProjectManager::getInstance()->getActiveProject()->undoStack()->push(addCommand);
                
                /* Connect the scene with the asso-item, so the asso-item can calculate if it is selected or not */
//                connect(this, &GraphicsScene::mousePressed, m_assoItemTmp, &AssociationItem::mousePressed);
//                connect(this, &GraphicsScene::mouseReleased, m_assoItemTmp, &AssociationItem::mouseReleased);
//                connect(this, &GraphicsScene::mouseMoved, m_assoItemTmp, &AssociationItem::mouseMoved);
//                connect(m_assoItemTmp, &AssociationItem::deleteMe, this, &GraphicsScene::deleteAssoItem);
                
                /* Adding the new item via undo-framework - see redo() for classitem adding */
                // addItem(m_assoItemTmp);
                
                isAssociationCreated = true;
                m_assoItemTmp->setItemBegin(((ClassItem*)selectedItems().at(0)));
            }
        }
        /* If the asso item has been created and the second mouse press is not inside the first item */
        else if (isAssociationCreated && !m_assoItemTmp->getItemBegin()->isUnderMouse())
        {
            /* Add the second class item here */
            if (!(ClassItem*)selectedItems().isEmpty())
            {
                m_assoItemTmp->setItemEnd(((ClassItem*)selectedItems().at(0)));
                /* Drawing is finished */
                isAssociationCreated = false;
                isAssociationTriggered = false;
                /* Now the connection */
                drawAssociationConnection();
                /* Tell parent we are finished with asso item */
                emit resetAssociationToolbar();
                /* Create connection to classitems for move and resize signals */
                connect(m_assoItemTmp->getItemBegin(), &ClassItem::itemMoving, this, &GraphicsScene::redrawAssociation);
                connect(m_assoItemTmp->getItemEnd(), &ClassItem::itemMoving, this, &GraphicsScene::redrawAssociation);
                connect(m_assoItemTmp->getItemBegin(), &ClassItem::itemResizing, this, &GraphicsScene::redrawAssociation);
                connect(m_assoItemTmp->getItemEnd(), &ClassItem::itemResizing, this, &GraphicsScene::redrawAssociation);
            }
        }
    }
}


void GraphicsScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    /* Send to all asso-items, that mouse in scene released */
    emit mouseReleased();
    
    QGraphicsScene::mouseReleaseEvent(event);
}

/*!
 * \brief This function will be called, whenever a class item will be moved. The asso-item between two class items will be updated.
 * If there are more than 2 points, then the class item will move against the last break point. */
void GraphicsScene::redrawAssociation(ClassItem *movingItem)
{
    /* Asso-Item drawn, now the class items can move again */
    QList<QGraphicsItem*> allItems = this->items();
    QListIterator<QGraphicsItem*> iter(allItems);
    while (iter.hasNext())
    {
        if (AssociationItem *assoItem = dynamic_cast<AssociationItem*>(iter.next()))
        {
            if (assoItem->associatedWithThisClass(movingItem))
            {
                QPointF fromPoint;
                QPointF toPoint;
                
                if (assoItem->points().count() == 2)
                {
                    /* If the class-item on the ending side is moving */
                    ClassItem *item1 = assoItem->getItemBegin();
                    ClassItem *item2 = assoItem->getItemEnd();
                    
                    /* Now connecting two items : calculate the beginning and end point by calculating the two class items position */
                    fromPoint = assoItem->getPoint(0);
                    toPoint = assoItem->getPoint(1);

                    if (item2->className() == movingItem->className())
                    {
                        /* First four cases : items are in diagonal position to each other */
                        if (item1->getTopLeftPos().x() > item2->getBottomRightPos().x() && item1->getTopLeftPos().y() > item2->getBottomRightPos().y())
                        {
                            /* item2 TopLeft to item1*/
                            fromPoint = item1->getTopLeftPos();
                            toPoint = item2->getBottomRightPos();
                        }
                        else if (item1->getTopRightPos().x() < item2->getBottomLeftPos().x() && item1->getTopRightPos().y() > item2->getBottomLeftPos().y())
                        {
                            /* item2 TopRight to item1*/
                            fromPoint = item1->getTopRightPos();
                            toPoint = item2->getBottomLeftPos();
                        }
                        else if (item1->getBottomRightPos().x() < item2->getTopLeftPos().x() && item1->getBottomRightPos().y() < item2->getTopLeftPos().y())
                        {
                            /* item2 BottomRight to item1 */
                            fromPoint = item1->getBottomRightPos();
                            toPoint = item2->getTopLeftPos();
                        }
                        else if (item1->getBottomLeftPos().x() > item2->getTopRightPos().x() && item1->getBottomLeftPos().y() < item2->getTopRightPos().y())
                        {
                            /* item2 BottomLeft to item1 */
                            fromPoint = item1->getBottomLeftPos();
                            toPoint = item2->getTopRightPos();
                        }
                        /* Now decide if the class items are horizontal or vertical to each other */
                        else if (item1->getTopLeftPos().x() > item2->getBottomRightPos().x())
                        {
                            /* Item1 is on the right side */
                            if (fromPoint.y() > item2->getBottomRightPos().y())
                            {
                                /* If moving upwards */
                                fromPoint.setX(item1->pos().x());
                                fromPoint.setY(item2->getBottomRightPos().y());                            
                                toPoint.setX(item2->pos().x() + item2->boundingRect().width());
                                toPoint.setY(item2->getBottomRightPos().y());
                            }
                            else if (fromPoint.y() < item2->getTopRightPos().y())
                            {
                                /* If moving downwards */
                                fromPoint.setX(item1->pos().x());
                                fromPoint.setY(item2->getTopRightPos().y());
                                toPoint.setX(item2->pos().x() + item2->boundingRect().width());
                                toPoint.setY(item2->getTopRightPos().y());                            
                            }
                            else
                            {
                                /* If moving only to left or right */
                                fromPoint.setX(item1->pos().x());
                                toPoint.setX(item2->pos().x() + item2->boundingRect().width());
                                toPoint.setY(fromPoint.y()); // To have the same y-level
                            }
                        }
                        else if (item1->getBottomRightPos().x() < item2->getTopLeftPos().x())
                        {
                            /* Item1 is on the left side */
                            if (fromPoint.y() > item2->getBottomLeftPos().y())
                            {
                                /* If moving upwards */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                fromPoint.setY(item2->getBottomLeftPos().y());
                                toPoint.setX(item2->pos().x());
                                toPoint.setY(item2->getBottomLeftPos().y());
                            }
                            else if (fromPoint.y() < item2->getTopLeftPos().y())
                            {
                                /* If moving downwards */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                fromPoint.setY(item2->getTopLeftPos().y());                            
                                toPoint.setX(item2->pos().x());
                                toPoint.setY(item2->getTopLeftPos().y());
                            }
                            else
                            {
                                /* If moving only to left or right */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                toPoint.setX(item2->pos().x());
                                toPoint.setY(fromPoint.y()); // To have the same y-level
                            }
                        }
                        else if (item1->getBottomLeftPos().y() < item2->getTopLeftPos().y())
                        {
                            /* Item1 is above item2 */
                            if (fromPoint.x() < item2->getTopLeftPos().x())
                            {
                                /* If moving to right */
                                fromPoint.setX(item2->getTopLeftPos().x());
                                fromPoint.setY(item1->pos().y() + item1->boundingRect().height());
                                toPoint.setX(item2->getTopLeftPos().x());
                                toPoint.setY(item2->pos().y());
                            }
                            else if (fromPoint.x() > item2->getTopRightPos().x())
                            {
                                /* If moving to left */
                                fromPoint.setX(item2->getTopRightPos().x());                            
                                fromPoint.setY(item1->pos().y() + item1->boundingRect().height());
                                toPoint.setX(item2->getTopRightPos().x());
                                toPoint.setY(item2->pos().y());
                            }
                            else
                            {
                                /* If moving only to up or down */
                                fromPoint.setY(item1->pos().y() + item1->boundingRect().height());
                                toPoint.setX(fromPoint.x());
                                toPoint.setY(item2->pos().y());
                            }
                        }
                        else if (item1->getTopLeftPos().y() > item2->getBottomLeftPos().y())
                        {
                            /* Item1 is below item2 */
                            if (fromPoint.x() < item2->getBottomLeftPos().x())
                            {
                                /* If moving to right */
                                fromPoint.setX(item2->getBottomLeftPos().x());                            
                                fromPoint.setY(item1->pos().y());
                                toPoint.setX(item2->getBottomLeftPos().x());
                                toPoint.setY(item2->pos().y() + item2->boundingRect().height());
                            }
                            else if (fromPoint.x() > item2->getBottomRightPos().x())
                            {
                                /* If moving to left */
                                fromPoint.setX(item2->getBottomRightPos().x());
                                fromPoint.setY(item1->pos().y());                            
                                toPoint.setX(item2->getBottomRightPos().x());
                                toPoint.setY(item2->pos().y() + item2->boundingRect().height());
                            }
                            else
                            {
                                /* If moving only to up or down */
                                fromPoint.setY(item1->pos().y());
                                toPoint.setX(fromPoint.x());
                                toPoint.setY(item2->pos().y() + item2->boundingRect().height());
                            }
                        }
                    }
                    else /* If the other class item on the beginning-point is moving */
                    {   
                        /* First four cases : items are in diagonal position to each other */
                        if (item2->getTopLeftPos().x() > item1->getBottomRightPos().x() && item2->getTopLeftPos().y() > item1->getBottomRightPos().y())
                        {
                            /* item1 TopLeft to item2*/
                            fromPoint = item1->getBottomRightPos();
                            toPoint = item2->getTopLeftPos();
                        }
                        else if (item2->getTopRightPos().x() < item1->getBottomLeftPos().x() && item2->getTopRightPos().y() > item1->getBottomLeftPos().y())
                        {
                            /* item1 TopRight to item2*/
                            fromPoint = item1->getBottomLeftPos();
                            toPoint = item2->getTopRightPos();
                        }
                        else if (item2->getBottomRightPos().x() < item1->getTopLeftPos().x() && item2->getBottomRightPos().y() < item1->getTopLeftPos().y())
                        {
                            /* item1 BottomRight to item2 */
                            fromPoint = item1->getTopLeftPos();
                            toPoint = item2->getBottomRightPos();
                        }
                        else if (item2->getBottomLeftPos().x() > item1->getTopRightPos().x() && item2->getBottomLeftPos().y() < item1->getTopRightPos().y())
                        {
                            /* item1 BottomLeft to item2 */
                            fromPoint = item1->getTopRightPos();
                            toPoint = item2->getBottomLeftPos();
                        }
                        /* Now decide if the class items are horizontal or vertical to each other */
                        else if (item2->getTopLeftPos().x() > item1->getBottomRightPos().x())
                        {
                            /* Item1 is on the left side of item2 */
                            if (toPoint.y() > item1->getBottomRightPos().y())
                            {
                                /* If moving upwards */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                fromPoint.setY(item1->getBottomRightPos().y());
                                toPoint.setX(item2->pos().x());
                                toPoint.setY(item1->getBottomRightPos().y());
                            }
                            else if (toPoint.y() < item1->getTopRightPos().y())
                            {
                                /* If moving downwards */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                fromPoint.setY(item1->getTopRightPos().y());
                                toPoint.setX(item2->pos().x());
                                toPoint.setY(item1->getTopRightPos().y());
                            }
                            else
                            {
                                /* If moving only to left or right */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                toPoint.setX(item2->pos().x());
                                fromPoint.setY(toPoint.y());
                            }
                        }
                        else if (item2->getTopRightPos().x() < item1->getBottomLeftPos().x())
                        {
                            /* Item1 is on the right side of item2 */
                            if (toPoint.y() > item1->getBottomLeftPos().y())
                            {
                                /* If moving upwards */
                                fromPoint.setX(item1->pos().x());
                                fromPoint.setY(item1->getBottomLeftPos().y());
                                toPoint.setX(item2->pos().x() + item2->boundingRect().width());
                                toPoint.setY(item1->getBottomLeftPos().y());
                            }
                            else if (toPoint.y() < item1->getTopLeftPos().y())
                            {
                                /* If moving downwards */
                                fromPoint.setX(item1->pos().x());
                                fromPoint.setY(item1->getTopLeftPos().y());
                                toPoint.setX(item2->pos().x() + item2->boundingRect().width());
                                toPoint.setY(item1->getTopLeftPos().y());
                            }
                            else
                            {
                                /* If moving only to left or right */
                                fromPoint.setX(item1->pos().x());
                                fromPoint.setY(toPoint.y());
                                toPoint.setX(item2->pos().x() + item2->boundingRect().width());
                            }
                        }
                        else if (item2->getTopLeftPos().y() > item1->getBottomLeftPos().y())
                        {
                            /* Item1 is above item2 */
                            if (toPoint.x() < item1->getBottomLeftPos().x())
                            {
                                /* If moving to right */
                                fromPoint.setX(item1->pos().x());
                                fromPoint.setY(item1->getBottomLeftPos().y());
                                toPoint.setX(item1->pos().x());
                                toPoint.setY(item2->pos().y());
                            }
                            else if (toPoint.x() > item1->getBottomRightPos().x())
                            {
                                /* If moving to left */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                fromPoint.setY(item1->getBottomRightPos().y());
                                toPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                toPoint.setY(item2->pos().y());
                            }
                            else
                            {
                                /* If moving only to up or down */
                                fromPoint.setX(toPoint.x());
                                fromPoint.setY(item1->getBottomRightPos().y());
                                toPoint.setY(item2->pos().y());
                            }
                        }
                        else if (item2->getBottomLeftPos().y() < item1->getTopLeftPos().y())
                        {
                            /* Item1 is below item2 */
                            if (toPoint.x() < item1->getTopLeftPos().x())
                            {
                                /* If moving to right */
                                fromPoint.setX(item1->pos().x());
                                fromPoint.setY(item1->getTopLeftPos().y());
                                toPoint.setX(item1->pos().x());
                                toPoint.setY(item2->getBottomRightPos().y());
                            }
                            else if (toPoint.x() > item1->getTopRightPos().x())
                            {
                                /* If moving to left */
                                fromPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                fromPoint.setY(item1->getTopRightPos().y());
                                toPoint.setX(item1->pos().x() + item1->boundingRect().width());
                                toPoint.setY(item2->getBottomRightPos().y());
                            }
                            else
                            {
                                /* If moving only to up or down */
                                fromPoint.setX(toPoint.x());
                                fromPoint.setY(item1->getTopRightPos().y());
                                toPoint.setY(item2->pos().y() + item2->boundingRect().height());
                            }
                        }
                    }
                    
                    /* Now save the points */
                    assoItem->setFirstPoint(fromPoint);
                    assoItem->setLastPoint(toPoint);
    
                    assoItem->update();
                    update();
                }
                else
                /* There are more than two points: move only the point on the class item because the break point will not move */
                {
                    ClassItem *item;
                    
                    /* Item1 is moving */
                    if (assoItem->getItemBegin()->className() == movingItem->className())
                    {
                        item = assoItem->getItemBegin();
                        /* Point on the beginning item */
                        fromPoint = assoItem->getPoint(0);
                        /* Next break point */
                        toPoint = assoItem->getPoint(1);
                    }
                    else
                    {
                        item = assoItem->getItemEnd();
                        /* Point on the end item */
                        fromPoint = assoItem->getPoint(assoItem->points().count()-1);
                        /* Next break point */
                        toPoint = assoItem->getPoint(assoItem->points().count()-2);
                    }

                    /* Define points for readability */
                    QPointF topLeft(item->pos().x(), item->pos().y());
                    QPointF bottomLeft(item->pos().x(), item->pos().y()+item->boundingRect().height());
                    QPointF topRight(item->pos().x()+item->boundingRect().width(), item->pos().y());
                    QPointF bottomRight(item->pos().x()+item->boundingRect().width(), item->pos().y()+item->boundingRect().height());
                    
                    /* Origin point is the begining point of the line. */
                    QPointF originPoint(toPoint);
                    
                    if (toPoint.y() >= topLeft.y() && toPoint.y() <= bottomRight.y())
                    {
                        /* If mouse pointer moves on the right side of the item */
                        if (toPoint.x() >= bottomRight.x())
                            originPoint.setX(bottomRight.x());
                        /* or if mouse pointer moves on the left side of the item */
                        else if (toPoint.x() <= topLeft.x())
                            originPoint.setX(topLeft.x());
                
                        originPoint.setY(toPoint.y());
                    }
                    else if (toPoint.x() >= topLeft.x() && toPoint.x() <= bottomRight.x())
                    {
                        /* If mouse pointer moves above the item */
                        if (toPoint.y() <= topLeft.y())
                            originPoint.setY(topLeft.y());
                        /* or if mouse pointer moves below the item */
                        else if (toPoint.y() >= bottomRight.y())
                            originPoint.setY(bottomRight.y());
                        
                        originPoint.setX(toPoint.x());
                    }
                    /* At this point mouse pointer is on one of the corner */
                    else if (toPoint.x() < topLeft.x() && toPoint.y() < topLeft.y())
                        originPoint = topLeft;
                    else if (toPoint.x() > topRight.x() && toPoint.y() < topRight.y())
                        originPoint = topRight;
                    else if (toPoint.x() > bottomRight.x() && toPoint.y() > bottomRight.y())
                        originPoint = bottomRight;
                    else if (toPoint.x() < bottomLeft.x() && toPoint.y() > bottomLeft.y())
                        originPoint = bottomLeft;
                    
                    /* Item1 is moving */
                    if (assoItem->getItemBegin()->className() == movingItem->className())
                        assoItem->setFirstPoint(originPoint);
                    else
                        /* Item2 is moving */
                        assoItem->setLastPoint(originPoint);
                    
                    assoItem->update();
                    update();
                }
                /* Calculate the new positions of the literals, if there are any */
                assoItem->moveLiterals();
            }
        }
    }
}


