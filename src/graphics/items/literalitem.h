#ifndef LITERALITEM_H
#define LITERALITEM_H

#include <QGraphicsSimpleTextItem>



class LiteralItem : public QGraphicsSimpleTextItem
{
public:
    LiteralItem(QGraphicsItem *parent = 0);
    
protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
};

#endif // LITERALITEM_H
