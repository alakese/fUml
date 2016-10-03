#ifndef ASSOCIATIONITEM_H
#define ASSOCIATIONITEM_H

#include <QGraphicsObject>
#include "../../globals/globals.h"

QT_BEGIN_NAMESPACE
class QLabel;
QT_END_NAMESPACE
class ClassItem;
class LiteralItem;



/*!
 * \class AssociationItem
 * \brief TODO explain
 */
class AssociationItem : public QGraphicsObject
{
    Q_OBJECT

    /* This is the id of this class : -1 new created, not saved in database yet or id from database */
    int m_id;
    /* Defines, which relationt this item has */
    // enum ITEM_TYPE { ASSOCIATION = 0, AGGREGATION, COMPOSITION };
    ITEM_TYPE m_relationType;
    /* State of the item : is being created now and created
     * CREATING : First point selected on class an item, second is free
     * CREATED : Second point placed yet
     * CREATING_FINISHED : Creating process is finished */
    enum ITEM_STATE { CREATING = 0, CREATED, CREATING_FINISHED };
    ITEM_STATE m_itemState;
    /* Beginning end the end of the association depending on the position of the items */
    ClassItem *m_ptClassItemBegin;
    ClassItem *m_ptClassItemEnd;
    /* Multiplicities :  */
    LiteralItem *m_pMultiplyBeginItem;
    LiteralItem *m_pMultiplyEndItem;
    /* Roles */
    LiteralItem *m_pRoleBeginItem;
    LiteralItem *m_pRoleEndItem;
    /* Description */
    LiteralItem *m_pDescription;
    /* Selection line */
    QPolygonF m_selectionPolygon;
    /* All the other points between two items */
    QList<QPointF> m_points;
    /* The id's in the database */
    QList<int> m_idPoints; 
    /* For undo */
    QList<QPointF> m_oldPoints;
    /* Selection line - dosh dot line */
    QPolygonF m_Polygon;
    bool isItemSelected;
    /* Item may move */
    bool itemCanMove;
    enum MOVE_POINTS { BOTH_PTs = 0, BEGIN_PT, END_PT, BREAK_PT };
    MOVE_POINTS moveWhichPoints;
    int m_indWhichBreakPointIsMoving;
    /* Last pressed coordinate */
    QPointF m_pLastPressedPos;
    
public:
    AssociationItem();
    ~AssociationItem();
    bool addPoint(const QPointF &);
    QRectF boundingRect() const;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
    bool associatedWithThisClass(ClassItem *) const;
    void moveLiterals();
    void removeBreakpoint(int);
    void itemCreatedFully();
    
    /* Getter Setter */
    void addPointID(int);
    void updatePointID(int, int);
    int getPointID(int);
    QPointF getPoint(int) const;
    void setFirstPoint(const QPointF &);
    void setLastPoint(const QPointF &);
    ClassItem *getItemEnd() const;
    void setItemEnd(ClassItem *value);
    ClassItem *getItemBegin() const;
    void setItemBegin(ClassItem *value);
    QList<QPointF> points() const;
    void setPoints(const QList<QPointF> &);
    QList<QPointF> oldPoints() const;
    void insertBreakpoint(const QPointF &, int);
    int id() const;
    void setId(int);
    ITEM_TYPE getRelationType();
    void setRelationType(ITEM_TYPE);
    QString getMultiplicityBeginInfo() const;
    void setMultiplicityBeginInfo(const QString &, qreal, qreal);
    QString getMultiplicityEndInfo() const;
    void setMultiplicityEndInfo(const QString &, qreal, qreal);
    QString getRoleBeginInfo() const;
    void setRoleBeginInfo(const QString &, qreal, qreal);
    QString getRoleEndInfo() const;
    void setRoleEndInfo(const QString &, qreal, qreal);
    QString getDescriptionInfo() const;
    void setDescriptionInfo(const QString &, qreal, qreal);
    ClassItem *getClassItemBegin() const;
    void setClassItemBegin(ClassItem *getClassItemBegin);
    ClassItem *getClassItemEnd() const;
    void setClassItemEnd(ClassItem *getClassItemEnd);
    
private:
    bool isMouseSelectingThisItem(const QPointF &);
    bool isMouseBetweenThesePoints(const QPointF &, const QPointF &, const QPointF &);
    void callContextMenu(bool);
    void deleteItem();
    void addMulForBeginItem();
    void addMulForEndItem();
    void addRoleForBeginItem();
    void addRoleForEndItem();
    void addDescription();
    void addBreakpoint();
    void deleteBreakpoint();
    void removeMulFromBeginItem();
    void removeMulFromEndItem();
    void removeRoleFromBeginItem();
    void removeRoleFromEndItem();
    void removeDescription();
    QVector2D calcLiteralPos(const QPointF &, const QPointF &);
    QVector2D calcDescriptionPos(const QPointF &, const QPointF &);
    void move(const QPointF &);
    bool isBeginPointSelected(const QPointF &);
    bool isEndPointSelected(const QPointF &);
    bool isBreakPointSelected(const QPointF &);
    QPointF rotatePoint(int, const QPointF &, const QPointF &);
    
public slots:
    void mousePressed(const QPointF &, Qt::MouseButtons);
    void mouseReleased();
    void mouseMoved(const QPointF &);
    
signals:
    void deleteMe(AssociationItem *);
};


#endif // ASSOCIATIONITEM_H
