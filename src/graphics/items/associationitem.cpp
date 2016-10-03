#include "associationitem.h"
#include "classitem.h"
#include "../items/literalitem.h"
#include "../../undo/assoitem/nsAssociationItem_deletecommand.h"
#include "../../undo/assoitem/nsAssociationItem_movecommand.h"
#include "../../undo/assoitem/nsAssociationItem_addbreakpointcommand.h"
#include "../../undo/assoitem/nsAssociationItem_deletebreakpointcommand.h"
#include "../../undo/assoitem/nsAssociationItem_addliteralcommand.h"
#include "../../undo/assoitem/nsAssociationItem_deleteliteralcommand.h"
#include "../../project/project.h"
#include "../../project/projectmanagement.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsScene>
#include <QInputDialog>
#include <QDir>
#include <QLineEdit>
#include <QVector2D>
#include <cmath>

#include <QDebug>



namespace
{
    const qreal offset = 5.0;
    const qreal pi = 3.14159265358979323846264338327950288419717;
}

AssociationItem::AssociationItem()
{
    m_id = -1;
    
    m_itemState = CREATING;
    
    m_relationType = ASSOCIATION;
    
    isItemSelected = false;
    itemCanMove = false;
    
    m_pMultiplyBeginItem = NULL;
    m_pMultiplyEndItem = NULL;
    m_pRoleBeginItem = NULL;
    m_pRoleEndItem = NULL;
    m_pDescription = NULL;
        
//    m_pMultiplyBeginItem = new LiteralItem(this);
//    m_pMultiplyBeginItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
//    m_pMultiplyEndItem = new LiteralItem(this);
//    m_pMultiplyEndItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
//    m_pRoleBeginItem = new LiteralItem(this);
//    m_pRoleBeginItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
//    m_pRoleEndItem = new LiteralItem(this);
//    m_pRoleEndItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
//    m_pDescription = new LiteralItem(this);
//    m_pDescription->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    
    /* If this item will be moved, which part should move : both sides, only the beginning point or end point */
    moveWhichPoints = BOTH_PTs;
    m_indWhichBreakPointIsMoving = -1; //-1 : none
}

AssociationItem::~AssociationItem()
{
    if (m_pMultiplyBeginItem)
    {
        delete m_pMultiplyBeginItem;
        m_pMultiplyBeginItem = NULL;
    }
    
    if (m_pMultiplyEndItem)
    {
        delete m_pMultiplyEndItem;
        m_pMultiplyEndItem = NULL;
    }
    
    if (m_pRoleBeginItem)
    {
        delete m_pRoleBeginItem;
        m_pRoleBeginItem = NULL;
    }
    
    if (m_pRoleEndItem)
    {
        delete m_pRoleEndItem;
        m_pRoleEndItem = NULL;
    }
    
    if (m_pDescription)
    {
        delete m_pDescription;
        m_pDescription = NULL;
    }
}

/*!
 * \brief This function adds a point between the beginning and the end point.
 * To select an asso-item, there is a offset-width, which allows user to press the line easly.
 * But the pressed point can lay anywhere between two offsets area. This point must be taken on to asso-item.
 * 
 * PS: Formel -> Fußpunkt C des Lotes von B auf die Gerade durch P und A see https://homepages.thm.de
 */
void AssociationItem::addBreakpoint()
{
    qreal rectWidthHalf = 3.0;
    qreal rectWidth = 6.0;

    /* First find out, between which points did the user add a breakpoint */
    QPointF point0;
    QPointF point1;
    
    int i = 0;
    for (; i < m_points.count() - 1; ++i)
        if (isMouseBetweenThesePoints(m_pLastPressedPos, m_points[i], m_points[i+1]))
        {
            point0 = m_points[i];
            point1 = m_points[i+1];
            break;
        }
    
    /* Calculate the pointpos on the line */
    QPointF g = point1 - point0;
    qreal gSquare = g.x()*g.x() + g.y()*g.y();
    QPointF b = m_pLastPressedPos - point0;
    qreal bg = b.x()*g.x() + b.y()*g.y();
    QPointF c = (bg / gSquare) * g;
    QPointF target = point0 + c;
    
    /* Add the point to the list */
    // m_points.insert(i+1, target);
    /* Adding the new item via undo-framework */
    nsAssociationItem::AddBreakpointCommand *command = new nsAssociationItem::AddBreakpointCommand(this, target, i+1);
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    
    scene()->update(QRect(target.x() - rectWidthHalf, target.y() - rectWidthHalf, rectWidth, rectWidth));
}

void AssociationItem::addDescription()
{
    QString text = QInputDialog::getText(0, tr("Add description"), tr("Enter description :"));
    if (!text.isEmpty())
    {
        if (m_pDescription == NULL)
        {
            m_pDescription = new LiteralItem(this);
            m_pDescription->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        }
        
        /* Calculate the position of the description between two class items */
        /* Description shall be in the middle, if the num of points is even */
        int indPoint = m_points.count() / 2;
        if (m_points.count() % 2 == 0)
        {
            QVector2D newPos = calcDescriptionPos(m_points[indPoint-1], m_points[indPoint]);
            m_pDescription->setPos(m_points[indPoint-1].x() + newPos.x(), m_points[indPoint-1].y() + newPos.y());
        }
        /* Or above the middle point, if odd */
        else
        {
            int indPoint = m_points.count() / 2;
            m_pDescription->setPos(m_points[indPoint]);
        }
        //m_pDescription->setText(text);
        nsAssociationItem::AddLiteralCommand *command = new nsAssociationItem::AddLiteralCommand(m_pDescription, text);
        ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    }
}

void AssociationItem::addMulForBeginItem()
{
    QString title(QString("Add Multiplicity for %1").arg(m_ptClassItemBegin->className()));
    QString text = QInputDialog::getText(0, title, tr("Enter multiplicity :"));
    if (!text.isEmpty())
    {
        if (m_pMultiplyBeginItem == NULL)
        {
            m_pMultiplyBeginItem = new LiteralItem(this);
            m_pMultiplyBeginItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        }
        
        /* Calculate the position of the text near to m_ptClassItemBegin */
        QPointF rotatedPoint = rotatePoint(45, m_points[0], m_points[1]);
        QVector2D newPos = calcLiteralPos(m_points[0], rotatedPoint);
        m_pMultiplyBeginItem->setPos(m_points[0].x() + newPos.x(), m_points[0].y() + newPos.y());
        //m_pMultiplyBeginItem->setText(text);
        nsAssociationItem::AddLiteralCommand *command = new nsAssociationItem::AddLiteralCommand(m_pMultiplyBeginItem, text);
        ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    }
}

void AssociationItem::addMulForEndItem()
{
    QString title(QString("Add Multiplicity for %1").arg(m_ptClassItemEnd->className()));
    QString text = QInputDialog::getText(0, title, tr("Enter multiplicity :"));
    if (!text.isEmpty())
    {
        if (m_pMultiplyEndItem == NULL)
        {
            m_pMultiplyEndItem = new LiteralItem(this);
            m_pMultiplyEndItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        }
        
        /* Calculate the position of the text near to m_pMultiplyEndItem */
        QPointF rotatedPoint = rotatePoint(360-45, m_points.last(), m_points[m_points.count()-2]);
        QVector2D newPos = calcLiteralPos(m_points.last(), rotatedPoint);
        m_pMultiplyEndItem->setPos(m_points.last().x() + newPos.x(), m_points.last().y() + newPos.y());
        //m_pMultiplyEndItem->setText(text);
        nsAssociationItem::AddLiteralCommand *command = new nsAssociationItem::AddLiteralCommand(m_pMultiplyEndItem, text);
        ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    }
}

bool AssociationItem::addPoint(const QPointF &pt)
{
    m_points.append(pt);
    // TODO return
    return true;
}

void AssociationItem::addRoleForBeginItem()
{
    QString title(QString("Add Role for %1").arg(m_ptClassItemBegin->className()));
    QString text = QInputDialog::getText(0, title, tr("Enter role :"));
    if (!text.isEmpty())
    {
        if (m_pRoleBeginItem == NULL)
        {
            m_pRoleBeginItem = new LiteralItem(this);
            m_pRoleBeginItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        }
        
        /* Calculate the position of the text near to m_ptClassItemBegin */
        QPointF rotatedPoint = rotatePoint(360-45, m_points[0], m_points[1]);
        QVector2D newPos = calcLiteralPos(m_points[0], rotatedPoint);
        m_pRoleBeginItem->setPos(m_points[0].x() + newPos.x(), m_points[0].y() + newPos.y());
        //m_pRoleBeginItem->setText(text);
        nsAssociationItem::AddLiteralCommand *command = new nsAssociationItem::AddLiteralCommand(m_pRoleBeginItem, text);
        ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    }
}

void AssociationItem::addRoleForEndItem()
{
    QString title(QString("Add Role for %1").arg(m_ptClassItemEnd->className()));
    QString text = QInputDialog::getText(0, title, tr("Enter role :"));
    if (!text.isEmpty())
    {
        if (m_pRoleEndItem == NULL)
        {
            m_pRoleEndItem = new LiteralItem(this);
            m_pRoleEndItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
        }
        
        /* Calculate the position of the text near to m_ptClassItemBegin */
        QPointF rotatedPoint = rotatePoint(45, m_points.last(), m_points[m_points.count()-2]);
        QVector2D newPos = calcLiteralPos(m_points.last(), rotatedPoint);
        m_pRoleEndItem->setPos(m_points.last().x() + newPos.x(), m_points.last().y() + newPos.y());
        //m_pRoleEndItem->setText(text);
        nsAssociationItem::AddLiteralCommand *command = new nsAssociationItem::AddLiteralCommand(m_pRoleEndItem, text);
        ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    }
}

/*!
 * \brief This function checks, if the given class item is associated. */
bool AssociationItem::associatedWithThisClass(ClassItem *item) const
{
    return item->className() == m_ptClassItemBegin->className() || item->className() == m_ptClassItemEnd->className();
}

QRectF AssociationItem::boundingRect() const
{
    /* Return zero : for selection we need to calculate the selection area by ourselves, because a line-rect can take
     * more place as the line should. Rect will be calculcated as a real rectangle, not as the surrounding area. */
    return QRectF();
}

/*!
 * \brief This function calculates the position of the description between the two class items */
QVector2D AssociationItem::calcDescriptionPos(const QPointF &ptA, const QPointF &ptB)
{
    float offset = 0.0f;
    /* Calculate the position of the description between two class items */
    QVector2D vDiff;

    /* 1. Find the vector between two class items */
    vDiff.setX( (ptA.x() - ptB.x()) * -1 );// -1 because qt-coord system
    vDiff.setY( (ptA.y() - ptB.y()) * -1 );
    /* 2. Calculate the vector normalize and the half of the vector length */
    offset = vDiff.length()/2.0f;
    /* 3. Position must be moved in half of font+text length */
// TODO    QFontMetrics fontMetrics(); 
//    int widthClassName = fontMetrics.width(string);
    
    return vDiff.normalized()*offset; 
}

/*!
 * \brief This function calculates the position of the multiplicty or the role for a class item */
QVector2D AssociationItem::calcLiteralPos(const QPointF &ptA, const QPointF &ptB)
{
    float offset = 20.0f;
    /* Calculate the position of the text near to m_ptClassItemBegin */
    QVector2D difference, normalized;
    /* Always X pixels distance to class item in vector direction */
    /* 1. Find the vector between two class items */
    difference.setX( (ptA.x() - ptB.x()) * -1);// -1 because qt-coord system
    difference.setY( (ptA.y() - ptB.y()) * -1);
    /* 2. Calculate the vector normalize : a/|a| */
    normalized = difference.normalized()*offset; 
    /* 3. Calculate the new pos */
    return normalized;
}

/*! 
 * Context menü opens when the user presses r-click on an asso item.
 * If a breakpoint is selected, then delete option will be enable. */
void AssociationItem::callContextMenu(bool isBreakpointSelected)
{
    QString menuItem1(QString("Add multiplicity for %1").arg(m_ptClassItemBegin->className()));
    QString menuItem2(QString("Add multiplicity for %1").arg(m_ptClassItemEnd->className()));
    QString menuItem3(QString("Add role for %1").arg(m_ptClassItemBegin->className()));
    QString menuItem4(QString("Add role for %1").arg(m_ptClassItemEnd->className()));

    QMenu contextMenu;

    QAction *p_addPoint = contextMenu.addAction(QIcon(), "Add breakpoint");
    connect(p_addPoint, &QAction::triggered, this, &AssociationItem::addBreakpoint);
    QAction *p_addMulA = contextMenu.addAction(QIcon(), menuItem1);
    connect(p_addMulA, &QAction::triggered, this, &AssociationItem::addMulForBeginItem);
    QAction *p_addMulB = contextMenu.addAction(QIcon(), menuItem2);
    connect(p_addMulB, &QAction::triggered, this, &AssociationItem::addMulForEndItem);
    QAction *p_addRolA = contextMenu.addAction(QIcon(), menuItem3);
    connect(p_addRolA, &QAction::triggered, this, &AssociationItem::addRoleForBeginItem);
    QAction *p_addRolB = contextMenu.addAction(QIcon(), menuItem4);
    connect(p_addRolB, &QAction::triggered, this, &AssociationItem::addRoleForEndItem);
    QAction *p_addDesc = contextMenu.addAction(QIcon(), "Add description");
    connect(p_addDesc, &QAction::triggered, this, &AssociationItem::addDescription);
    
    contextMenu.addSeparator();
   
    bool addSeperator = false;
    
    if (isBreakpointSelected)
    {
        addSeperator = true;
        QAction *p_delPoint = contextMenu.addAction(QIcon(), "Remove breakpoint");
        connect(p_delPoint, &QAction::triggered, this, &AssociationItem::deleteBreakpoint);
    }
    
    if (m_pMultiplyBeginItem != NULL && !m_pMultiplyBeginItem->text().isEmpty())
    {
        addSeperator = true;
        QString menuItem(QString("Remove multiplicity from %1").arg(m_ptClassItemBegin->className()));
        QAction *p_remove = contextMenu.addAction(QIcon(), menuItem);
        connect(p_remove, &QAction::triggered, this, &AssociationItem::removeMulFromBeginItem);
    }

    if (m_pMultiplyEndItem != NULL && !m_pMultiplyEndItem->text().isEmpty())
    {
        addSeperator = true;
        QString menuItem(QString("Remove multiplicity from %1").arg(m_ptClassItemEnd->className()));
        QAction *p_remove = contextMenu.addAction(QIcon(), menuItem);
        connect(p_remove, &QAction::triggered, this, &AssociationItem::removeMulFromEndItem);
    }
    
    if (m_pRoleBeginItem != NULL && !m_pRoleBeginItem->text().isEmpty())
    {
        addSeperator = true;
        QString menuItem(QString("Remove role from %1").arg(m_ptClassItemBegin->className()));
        QAction *p_remove = contextMenu.addAction(QIcon(), menuItem);
        connect(p_remove, &QAction::triggered, this, &AssociationItem::removeRoleFromBeginItem);
    }
    
    if (m_pRoleEndItem != NULL && !m_pRoleEndItem->text().isEmpty())
    {
        addSeperator = true;
        QString menuItem(QString("Remove role from %1").arg(m_ptClassItemEnd->className()));
        QAction *p_remove = contextMenu.addAction(QIcon(), menuItem);
        connect(p_remove, &QAction::triggered, this, &AssociationItem::removeRoleFromEndItem);
    }
    
    if (m_pDescription != NULL && !m_pDescription->text().isEmpty())
    {
        addSeperator = true;
        QAction *p_remove = contextMenu.addAction(QIcon(), "Remove description");
        connect(p_remove, &QAction::triggered, this, &AssociationItem::removeDescription);
    }
    
    if (addSeperator)
        contextMenu.addSeparator();
   
    QAction *p_delete = contextMenu.addAction(QIcon(), "Remove item");
    connect(p_delete, &QAction::triggered, this, &AssociationItem::deleteItem);
    
    contextMenu.exec(QCursor::pos());
}

/*!
 * \brief This function deletes the breakpoint after the user removes it 
 */
void AssociationItem::deleteBreakpoint()
{
    //m_points.removeAt(m_indWhichBreakPointIsMoving);
    
    /* Adding the new item via undo-framework - see redo() for classitem adding */
    nsAssociationItem::DeleteBreakpointCommand *command = new nsAssociationItem::DeleteBreakpointCommand(this,
                                                          m_points[m_indWhichBreakPointIsMoving], m_indWhichBreakPointIsMoving);
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    
    m_indWhichBreakPointIsMoving = -1;
    scene()->update();
}

/*!
 * \brief This function deletes this item. */
void AssociationItem::deleteItem()
{
    emit deleteMe(this);
}

QPointF AssociationItem::getPoint(int index) const
{
    if (index >= m_points.count())
        return QPointF();
    
    return m_points[index];
}

/*!
 * \brief This function inserts a new breakpoint into the list. Will be called from Undo-Command.
 * See nsAssociationItem::AddBreakpointCommand */
void AssociationItem::insertBreakpoint(const QPointF &point, int index)
{
    m_points.insert(index, point);
    m_idPoints.insert(index, -1);
    scene()->update();
}

/*!
 * \brief This function checks, if the beginning point is selected. */
bool AssociationItem::isBeginPointSelected(const QPointF &mouse)
{
    qreal rectWidthHalf = 3.0;
    qreal rectWidth = 6.0;
    
    return mouse.x() >= m_points[0].x() - rectWidthHalf && mouse.y() >= m_points[0].y() - rectWidthHalf &&
           mouse.x() <= m_points[0].x() + rectWidth && mouse.y() <= m_points[0].y() + rectWidth;
}

/*!
 * \brief This function checks, if a break point is selected. */
bool AssociationItem::isBreakPointSelected(const QPointF &mouse)
{
    qreal rectWidthHalf = 3.0;
    qreal rectWidth = 6.0;
    
    /* Not checking beginning and the end point */
    for (int i = 1; i < m_points.count() - 1; ++i)
        if (mouse.x() >= m_points[i].x() - rectWidthHalf && mouse.y() >= m_points[i].y() - rectWidthHalf &&
               mouse.x() <= m_points[i].x() + rectWidth && mouse.y() <= m_points[i].y() + rectWidth)
        {
            m_indWhichBreakPointIsMoving = i;
            return true;
        }
    
    m_indWhichBreakPointIsMoving = -1;
    return false;
}

/*!
 * \brief This function checks, if the end point is selected. */
bool AssociationItem::isEndPointSelected(const QPointF &mouse)
{
    qreal rectWidthHalf = 3.0;
    qreal rectWidth = 6.0;
    
    return mouse.x() >= m_points.last().x() - rectWidthHalf && mouse.y() >= m_points.last().y() - rectWidthHalf &&
            mouse.x() <= m_points.last().x() + rectWidth && mouse.y() <= m_points.last().y() + rectWidth;
}

/*!
 * \brief This function checks if the mouse position is between two given point.
 * This is necessary to know when the user adds a new breakpoint.
 */
bool AssociationItem::isMouseBetweenThesePoints(const QPointF &mousePt, const QPointF &pt1, const QPointF &pt2) 
{
    QVector<QPointF> linePoints;
    QLineF line(pt1, pt2);
    
    /* equation m = (y2-y1)=m(x1-x2)*/
    if (line.p2().x() == line.p1().x())
    {
        // special case : x1 = x2, line is vertical, not a function
        int yBegin = line.p1().y() < line.p2().y() ? line.p1().y() : line.p2().y();
        int yEnd = line.p1().y() > line.p2().y() ? line.p1().y() : line.p2().y();
        int x = line.p1().x(); // no matters which one

        for (int y = yBegin; y < yEnd; ++y)
            linePoints.append(QPointF(x, y));
    }
    else
    {
        float m = (float)(line.p2().y() - line.p1().y()) / (float)(line.p2().x() - line.p1().x());

        int xBegin = line.p1().x() < line.p2().x() ? line.p1().x() : line.p2().x();
        int xEnd = line.p1().x() > line.p2().x() ? line.p1().x() : line.p2().x();

        for (int x = xBegin; x < xEnd; ++x)
        {
            //equation y=mx+b
            int y = m * (x - line.p1().x()) + line.p1().y();
            linePoints.append(QPoint(x, y));
        }
    }
    
    /* is it over the line */
    foreach(QPointF p, linePoints)
    {
        int xMin = p.x() - offset;
        int xMax = p.x() + offset;
        int yMin = p.y() - offset;
        int yMax = p.y() + offset;

        if (mousePt.x() > xMin && mousePt.x() < xMax && mousePt.y() > yMin && mousePt.y() < yMax)
            return true;
    }

    return false;
}

/*!
 * \brief This function checks if the mouse position is near to the line. If so, then the line will be selected. */
bool AssociationItem::isMouseSelectingThisItem(const QPointF &pt) 
{
    for (int ind = 0; ind < m_points.count() - 1; ++ind)
    {
        QVector<QPointF> linePoints;
        QLineF line(m_points[ind], m_points[ind+1]);
        
        /* equation m = (y2-y1)=m(x1-x2)*/
        if (line.p2().x() == line.p1().x())
        {
            // special case : x1 = x2, line is vertical, not a function
            int yBegin = line.p1().y() < line.p2().y() ? line.p1().y() : line.p2().y();
            int yEnd = line.p1().y() > line.p2().y() ? line.p1().y() : line.p2().y();
            int x = line.p1().x(); // no matters which one
    
            for (int y = yBegin; y < yEnd; ++y)
                linePoints.append(QPointF(x, y));
        }
        else
        {
            float m = (float)(line.p2().y() - line.p1().y()) / (float)(line.p2().x() - line.p1().x());
    
            int xBegin = line.p1().x() < line.p2().x() ? line.p1().x() : line.p2().x();
            int xEnd = line.p1().x() > line.p2().x() ? line.p1().x() : line.p2().x();
    
            for (int x = xBegin; x < xEnd; ++x)
            {
                //equation y=mx+b
                int y = m * (x - line.p1().x()) + line.p1().y();
                linePoints.append(QPoint(x, y));
            }
        }
        
        /* is it over the line */
        int i = 0;
        foreach(QPointF p, linePoints)
        {
            /* To avoid selection of both the class item and the point */
            if (i++ < offset)
                continue;
                
            int xMin = p.x() - offset;
            int xMax = p.x() + offset;
            int yMin = p.y() - offset;
            int yMax = p.y() + offset;
    
            if (pt.x() > xMin && pt.x() < xMax && pt.y() > yMin && pt.y() < yMax)
                return true;
        }
    }

    return false;
}

/*!
 * \brief This function will be called whenever the user places the both points of the asso-item on both class items. */
void AssociationItem::itemCreatedFully()
{
    m_itemState = CREATED;
}

/*!
 * \brief This function will be called, whenever the mouse is pressed in the scene. */
void AssociationItem::mousePressed(const QPointF &pt, Qt::MouseButtons button)
{
    /* At this point the user clicked the rect-area of the line - calculate if the mouse is in a selection zone */
    if (isMouseSelectingThisItem(pt))
        isItemSelected = true;
    else
        isItemSelected = false;
    
    scene()->update();
    update();

    /* If l-click, item may move */
    if (isItemSelected && button == Qt::LeftButton)
    {
        itemCanMove = true;
        
        /* Store current points for undo */
        m_oldPoints = m_points;
        
        if(isBeginPointSelected(pt))
            moveWhichPoints = BEGIN_PT;
        else if (isEndPointSelected(pt))
            moveWhichPoints = END_PT;
        else if (isBreakPointSelected(pt))
            moveWhichPoints = BREAK_PT;
        else
            moveWhichPoints = BOTH_PTs;
    }
    
    /* If r-click, open context menu */
    if (isItemSelected && button == Qt::RightButton)
    {
        m_pLastPressedPos = pt;
        callContextMenu(isBreakPointSelected(pt));
    }
}

/*!
 * \brief This function will be called, whenever the mouse is released in the scene. */
void AssociationItem::mouseReleased()
{
    /* If points changed, call undo : if only asso item is created fully; 
     * do not use movecommand after the connection is fresh created. */
    if (itemCanMove && m_itemState == CREATING_FINISHED && m_oldPoints != m_points) 
    {
        /* Adding the new item via undo-framework - see redo() for classitem adding */
        nsAssociationItem::MoveCommand *command = new nsAssociationItem::MoveCommand(this);
        ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    }
    
    itemCanMove = false;
    moveWhichPoints = BOTH_PTs;
    m_indWhichBreakPointIsMoving = -1;
    
    /* At this point item is created fully, go to next state */
    if (m_itemState == CREATED)
        m_itemState = CREATING_FINISHED;
}

void AssociationItem::mouseMoved(const QPointF &scenePt)
{
    /* Move the asso item */
    if (itemCanMove)
        /* Now move it, if only connected to other class item */
        move(scenePt);
}

/*!
 * \brief This function moves the asso item. */
void AssociationItem::move(const QPointF &scenePt)
{
    /* Move this asso, if the class items are not in diagonal positions to each other */
    QPointF mousePoint(scenePt);
    
    /* If one of the breakpoint is moving */
    if (moveWhichPoints == BREAK_PT)
    {
        if (m_indWhichBreakPointIsMoving != -1)
        {
            /* Move the breakpoint */
            m_points[m_indWhichBreakPointIsMoving] = scenePt;
            
            /* Move the point on the class item, if the breakpoint is connected to a class item */
            if (m_indWhichBreakPointIsMoving == 1)
            {
                /* Then this is the breakpoint connected to item 1 */
                ClassItem *item = getItemBegin();
                /* Next break point */
                QPointF toPoint = getPoint(1);

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
                setFirstPoint(originPoint);
            }
            if (m_indWhichBreakPointIsMoving == m_points.count()-2)
            {
                /* Then this is the breakpoint connected to item 2 */
                ClassItem *item = getItemEnd();
                /* Next break point */
                QPointF toPoint = getPoint(m_points.count()-2);

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
                
                /* Item2 is moving */
                setLastPoint(originPoint);
            }
        }
    }
    /* Or moving beginning or end point, or both */
    else
    {
        /* If on diagonal-position */
        if (m_ptClassItemBegin->getTopLeftPos().x() > m_ptClassItemEnd->getBottomRightPos().x() && 
                m_ptClassItemBegin->getTopLeftPos().y() > m_ptClassItemEnd->getBottomRightPos().y())
        {
            /* item 2
             *        item 1 */
            if(moveWhichPoints == BEGIN_PT)
            {
                if(mousePoint.x() >= m_ptClassItemBegin->getTopLeftPos().x() && 
                        m_points[0].y() == m_ptClassItemBegin->getTopLeftPos().y() && 
                        mousePoint.x() <= m_ptClassItemBegin->getTopRightPos().x())
                {
                    /* Point is moving on above of item 1*/
                    m_points[0].setX(mousePoint.x());
                    m_points[0].setY(m_ptClassItemBegin->getTopLeftPos().y());
                }
                if (mousePoint.y() >= m_ptClassItemBegin->getTopLeftPos().y() && 
                        m_points[0].x() == m_ptClassItemBegin->getTopLeftPos().x() && 
                        mousePoint.y() <= m_ptClassItemBegin->getBottomLeftPos().y())
                {
                    /* Point is moving on left side of item 1 */
                    m_points[0].setX(m_ptClassItemBegin->getTopLeftPos().x());
                    m_points[0].setY(mousePoint.y());
                }
            }
            if (moveWhichPoints == END_PT)
            {
                if(mousePoint.x() <= m_ptClassItemEnd->getBottomRightPos().x() &&
                        m_points.last().y() == m_ptClassItemEnd->getBottomRightPos().y() && 
                        mousePoint.x() >= m_ptClassItemEnd->getBottomLeftPos().x())
                {
                    /* Point is moving on above of item 2*/
                    m_points.last().setX(mousePoint.x());
                    m_points.last().setY(m_ptClassItemEnd->getBottomRightPos().y());
                }
                if (mousePoint.y() <= m_ptClassItemEnd->getBottomRightPos().y() && 
                        mousePoint.y() >= m_ptClassItemEnd->getTopRightPos().y() && 
                        m_points.last().x() == m_ptClassItemEnd->getTopRightPos().x())
                {
                    /* Point is moving on right side of item 2 */
                    m_points.last().setX(m_ptClassItemEnd->getTopRightPos().x());
                    m_points.last().setY(mousePoint.y());
                }
            }
        }
        else if (m_ptClassItemBegin->getTopRightPos().x() < m_ptClassItemEnd->getBottomLeftPos().x() && 
                 m_ptClassItemBegin->getTopRightPos().y() > m_ptClassItemEnd->getBottomLeftPos().y())
        {
            /*        item 2
             * item 1        */
            if(moveWhichPoints == BEGIN_PT)
            {
                if(mousePoint.x() <= m_ptClassItemBegin->getTopRightPos().x() && 
                        m_points[0].y() == m_ptClassItemBegin->getTopRightPos().y() && 
                        mousePoint.x() >= m_ptClassItemBegin->getTopLeftPos().x())
                {
                    /* Point is moving on bottom of item 1*/
                    m_points[0].setX(mousePoint.x());
                    m_points[0].setY(m_ptClassItemBegin->getTopLeftPos().y());
                }
                if (mousePoint.y() >= m_ptClassItemBegin->getTopRightPos().y() && 
                        m_points[0].x() == m_ptClassItemBegin->getTopRightPos().x() && 
                        mousePoint.y() <= m_ptClassItemBegin->getBottomRightPos().y())
                {
                    /* Point is moving on left side of item 1 */
                    m_points[0].setX(m_ptClassItemBegin->getTopRightPos().x());
                    m_points[0].setY(mousePoint.y());
                }
            }
            if (moveWhichPoints == END_PT)
            {
                if(mousePoint.x() >= m_ptClassItemEnd->getBottomLeftPos().x() &&
                        m_points.last().y() == m_ptClassItemEnd->getBottomRightPos().y() && 
                        mousePoint.x() <= m_ptClassItemEnd->getBottomRightPos().x())
                {
                    /* Point is moving on above of item 2*/
                    m_points.last().setX(mousePoint.x());
                    m_points.last().setY(m_ptClassItemEnd->getBottomRightPos().y());
                }
                if (mousePoint.y() <= m_ptClassItemEnd->getBottomLeftPos().y() && 
                        mousePoint.y() >= m_ptClassItemEnd->getTopLeftPos().y() && 
                        m_points.last().x() == m_ptClassItemEnd->getTopLeftPos().x())
                {
                    /* Point is moving on right side of item 2 */
                    m_points.last().setX(m_ptClassItemEnd->getTopLeftPos().x());
                    m_points.last().setY(mousePoint.y());
                }
            }
        }
        else if (m_ptClassItemBegin->getBottomRightPos().x() < m_ptClassItemEnd->getTopLeftPos().x() && 
                 m_ptClassItemBegin->getBottomRightPos().y() < m_ptClassItemEnd->getTopLeftPos().y())
        {
            /* item 1
             *         item 2 */
            if(moveWhichPoints == BEGIN_PT)
            {
                if(mousePoint.x() <= m_ptClassItemBegin->getBottomRightPos().x() && 
                        m_points[0].y() == m_ptClassItemBegin->getBottomRightPos().y() && 
                        mousePoint.x() >= m_ptClassItemBegin->getBottomLeftPos().x())
                {
                    /* Point is moving on bottom of item 1*/
                    m_points[0].setX(mousePoint.x());
                    m_points[0].setY(m_ptClassItemBegin->getBottomRightPos().y());
                }
                if (mousePoint.y() >= m_ptClassItemBegin->getTopRightPos().y() && 
                        m_points[0].x() == m_ptClassItemBegin->getBottomRightPos().x() && 
                        mousePoint.y() <= m_ptClassItemBegin->getBottomRightPos().y())
                {
                    /* Point is moving on left side of item 1 */
                    m_points[0].setX(m_ptClassItemBegin->getBottomRightPos().x());
                    m_points[0].setY(mousePoint.y());
                }
            }
            if (moveWhichPoints == END_PT)
            {
                if(mousePoint.x() >= m_ptClassItemEnd->getTopLeftPos().x() &&
                        m_points.last().y() == m_ptClassItemEnd->getTopLeftPos().y() && 
                        mousePoint.x() <= m_ptClassItemEnd->getTopRightPos().x())
                {
                    /* Point is moving on above of item 2*/
                    m_points.last().setX(mousePoint.x());
                    m_points.last().setY(m_ptClassItemEnd->getTopLeftPos().y());
                }
                if (mousePoint.y() >= m_ptClassItemEnd->getTopLeftPos().y() && 
                        mousePoint.y() <= m_ptClassItemEnd->getBottomLeftPos().y() && 
                        m_points.last().x() == m_ptClassItemEnd->getTopLeftPos().x())
                {
                    /* Point is moving on right side of item 2 */
                    m_points.last().setX(m_ptClassItemEnd->getTopLeftPos().x());
                    m_points.last().setY(mousePoint.y());
                }
            }
        }
        else if (m_ptClassItemBegin->getBottomLeftPos().x() > m_ptClassItemEnd->getTopRightPos().x() && 
                 m_ptClassItemBegin->getBottomLeftPos().y() < m_ptClassItemEnd->getTopRightPos().y())
        {
            /*         item 1
             * item 2         */
            if(moveWhichPoints == BEGIN_PT)
            {
                if(mousePoint.x() >= m_ptClassItemBegin->getBottomLeftPos().x() && 
                        m_points[0].y() == m_ptClassItemBegin->getBottomLeftPos().y() && 
                        mousePoint.x() <= m_ptClassItemBegin->getBottomRightPos().x())
                {
                    /* Point is moving on bottom of item 1*/
                    m_points[0].setX(mousePoint.x());
                    m_points[0].setY(m_ptClassItemBegin->getBottomRightPos().y());
                }
                if (mousePoint.y() >= m_ptClassItemBegin->getTopLeftPos().y() && 
                        m_points[0].x() == m_ptClassItemBegin->getBottomLeftPos().x() && 
                        mousePoint.y() <= m_ptClassItemBegin->getBottomLeftPos().y())
                {
                    /* Point is moving on left side of item 1 */
                    m_points[0].setX(m_ptClassItemBegin->getBottomLeftPos().x());
                    m_points[0].setY(mousePoint.y());
                }
            }
            if (moveWhichPoints == END_PT)
            {
                if(mousePoint.x() <= m_ptClassItemEnd->getTopRightPos().x() &&
                        m_points.last().y() == m_ptClassItemEnd->getTopLeftPos().y() && 
                        mousePoint.x() >= m_ptClassItemEnd->getTopLeftPos().x())
                {
                    /* Point is moving on above of item 2*/
                    m_points.last().setX(mousePoint.x());
                    m_points.last().setY(m_ptClassItemEnd->getTopLeftPos().y());
                }
                if (mousePoint.y() >= m_ptClassItemEnd->getTopRightPos().y() && 
                        mousePoint.y() <= m_ptClassItemEnd->getBottomRightPos().y() && 
                        m_points.last().x() == m_ptClassItemEnd->getTopRightPos().x())
                {
                    /* Point is moving on right side of item 2 */
                    m_points.last().setX(m_ptClassItemEnd->getTopRightPos().x());
                    m_points.last().setY(mousePoint.y());
                }
            }
        }
        /* item 2   item 1 */
        else if (m_ptClassItemBegin->getTopLeftPos().x() > m_ptClassItemEnd->getTopRightPos().x())
        {
            /* To move both points, there must be no breakpoints on the line */
            if (m_points.count() == 2 && moveWhichPoints == BOTH_PTs)
            {
                /* Check, if the new point is still on the allowed borders.*/
                qreal maxTL = m_ptClassItemBegin->getTopLeftPos().y() > m_ptClassItemEnd->getTopLeftPos().y() 
                        ? m_ptClassItemBegin->getTopLeftPos().y() : m_ptClassItemEnd->getTopLeftPos().y();
                qreal minBL = m_ptClassItemBegin->getBottomLeftPos().y() < m_ptClassItemEnd->getBottomLeftPos().y() 
                        ? m_ptClassItemBegin->getBottomLeftPos().y() : m_ptClassItemEnd->getBottomLeftPos().y();
                
                if (mousePoint.y() >= maxTL && mousePoint.y() <= minBL)
                {
                    m_points[0].setX(m_ptClassItemBegin->getTopLeftPos().x()); 
                    m_points.last().setX(m_ptClassItemEnd->getTopRightPos().x());
                    m_points[0].setY(mousePoint.y());
                    m_points.last().setY(mousePoint.y());
                }
            }
            else
            {
                if(moveWhichPoints == BEGIN_PT)
                {
                    /* If item 2 top is above item 1 top, then the point may move over item 1 */
                    if(m_ptClassItemBegin->getTopLeftPos().y() > m_ptClassItemEnd->getTopRightPos().y() && // This checks if the item 2 is above item 1
                            mousePoint.x() >= m_ptClassItemBegin->getTopLeftPos().x() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemBegin->getTopRightPos().x() &&       /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].y() == m_ptClassItemBegin->getTopLeftPos().y() &&       /* Move only if point on top of item1 */ 
                            m_points.last().y() < m_points[0].y())                                  /* And the end point must be above the beginning point */
                    {
                        /* Point is moving on above of item 1*/
                        m_points[0].setX(mousePoint.x());
                        m_points[0].setY(m_ptClassItemBegin->getTopLeftPos().y());
                    }
                    /* If item 2 bottom is below item 1 bottom, then the point may move under item 1 */
                    if(m_ptClassItemBegin->getBottomLeftPos().y() < m_ptClassItemEnd->getBottomRightPos().y() && // This checks if the item 2 is below item 1
                            mousePoint.x() >= m_ptClassItemBegin->getBottomLeftPos().x() &&     /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemBegin->getBottomRightPos().x() &&    /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].y() == m_ptClassItemBegin->getBottomLeftPos().y() &&    /* Move only if point on bottom of item1 */ 
                            m_points.last().y() > m_points[0].y())                                  /* And the end point must be below the beginning point */
                    {
                        /* Point is moving on above of item 1*/
                        m_points[0].setX(mousePoint.x());
                        m_points[0].setY(m_ptClassItemBegin->getBottomLeftPos().y());
                    }
                    
                    /* Only on the left side of item 1 */
                    if (mousePoint.y() >= m_ptClassItemBegin->getTopLeftPos().y() && 
                            mousePoint.y() <= m_ptClassItemBegin->getBottomLeftPos().y() &&
                            m_points[0].x() == m_ptClassItemBegin->getTopLeftPos().x())
                    {
                        m_points[0].setX(m_ptClassItemBegin->getTopLeftPos().x());
                        m_points[0].setY(mousePoint.y());
                    }
                }
                else if (moveWhichPoints == END_PT)
                {
                    /* If item 1 top is above item 2 top, then the point may move over item 2 */
                    if(m_ptClassItemBegin->getTopLeftPos().y() < m_ptClassItemEnd->getTopRightPos().y() && // This checks if the item 1 is above item 2
                            mousePoint.x() >= m_ptClassItemEnd->getTopLeftPos().x() &&  /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemEnd->getTopRightPos().x() && /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().y() == m_ptClassItemEnd->getTopLeftPos().y() && /* Move only if point on top of item2 */
                            m_points[0].y() < m_points.last().y())                          /* And the end point must be above the beginning point */
                    {
                        /* Point is moving on above of item 2 */
                        m_points.last().setX(mousePoint.x());
                        m_points.last().setY(m_ptClassItemEnd->getTopRightPos().y());
                    }
                    /* If item 1 bottom is below item 2 bottom, then the point may move under item 2 */
                    if(m_ptClassItemBegin->getBottomLeftPos().y() > m_ptClassItemEnd->getBottomRightPos().y() && // This checks if the item 2 is below item 1
                            mousePoint.x() >= m_ptClassItemEnd->getBottomLeftPos().x() &&   /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemEnd->getBottomRightPos().x() &&  /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().y() == m_ptClassItemEnd->getBottomLeftPos().y() &&  /* Move only if point on bottom of item2 */ 
                            m_points[0].y() > m_points.last().y())                              /* And the end point must be below the beginning point */
                    {
                        /* Point is moving on below item 2 */
                        m_points.last().setX(mousePoint.x());
                        m_points.last().setY(m_ptClassItemEnd->getBottomLeftPos().y());
                    }
                    
                    /* Only on the right side of item 2 */
                    if (mousePoint.y() >= m_ptClassItemEnd->getTopRightPos().y() && 
                            mousePoint.y() <= m_ptClassItemEnd->getBottomRightPos().y() &&
                            m_points.last().x() == m_ptClassItemEnd->getTopRightPos().x())
                    {
                        /* Point is moving on left side of item 2 */
                        m_points.last().setX(m_ptClassItemEnd->getTopRightPos().x());
                        m_points.last().setY(mousePoint.y());
                    }
                }
            }
        }
        /* item 1   item 2 */
        else if (m_ptClassItemBegin->getTopRightPos().x() < m_ptClassItemEnd->getTopLeftPos().x())
        {
            /* To move both points, there must be no breakpoints on the line */
            if (m_points.count() == 2 && moveWhichPoints == BOTH_PTs)
            {
                /* Check, if the new point is still on the allowed borders.*/
                qreal maxTL = m_ptClassItemBegin->getTopLeftPos().y() > m_ptClassItemEnd->getTopLeftPos().y() 
                        ? m_ptClassItemBegin->getTopLeftPos().y() : m_ptClassItemEnd->getTopLeftPos().y();
                qreal minBL = m_ptClassItemBegin->getBottomLeftPos().y() < m_ptClassItemEnd->getBottomLeftPos().y() 
                        ? m_ptClassItemBegin->getBottomLeftPos().y() : m_ptClassItemEnd->getBottomLeftPos().y();
                
                if (mousePoint.y() >= maxTL && mousePoint.y() <= minBL)
                {
                    m_points[0].setX(m_ptClassItemBegin->getTopRightPos().x()); 
                    m_points.last().setX(m_ptClassItemEnd->getTopLeftPos().x());
                    m_points[0].setY(mousePoint.y());
                    m_points.last().setY(mousePoint.y());
                }
            }
            else
            {
                if(moveWhichPoints == BEGIN_PT)
                {
                    /* If item 2 top is above item 1 top, then the point may move over item 1 */
                    if(m_ptClassItemBegin->getTopRightPos().y() > m_ptClassItemEnd->getTopLeftPos().y() && // This checks if the item 2 is above item 1
                            mousePoint.x() >= m_ptClassItemBegin->getTopLeftPos().x() &&    /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemBegin->getTopRightPos().x() &&   /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].y() == m_ptClassItemBegin->getTopRightPos().y() &&  /* Move only if point on top of item1 */
                            m_points.last().y() < m_points[0].y())                              /* And the end point must be above the beginning point */
                    {
                        /* Point is moving on above of item 1*/
                        m_points[0].setX(mousePoint.x());
                        m_points[0].setY(m_ptClassItemBegin->getTopRightPos().y());
                    }
                    /* If item 2 bottom is below item 1 bottom, then the point may move under item 1 */
                    if(m_ptClassItemBegin->getBottomRightPos().y() < m_ptClassItemEnd->getBottomLeftPos().y() && // This checks if the item 2 is below item 1
                            mousePoint.x() >= m_ptClassItemBegin->getBottomLeftPos().x() &&     /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemBegin->getBottomRightPos().x() &&    /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].y() == m_ptClassItemBegin->getBottomRightPos().y() &&   /* Move only if point on bottom of item1 */
                            m_points.last().y() > m_points[0].y())                                  /* And the end point must be below the beginning point */
                    {
                        /* Point is moving on above of item 1*/
                        m_points[0].setX(mousePoint.x());
                        m_points[0].setY(m_ptClassItemBegin->getBottomRightPos().y());
                    }
                    
                    /* Only on the left side of item 1 */
                    if (mousePoint.y() >= m_ptClassItemBegin->getTopLeftPos().y() && 
                            mousePoint.y() <= m_ptClassItemBegin->getBottomLeftPos().y() &&
                            m_points[0].x() == m_ptClassItemBegin->getTopRightPos().x())
                    {
                        m_points[0].setX(m_ptClassItemBegin->getTopRightPos().x());
                        m_points[0].setY(mousePoint.y());
                    }
                }
                else if (moveWhichPoints == END_PT)
                {
                    /* If item 1 top is above item 2 top, then the point may move over item 2 */
                    if(m_ptClassItemBegin->getTopRightPos().y() < m_ptClassItemEnd->getTopLeftPos().y() && // This checks if the item 1 is above item 2
                            mousePoint.x() >= m_ptClassItemEnd->getTopLeftPos().x() &&      /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemEnd->getTopRightPos().x() &&     /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().y() == m_ptClassItemEnd->getTopLeftPos().y() &&     /* Move only if point on top of item2 */
                            m_points[0].y() < m_points.last().y())                              /* And the end point must be above the beginning point */
                    {
                        /* Point is moving on above of item 2 */
                        m_points.last().setX(mousePoint.x());
                        m_points.last().setY(m_ptClassItemEnd->getTopLeftPos().y());
                    }
                    /* If item 1 bottom is below item 2 bottom, then the point may move under item 2 */
                    if(m_ptClassItemBegin->getBottomRightPos().y() > m_ptClassItemEnd->getBottomLeftPos().y() && // This checks if the item 2 is below item 1
                            mousePoint.x() >= m_ptClassItemEnd->getBottomLeftPos().x() &&   /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemEnd->getBottomRightPos().x() &&  /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().y() == m_ptClassItemEnd->getBottomLeftPos().y() &&  /* Move only if point on bottom of item2 */
                            m_points[0].y() > m_points.last().y())                              /* And the end point must be below the beginning point */
                    {
                        /* Point is moving on below item 2 */
                        m_points.last().setX(mousePoint.x());
                        m_points.last().setY(m_ptClassItemEnd->getBottomLeftPos().y());
                    }
                    
                    /* Only on the right side of item 2 */
                    if (mousePoint.y() >= m_ptClassItemEnd->getTopLeftPos().y() && 
                            mousePoint.y() <= m_ptClassItemEnd->getBottomLeftPos().y() &&
                            m_points.last().x() == m_ptClassItemEnd->getTopLeftPos().x())
                    {
                        /* Point is moving on left side of item 2 */
                        m_points.last().setX(m_ptClassItemEnd->getTopLeftPos().x());
                        m_points.last().setY(mousePoint.y());
                    }
                }
            }
        }
        /* item1
         * 
         * item2 */
        else if (m_ptClassItemBegin->getBottomLeftPos().y() < m_ptClassItemEnd->getTopLeftPos().y())
        {
            /* To move both points, there must be no breakpoints on the line */
            if (m_points.count() == 2 && moveWhichPoints == BOTH_PTs)
            {
                /* Check, if the new point is still on the allowed borders.*/
                qreal maxTL = m_ptClassItemBegin->getTopLeftPos().x() >= m_ptClassItemEnd->getTopLeftPos().x() 
                        ? m_ptClassItemBegin->getTopLeftPos().x() : m_ptClassItemEnd->getTopLeftPos().x();
                qreal minBR = m_ptClassItemBegin->getBottomRightPos().x() <= m_ptClassItemEnd->getBottomRightPos().x() 
                        ? m_ptClassItemBegin->getBottomRightPos().x() : m_ptClassItemEnd->getBottomRightPos().x();
                
                if (mousePoint.x() >= maxTL && mousePoint.x() <= minBR)
                {
                    m_points[0].setX(mousePoint.x());
                    m_points[0].setY(m_ptClassItemBegin->getBottomLeftPos().y());
                    m_points.last().setX(mousePoint.x());
                    m_points.last().setY(m_ptClassItemEnd->getTopLeftPos().y());
                }
            }
            else
            {
                if(moveWhichPoints == BEGIN_PT)
                {
                    /* If item 2 is more on the left side of item 1 */
                    if(m_ptClassItemBegin->getBottomLeftPos().x() > m_ptClassItemEnd->getTopLeftPos().x() && // This checks if the item 2 is on the left side of item 1
                            mousePoint.y() >= m_ptClassItemBegin->getTopLeftPos().y() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemBegin->getBottomLeftPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].x() == m_ptClassItemBegin->getTopLeftPos().x() &&       /* Move only if point on the left side of item1 */
                            m_points[0].x() > m_points.last().x())                                  /* And the end point must be lefter then the beginning point */
                    {
                        m_points[0].setX(m_ptClassItemBegin->getTopLeftPos().x());
                        m_points[0].setY(mousePoint.y());
                    }
                    /* If item 2 is more on the right side of item 1 */
                    if(m_ptClassItemBegin->getBottomRightPos().x() < m_ptClassItemEnd->getTopRightPos().x() && // This checks if the item 2 is below item 1
                            mousePoint.y() >= m_ptClassItemBegin->getTopRightPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemBegin->getBottomRightPos().y() &&    /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].x() == m_ptClassItemBegin->getBottomRightPos().x() &&   /* Move only if point on the right side of item1 */
                            m_points.last().x() > m_points[0].x())                                  /* And the end point must be more on the right side of the beginning point */
                    {
                        m_points[0].setX(m_ptClassItemBegin->getTopRightPos().x());
                        m_points[0].setY(mousePoint.y());                }
                    
                    /* Item 1 is direkt above item 2 */
                    if (mousePoint.x() >= m_ptClassItemBegin->getTopLeftPos().x() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemBegin->getBottomRightPos().x() && /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].y() == m_ptClassItemBegin->getBottomRightPos().y()) /* Move only if point below item 1 */
                    {
                        m_points[0].setY(m_ptClassItemBegin->getBottomRightPos().y());
                        m_points[0].setX(mousePoint.x());
                    }
                }
                else if (moveWhichPoints == END_PT)
                {
                    /* If item 1 is more on the left side of item 2 */
                    if(m_ptClassItemBegin->getBottomLeftPos().x() < m_ptClassItemEnd->getTopLeftPos().x() && // This checks if the item 1 is on the left side of item 2
                            mousePoint.y() >= m_ptClassItemEnd->getTopLeftPos().y() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemEnd->getBottomLeftPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().x() == m_ptClassItemEnd->getTopLeftPos().x() &&       /* Move only if point on the left side of item2 */
                            m_points[0].x() < m_points.last().x())                                  /* And the beginning point must be more lefter then the end point */
                    {
                        m_points.last().setX(m_ptClassItemEnd->getTopLeftPos().x());
                        m_points.last().setY(mousePoint.y());
                    }
                    /* If item 1 is more on the right side of item 2 */
                    if(m_ptClassItemBegin->getBottomRightPos().x() > m_ptClassItemEnd->getTopRightPos().x() && // This checks if the item 1 is righter then item 2
                            mousePoint.y() >= m_ptClassItemEnd->getTopRightPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemEnd->getBottomRightPos().y() &&    /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().x() == m_ptClassItemEnd->getBottomRightPos().x() &&   /* Move only if point on the right side of item 2 */
                            m_points[0].x() > m_points.last().x())                                  /* And the end point must be more on the right side of the beginning point */
                    {
                        m_points.last().setX(m_ptClassItemEnd->getTopRightPos().x());
                        m_points.last().setY(mousePoint.y());                }
                    
                    /* Item 1 is direkt above item 2 */
                    if (mousePoint.x() >= m_ptClassItemEnd->getTopLeftPos().x() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemEnd->getBottomRightPos().x() && /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().y() == m_ptClassItemEnd->getTopRightPos().y()) /* Move only if point above item 2 */
                    {
                        m_points.last().setY(m_ptClassItemEnd->getTopRightPos().y());
                        m_points.last().setX(mousePoint.x());
                    }
                }
            }
        }
        /* item2
         * 
         * item1 */
        else if (m_ptClassItemBegin->getTopLeftPos().y() > m_ptClassItemEnd->getBottomLeftPos().y())
        {
            /* To move both points, there must be no breakpoints on the line */
            if (m_points.count() == 2 && moveWhichPoints == BOTH_PTs)
            {
                /* Check, if the new point is still on the allowed borders.*/
                qreal maxTL = m_ptClassItemBegin->getTopLeftPos().x() >= m_ptClassItemEnd->getTopLeftPos().x() 
                        ? m_ptClassItemBegin->getTopLeftPos().x() : m_ptClassItemEnd->getTopLeftPos().x();
                qreal minBR = m_ptClassItemBegin->getBottomRightPos().x() <= m_ptClassItemEnd->getBottomRightPos().x() 
                        ? m_ptClassItemBegin->getBottomRightPos().x() : m_ptClassItemEnd->getBottomRightPos().x();
                
                if (mousePoint.x() >= maxTL && mousePoint.x() <= minBR)
                {
                    m_points[0].setX(mousePoint.x());
                    m_points[0].setY(m_ptClassItemBegin->getTopLeftPos().y());
                    m_points.last().setX(mousePoint.x());
                    m_points.last().setY(m_ptClassItemEnd->getBottomLeftPos().y());
                }
            }
            else
            {
                if(moveWhichPoints == BEGIN_PT)
                {
                    /* If item 2 is more on the left side of item 1 */
                    if(m_ptClassItemBegin->getTopLeftPos().x() > m_ptClassItemEnd->getBottomLeftPos().x() && // This checks if the item 2 is on the left side of item 1
                            mousePoint.y() >= m_ptClassItemBegin->getTopLeftPos().y() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemBegin->getBottomLeftPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].x() == m_ptClassItemBegin->getTopLeftPos().x() &&       /* Move only if point on the left side of item1 */
                            m_points[0].x() > m_points.last().x())                                  /* And the end point must be lefter then the beginning point */
                    {
                        m_points[0].setX(m_ptClassItemBegin->getTopLeftPos().x());
                        m_points[0].setY(mousePoint.y());
                    }
                    /* If item 2 is more on the right side of item 1 */
                    if(m_ptClassItemBegin->getTopRightPos().x() < m_ptClassItemEnd->getBottomRightPos().x() && // This checks if the item 2 is below item 1
                            mousePoint.y() >= m_ptClassItemBegin->getTopRightPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemBegin->getBottomRightPos().y() &&    /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].x() == m_ptClassItemBegin->getTopRightPos().x() &&   /* Move only if point on the right side of item1 */
                            m_points.last().x() > m_points[0].x())                                  /* And the end point must be more on the right side of the beginning point */
                    {
                        m_points[0].setX(m_ptClassItemBegin->getTopRightPos().x());
                        m_points[0].setY(mousePoint.y());                }
                    
                    /* Item 2 is direkt above item 1 */
                    if (mousePoint.x() >= m_ptClassItemBegin->getTopLeftPos().x() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemBegin->getBottomRightPos().x() && /* Check, if the new point is still on the allowed borders.*/
                            m_points[0].y() == m_ptClassItemBegin->getTopRightPos().y()) /* Move only if point below item 1 */
                    {
                        m_points[0].setY(m_ptClassItemBegin->getTopRightPos().y());
                        m_points[0].setX(mousePoint.x());
                    }
                }
                else if (moveWhichPoints == END_PT)
                {
                    /* If item 1 is more on the left side of item 2 */
                    if(m_ptClassItemBegin->getTopLeftPos().x() < m_ptClassItemEnd->getBottomLeftPos().x() && // This checks if the item 1 is on the left side of item 2
                            mousePoint.y() >= m_ptClassItemEnd->getTopLeftPos().y() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemEnd->getBottomLeftPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().x() == m_ptClassItemEnd->getTopLeftPos().x() &&       /* Move only if point on the left side of item2 */
                            m_points[0].x() < m_points.last().x())                                  /* And the beginning point must be more lefter then the end point */
                    {
                        m_points.last().setX(m_ptClassItemEnd->getTopLeftPos().x());
                        m_points.last().setY(mousePoint.y());
                    }
                    /* If item 1 is more on the right side of item 2 */
                    if(m_ptClassItemBegin->getTopRightPos().x() > m_ptClassItemEnd->getBottomRightPos().x() && // This checks if the item 1 is righter then item 2
                            mousePoint.y() >= m_ptClassItemEnd->getTopRightPos().y() &&     /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.y() <= m_ptClassItemEnd->getBottomRightPos().y() &&    /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().x() == m_ptClassItemEnd->getBottomRightPos().x() &&   /* Move only if point on the right side of item 2 */
                            m_points[0].x() > m_points.last().x())                                  /* And the end point must be more on the right side of the beginning point */
                    {
                        m_points.last().setX(m_ptClassItemEnd->getTopRightPos().x());
                        m_points.last().setY(mousePoint.y());                }
                    
                    /* Item 1 is direkt above item 2 */
                    if (mousePoint.x() >= m_ptClassItemEnd->getBottomLeftPos().x() &&        /* Check, if the new point is still on the allowed borders.*/
                            mousePoint.x() <= m_ptClassItemEnd->getBottomRightPos().x() && /* Check, if the new point is still on the allowed borders.*/
                            m_points.last().y() == m_ptClassItemEnd->getBottomRightPos().y()) /* Move only if point above item 2 */
                    {
                        m_points.last().setY(m_ptClassItemEnd->getBottomRightPos().y());
                        m_points.last().setX(mousePoint.x());
                    }
                }
            }
        }
    }
    
    /* Calculate the new positions of the literals, if there are any */
    moveLiterals();
}

/*!
 * \brief This function calculates the new position of the literals, after any position of asso item
 * points will be changed.  */
void AssociationItem::moveLiterals()
{
    /* Calculate the new position of the literals : they shall not be on the line, so rotate 45° on each position */
    if (m_pMultiplyBeginItem != NULL && !m_pMultiplyBeginItem->text().isEmpty())
    {
        QPointF rotatedPoint = rotatePoint(45, m_points[0], m_points[1]);
        QVector2D newPos = calcLiteralPos(m_points[0], rotatedPoint);
        m_pMultiplyBeginItem->setPos(m_points[0].x() + newPos.x(), m_points[0].y() + newPos.y());
    }
    if (m_pMultiplyEndItem != NULL && !m_pMultiplyEndItem->text().isEmpty())
    {
        QPointF rotatedPoint = rotatePoint(360-45, m_points.last(), m_points[m_points.count()-2]);
        QVector2D newPos = calcLiteralPos(m_points.last(), rotatedPoint);
        m_pMultiplyEndItem->setPos(m_points.last().x() + newPos.x(), m_points.last().y() + newPos.y());
    }
    if (m_pRoleBeginItem != NULL && !m_pRoleBeginItem->text().isEmpty())
    {
        QPointF rotatedPoint = rotatePoint(360-45, m_points[0], m_points[1]);
        QVector2D newPos = calcLiteralPos(m_points[0], rotatedPoint);
        m_pRoleBeginItem->setPos(m_points[0].x() + newPos.x(), m_points[0].y() + newPos.y());
    }
    if (m_pRoleEndItem != NULL && !m_pRoleEndItem->text().isEmpty())
    {
        QPointF rotatedPoint = rotatePoint(45, m_points.last(), m_points[m_points.count()-2]);
        QVector2D newPos = calcLiteralPos(m_points.last(), rotatedPoint);
        m_pRoleEndItem->setPos(m_points.last().x() + newPos.x(), m_points.last().y() + newPos.y());
    }
    if (m_pDescription != NULL && !m_pDescription->text().isEmpty())
    {
        /* Description shall be in the middle, if the num of points is even */
        int indPoint = m_points.count() / 2;
        if (m_points.count() % 2 == 0)
        {
            QVector2D newPos = calcDescriptionPos(m_points[indPoint-1], m_points[indPoint]);
            m_pDescription->setPos(m_points[indPoint-1].x() + newPos.x(), m_points[indPoint-1].y() + newPos.y());
        }
        /* Or above the middle point, if odd */
        else
        {
            int indPoint = m_points.count() / 2;
            m_pDescription->setPos(m_points[indPoint]);
        }
    }
    
    scene()->update();
}

void AssociationItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (m_points.count() < 2)
        return;

    for (int i = 0; i < m_points.count() - 1; ++i)
    {
        /* Draw the line using QLineF */
        QLineF line(m_points[i], m_points[i+1]);

        painter->setPen(QPen(Qt::black, 1, Qt::SolidLine));
        painter->drawLine(line);

        /* Draw the points as a rectangle for each one */
        painter->save();
        painter->setBrush(Qt::black);
        qreal rectWidthHalf = 3.0;
        qreal rectWidth = 6.0;
        painter->drawRect(m_points[i].x() - rectWidthHalf, m_points[i].y() - rectWidthHalf, rectWidth, rectWidth);
        if (i == m_points.count() - 2) // draw the last one too, the loop is till count-1
            painter->drawRect(m_points[i+1].x() - rectWidthHalf, m_points[i+1].y() - rectWidthHalf, rectWidth, rectWidth);
        painter->restore();
                
        /* Is item selected */
        if (isItemSelected)
        {
            /* Here only draw the selection lines */
            prepareGeometryChange(); 
            QPolygonF nPolygon;
            qreal radAngle = line.angle()* pi / 180;
            qreal dx = offset * sin(radAngle);
            qreal dy = offset * cos(radAngle);
            QPointF offset1 = QPointF(dx, dy);
            QPointF offset2 = QPointF(-dx, -dy);
            nPolygon << line.p1() + offset1 << line.p1() + offset2 << line.p2() + offset2 << line.p2() + offset1;
            m_Polygon = nPolygon;
            
            painter->setPen(QPen(Qt::darkGray, 1, Qt::DashLine));
            painter->drawPolygon(m_Polygon);
        }
    }
}

/*!
 * \brief This function will be called, if the user presses undo after adding a breakpoint. */
void AssociationItem::removeBreakpoint(int index)
{
    m_points.removeAt(index);
    m_idPoints.removeAt(index);
    scene()->update();
    moveLiterals();
}

void AssociationItem::removeMulFromBeginItem()
{
    //m_pMultiplyBeginItem->setText("");
    nsAssociationItem::DeleteLiteralCommand *command = new nsAssociationItem::DeleteLiteralCommand(m_pMultiplyBeginItem);
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
}

void AssociationItem::removeMulFromEndItem()
{
    //m_pMultiplyEndItem->setText("");
    nsAssociationItem::DeleteLiteralCommand *command = new nsAssociationItem::DeleteLiteralCommand(m_pMultiplyEndItem);
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
}

void AssociationItem::removeRoleFromBeginItem()
{
    //m_pRoleBeginItem->setText("");
    nsAssociationItem::DeleteLiteralCommand *command = new nsAssociationItem::DeleteLiteralCommand(m_pRoleBeginItem);
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
}

void AssociationItem::removeRoleFromEndItem()
{
    //m_pRoleEndItem->setText("");
    nsAssociationItem::DeleteLiteralCommand *command = new nsAssociationItem::DeleteLiteralCommand(m_pRoleEndItem);
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
}

void AssociationItem::removeDescription()
{
    //m_pDescription->setText("");
    nsAssociationItem::DeleteLiteralCommand *command = new nsAssociationItem::DeleteLiteralCommand(m_pDescription);
    ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
}


/*!
 * \brief This function rotates the point ptRotate over the origin point.
 * This is necessary for the positioning of the role and the multiplicities. */
QPointF AssociationItem::rotatePoint(int degree, const QPointF &ptOrigin, const QPointF &ptRotate)
{
    qreal rotAngle = degree * (pi / 180.0);
    qreal sinAngle = sin(rotAngle);
    qreal cosAngle = cos(rotAngle);

    /* Tranlate the point to the center of the axis-system */
    qreal px = ptRotate.x() - ptOrigin.x();
    qreal py = ptRotate.y() - ptOrigin.y();

    /* Rotate the point */
    qreal xnew = px * cosAngle + py * sinAngle;
    qreal ynew = py * cosAngle - px * sinAngle;
    
    /* translate point back */
    px = xnew + ptOrigin.x();
    py = ynew + ptOrigin.y();

    return QPointF(px, py);
}



/* Setters getters */

void AssociationItem::setFirstPoint(const QPointF &pt)
{
    m_points[0] = pt;
}

void AssociationItem::setLastPoint(const QPointF &pt)
{
    m_points.last() = pt;
}

/* Getters Setters */
ClassItem *AssociationItem::getItemEnd() const
{
    return m_ptClassItemEnd;
}

void AssociationItem::setItemEnd(ClassItem *value)
{
    m_ptClassItemEnd = value;
}

ClassItem *AssociationItem::getItemBegin() const
{
    return m_ptClassItemBegin;
}

void AssociationItem::setItemBegin(ClassItem *value)
{
    m_ptClassItemBegin = value;
}

QList<QPointF> AssociationItem::points() const
{
    return m_points;
}

void AssociationItem::setPoints(const QList<QPointF> &points)
{
    m_points = points;
    scene()->update();
    moveLiterals();
}

QList<QPointF> AssociationItem::oldPoints() const
{
    return m_oldPoints;
}

int AssociationItem::id() const
{
    return m_id;
}

void AssociationItem::setId(int value)
{
    m_id = value;
}

ITEM_TYPE AssociationItem::getRelationType()
{
    return m_relationType; // TODO typecast not good here
}

void AssociationItem::setRelationType(ITEM_TYPE type)
{
    m_relationType = type;
}

ClassItem *AssociationItem::getClassItemBegin() const
{
    return m_ptClassItemBegin;
}

void AssociationItem::setClassItemBegin(ClassItem *ptClassItemBegin)
{
    m_ptClassItemBegin = ptClassItemBegin;
}

ClassItem *AssociationItem::getClassItemEnd() const
{
    return m_ptClassItemEnd;
}

void AssociationItem::setClassItemEnd(ClassItem *ptClassItemEnd)
{
    m_ptClassItemEnd = ptClassItemEnd;
}

/*!
 * \brief This function adds the id of a point. Id is in database unique. */
void AssociationItem::addPointID(int id)
{
    m_idPoints.append(id);
}

/*!
 * \brief This function updates the id of a point. Id is in database unique. */
void AssociationItem::updatePointID(int index, int id)
{
    Q_ASSERT(index < m_idPoints.count());
    m_idPoints[index] = id;
}

int AssociationItem::getPointID(int index)
{
    return m_idPoints[index];
}

void AssociationItem::setMultiplicityBeginInfo(const QString &text, qreal posX, qreal posY)
{
    if (m_pMultiplyBeginItem == NULL)
    {
        m_pMultiplyBeginItem = new LiteralItem(this);
        m_pMultiplyBeginItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    }
    
    m_pMultiplyBeginItem->setPos(posX, posY);
    m_pMultiplyBeginItem->setText(text);
}

void AssociationItem::setMultiplicityEndInfo(const QString &text, qreal posX, qreal posY)
{
    if (m_pMultiplyEndItem == NULL)
    {
        m_pMultiplyEndItem = new LiteralItem(this);
        m_pMultiplyEndItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    }
    
    m_pMultiplyEndItem->setPos(posX, posY);
    m_pMultiplyEndItem->setText(text);
}

void AssociationItem::setRoleBeginInfo(const QString &text, qreal posX, qreal posY)
{
    if (m_pRoleBeginItem == NULL)
    {
        m_pRoleBeginItem = new LiteralItem(this);
        m_pRoleBeginItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    }
    
    m_pRoleBeginItem->setPos(posX, posY);
    m_pRoleBeginItem->setText(text);
}

void AssociationItem::setRoleEndInfo(const QString &text, qreal posX, qreal posY)
{
    if (m_pRoleEndItem == NULL)
    {
        m_pRoleEndItem = new LiteralItem(this);
        m_pRoleEndItem->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    }
    
    m_pRoleEndItem->setPos(posX, posY);
    m_pRoleEndItem->setText(text);
}

void AssociationItem::setDescriptionInfo(const QString &text, qreal posX, qreal posY)
{
    if (m_pDescription == NULL)
    {
        m_pDescription = new LiteralItem(this);
        m_pDescription->setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    }
    
    m_pDescription->setPos(posX, posY);
    m_pDescription->setText(text);    
}

/*!
 * \brief This function returns the info : text;pos_x;pos_y */
QString AssociationItem::getMultiplicityBeginInfo() const
{
    QString ret;
    if (m_pMultiplyBeginItem && !m_pMultiplyBeginItem->text().isEmpty())
        ret = QString("%1;%2;%3").arg(m_pMultiplyBeginItem->text())
                                 .arg(m_pMultiplyBeginItem->pos().x())
                                 .arg(m_pMultiplyBeginItem->pos().y());
                
    return ret;
}

/*!
 * \brief This function returns the info : text;pos_x;pos_y */
QString AssociationItem::getMultiplicityEndInfo() const
{
    QString ret;
    if (m_pMultiplyEndItem && !m_pMultiplyEndItem->text().isEmpty())
        ret = QString("%1;%2;%3").arg(m_pMultiplyEndItem->text())
                                 .arg(m_pMultiplyEndItem->pos().x())
                                 .arg(m_pMultiplyEndItem->pos().y());
    
    return ret;
}

/*!
 * \brief This function returns the info : text;pos_x;pos_y */
QString AssociationItem::getRoleBeginInfo() const
{
    QString ret;
    if (m_pRoleBeginItem && !m_pRoleBeginItem->text().isEmpty())
        ret = QString("%1;%2;%3").arg(m_pRoleBeginItem->text())
                                 .arg(m_pRoleBeginItem->pos().x())
                                 .arg(m_pRoleBeginItem->pos().y());
    
    return ret;
}

/*!
 * \brief This function returns the info : text;pos_x;pos_y */
QString AssociationItem::getRoleEndInfo() const
{
    QString ret;
    if (m_pRoleEndItem && !m_pRoleEndItem->text().isEmpty())
        ret = QString("%1;%2;%3").arg(m_pRoleEndItem->text())
                                 .arg(m_pRoleEndItem->pos().x())
                                 .arg(m_pRoleEndItem->pos().y());
    
    return ret;
}

/*!
 * \brief This function returns the info : text;pos_x;pos_y */
QString AssociationItem::getDescriptionInfo() const
{
    QString ret;
    if (m_pDescription && !m_pDescription->text().isEmpty())
        ret = QString("%1;%2;%3").arg(m_pDescription->text())
                                 .arg(m_pDescription->pos().x())
                                 .arg(m_pDescription->pos().y());
    
    return ret;
}


