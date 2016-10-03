#ifndef GRAPHICSSCENE_H
#define GRAPHICSSCENE_H

#include <QGraphicsScene>

QT_BEGIN_NAMESPACE
class QGraphicsLineItem;
QT_END_NAMESPACE
class AssociationItem;
class ClassItem;

class GraphicsScene : public QGraphicsScene
{
    Q_OBJECT

    /* Temp asso item, which will be used at the beginning of creation */
    AssociationItem *m_assoItemTmp;
    /* Did the user pressed the toolbar */
    bool isAssociationTriggered;
    /* Is it done? */
    bool isAssociationCreated;
    /* Beginning point of the asso item */
    QPointF m_begPoint;
    /* End point of the asso item */
    QPointF m_endPoint;
    
public:
    explicit GraphicsScene(QObject *parent = 0);
    void addItemCanceled();
    void addItemTriggered();
    
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    
private:
    void drawAssociationMoving(const QPointF &);
    void drawAssociationConnection();
    
signals:
    void resetAssociationToolbar();
    void mousePressed(const QPointF &, Qt::MouseButtons);
    void mouseReleased();
    void mouseMoved(const QPointF &);
    
public slots:
    void redrawAssociation(ClassItem *);
    void deleteAssoItem(AssociationItem *);
};


#endif // GRAPHICSSCENE_H
