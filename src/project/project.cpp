#include "project.h"
#include "../db/sqldatabase.h"
#include "../graphics/graphicsscene.h"
#include "../graphics/items/classitem.h"
#include "../graphics/items/associationitem.h"
#include "../gui/monitor/monitor.h"
#include "generatecode.h"
#include <QPrinter>
#include <QGraphicsView>
#include <QApplication>
#include <QRegularExpression>
#include <QDir>
#include <QFile>
#include <QUndoStack>
#include <QTextStream>

#include <QDebug>


Project::Project(QString &name, QString &path, int width, int height, GraphicsScene *scene)
    : m_strProjectName(name), m_strProjectPath(path), m_width(width), m_height(height), m_pScene(scene)
{
    m_pSQLDatabase = new SqlDatabase(m_strProjectPath, m_strProjectName);
    m_pSQLDatabase->createTables();
    m_bSetDirty = false;
    m_pGridLines = NULL;
    m_gridDensity = 20;
    m_bSnapGrid = true;
    m_pUndoStack = new QUndoStack(this);
}

/*!
 * \brief This function generates .c and .h files for each class item defined in the scene.
 * * The file has the extension .tmp for "template" in binary format. See BinaryWriter.
 * * The generated file will be stored in a folder /SGen in project path. */
void Project::generateCode()
{
    /* Get the exe path */
    QString tmpFilePath = QCoreApplication::applicationDirPath() + "/../templates/ClassTemplate.tmp";

    GenerateCode genCode;
    genCode.generate(tmpFilePath, m_strProjectPath);

    MonitorManager::getInstance()->logMsg("INFO : Code generated successfully", 0);
}

QList<int> Project::getRelatedAssociationItems(int classItemID)
{
    QList<int> assoIDs;
    
    /* Get the class items from scene which are stored in database */
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    
    ClassItem *classItem;
    /* Get the list of association items, which are in db (meaning, id != -1 */
    while (i.hasNext())
        if ((classItem = dynamic_cast<ClassItem*>(i.next())) != NULL)
            if (classItem->id() == classItemID)
                /* Class item found */
                break;
    
    i.toFront();
    while (i.hasNext())
        if (AssociationItem *assoItem = dynamic_cast<AssociationItem*>(i.next()))
            if (assoItem->associatedWithThisClass(classItem))
                assoIDs.append(assoItem->id());
    
    /* If asso items found, return an empty list */
    return assoIDs; 
}

/*!
 * \brief This function stores the items in the database. */
void Project::saveTheSceneToDatabase()
{
    /* Infos from the scene */
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    
    /* Remove all items from the database, which been deleted from the scene */
    QList<int> associationItemsInScene;
    while (i.hasNext())
        if (AssociationItem *item = dynamic_cast<AssociationItem*>(i.next()))
            /* If not stored into db yet */
            if (item->id() != -1)
                associationItemsInScene.append(item->id());
    /* Remove all the association items from the db, which dont exist in scene anymore */
    m_pSQLDatabase->removeAssociationItems(associationItemsInScene);
    
    /* Get the class items from scene which are stored in database */
    i.toFront();
    QList<ClassItem*> listClassItems;
    /* Store the item, if it is in db */
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            if (item->id() != -1)
                listClassItems.append(item);
    /* No these class items will not be removed, but the ones that do not exist in scene anymore and still stored in database */
    m_pSQLDatabase->removeDeletedClassItems(listClassItems);
      
    /* Add a new item or update the existing one */
    i.toFront();
    /* First add class items then associations */
    while (i.hasNext())
    {
        /* Gridlines will not be stored in the database */
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            /* Add this item to db or update the existing ones */
            /* If the item has not been stored to db yet */
            if (item->id() == -1)
            {
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("INFO : Adding the element %0 to database %1 ", 2,
                                                          item->className().toLocal8Bit().data(),
                                                          m_pSQLDatabase->databaseName().toLocal8Bit().data());
                m_pSQLDatabase->addNewClassItem(item, false); // false : no undo
            }
            else
            {
                /* Then the item will be updated */
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("INFO : Updating the element %0 in database %1 ", 2,
                                                          item->className().toLocal8Bit().data(),
                                                          m_pSQLDatabase->databaseName().toLocal8Bit().data());
                /* Now check, if the item is in db */
                if (m_pSQLDatabase->isClassItemInDB(item))
                    /* Then the item is in db, only update it. */
                    m_pSQLDatabase->updateClassItem(item, false); // false : no undo 
                else
                    /* info : if the user deletes the class item and then saves the project then undos once and 
                     * then tries to save again -> the item is in scene but not in db. Then it must be added. */
                    m_pSQLDatabase->addNewClassItem(item, true); // true : this is undo
            }
        }
    }
    /* Now associations can get class item id's */
    i.toFront();
    while (i.hasNext())
    {
        enrollment i ve seminar i sil...kaydet
        sonra iki defa undo yap kaydet-> bazi assolar eskik
        if (AssociationItem *item = dynamic_cast<AssociationItem*>(i.next()))
        {
            if (item->id() == -1)
            {
                /* Save */
                m_pSQLDatabase->addNewAssociationItem(item);
                 
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("INFO : Adding the association to database %0 ", 1,
                                                          m_pSQLDatabase->databaseName().toLocal8Bit().data());
            }
            else
            {
                /* Now check, if the item is in db */
                if (m_pSQLDatabase->isAssoItemInDB(item))
                    /* Update */
                     m_pSQLDatabase->updateAssociationItem(item);
                else
                    /* Save again : this is undo */
                    m_pSQLDatabase->addNewAssociationItem(item);
                 
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("INFO : Updating the association in database %0 ", 1,
                                                          m_pSQLDatabase->databaseName().toLocal8Bit().data());
                
            }
        }
    }
}

/*!
 * \brief This function searches a given project name in its list.
 * Returns true, if item is in this project. */
bool Project::find(const QString &name)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            if (item->className() == name)
                /* Item is in list */
                return true;
    
    return false;
}

/*!
 * \brief This function sets the id of the class item sent from the database. After a class item will be inserted
 * into a database, it gets an id automatically from the database. We need this id for further operations on database
 * such as updating the item information. */
void Project::setItemIdAsDbId(const QString &className, int id)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            if (item->className() == className)
                /* Set item id to id */
                item->setID(id);
}

/*!
 * \brief This function has the same purpose as setItemIdAsDbId() but for members only. */
void Project::setMemberIdAsDbId(const QString &className, const QString &memberName, int mID)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
    {
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            if (item->className() == className)
            {
                /* Set item id to id */
                QList<MemberProperties *> listMembers = item->getListMemberProperties();
                foreach(MemberProperties *member, listMembers)
                    if (member->name == memberName)
                        member->id = mID;
            }
        }
    }
}

/*!
 * \brief This function has the same purpose as setItemIdAsDbId() but for parameters only. */
void Project::setParameterIdAsDbId(const QString &className, const QString &parameterName, int fID, int pID)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
    {
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            if (item->className() == className)
            {
                /* Set item id to id */
                QList<FunctionProperties *> listFunctions = item->getListFunctionProperties();
                foreach(FunctionProperties *function, listFunctions)
                    if (function->id == fID)
                        foreach(ParameterProperties *parameter, function->parameters)
                            if (parameter->name == parameterName)
                                parameter->id = pID;
            }
        }
    }
}

///*!
// * \brief This function removes the class item from a list, which will be used to remove it from the db.
// * This is necessary, if the user removes the item and presses the "undo". */
//void Project::removeFromDeleteList(ClassItem *pClassItem)
//{
//    /* Remove the class item from the list */
//    for (int i = 0; i < m_pOnDeleteItems.count(); ++i)
//        if (m_pOnDeleteItems[i]->className() == pClassItem->className())
//            m_pOnDeleteItems.removeAt(i);
//    m_pOnDeleteItems in oldugu her yere class item ve assolarini ekle ve cikar (su anki durum : ayni listeye ekliyoruz )
//    /* And remove the related association items too : 
//     * at this point, the association items and the class item are in the scene. */
//    QList<QGraphicsItem*> allItems = m_pScene->items();
//    QListIterator<QGraphicsItem*> iter(allItems);
//    while (iter.hasNext())
//        if (AssociationItem *assoItem = dynamic_cast<AssociationItem*>(iter.next()))
//            if (assoItem->associatedWithThisClass(pClassItem))
//                how to find -- all asso items after classitem belong to this class item
//}

///*!
// * \brief This function adds the class item into a list, which will be used to remove it from the db. */
//void Project::addToDeleteList(ClassItem *pClassItem)
//{
//    m_pOnDeleteItems.append(pClassItem);
//}

/*!
 * \brief This function has the same purpose as setItemIdAsDbId() but for functions only.
 * A function is only with its return type, name and paramaters unique. */
void Project::setFunctionIdAsDbId(const QString &className, const QString &functionName, 
                                  const QString &returnType, const QList<ParameterProperties*> parameters, int fID)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
    {
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            if (item->className() == className)
            {
                /* Set item id to id */
                QList<FunctionProperties *> listFunctions = item->getListFunctionProperties();
                foreach(FunctionProperties *function, listFunctions)
                {
                    if (function->name == functionName && function->returnType == returnType && function->parameters.count() == parameters.count())
                    {
                        for (int i = 0; i < parameters.count(); ++i)
                            if (function->parameters[i] != parameters[i])
                                continue; // this is not the function we seek : this has different parameters
                                
                        /* We have found the function, set the id */
                        function->id = fID;
                    }
                }
            }
        }
    }
}

/*!
 * \brief This function returns the last index-no for the initial name "ClassItem".
 * For example ClassName_5 is the last item-name with index no 5, then return 6. */
int Project::getLastInstanceNo()
{
    int no = 0;
    int highestNo = 0;

    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            int lastIndex = item->className().lastIndexOf("_");
            QString rig = item->className().right(item->className().size()-lastIndex-1); // -1 for "_"
            no = rig.toInt();
            if (highestNo < no)
                highestNo = no;
        }
    
    return highestNo;
}

///*!
// * \brief This function counts the occurances of the given name in the items-list. */
//int Project::contains(const QString &name)
//{
//    int count = 0;
//    QList<QGraphicsItem*> items = m_pScene->items();
//    QListIterator<QGraphicsItem*> i(items);
//    while (i.hasNext())
//        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
//            if (item->className().contains(name))
//                count++;
    
//    return count;
//}

/*!
 * \brief This function renames the item in the project list and updates the scene. */
void Project::renameItem(const QString &oldName, const QString &newName)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            if (item->className() == oldName)
            {
                item->setClassName(newName, true);
                setDirty(true);
            }
}

/*!
 * \brief This function returns the class item of a given name. This function will be used
 * in TreeWidget to use the class item as an argument for DeleteCommand. */
void Project::updateClassItem(ClassItem *oldItem, ClassItem *newItem)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
    {
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            if (oldItem->className() == item->className())
            {
                /* Clear the old info */
                delete item;
                item = NULL;
                /* Item found, update info */
                item = newItem;
                /* If name changed, call renameItem */
                if (oldItem->className() != newItem->className())
                    renameItem(oldItem->className(), newItem->className());
            }
        }
    }
}

/*!
 * \brief This function returns the class item of a given name. This function will be used
 * in TreeWidget to use the class item as an argument to DeleteCommand. */
ClassItem *Project::getClassItem(const QString &name)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            if (item->className() == name)
            {
                return item;
            }
    
    return NULL;
}
  
/*!
 * \brief This function removes a class item from the scene. */
void Project::removeClassItem(const QString &name)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            if (item->className() == name)
            {
                /* Remove the class item from the scene
                 * PS : related asso items will be removed in nsClassItem::DeleteCommand */
                m_pScene->removeItem(item);
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("INFO : Item %0 deleted from the scene", 1, name.toLocal8Bit().data());
                /* And finisht the work here */
                return;
            }
}

/*!
 * \brief This function selects an item in the scene. For example when the user presses the item in the tree-list. */
void Project::selectItem(const QString &name)
{
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            if (item->className() == name)
            {
                /* Select this item */
                item->setSelected(true);
                /* Show the item in the center of the view */
                QList<QGraphicsView*> views = m_pScene->views();
                /* There is exact only one view */
                views.at(0)->centerOn(item);
            }
            else
            {
                /* If another item is selected, then de-select the others */
                item->setSelected(false);
            }
        }
}

/*!
 * \brief This function created the gridlines. There were three options tested, but this one is faster in rendering : 
 * moving items is faster than subclassing QGraphicsScene or QGraphicsItem (see documents).
 * 
 * The size is the page size of a default printer. TODO : setting as fit in pages or tile in pages */
void Project::createGridlines(QPrinter *printer)
{
    if (!m_pGridLines)
    {
        /* Get the page size of the printer */
        QSize size = printer->paperSize(QPrinter::Point).toSize();
        /* Line pen is transparent */
        QPen pen(QColor(150, 150, 150, 100));
        m_pGridLines = new QGraphicsItemGroup(); 
        
        /* Draw the page in black in the scene */
        QGraphicsRectItem *it = new QGraphicsRectItem(0, 0, size.width(), size.height());
        it->setZValue(-1100);
        m_pGridLines->addToGroup(it);
        
        /* TODO : delete or change this later - Write the info that this is A4 */
        QGraphicsTextItem *tItem = new QGraphicsTextItem("A4");
        tItem->setPos(size.width()/2, size.height()/2);
        tItem->setDefaultTextColor(QColor(150, 150, 150, 75));
        tItem->setScale(8.0f);

        m_pGridLines->addToGroup(tItem);
        
        /* Draw the transparent lines */
        for (int x = 0; x <= size.width(); x += m_gridDensity)
        {
            QGraphicsLineItem *item = new QGraphicsLineItem(x, 0, x, size.height());
            item->setPen(pen);
            item->setZValue(-1100);
            m_pGridLines->addToGroup(item);
        }

        for (int y = 0; y <= size.height(); y += m_gridDensity)
        {
            QGraphicsLineItem *item = new QGraphicsLineItem(0, y, size.width(), y);
            item->setPen(pen);
            item->setZValue(-1000);
            m_pGridLines->addToGroup(item);
        }
        
        /* Add the grid to the scene */
        scene()->addItem(m_pGridLines);
    }
}



/* ----------------------------------------------- */
/* All the functions below are setters and getters */
/* ----------------------------------------------- */


/*!
 * \brief This function shows or hides the gridlines. */
void Project::showGridlines(bool on)
{
    m_pGridLines->setVisible(on);
}

/*!
 * \brief This function returns all the items from the db. */
bool Project::loadItemsToScene(QList<QGraphicsObject *> &items)
{
    /* classes is out parameter */
    return m_pSQLDatabase->getAllSceneItems(items);
}

/*!
 * \brief This function returns the full path of the project. */
QString Project::fileNameFullPath()
{
    return QString("%1/%2.pj").arg(m_strProjectPath).arg(m_strProjectName);
}

QUndoStack *Project::undoStack() const
{
    return m_pUndoStack;
}

void Project::setSnapGrid(bool on)
{
    m_bSnapGrid = on;
}

bool Project::snapGrid()
{
    return m_bSnapGrid;
}

int Project::gridDensity() const
{
    return m_gridDensity;
}

bool Project::isGridVisible()
{
    return m_pGridLines->isVisible();
}

bool Project::dirty() const
{
    return m_bSetDirty;
}

void Project::setDirty(bool bSetDirty)
{
    /* If the state has not been changed, then dont send a signal */
    if (m_bSetDirty == bSetDirty)
        return;
    
    m_bSetDirty = bSetDirty;
    emit setProjectDirty(m_strProjectName);
}

int Project::width() const
{
    return m_width;
}

void Project::setWidth(int width)
{
    m_width = width;
}

int Project::height() const
{
    return m_height;
}

void Project::setHeight(int height)
{
    m_height = height;
}

QString Project::databasePath()
{
    return QString("%1/%2").arg(m_strProjectPath).arg(m_strProjectName);
}

QString Project::projectName()
{
    return m_strProjectName;
}

QString Project::projectPath()
{
    return m_strProjectPath;
}

GraphicsScene *Project::scene() const
{
    return m_pScene;
}

void Project::setScene(GraphicsScene *scene)
{
    m_pScene = scene;
}

QList<ClassItem *> *Project::getClassItems()
{
    // TOD mem leak here?
    QList<ClassItem*> *classItems = new QList<ClassItem*>();
    QList<QGraphicsItem*> items = m_pScene->items();
    QListIterator<QGraphicsItem*> i(items);
    while (i.hasNext())
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
            classItems->append(item);
    
    return classItems;
}



/* ----------------------------------------------- */
