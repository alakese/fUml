#include "classitem.h"
#include "../../project/projectmanagement.h"
#include "../../project/project.h"
#include "../../gui/dialogs/headerinfodlg.h"
#include "../../gui/dialogs/classitemdialog.h"
#include "../../globals/globals.h"
#include "../../graphics/graphicsscene.h"
#include "../../undo/classitem/nsClassItem_changefontcommand.h"
#include <QObject>
#include <QPainter>
#include <QTimer>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QGraphicsView>
#include <QAction>
#include <QMenu>
#include <QFontDialog>
#include <QKeyEvent>
#include <limits>

#include <QDebug>


// TODO add key events like <- to move left with one pix wide

using namespace std;

namespace
{
    /* To give some empty space between the borders and the text */
    const qint8 headerTextSpacing = 10;
    /* While changing the rect size into a bigger size, after the text will be splitted
       a last letter will not be appended to the original state because of a 
       "if control", which cuts the string earlier : if (textWidth > rectWidth). 
       So we put some marging to avoid it and to optimize the resizing, if not 
       necessary (e.g when rectWidth is bigger then the textWidth) */
    const qint8 resizingBufferControl = 15;
    /* When the item is selected, we need some place for resizing points */
    const qint8 itemIsSelectedWindowBuffer = 10;
    const qint8 minimumWidth = 100;
    const qint8 minimumHeightForClassItemParts = 20;
    /* This indicates that the position in m_ptOldPosition not valid scene-pos */
    const quint32 noSceneValidVal = numeric_limits<quint32>::max();
    /* This is spacing between header-text and member-line and between member-text and function-line */
    const qint8 spaceLineToText = 10;
    const qint8 spaceTextToLine = 10;
    const qint8 spaceTextToText = 10;
}

/*!
 * \brief This c'tor is for new items. The new items has no members or functions at beginning. */
ClassItem::ClassItem(GeneralProperties *generalProperties, GUIProperties *guiProperties)
{
    /* Id will be -1, if not inserted into db, else gets its id info from db */
    m_pGeneralProperties = generalProperties;
    
    setGUIProperties(guiProperties);
}

ClassItem::ClassItem(GeneralProperties *generalProperties, GUIProperties *guiProperties, 
                     const QList<MemberProperties *> &memberProperties, const QList<FunctionProperties *> &functionProperties)
{
    /* Id will be -1, if not inserted into db, else gets its id info from db */
    m_pGeneralProperties = generalProperties;
    m_listMemberProperties = memberProperties;
    m_listFunctionProperties = functionProperties;
   
    setGUIProperties(guiProperties);
}

/*!
 * \brief Returns the gui properties of a class item in a GUIProperties structure. */
GUIProperties *ClassItem::getGUIProperties() const
{
    return new GUIProperties(pos().toPoint(), boundingRect().toRect(), m_rectHeader.height(), m_rectMembers.height(), m_rectFunctions.height(), font());
}

/*!
 * \brief This function sets the gui property elements values. */
void ClassItem::setGUIProperties(GUIProperties *guiProperties)
{
    /* Using minimum heights */
    int widthClassItem = guiProperties->boundaryRect.width();
    int heightHeader = minimumHeightForClassItemParts;
    int heightBodyMembers = minimumHeightForClassItemParts;
    int heightBodyFunctions = minimumHeightForClassItemParts;
    /* Widht of the class item : same width for all three parts */
    m_minWidth = minimumWidth;
    /* If true, item size will be fixed and exact to the longest item-string
     * If false, item size will be user dependend. */
    m_bCalculateFixWidth = false;
    /* If true, the item names (info) will be splitted */
    m_bSplitTheString = false;
            
    /* If the given heights are more than minimum values, then use the given heights : e.g loading a project */
    if (guiProperties->headerHeight > heightHeader)
        heightHeader = guiProperties->headerHeight;
    if (guiProperties->membersHeight > heightBodyMembers)
        heightBodyMembers = guiProperties->membersHeight;
    if (guiProperties->functionsHeight > heightBodyFunctions)
        heightBodyFunctions = guiProperties->functionsHeight;
    
    /* Some settings */
    m_brushHeader.setColor(QColor(48, 243, 238));
    m_brushHeader.setStyle(Qt::SolidPattern);
    m_penHeader.setColor(Qt::black);
    m_penHeaderText.setColor(Qt::black);
    m_brushMembers.setColor(QColor(230, 230, 220));
    m_brushMembers.setStyle(Qt::SolidPattern);
    m_brushFunctions.setColor(QColor(230, 230, 220));
    m_brushFunctions.setStyle(Qt::SolidPattern);

    /* Set position in scene coordinates - setPos() uses the coordinates in scene */
    setPos(guiProperties->positionInScene.x(), guiProperties->positionInScene.y());    
    /* Rect-informations */
    m_rectHeader.setRect(0, 0, widthClassItem, heightHeader);
    m_rectMembers.setRect(0, heightHeader, widthClassItem, heightBodyMembers); 
    /* From the height of the header + members height */
    m_rectFunctions.setRect(0, heightHeader + heightBodyMembers, widthClassItem, heightBodyFunctions);
    /* Font properties */
    m_font = guiProperties->font;
    /* Font for undo commands */
    m_oldFont = guiProperties->font;
    
    /* Some class settings */        
    m_bCanResizeNow = false;
    m_bIsMouseOverTheResizePoint = false;
    setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable);
    setAcceptHoverEvents(true);
    
    /* Initialize the point with max-value, which indicates that there is no last position saved yet */
    m_ptOldPosition.setX(noSceneValidVal);
    m_ptOldPosition.setY(noSceneValidVal);
    
    /*                Bit4 Bit3 Bit2 Bit1 Bit0
     * Inital state :   0   0    1    0    1    (see DisplaysPage in propertieswindow) */
    m_displayOptions = BIT1 + BIT3;
    
    /* Default : Item can move - unless a relation-item selected */
    m_bMoveResize = true;
    m_bSendItemResSigToAssoItems = false;
}

/*!
 * \brief QGraphicsItem inherited function. Must implement the bounding border of the item. */
QRectF ClassItem::boundingRect() const
{
    return QRectF(0, 0, m_rectHeader.width(), m_rectHeader.height() + m_rectMembers.height() + m_rectFunctions.height());
}


/*!
 * \brief This function calls the properties dialog. */
void ClassItem::changeItemProperty(int currentPage)
{
    /* Save the old name, if the name is changed, we will send a signal to treewidget to update the view  */
    QString oldClassName = m_pGeneralProperties->name;
    /* Save the id of the item : id is not visible to the user, thus will not be used/get from the dialog */
    int classIDTmp = m_pGeneralProperties->id;
    /* Create the dialog with current info */
    ClassItemDialog dlg(m_pGeneralProperties, m_listMemberProperties, m_listFunctionProperties, currentPage, m_font, m_displayOptions);
    if (dlg.exec() == QDialog::Accepted)
    {
        /* Remove the properties from the memory first */
        delete m_pGeneralProperties;
        m_pGeneralProperties = NULL;

        /* Now get the info */
        m_pGeneralProperties = dlg.getGeneralInformation();
        m_listMemberProperties = dlg.getMemberInformation();
        m_listMemberToDelete = dlg.getDeleteMemberInformation();
        m_listFunctionProperties = dlg.getFunctionInformation();
        m_listFunctionToDelete = dlg.getDeleteFunctionInformation();
        m_listParameterToDelete = dlg.getDeleteParameterInformation();
        /* Set the id info back */
        m_pGeneralProperties->id = classIDTmp;

        /* Tell treewidget that class name changed, if changed */
        if (m_pGeneralProperties->name != oldClassName)
            emit itemRenamed(oldClassName, m_pGeneralProperties->name);

        /* Change the font */
        if (m_font != dlg.getNewFont())
        {
            m_oldFont = m_font;
            /* New font */        
            m_font = dlg.getNewFont();
            /* Create the change font command */
            nsClassItem::ChangeFontCommand *command = new nsClassItem::ChangeFontCommand(this);
            ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
        }

        m_displayOptions = dlg.getDisplayOptions();
        
        /* To activate the save button, if the property will be changed */
        emit itemPropertyChanged();
        
        /* If there are any association items, then send the size-info of resized item */
        m_bSendItemResSigToAssoItems = true;
    }
}

/*!
 * \brief This function opens a context menu, wenn item is right-clicked. */
void ClassItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    Q_UNUSED(event);
    /* Context menü opens when the user presses r-click on a class item */
    QMenu contextMenu;
    QAction *p_actProperties = contextMenu.addAction(QIcon(), tr("Properties..."));
    connect(p_actProperties, &QAction::triggered, this, &ClassItem::changeItemProperty);
    QAction *p_actDeleteItem = contextMenu.addAction(QIcon(), tr("Delete item"));
    connect(p_actDeleteItem, &QAction::triggered, this, &ClassItem::deleteClassItem);
    QAction *p_actChangeFont = contextMenu.addAction(QIcon(), tr("Change font..."));
    connect(p_actChangeFont, &QAction::triggered, this, &ClassItem::changeFont);
    contextMenu.addSeparator();
    QAction *p_actFixSize = contextMenu.addAction(QIcon(), tr("Fix size"));
    p_actFixSize->setCheckable(true);
    p_actFixSize->setChecked(m_bCalculateFixWidth);
    connect(p_actFixSize, &QAction::triggered, this, &ClassItem::fixItemSize);
    QAction *p_actSplitNames = contextMenu.addAction(QIcon(), tr("Split items"));
    p_actSplitNames->setCheckable(true);
    p_actSplitNames->setChecked(m_bSplitTheString);
    connect(p_actSplitNames, &QAction::triggered, this, &ClassItem::splitText);

    contextMenu.exec(QCursor::pos());
}

/*!
 * \brief This function opens a font dialog and changes the font settings. */
void ClassItem::changeFont()
{
    bool isOkPressed = false;
    /* Open font dialog */
    QFont font = QFontDialog::getFont(&isOkPressed, m_font);
    if (isOkPressed)
    {
        /* Store for ChangeFontCommand */
        m_oldFont = m_font;
        /* New font */        
        m_font = font;
        /* Create the change font command */
        nsClassItem::ChangeFontCommand *command = new nsClassItem::ChangeFontCommand(this);
        ProjectManager::getInstance()->getActiveProject()->undoStack()->push(command);
    }

    update();
}


/*!
 * \brief This function deletes a class item. */
void ClassItem::deleteClassItem()
{
    /* Tell treewidget to remove the item via DeleteCommand - we use here only one signal because we need to use
     * DeleteCommand() and DeleteCommand() needs to know the topLevelItem() from TreeWidget */
    emit classItemDeleted(m_pGeneralProperties->name);
}


/*!
 * \brief This function draws the header part of the class and writes the class name in it.
 * If the size of the text is bigger then the size of the rect, then the text will be cut
 * in parts and the height of the header will be resized. */
void ClassItem::drawItem(QPainter *painter)
{
    painter->setPen(m_penHeaderText);
    
    /* Font size */
    QFontMetrics fontMetrics(m_font);
    //QSize fontSize(fontMetrics.width(m_pGeneralProperties->name), fontMetrics.height());

    /* Font size and the size of a font in the scene are not proportional or accurate. The factor
     * 0.6 gives good results with different fonts and sizes. Therefore calculate it once, because
     * it will be called lots of times. */
    qint8 fontSizeInScene = (qint8)(fontMetrics.height() * 0.6f);
    
    /* First text position : the text's (0,0) point is not top-left, it is bottom-left. 
     * (x,y) goes to up, not down */
    qint8 firstSpaceCorrection = fontSizeInScene;
    
    /* Position of the first text here : this will be updated in the functions */
    int posHeightOfTextLines = firstSpaceCorrection + spaceLineToText;
    
    drawHeader(painter, &posHeightOfTextLines, fontSizeInScene);
    drawMembers(painter, &posHeightOfTextLines, fontSizeInScene);
    drawFunctions(painter, &posHeightOfTextLines, fontSizeInScene);
}

/*!
 * \brief This function draws the header part and writes the name, nameSpace and stereotype if given. */
void ClassItem::drawHeader(QPainter *painter, int *posHeightOfTextLines, qint8 fontSizeInScene)
{
    /* Font size */
    QFontMetrics fontMetrics(m_font);
    
    /* Does the name fit in the rect-width, if not split it */
    QString className;
    if (!m_pGeneralProperties->nameSpace.isEmpty())
    {
        /* Does the name involve nameSpace */
        className.append(m_pGeneralProperties->nameSpace);
        className.append("::");
    }
    className.append(m_pGeneralProperties->name);
    QStringList classNameSplitted = splitItemString(className);
            
    /* Is there a stereotype given? */
    int heightForStereotype = 0;
    QStringList stereotypeSplitted;
    if (!m_pGeneralProperties->stereotype.isEmpty())
    {
        QString stereo = m_pGeneralProperties->stereotype;
        stereo.prepend("<<");
        stereo.append(">>");
        stereotypeSplitted = splitItemString(stereo);
        /* One space to last part of the classname + fontsize */
        if (m_bSplitTheString)
            heightForStereotype = stereotypeSplitted.size() * (spaceTextToText + fontSizeInScene);
        else
            /* Hiding the text */
            heightForStereotype = spaceTextToText + fontSizeInScene;
    }

    /* Calculate the min width : if user selects fixed size */
    if (m_bCalculateFixWidth)
        setClassItemWidth(calculateMinWidth());
    
    /* Update rect info - change the size of the header-rect because of the text-size and stereotype, if given */
    if (m_bSplitTheString)
        /* Split the name */
        m_rectHeader.setHeight(spaceLineToText + classNameSplitted.size()*fontSizeInScene + 
                           (classNameSplitted.size() > 1 ? (classNameSplitted.size()-1)*spaceTextToText : 0) /* if there are e.g two strings, we need one spaceTextToText*/
                            + heightForStereotype + spaceTextToLine);
    else
        /* Hide the name : show only the part which fits in the item-area */
        m_rectHeader.setHeight(spaceLineToText + fontSizeInScene + heightForStereotype + spaceTextToLine);

    /* Draw the header */
    painter->setPen(m_penHeader);
    painter->fillRect(m_rectHeader, m_brushHeader);
    painter->drawRoundedRect(m_rectHeader, 1, 1);
   
    /* Is the class an abstract class? (then italic) */
    if (m_pGeneralProperties->isAbstract)
        m_font.setItalic(true);
    else
        m_font.setItalic(false);
    painter->setFont(m_font);
   
    /* Place the classname as splitted to the header rect-area from up to down */
    foreach(QString strTextLine, classNameSplitted) 
    {
        /* Center the text */
        int posBeginForCenteredText = m_rectHeader.x();
        int widthOfCurrLine = fontMetrics.width(strTextLine);
        int diff = m_rectHeader.width() - widthOfCurrLine;
        posBeginForCenteredText = posBeginForCenteredText + diff / 2.0;
        /* Draw it now */
        painter->drawText(posBeginForCenteredText, *posHeightOfTextLines, strTextLine);
        /* Calculate the pos of the next line */
        *posHeightOfTextLines += fontSizeInScene + spaceTextToText;
        /* If the name will not be splitted and the window is not fixed, then show only the first part of the name */
        /* Hide the rest */
        if (!m_bSplitTheString)
            break;
    }
    
    /* Set the font to back default state */
    m_font.setItalic(false);
    painter->setFont(m_font);
    
    /* -- STEREOTYPE -- */
    /* Place the stereotype under the class name */
    if (!m_pGeneralProperties->stereotype.isEmpty())
    {
        /* Add the stereotype chars */
        foreach(QString strTextLine, stereotypeSplitted) 
        {
            /* Calc the center for the text */
            int posCenter = m_rectHeader.x();
            int width = fontMetrics.width(strTextLine);
            int diff = m_rectHeader.width() - width;
            posCenter = posCenter + diff / 2.0;
            
            /* Draw the text */
            painter->drawText(posCenter, *posHeightOfTextLines, strTextLine);
            
            /* Add the new space to next line */
            *posHeightOfTextLines += fontSizeInScene + spaceTextToText;
            /* Hide the rest */
            if (!m_bSplitTheString)
                break;
        }
    }
}


/*!
 * \brief This function writes members. */
void ClassItem::drawMembers(QPainter *painter, int *posHeightOfTextLines, qint8 fontSizeInScene)
{
    /* Find how many members must be splitted */
    int membersCount = 0;
    
    /* If members empty or "show members" deactivated, then we need the min height : which is one item height */
    if (m_listMemberProperties.isEmpty() || GET_BIT(m_displayOptions, BIT1) == 0)
    {
        membersCount = 1;
    }
    else
    {
        /* Now split the string */
        foreach (MemberProperties *member, m_listMemberProperties)
        {
            /* If show only public activated */
            if (GET_BIT(m_displayOptions, BIT5) == 1 && member->visibility != PUBLIC)
                continue;

            /* If the user wants to see the text splitted */
            if (m_bSplitTheString)
            {                
                /* Calculate the size with splitted texts */
                QStringList memberSplitted = splitItemString(member->string(GET_BIT(m_displayOptions, BIT2) == 1));
                membersCount += memberSplitted.size();
            }
            else
            {
                /* Here we show only the first part of the text, rest of the text will be hidden */
                /* Count the members : if only public members will be shown, then count only public members */
                // membersCount = m_listMemberProperties.size();
                membersCount++;
            }
        }
    }
    
    /* Draw the members area : calculate the rect - if there is no members, calculate the min height for empty-area */
    int memHeight = spaceLineToText + membersCount*fontSizeInScene + (membersCount - 1)*spaceTextToText + spaceTextToLine;
    m_rectMembers.setRect(0, m_rectHeader.height(), m_rectMembers.width(), memHeight);
    
    /* Draw the rect */
    painter->fillRect(m_rectMembers, m_brushMembers);
    painter->drawRoundedRect(m_rectMembers, 1, 1);
    
    /* Calculate the text position */
    *posHeightOfTextLines += spaceLineToText;
    
    /* If there are no members, then the height must be min one item high because of functions */
    if (m_listMemberProperties.isEmpty() || GET_BIT(m_displayOptions, BIT1) == 0)
    {
        *posHeightOfTextLines += fontSizeInScene + spaceTextToLine;
    }
    else 
    {
        foreach(MemberProperties *member, m_listMemberProperties)
        {
            /* If show only public activated */
            if (GET_BIT(m_displayOptions, BIT5) == 1 && member->visibility != PUBLIC)
                continue;
            
            /* If member is static, then with underline */
            if (member->isStatic)
                m_font.setUnderline(true);
            else
                m_font.setUnderline(false);
            painter->setFont(m_font);
            
            /* Split the member string? */
            QStringList memberSplitted = splitItemString(member->string(GET_BIT(m_displayOptions, BIT2) == 1));
                        
            foreach (QString memberPart, memberSplitted)
            {
                /* Calculate next step */
                painter->drawText(0, *posHeightOfTextLines, memberPart);
                *posHeightOfTextLines += fontSizeInScene + spaceTextToText;
                
                /* If not splitted, then rest will be hidden */
                if (!m_bSplitTheString)
                    break;
            }
        }
    }
    
    /* Set the font to back default state */
    m_font.setItalic(false);
    painter->setFont(m_font);
}

/*!
 * \brief This function writes functions. */
void ClassItem::drawFunctions(QPainter *painter, int *posHeightOfTextLines, qint8 fontSizeInScene)
{
    /* Find how many members must be splitted */
    int functionsCount = 0;
    if (m_listFunctionProperties.isEmpty() || GET_BIT(m_displayOptions, BIT3) == 0)
    {
        /* If functions empty, then we need the min height : which is one item height */
        functionsCount = 1;
    }
    else
    {
        foreach(FunctionProperties *function, m_listFunctionProperties)
        {
            /* If show only public activated */
            if (GET_BIT(m_displayOptions, BIT5) == 1 && function->visibility != PUBLIC)
                continue;
                
            /* If the user wants to see the text splitted */
            if (m_bSplitTheString)
            {
                /* Calculate the size with splitted texts */
                QStringList functionSplitted = splitItemString(function->string(GET_BIT(m_displayOptions, BIT4) == 1));
                functionsCount += functionSplitted.size();
            }
            else
            {
                /* Here we show only the first part of the text, rest of the text will be hidden */
                // functionsCount = m_listFunctionProperties.size();
                functionsCount++;
            }
        }
    }
    /* Draw the functions area : calculate the rect - if there is no members, calculate the min height for empty-area */
    int funHeight = spaceLineToText + functionsCount*fontSizeInScene + (functionsCount - 1)*spaceTextToText + spaceTextToLine;
    m_rectFunctions.setRect(0, m_rectHeader.height() + m_rectMembers.height(), m_rectFunctions.width(), funHeight);
    
    /* Draw the rect */
    painter->fillRect(m_rectFunctions, m_brushFunctions);
    painter->drawRoundedRect(m_rectFunctions, 1, 1);
    
    /* Write the text */
    *posHeightOfTextLines += spaceLineToText;
    
    if (GET_BIT(m_displayOptions, BIT3) == 1)
    {
        foreach(FunctionProperties *function, m_listFunctionProperties)
        {
            /* If show only public activated */
            if (GET_BIT(m_displayOptions, BIT5) == 1 && function->visibility != PUBLIC)
                continue;
            
            /* If function is static, then with underline */
            if (function->isStatic)
                m_font.setUnderline(true);
            else
                m_font.setUnderline(false);
            
            /* If function is virtual, then italic */
            if (function->isVirtual)
                m_font.setItalic(true);
            else
                m_font.setItalic(false);
            painter->setFont(m_font);
            
            /* Split the member string? */
            QStringList functionSplitted = splitItemString(function->string(GET_BIT(m_displayOptions, BIT4) == 1));
            foreach (QString funcPart, functionSplitted)
            {
                /* Calculate next step */
                painter->drawText(0, *posHeightOfTextLines, funcPart);
                *posHeightOfTextLines += fontSizeInScene + spaceTextToText;
                
                /* If not splitted, then rest will be hidden */
                if (!m_bSplitTheString)
                    break;
            }
        }
    }
}

/*!
 * \brief This function draws the item as dotted item if selected. */
void ClassItem::drawItemAsSelectedDots(QPainter *painter)
{
    QRectF rec = boundingRect();
    QPen pen(Qt::white);
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);
    painter->drawRoundedRect(rec, 1, 1);
}

/*!
 * \brief This function draws the resize points on the corner(s). */
void ClassItem::drawResizePoints(QPainter *painter)
{
    /* Draw only one point for now : bottom right */
    QRectF rec = boundingRect();

    QBrush brush(Qt::SolidPattern);
    QPen pen(Qt::black);
    painter->setBrush(brush);
    painter->setPen(pen);
    /* Draw a rectangle to show to the user the resizing point-corner */
    QPainterPath triangle;
    /* Set pen to this point */
    triangle.moveTo (rec.width() - 8, rec.height());
    /* Draw a filled triangle at bottom-right corner for scaling the item */
    triangle.lineTo(rec.width(), rec.height());
    triangle.lineTo(rec.width(), rec.height() - 8);
    triangle.lineTo(rec.width() - 8, rec.height());
    painter->fillPath(triangle, brush);
}

/*!
 * \brief The class item's view can have a fix size, which is the longest member or function name. */
void ClassItem::fixItemSize()
{
    m_bCalculateFixWidth = !m_bCalculateFixWidth;
    scene()->update();
    // TODO item fixed olunca size büyüyor ama hover area büyümüypr. sadece itemi hareket ettirince oluyor.
}

/*!
 * \brief This function highlights the class item, wenn the mouse is over it. */
void ClassItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    m_brushHeader.setColor(QColor(128, 248, 244));
    m_brushMembers.setColor(QColor(245, 245, 235));
    m_brushFunctions.setColor(QColor(245, 245, 235));
    
    QGraphicsItem::hoverEnterEvent(event);
    
    /* We have to update the scene : if there are connection points on the class item, then they have to be updated to
     * otherwise the point vanishes */
    scene()->update();
}

/*!
 * \brief This function paints the class item with its default color, in order to end the 
 * highlighting, when the cursor is not over the class item. */
void ClassItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    m_brushHeader.setColor(QColor(48, 243, 238));
    m_brushMembers.setColor(QColor(230, 230, 220));
    m_brushFunctions.setColor(QColor(230, 230, 220));

    QGraphicsItem::hoverLeaveEvent(event);
    
    /* We have to update the scene : if there are connection points on the class item, then they have to be updated to
     * otherwise the point vanishes */
    scene()->update();
}

/*!
 * \brief This function determines, whether the mouse is on resize button. */
void ClassItem::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    /* If window is fixed, no resize possible */
    if (m_bCalculateFixWidth)
        return;

    QRectF rec = boundingRect();
    
    /* if the mouse is over the resize point, then the user can resize only if he presses the mouse button */
    if (isSelected() && event->pos().x() >= rec.width()-8 && event->pos().y() >= rec.height()-8)
    {
        this->setCursor(Qt::SizeFDiagCursor);
        /* See mouse press event at this point */
        m_bIsMouseOverTheResizePoint = true; 
    }
    else 
    {
        /* Otherwise no selection is possible */
        this->setCursor(Qt::ArrowCursor);
        m_bIsMouseOverTheResizePoint = false;
        m_bCanResizeNow = false;
    }
    
    QGraphicsItem::hoverMoveEvent(event);
}

/*!
 * \brief This function determines if the item can be resized. */
void ClassItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    /* if true, then mouse is over the little point for resizing */
    if (m_bIsMouseOverTheResizePoint)
    {
        /* Keep old size in mind for ResizeCommand */
        storeCurrentSize(); 
        /* user can resize now, until he releases the mouse button, unless the window is not fixed */
        if (!m_bCalculateFixWidth)
            m_bCanResizeNow = true;
    }

    /* Store the current position in scene-coordinates, to check after release if the item will be moved */
    storePosition();    
    
    QGraphicsItem::mousePressEvent(event);
}

/*!
 * \brief This function resizes the item and sends a signal to mainwindow to call resizecommand. */
void ClassItem::resizeItem(int newWidth, int newHeight)
{
    /* Sets header, member and function rects widths */        
    if (newWidth >= minimumWidth)
        setClassItemWidth(newWidth);
    
    /* When scaling, change only last part height which is functions */
    if (newHeight >= minimumHeightForClassItemParts)
        m_rectFunctions.setHeight(newHeight - (m_rectHeader.height() + m_rectMembers.height()));
    
    /* Call paint */
    update();
    
    /* Send a signal for ResizeCommand */
    emit itemScaled(this);
}

/*!
 * \brief This function stores the position for undo-commands. */
void ClassItem::storePosition()
{
    m_ptOldPosition = pos();
}

/*!
 * \brief This function resizes the item, if resize-conditions are met. */
void ClassItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_bMoveResize)
        /* Then the user is creating a relation-item, wait until he is finished */
        return;
    
    /* if true, then resize the item */
    if (m_bCanResizeNow) 
    {
        /* QGraphicsScene expects all items remain unchanged unless it is notified.
           If you want to change an item's geometry, you must first call this to allow 
           QGraphicsScene to update its bookkeeping...
           PS: There is a problem with resizing QGraphicsItem's Event-Area like hover-area.
               If we don't call this function, then the area for hover will not be updated. */  
        prepareGeometryChange();
        /* Resize now */
        qreal dx = event->pos().x() - event->lastPos().x();
        qreal dy = event->pos().y() - event->lastPos().y();
        
        int newWidth = m_rectHeader.width() + dx;
        int newHeight = m_rectFunctions.height() + dy;

        /* Sets header, member and function rects widths */        
        if (newWidth >= minimumWidth)
            setClassItemWidth(newWidth);
        
        /* When scaling, change only last part height which is functions */
        if (newHeight >= minimumHeightForClassItemParts)
            m_rectFunctions.setHeight(newHeight);
        
        /* Call paint */
        update();
        
        /* If there are any association items, then send the size-info of resized item */
        emit itemResizing(this);
    }
    else
    {
        /* Use inherited funtion, only if item will not be resized */
        QGraphicsItem::mouseMoveEvent(event);
        /* Now update the position for snap grid if on */
        if (ProjectManager::getInstance()->getActiveProject()->snapGrid())
        {
            /* Just calculate the snap points from its position*/
            QPoint pt(this->m_rectHeader.x(), this->m_rectHeader.y());
            /* We need to know the pos in scene coordinates */
            QPointF ptScene = mapToScene(pt);
            /* Get density from project TODO : put it in settings? */
            int density = ProjectManager::getInstance()->getActiveProject()->gridDensity();
            /* New points are just the corners in the grid-lines */
            setX(((int)(ptScene.x() / density)) * density);
            setY(((int)(ptScene.y() / density)) * density);
        }
        
        /* If there are any association items, then send the position of the moved item */
        emit itemMoving(this);
    }
}

/*!
 * \brief This function avoids resizing. */
void ClassItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_bCanResizeNow)
    {
        /* Send a signal for ResizeCommand */
        emit itemScaled(this);
        /* If mouse is released, item can not be resized anymore */
        m_bCanResizeNow = false;
    }
    
    /* Send a signal if the item is moved : signal for MoveCommand */
    if (pos() != m_ptOldPosition)
        emit itemMoved(this);
    else
    {
        /* Clear the last saved position */
        m_ptOldPosition.setX(noSceneValidVal);
        m_ptOldPosition.setY(noSceneValidVal);
    }
                
    QGraphicsItem::mouseReleaseEvent(event);
}

/*!
 * \brief ClassItem::mouseDoubleClickEvent */
void ClassItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
        return;
    
    /* Get the clicked area */
    if (event->pos().y() <= m_rectHeader.height())
    {
        /* Open the properties dialog : select the first page in the properties dialog */
        changeItemProperty(GENERALPAGE);
    }
    else if (event->pos().y() > m_rectHeader.height() && event->pos().y() <=  m_rectHeader.height() + m_rectMembers.height())
    {
        /* Open the properties dialog : select the second page in the properties dialog */
        changeItemProperty(MEMBERSPAGE);
    }
    else if (event->pos().y() > m_rectHeader.height() + m_rectMembers.height() && 
             event->pos().y() <= m_rectFunctions.height() + m_rectHeader.height() + m_rectMembers.height())
    {
        /* Open the properties dialog : select the third page in the properties dialog */
        changeItemProperty(FUNCTIONSPAGE);
    }
   
    QGraphicsItem::mouseDoubleClickEvent(event);
}

/*!
 * \brief QGraphicsItem inherited function. Paints the item. */
void ClassItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    /* Draw the item in three parts */
    drawItem(painter);

    /* If true, draw the resize point in only bottom-right corners */
    if (isSelected())
    {
        drawItemAsSelectedDots(painter);
        if (!m_bCalculateFixWidth)
            drawResizePoints(painter);
    }
    
    if (m_bSendItemResSigToAssoItems)
    {
        m_bSendItemResSigToAssoItems = false;
        /* Tell asso item, that the class item has been resized */
        emit itemResizing(this);
    }        
}


/*!
 * \brief If true, the names of the members and functions will be splitted by item's window size. */
void ClassItem::splitText()
{
    m_bSplitTheString = !m_bSplitTheString;
    scene()->update();
    update();
}


/*!
 * \brief This function splits the given text, if the text does not fit into the header rect. */
QStringList ClassItem::splitItemString(const QString &string)
{
    /* If fixed size selected, then dont split the class name */
    if (m_bCalculateFixWidth)
        return QStringList() << string;

    /* Now split the name */
    QStringList list;    
    
    /* Does it fit */
    QFontMetrics fontMetrics(m_font);
    int widthClassName = fontMetrics.width(string);
    
    /* if true, then it fits */
    if (widthClassName < m_rectHeader.width())
        list << string;

    /* if false, then resplitt the header text */
    else
    {
        QString strBuffer;
    
        /* An easy algo : take a char and put it in a row if it fits */
        for(int i = 0; i < string.length(); ++i) {
            /* Take the first letter */    
            QChar c = string.at(i);
            /* New possible string */
            QString strNew = strBuffer + QString(c);
            /* Does it fit */
            int width = fontMetrics.width(strNew);
            /* if true, then it fits... */
            if (width < m_rectHeader.width()) 
            {
                /* Also append it */
                strBuffer += c;
            }
            else
            {
                /* If not, then we reached the max quote, append the line */
                list.append(strBuffer);
                /* Clear the buffer */
                strBuffer = "";
                /* And begin with a new row with appending the last char */
                strBuffer = c;
            }
        }
        
        /* if true, then there are rest of the text in it, append */
        if (!strBuffer.isEmpty())
        {
            list.append(strBuffer);
        }
    }
    return list;
}

/*!
 * \brief This function calculates min width for the item's area. */
int ClassItem::calculateMinWidth()
{
    QFontMetrics fontMetrics(m_font);
    
    int width = 0;
    
    /* Does the name involve nameSpace */
    QString className;
    if (!m_pGeneralProperties->nameSpace.isEmpty())
    {
        className.append(m_pGeneralProperties->nameSpace);
        className.append("::");
    }
    className.append(m_pGeneralProperties->name);
    
    /* Check name */
    if (fontMetrics.width(className) > width)
        width = fontMetrics.width(className);
    
    /* Check stereotype */
    if (fontMetrics.width(m_pGeneralProperties->stereotype) > width)
        width = fontMetrics.width(m_pGeneralProperties->stereotype);
    
    /* Check members */
    foreach(MemberProperties *member, m_listMemberProperties)
    {
        int w = fontMetrics.width(member->string(GET_BIT(m_displayOptions, BIT2) == 1));
        if (w > width)
            width = w;
    }
    /* Check functions */
    foreach(FunctionProperties *function, m_listFunctionProperties)
    {
        int w = fontMetrics.width(function->string(GET_BIT(m_displayOptions, BIT4) == 1));
        if (w > width)
            width = w;
    }
   
    /* Width must have a minimum ? */
    if (width < minimumWidth)
        width = minimumWidth;
    
    return width;
}

/*!
 * \brief This function adds a new member. */
bool ClassItem::addMember(const QString &type, const QString &name)
{
    /* Check if this member exists */
    foreach (MemberProperties *tmp, m_listMemberProperties)
        if (tmp->name == name)
            return false;

    MemberProperties *newMember = new MemberProperties();
    newMember->type = type;
    newMember->name = name;
    /* These are default values */
    newMember->isStatic = false;
    newMember->visibility = PRIVATE;
    m_listMemberProperties.append(newMember);
    
    return true;
}

/*!
 * \brief This function adds a new function. */
bool ClassItem::addFunction(const QString &retType, const QString &name, const QList<ParameterProperties *> *parameters)
{
    /* Check if this function exists */
    foreach (FunctionProperties *function, m_listFunctionProperties)
    {
        if (function->name == name && function->returnType == retType && function->parameters.count() == parameters->count())
        {
            int matchCount = 0;
            for (int i = 0; i < function->parameters.count(); ++i)
                if (function->parameters[i]->type == parameters->at(i)->type)
                    matchCount++;
            
            /* Then found */
            if (matchCount == function->parameters.count())
                return false;
        }
    }
    
    FunctionProperties *newFunc = new FunctionProperties();
    newFunc->returnType = retType;
    newFunc->name = name;
    /* If there are parameters */
    newFunc->parameters = *parameters;
    /* These are default values */
    newFunc->isStatic = false;
    newFunc->isVirtual = false;
    newFunc->visibility = PUBLIC;
    m_listFunctionProperties.append(newFunc);
    
    return true;
}

/*!
 * \brief This function removes a member from the list. */
void ClassItem::removeMember(MemberProperties *member)
{
    for (int i = 0; i < m_listMemberProperties.count(); ++i)
        if (m_listMemberProperties[i]->name == member->name)
            m_listMemberProperties.removeAt(i);
}

/*!
 * \brief This function removes a function from the list. */
void ClassItem::removeFunction(FunctionProperties *function)
{
    for (int i = 0; i < m_listFunctionProperties.count(); ++i)
        if (m_listFunctionProperties[i]->name == function->name && m_listFunctionProperties[i]->returnType == function->returnType)
        {
            /* Check now for parameters */
            if (m_listFunctionProperties[i]->parameters.count() == 0 && function->parameters.count() == 0)
            {
                m_listFunctionProperties.removeAt(i);
                return;
            }
            else if (m_listFunctionProperties[i]->parameters.count() == function->parameters.count())
            {
                int countMatch = 0;
                for (int j = 0; j < function->parameters.count(); ++j)
                    if (m_listFunctionProperties[i]->parameters[j]->type == function->parameters[j]->type)
                        countMatch++;
                
                /* If every parameter has the same type */
                if (function->parameters.count() == countMatch)
                {
                    m_listFunctionProperties.removeAt(i);
                    return;
                }
            }
        }
}

/* ----------------------------------------------- */
/* All the functions below are setters and getters */
/* ----------------------------------------------- */

/*!
 * \brief This function stores the current size of the class item. If the size changes, this item will send a signal to ResizeCommand
 * with the new size-information. A class item has three parts. That part which will be resized will be sent. */
void ClassItem::storeCurrentSize()
{
    m_oldWidth = m_rectHeader.width();
    m_oldHeightHeader = m_rectHeader.height();
    m_oldHeightFunctions = m_rectFunctions.height();
    m_oldHeightMembers = m_rectMembers.height();
}

/*!
 * \brief This function will be called, if an item will be deleted from the scene and then saved/stored into db.
 * If then the user redo's the item (item will be inserted into scene again) and saves again, these item will become
 * new DB-IDs. */
void ClassItem::resetIDs()
{
    /* Reset all IDs */
    m_pGeneralProperties->id = -1;
    
    foreach (MemberProperties *member, m_listMemberProperties)
        member->id = -1;
    
    foreach (FunctionProperties *function, m_listFunctionProperties)
    {
        function->id = -1;
        foreach (ParameterProperties *parameter, function->parameters)
            parameter->id = -1;
    }
}

/*!
 * \brief This function is a helper class to set the width of the class item.
 * Class item has a three parts, which have the same width but different heights. */
void ClassItem::setClassItemWidth(int width) 
{
    m_rectHeader.setWidth(width);
    m_rectMembers.setWidth(width);
    m_rectFunctions.setWidth(width);
}

/*!
 * \brief This function sets the new class name and calls update() to calculate the new splitts. */
void ClassItem::setClassName(const QString &className, bool isUpdate)
{
    m_pGeneralProperties->name = className;
    if (isUpdate)
        update();
}

int ClassItem::id() const
{
    return m_pGeneralProperties->id;
}

void ClassItem::setID(int id)
{
    m_pGeneralProperties->id = id;
}

QString ClassItem::className() const
{
    return m_pGeneralProperties->name;
}

QFont ClassItem::fontHeaderText() const
{
    return m_font;
}

void ClassItem::setFontHeaderText(const QFont &fontHeaderText)
{
    m_font = fontHeaderText;
}

QRect ClassItem::rectFunctions() const
{
    return m_rectFunctions;
}

void ClassItem::setRectFunctions(const QRect &rectBodyFunctions)
{
    m_rectFunctions = rectBodyFunctions;
}

QRect ClassItem::rectMembers() const
{
    return m_rectMembers;
}

void ClassItem::setRectMembers(const QRect &rectBodyMembers)
{
    m_rectMembers = rectBodyMembers;
}

QRect ClassItem::rectHeader() const
{
    return m_rectHeader;
}

void ClassItem::setRectHeader(const QRect &rectHeader)
{
    m_rectHeader = rectHeader;
}

QPointF ClassItem::oldPosition() const
{
    return m_ptOldPosition;
}

int ClassItem::oldWidth() const
{
    return m_oldWidth;
}

int ClassItem::oldHeightHeader() const
{
    return m_oldHeightHeader;
}

int ClassItem::oldHeightFunctions() const
{
    return m_oldHeightFunctions;
}

int ClassItem::oldHeightMembers() const
{
    return m_oldHeightMembers;
}

QFont ClassItem::font() const
{
    return m_font;
}

void ClassItem::setFont(const QFont &font)
{
    m_font = font;
    update();
}

QFont ClassItem::oldFont() const
{
    return m_oldFont;
}

GeneralProperties *ClassItem::getGeneralProperties() const
{
    return m_pGeneralProperties;
}

void ClassItem::setGeneralProperties(GeneralProperties *generalProperties)
{
    m_pGeneralProperties = generalProperties;
}

QList<MemberProperties *> ClassItem::getListMemberProperties() const
{
    return m_listMemberProperties;
}

void ClassItem::setListMemberProperties(const QList<MemberProperties *> &listMemberProperties)
{
    m_listMemberProperties = listMemberProperties;
}

QList<FunctionProperties *> ClassItem::getListFunctionProperties() const
{
    return m_listFunctionProperties;
}

void ClassItem::setListFunctionProperties(const QList<FunctionProperties *> &listFunctionProperties)
{
    m_listFunctionProperties = listFunctionProperties;
}

QList<MemberProperties *> ClassItem::getListMembersToDelete()
{
    return m_listMemberToDelete;
}

QList<FunctionProperties *> ClassItem::getListFunctionsToDelete()
{
    return m_listFunctionToDelete;
}

QList<ParameterProperties *> ClassItem::getListParametersToDelete()
{
    return m_listParameterToDelete;
}

bool ClassItem::hasCtor()
{
    foreach (FunctionProperties *function, m_listFunctionProperties)
        if (function->name == className())
            return true;
    
    return false;
}

QPointF ClassItem::getTopLeftPos()
{
    return pos();
}

QPointF ClassItem::getTopRightPos()
{
    QPointF pt(pos());
    pt.setX(pt.x() + boundingRect().width());
    
    return pt;
}

QPointF ClassItem::getBottomLeftPos()
{
    QPointF pt(pos());
    pt.setY(pt.y() + boundingRect().height());
    
    return pt;
}

QPointF ClassItem::getBottomRightPos()
{
    QPointF pt(pos());
    pt.setX(pt.x() + boundingRect().width());
    pt.setY(pt.y() + boundingRect().height());
    
    return pt;
}

/*!
 * \brief These functions will be called, when a relation-item will be selected and drawn. */
bool ClassItem::canMoveResize() const
{
    return m_bMoveResize;
}

void ClassItem::setMoveResize(bool bMoveResize)
{
    m_bMoveResize = bMoveResize;
}


/* ----------------------------------------------- */
