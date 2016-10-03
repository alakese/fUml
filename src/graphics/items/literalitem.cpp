#include "literalitem.h"
#include "associationitem.h"
#include <QGraphicsScene>

#include <QDebug>


LiteralItem::LiteralItem(QGraphicsItem *parent) :
    QGraphicsSimpleTextItem(parent)
{
}

/*!
 * \brief In this function a literal item can be moved freely in all over the space.
 * The position of the item will be calculated new, if a class item moves.
 */
void LiteralItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (isSelected())
        parentItem()->scene()->update();
    
    QGraphicsSimpleTextItem::mouseMoveEvent(event);
}
