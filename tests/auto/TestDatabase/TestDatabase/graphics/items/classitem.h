#ifndef CLASSITEM_H
#define CLASSITEM_H

#include <QGraphicsItem>
#include <QBrush>
#include <QPen>
#include <QFont>
#include <QStringList>
#include "../../globals/properties.h"

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE


/*!
 * \class ClassItem
 * \brief This class paints a class item with three rects : header part, members part and functions part.
 * The communications with mouse will be implemented in this class.
 * 
 * One issue is that each item has a local coordinate and scene coordinate. If an item will be moved, then 
 * the position in the scene changes, but item-position in local coordinates stays fixed. See documentary for more information. */
class ClassItem : public QGraphicsObject
{
    Q_OBJECT
    
    /* Properties of a class item : gui properties will not be kept seperatly in a GUIProperties item, that would be here twice work */
    /* id of the table ClassItem will be kept in m_pGeneralProperties. ID of table GeneralProperties is unnecessary, because they both
     * have same row. */
    GeneralProperties *m_pGeneralProperties;
    QList<MemberProperties *> m_listMemberProperties;
    QList<FunctionProperties *> m_listFunctionProperties;
    /* If only one of these items will be deleted, them remove them from the db */
    QList<MemberProperties *> m_listMemberToDelete;
    QList<FunctionProperties *> m_listFunctionToDelete;
    QList<ParameterProperties *> m_listParameterToDelete;
    /* GUI properties */
    QRect m_rectHeader;
    QPen m_penHeader;
    QPen m_penHeaderText;
    QFont m_font;
    QBrush m_brushHeader;
    /* Members paint elements */
    QBrush m_brushMembers;
    QRect m_rectMembers;
    /* Functions paint elements */
    QBrush m_brushFunctions;
    QRect m_rectFunctions;
    /* Elements to determine if resize can be done*/
    bool m_bIsMouseOverTheResizePoint; 
    bool m_bCanResizeNow;
    /* Holds last position of the item for MoveCommand */
    QPointF m_ptOldPosition;
    /* Holds last size information of the item for ResizeCommand */
    int m_oldWidth;
    int m_oldHeightHeader;
    int m_oldHeightFunctions;
    int m_oldHeightMembers;
    /* Holds last font info */
    QFont m_oldFont;
    /* Minimum widht of the window */
    qint8 m_minWidth;
    /* Will the size be fixed */
    bool m_bCalculateFixWidth;
    /* If the size will not be fixed, then split or hide */
    bool m_bSplitTheString;
        
public:
    ClassItem(GeneralProperties *, GUIProperties *);
    ClassItem(GeneralProperties *, GUIProperties *, const QList<MemberProperties *> &, const QList<FunctionProperties *> &);
    QRectF boundingRect() const;
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *);
    void setClassName(const QString &, bool);
    QString className() const;
    QFont fontHeaderText() const;
    void setFontHeaderText(const QFont &);
    int id() const;
    void setID(int);
    void resetIDs();
    QRect rectHeader() const;
    void setRectHeader(const QRect &);
    QRect rectMembers() const;
    void setRectMembers(const QRect &);
    QRect rectFunctions() const;
    void setRectFunctions(const QRect &);
    QPointF oldPosition() const;
    int oldWidth() const;
    int oldHeightHeader() const;
    int oldHeightFunctions() const;
    int oldHeightMembers() const;
    QFont font() const;
    void setFont(const QFont &);
    QFont oldFont() const;
    void resizeItem(int, int);
    void storePosition();
    void storeCurrentSize();
    GeneralProperties *getGeneralProperties() const;
    void setGeneralProperties(GeneralProperties *);
    GUIProperties *getGUIProperties() const;
    void setGUIProperties(GUIProperties *);
    QList<MemberProperties *> getListMemberProperties() const;
    void setListMemberProperties(const QList<MemberProperties *> &);
    QList<FunctionProperties *> getListFunctionProperties() const;
    void setListFunctionProperties(const QList<FunctionProperties *> &);
    QList<MemberProperties *> getListMembersToDelete();
    QList<FunctionProperties *> getListFunctionsToDelete();
    QList<ParameterProperties *> getListParametersToDelete();
    bool addMember(const QString &, const QString &);
    bool addFunction(const QString &, const QString &, const QList<ParameterProperties *> *);
    void removeMember(MemberProperties*);
    void removeFunction(FunctionProperties*);
    
signals:
    void classNameChanged(const QString &, const QString &);
    void classItemDeleted(const QString &);
    void itemScaled(ClassItem *);
    void itemMoved(ClassItem *);
    void itemRenamed(const QString &, const QString &);
    void itemPropertyChanged();
    
private: 
    void drawItem(QPainter *);
    void drawItemAsSelectedDots(QPainter *);
    void drawResizePoints(QPainter *);
    void setClassItemWidth(int);
    void changeItemProperty(int currentPage = 0);
    void deleteClassItem();
    void changeFont();
    int calculateMinWidth();
    QStringList splitItemString(const QString &);
    void drawHeader(QPainter *, int *, qint8);
    void drawMembers(QPainter *, int *, qint8);
    void drawFunctions(QPainter *, int *, qint8);
    
protected: 
    void mousePressEvent(QGraphicsSceneMouseEvent *);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *);
    void hoverMoveEvent(QGraphicsSceneHoverEvent *);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
};

#endif // CLASSITEM_H
