#include "sqldatabase.h"
#include "../graphics/items/classitem.h"
#include "../graphics/items/associationitem.h"
#include "../project/projectmanagement.h"
#include "../project/project.h"
#include "../gui/monitor/monitor.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QDebug>

typedef struct _PointInfos
{
    int id;
    QPointF point;
    int drawingOrder;
} PointInfos;


SqlDatabase::SqlDatabase(const QString &dbPath, const QString &dbName) : 
    m_strDBName(dbName), m_strDBPath(dbPath)
{
    m_strDBFullPath = m_strDBPath + "/" + m_strDBName;
}

/*!
 * \brief This function creates tables, if not created. */
void SqlDatabase::createTables()
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db"); // TODO change sqlite to db
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR: DB-Connection to %0 failed while creating table(s)! : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());            
            return;
        }
    
        /* 1) Create table : ClassItem */
        QString queryString =   "CREATE  TABLE  IF NOT EXISTS ClassItem ("
                                "ID INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE , "
                                "Name TEXT NOT NULL  UNIQUE , xScene INTEGER NOT NULL , yScene INTEGER NOT NULL ,"
                                "x INTEGER NOT NULL , y INTEGER NOT NULL , Width INTEGER NOT NULL , Height INTEGER NOT NULL , "
                                "HeaderHeight INTEGER NOT NULL , MembersHeight INTEGER NOT NULL, FunctionsHeight INTEGER NOT NULL , "
                                "Font VARCHAR NOT NULL )";

        if (!executeQuery(queryString, "Can not create the Table ClassItem"))
        {
            sqlDatabase.close();
            return;
        }
        
        /* 2) Create table : GeneralProperties */
        queryString = "CREATE  TABLE  IF NOT EXISTS GeneralProperties ("
                "ID INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE , "
                "cID INTEGER NOT NULL, Stereotype TEXT, Package TEXT, "
                "isAbstract BOOL NOT NULL, Visibility INTEGER NOT NULL )";
        if (!executeQuery(queryString, "Can not create the Table GeneralProperties"))
        {
            sqlDatabase.close();
            return;
        }
        
        /* 3) Create table : MemberProperties */
        queryString = "CREATE  TABLE  IF NOT EXISTS MemberProperties ("
                "ID INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE , "
                "cID INTEGER NOT NULL , Type TEXT NOT NULL , Name TEXT NOT NULL , "
                "InitialValue TEXT, Stereotype TEXT, isStatic BOOL NOT NULL , "
                "Visibility INTEGER NOT NULL , Description TEXT )";
        if (!executeQuery(queryString, "Can not create the Table MemberProperties"))
        {
            sqlDatabase.close();
            return;
        }
        
        /* 4) Create table : FunctionProperties */
        queryString = "CREATE  TABLE  IF NOT EXISTS FunctionProperties ("
                "ID INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE , "
                "cID INTEGER NOT NULL , ReturnType TEXT NOT NULL , Name TEXT NOT NULL , "
                "Stereotype TEXT, isStatic BOOL NOT NULL , isVirtual BOOL NOT NULL , "
                "Visibility INTEGER NOT NULL , Description TEXT, Code TEXT )";
        if (!executeQuery(queryString, "Can not create the Table FunctionProperties"))
        {
            sqlDatabase.close();
            return;
        }
        
        /* 5) - Create table : ParameterProperties */
        queryString = "CREATE  TABLE  IF NOT EXISTS ParameterProperties ("
                "ID INTEGER PRIMARY KEY  NOT NULL  UNIQUE , fID INTEGER NOT NULL , "
                "Type TEXT NOT NULL , Name TEXT NOT NULL , InitialValue TEXT, "
                "Stereotype TEXT, Direction INTEGER NOT NULL )";
        if (!executeQuery(queryString, "Can not create the Table ParameterProperties"))
        {
            sqlDatabase.close();
            return;
        }
        
        /* 6) - Create table : RelationItem */
        queryString = "CREATE  TABLE  IF NOT EXISTS RelationItem ("
                "ID INTEGER PRIMARY KEY  NOT NULL  UNIQUE , RelationType INTEGER NOT NULL , "
                "cIDBegin INTEGER NOT NULL , cIDEnd INTEGER NOT NULL , "
                "MultiplicityBegin TEXT , MultiplicityEnd TEXT , RoleBegin TEXT , RoleEnd TEXT , Description TEXT )";
        if (!executeQuery(queryString, "Can not create the Table Relation"))
        {
            sqlDatabase.close();
            return;
        }
        
        /* 7) - Create table : RelationPoints */
        queryString = "CREATE  TABLE  IF NOT EXISTS RelationPoints ("
                "ID INTEGER PRIMARY KEY  NOT NULL  UNIQUE , rID INTEGER NOT NULL , "
                "x INTEGER NOT NULL , y INTEGER NOT NULL , DrawingOrder INTEGER NOT NULL )";
        if (!executeQuery(queryString, "Can not create the Table RelationPoints"))
        {
            sqlDatabase.close();
            return;
        }
        
        /* Close the connection to db */
        sqlDatabase.close();
        
        /* Send the info to monitor */
        if (MonitorManager::getInstance()->isDebugChecked())
            MonitorManager::getInstance()->logMsg("DEBUG : Table(s) are created succesfully!", 0);
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while creating table(s)!", 0);
    }
}

/*!
 * \brief This function executes the given sql command as parameter. */
bool SqlDatabase::executeQuery(const QString &queryString, const QString &errorMsg)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    
    query.prepare(queryString);
    if (!query.exec())
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : %0 %1", 2, errorMsg.toLocal8Bit().data(), query.lastError().text().toLocal8Bit().data());
        return false;
    }
    
    return true;
}

/*!
 * \brief This functions adds a new class item into the db. */
bool SqlDatabase::addNewClassItem(ClassItem *item, bool bUndo)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR: DB-Connection to %0 failed while adding a new item! : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        /* Add a new class into db.
           Inserting an entry into Tables ClassItem and GeneralProperties together at the same time. */
        query.setForwardOnly(true);
        /* 1. We use ignore, not to try to add a item, which is in db - IGNORE can be used only when
              the table has unique col(s) like here Name -
           2. Changing use with replace; if the entry is in db, then it will be replaced... PROB: ID will be incremented for even only one element
              by each replace-action.
           3. Using update = for example --> UPDATE ClassItem SET Name='hall' WHERE ID = 5 */
        query.prepare("INSERT OR IGNORE INTO ClassItem(Name, xScene, yScene, x, y, Width, Height, HeaderHeight, MembersHeight, FunctionsHeight, Font)"
                      "VALUES (:Name, :xScene, :yScene, :x, :y, :Width, :Height, :HeaderHeight, :MembersHeight, :FunctionsHeight, :Font)");
        query.bindValue(":Name", item->getGeneralProperties()->name);
        query.bindValue(":xScene", item->getGUIProperties()->positionInScene.x());
        query.bindValue(":yScene", item->getGUIProperties()->positionInScene.y());
        query.bindValue(":x", item->getGUIProperties()->boundaryRect.x());
        query.bindValue(":y", item->getGUIProperties()->boundaryRect.y());
        query.bindValue(":Width", item->getGUIProperties()->boundaryRect.width());
        query.bindValue(":Height", item->getGUIProperties()->boundaryRect.height());
        query.bindValue(":HeaderHeight", item->getGUIProperties()->headerHeight);
        query.bindValue(":MembersHeight", item->getGUIProperties()->membersHeight);
        query.bindValue(":FunctionsHeight", item->getGUIProperties()->functionsHeight);
        /* INFO : font family, pointSizeF, pixelSize, QFont::StyleHint, QFont::Weight, QFont::Style, underline, strikeOut, fixedPitch, rawMode */
        query.bindValue(":Font", item->getGUIProperties()->font.toString());
        
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not add the new item %0 into DB! (1) : %1", 2,
                                                  item->getGeneralProperties()->name.toLocal8Bit().data(),
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }

        /* Get ClassItem ID from the db */
        int classID = query.lastInsertId().toInt();
        item->setID(classID);
        
        /* Class Id must be saved into the item too, in order to decide if the item will be inserted or updated */
        ProjectManager::getInstance()->getActiveProject()->setItemIdAsDbId(item->getGeneralProperties()->name, classID);
        
        /* Entry into GeneralProperties */
        QSqlQuery queryGen(QSqlDatabase::database("DBConnectionName"));
        queryGen.setForwardOnly(true);
        queryGen.prepare("INSERT OR IGNORE INTO GeneralProperties(cID, Stereotype, Package, isAbstract, Visibility)"
                      "VALUES (:cID, :Stereotype, :Package, :isAbstract, :Visibility)");
        queryGen.bindValue(":cID", classID);
        queryGen.bindValue(":Stereotype", item->getGeneralProperties()->stereotype);
        queryGen.bindValue(":Package", item->getGeneralProperties()->nameSpace);
        queryGen.bindValue(":isAbstract", item->getGeneralProperties()->isAbstract);
        queryGen.bindValue(":Visibility", (int)item->getGeneralProperties()->visibility);
        
        if (!queryGen.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not add the new item %0 into DB! (2) : %1", 2,
                                                  item->getGeneralProperties()->name.toLocal8Bit().data(),
                                                  queryGen.lastError().text().toLocal8Bit().data());
            return false;
        }

        /* Add members */
        foreach(MemberProperties *member, item->getListMemberProperties())
        {
            if (!insertUpdateMember(item, member, classID, bUndo))
            {
                sqlDatabase.close();
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("ERROR : Can not add the new class item member info %0 - %1", 2,
                                                      member->name.toLocal8Bit().data(),
                                                      query.lastError().text().toLocal8Bit().data());
                return false;
            }
        }

        /* Add functions */
        foreach(FunctionProperties *function, item->getListFunctionProperties())
        {
            if (!insertUpdateFunction(item, function, classID, bUndo))
            {
                sqlDatabase.close();
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("ERROR : Can not add the new class item function info %0 - %1", 2,
                                                      function->name.toLocal8Bit().data(),
                                                      query.lastError().text().toLocal8Bit().data());
                return false;
            }
        }


        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("INFO : A new class added to DB", 0);

        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while adding a new item", 0);
        return false;
    }
    
    return true;
}

/*!
 * \brief This function adds an association item into database. */
bool SqlDatabase::addNewAssociationItem(AssociationItem *item)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR: DB-Connection to %0 failed while adding a new item! : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        /* Add a new association into db */
        query.setForwardOnly(true);
        query.prepare("INSERT OR IGNORE INTO RelationItem(RelationType, cIDBegin, cIDEnd, MultiplicityBegin, MultiplicityEnd,"
                      "RoleBegin, RoleEnd, Description)"
                      "VALUES (:RelationType, :cIDBegin, :cIDEnd, :MultiplicityBegin, :MultiplicityEnd, "
                      ":RoleBegin, :RoleEnd, :Description)");
        query.bindValue(":RelationType", item->getRelationType());
        query.bindValue(":cIDBegin", item->getItemBegin()->id());
        query.bindValue(":cIDEnd", item->getItemEnd()->id());
        query.bindValue(":MultiplicityBegin", item->getMultiplicityBeginInfo());
        query.bindValue(":MultiplicityEnd", item->getMultiplicityEndInfo());
        query.bindValue(":RoleBegin", item->getRoleBeginInfo());
        query.bindValue(":RoleEnd", item->getRoleEndInfo());
        query.bindValue(":Description", item->getDescriptionInfo());
        
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not add the new association item into DB! (1) : %0", 1,
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }

        /* Get ClassItem ID from the db */
        int assoID = query.lastInsertId().toInt();
        item->setId(assoID);

        /* Add points */
        int index = 0;
        foreach(QPointF pt, item->points())
        {
            QSqlQuery queryPt(QSqlDatabase::database("DBConnectionName"));
            /* Add a new association into db */
            queryPt.setForwardOnly(true);
            
            queryPt.prepare("INSERT OR IGNORE INTO RelationPoints(rID, x, y, DrawingOrder)"
                          "VALUES (:rID, :x, :y, :DrawingOrder)");
            queryPt.bindValue(":rID", item->id());
            queryPt.bindValue(":x", pt.x());
            queryPt.bindValue(":y", pt.y());
            /* The order of the points important to draw them */
            queryPt.bindValue(":DrawingOrder", index); 
            
            if (!queryPt.exec())
            {
                sqlDatabase.close();
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("ERROR : Can not add the new association item into DB! (1) : %0", 1,
                                                      queryPt.lastError().text().toLocal8Bit().data());
                return false;
            }
            
            /* Store the id for update */
            int ptID = queryPt.lastInsertId().toInt();
            item->updatePointID(index, ptID);
            
            ++index;
        }
            
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("INFO : A new association added to DB", 0);

        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while adding a new association item", 0);
        return false;
    }
    
    return true;
}

/*!
 * \brief This function returns all the items, which were in the scene. */
bool SqlDatabase::getAllSceneItems(QList<QGraphicsObject *> &items)
{
    bool retVal = getClassItems(items);
    retVal &= getAssociationItems(items);
    
    return retVal;
}

/*!
 * \brief This function returns the asso items. When the user loads a project, this function will be called. */
bool SqlDatabase::getAssociationItems(QList<QGraphicsObject *> &items)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");

        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while loading a project :  : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());
            return false;
        }

        QSqlQuery itemQuery(QSqlDatabase::database("DBConnectionName"));
        itemQuery.setForwardOnly(true);
        itemQuery.exec("SELECT * FROM RelationItem");
        
        /* For each asso item */
        while (itemQuery.next())
        {
            AssociationItem *assoItem = new AssociationItem();
            /* Store the class id and name */
            assoItem->setId(itemQuery.value(eRelationItem::ID).toInt());
            int type = itemQuery.value(eRelationItem::RELATIONTYPE).toInt();
            switch(type)
            {
            case 0: 
                assoItem->setRelationType(ASSOCIATION);
                break;
            case 1: 
                assoItem->setRelationType(AGGREGATION);
                break;
            case 2: 
                assoItem->setRelationType(COMPOSITION);
                break;
            }
            
            /* Find the class items */
            int classId1 = itemQuery.value(eRelationItem::cIDBEGIN).toInt();
            int classId2 = itemQuery.value(eRelationItem::cIDEND).toInt();
            foreach (QGraphicsObject *nextItem, items)
            {
                if (ClassItem *classItem = dynamic_cast<ClassItem*>(nextItem))
                {
                    if (classItem->id() == classId1)
                        assoItem->setClassItemBegin(classItem);

                    if (classItem->id() == classId2)
                        assoItem->setClassItemEnd(classItem);
                }
            }
            /* Set the multiplicity infos */
            {
                qreal posx, posy;
                QString mulInfo, text;
                QStringList mulInfoArr;
                
                mulInfo = itemQuery.value(eRelationItem::MULTIPLICITYBEGIN).toString();
                if (!mulInfo.isEmpty())
                {
                    mulInfoArr = mulInfo.split(";");
                    text = mulInfoArr[0];
                    posx = mulInfoArr[1].toFloat();
                    posy = mulInfoArr[2].toFloat();
                    assoItem->setMultiplicityBeginInfo(text, posx, posy);
                }
                
                mulInfo = itemQuery.value(eRelationItem::MULTIPLICITYEND).toString();
                if (!mulInfo.isEmpty())
                {
                    mulInfoArr = mulInfo.split(";");
                    text = mulInfoArr[0];
                    posx = mulInfoArr[1].toFloat();
                    posy = mulInfoArr[2].toFloat();
                    assoItem->setMultiplicityEndInfo(text, posx, posy);
                }
            }

            /* Set the role infos */
            {
                qreal posx, posy;
                QString roleInfo, text;
                QStringList roleInfoArr;
                
                roleInfo = itemQuery.value(eRelationItem::ROLEBEGIN).toString();
                if (!roleInfo.isEmpty())
                {
                    roleInfoArr = roleInfo.split(";");
                    text = roleInfoArr[0];
                    posx = roleInfoArr[1].toFloat();
                    posy = roleInfoArr[2].toFloat();
                    assoItem->setRoleBeginInfo(text, posx, posy);
                }
                
                roleInfo = itemQuery.value(eRelationItem::ROLEEND).toString();
                if (!roleInfo.isEmpty())
                {
                    roleInfoArr = roleInfo.split(";");
                    text = roleInfoArr[0];
                    posx = roleInfoArr[1].toFloat();
                    posy = roleInfoArr[2].toFloat();
                    assoItem->setRoleEndInfo(text, posx, posy);
                }
            }
            
            /* Set the description */
            {
                qreal posx, posy;
                QString descInfo, text;
                QStringList descInfoArr;
                
                descInfo = itemQuery.value(eRelationItem::DESCRIPTION).toString();
                if (!descInfo.isEmpty())
                {
                    descInfoArr = descInfo.split(";");
                    text = descInfoArr[0];
                    posx = descInfoArr[1].toFloat();
                    posy = descInfoArr[2].toFloat();
                    assoItem->setDescriptionInfo(text, posx, posy);
                }
            }
            /* Set the points */
            {
                QSqlQuery pointsQuery(QSqlDatabase::database("DBConnectionName"));
                pointsQuery.setForwardOnly(true);
                /* Get the related points */
                pointsQuery.prepare("SELECT * FROM RelationPoints WHERE rID=:rID");
                pointsQuery.bindValue(":rID", assoItem->id());

                if (!pointsQuery.exec())
                {
                    sqlDatabase.close();
                    /* Send the info to monitor - convert QString to char * */
                    MonitorManager::getInstance()->logMsg("ERROR : Can not read association item from DB! (1) : %0", 1,
                                                          pointsQuery.lastError().text().toLocal8Bit().data());
                    return false;
                }
                
                /* First read all points then add them using drawingorder */
                QList<PointInfos> points;
                while (pointsQuery.next())
                {
                    PointInfos ptInfos;
                    ptInfos.id = pointsQuery.value(eRelationPoints::ID).toInt();
                    qreal x = pointsQuery.value(eRelationPoints::X).toFloat(); // QPointF olmasina ragmen event->scenePos() int degerler dönüyor
                    qreal y = pointsQuery.value(eRelationPoints::Y).toFloat(); // TODO cok etki eder mi?
                    ptInfos.point = QPointF(x, y);
                    ptInfos.drawingOrder = pointsQuery.value(eRelationPoints::DRAWING_ORDER).toInt();
                    points.append(ptInfos);
                }
                
                /* Now add the points */
                QVector<PointInfos> pointsTmp;
                pointsTmp.resize(points.count()); //because at first QVector has one item, then adds ct-items into it
                foreach (PointInfos pt, points)
                    pointsTmp[pt.drawingOrder] = pt;
                
                foreach (PointInfos pt, pointsTmp)
                {
                    assoItem->addPoint(pt.point);
                    assoItem->addPointID(pt.id);
                }
            }
            items.append(assoItem);
        }
        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while loading the project", 0);
    }
    
    return true;
}

/*!
 * \brief This function returns the class items. When the user loads a project, this function will be called. */
bool SqlDatabase::getClassItems(QList<QGraphicsObject *> &classes)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");

        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while loading a project :  : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());
            return false;
        }

        QSqlQuery itemQuery(QSqlDatabase::database("DBConnectionName"));
        itemQuery.setForwardOnly(true);
        itemQuery.exec("SELECT * FROM ClassItem");

        /* For each class item */
        while (itemQuery.next())
        {
            GeneralProperties *genProp = new GeneralProperties;
            GUIProperties *guiProp = new GUIProperties;
            
            /* Store the class id and name */
            genProp->id = itemQuery.value(eClassItem::ID).toInt();
            genProp->name = itemQuery.value(eClassItem::CLASSNAME).toString();
                        
            /* Store gui properties */
            QFont font;
            guiProp->positionInScene = QPoint(itemQuery.value(eClassItem::XSCENE).toInt(), itemQuery.value(eClassItem::YSCENE).toInt());
            guiProp->boundaryRect = QRect(itemQuery.value(eClassItem::X).toInt(), itemQuery.value(eClassItem::Y).toInt(), 
                                         itemQuery.value(eClassItem::WIDTH).toInt(), itemQuery.value(eClassItem::HEIGHT).toInt());
            guiProp->headerHeight = itemQuery.value(eClassItem::HEADERH).toInt();
            guiProp->membersHeight = itemQuery.value(eClassItem::MEMBERSH).toInt();
            guiProp->functionsHeight = itemQuery.value(eClassItem::FUNCTIONSH).toInt();
            font.fromString(itemQuery.value(eClassItem::FONT).toString());
            guiProp->font = font;

            /* Query TABLE : GeneralProperties */
            QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
            query.setForwardOnly(true);
            query.prepare("SELECT * FROM GeneralProperties WHERE cID=:cID");
            query.bindValue(":cID", genProp->id);
            if (!query.exec())
            {
                /* If can not be executed, then there is a problem here, close and end */
                sqlDatabase.close();
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("DEBUG : Can not read table GeneralProperties %0", 1, 
                                                      query.lastError().text().toLocal8Bit().data());
                return false;
            }
            
            /* Store general properties */
            while (query.next())
            {
                genProp->stereotype  = query.value(2).toString(); // 3. col
                genProp->nameSpace = query.value(3).toString(); //4. col
                genProp->isAbstract  = query.value(4).toBool(); //5. col
                genProp->visibility = (Visibility)query.value(eGeneral::VISIBILITY).toInt();
            }
            
            /* Query TABLE : MemberProperties */
            QList<MemberProperties *> memProp;
            query.prepare("SELECT * FROM MemberProperties WHERE cID=:cID");
            query.bindValue(":cID", genProp->id);
            query.exec();
            while (query.next())
            {
                MemberProperties *member = new MemberProperties;
                member->id = query.value(eMember::ID).toInt();
                member->type = query.value(eMember::TYPE).toString();
                member->name = query.value(eMember::NAME).toString();
                member->stereotype = query.value(eMember::STEREOTYPE).toString();
                member->initValue = query.value(eMember::INITIAL_VALUE).toString();
                member->isStatic = query.value(eMember::IS_STATIC).toBool();
                member->visibility = (Visibility)query.value(eMember::VISIBILITY).toInt();                
                member->description = query.value(eMember::DESCRIPTION).toString();
                memProp.append(member);
            }
            
            /* Query TABLE : FunctionProperties */
            QList<FunctionProperties *> funcProp;            
            query.prepare("SELECT * FROM FunctionProperties WHERE cID=:cID");
            query.bindValue(":cID", genProp->id);
            query.exec();
            while (query.next())
            {
                FunctionProperties *function = new FunctionProperties;
                function->id = query.value(eFunction::ID).toInt();
                function->returnType = query.value(eFunction::RETURN_TYPE).toString();
                function->name = query.value(eFunction::NAME).toString();
                function->stereotype = query.value(eFunction::STEREOTYPE).toString();
                function->isStatic = query.value(eFunction::IS_STATIC).toBool();
                function->isVirtual = query.value(eFunction::IS_VIRTUAL).toBool();                
                function->visibility = (Visibility)query.value(eFunction::VISIBILITY).toInt();                
                function->description = query.value(eFunction::DESCRIPTION).toString();
                function->code= query.value(eFunction::CODE).toString();
                
                /* Query TABLE : ParameterProperties */
                QList<ParameterProperties *> parameters;
                /* New query is neccessary */
                QSqlQuery paramQuery(QSqlDatabase::database("DBConnectionName"));
                paramQuery.setForwardOnly(true);
                paramQuery.prepare("SELECT * FROM ParameterProperties WHERE fID=:fID");
                paramQuery.bindValue(":fID", function->id);
                paramQuery.exec();
                while(paramQuery.next())
                {
                    ParameterProperties *parameter = new ParameterProperties;
                    parameter->id = paramQuery.value(eParameter::ID).toInt();
                    parameter->name = paramQuery.value(eParameter::NAME).toString();
                    parameter->type = paramQuery.value(eParameter::TYPE).toString();
                    parameter->stereotype = paramQuery.value(eParameter::STEREOTYPE).toString();
                    parameter->initValue = paramQuery.value(eParameter::INITIAL_VALUE).toString();    
                    parameter->direction = (Direction)paramQuery.value(eParameter::DIRECTION).toInt();
                    parameters.append(parameter);
                }
                /* Last function info */
                function->parameters = parameters;
                funcProp.append(function);
            }
            classes.append(new ClassItem(genProp, guiProp, memProp, funcProp));
        }

        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while loading the project", 0);
    }
    
    return true;
}

bool SqlDatabase::updateAssociationItem(AssociationItem *item)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while adding a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());  
            return false;
        }
        
        /* Update the table classitem and generalproperties in the db */
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        
        /* Set up the query */
        /* RelationType, cIDBegin and cIDEnd can not be updated */
        query.prepare("UPDATE RelationItem SET "
                      "MultiplicityBegin=:MultiplicityBegin, MultiplicityEnd=:MultiplicityEnd, RoleBegin=:RoleBegin, "
                      "RoleEnd=:RoleEnd, Description=:Description WHERE ID=:ID");
        query.bindValue(":ID", item->id());
        query.bindValue(":MultiplicityBegin", item->getMultiplicityBeginInfo());
        query.bindValue(":MultiplicityEnd", item->getMultiplicityEndInfo());
        query.bindValue(":RoleBegin", item->getRoleBeginInfo());
        query.bindValue(":RoleEnd", item->getRoleEndInfo());
        query.bindValue(":Description", item->getDescriptionInfo());
        
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the new class item info %0", 
                                                  1, query.lastError().text().toLocal8Bit().data());
            return false;
        }
       
        /*  Update in two way : info updates or point(s) deleted
            Get the number of points stored in db */
        query.prepare("SELECT Count(*) FROM RelationPoints WHERE rID = :rID");
        query.bindValue(":rID", item->id());
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the association item in DB! (1) : %0", 1,
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* If the counts are not equal, some points must be removed from the database */
        int numPoints = 0; 
        if (query.next())
        {
            numPoints = query.value(0).toInt();
        }
        else
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the association item in DB! (1) : %0", 1,
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        if (numPoints != item->points().count())
        {
            QList<int> dbIDs, removedIDs;
            
            /* See which points are deletes */
            query.prepare("SELECT * FROM RelationPoints WHERE rID = :rID");
            query.bindValue(":rID", item->id());
            if (!query.exec())
            {
                sqlDatabase.close();
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("ERROR : Can not update the association item in DB! (1) : %0", 1,
                                                      query.lastError().text().toLocal8Bit().data());
                return false;
            }
            
            /* Save the id's in a list */
            while (query.next())
                dbIDs.append(query.value(eRelationPoints::ID).toInt());
            
            /* Find the deleted points */
            for (int j = 0; j < dbIDs.count(); ++j)
            {
                bool found = false;
                for (int i = 0; i < item->points().count(); ++i)
                    if (item->getPointID(i) == dbIDs[j])
                        found = true;
                
                if (!found)
                    removedIDs.append(dbIDs[j]);
            }
            
            /* Now delete these id's from database too */
            for (int i = 0; i < removedIDs.count(); ++i)
            {
                query.prepare("DELETE FROM RelationPoints WHERE ID=:ID AND rID=:rID");
                query.bindValue(":ID", removedIDs[i]);
                query.bindValue(":rID", item->id());
                
                if (!query.exec())
                {
                    sqlDatabase.close();
                    /* Send the info to monitor - convert QString to char * */
                    MonitorManager::getInstance()->logMsg("ERROR : Can not update the association item in DB! (1) : %0", 1,
                                                          query.lastError().text().toLocal8Bit().data());
                    return false;
                }
            }
        }
        
        /* Now update Points */
        for (int i = 0; i < item->points().count(); ++i)
        {
            /* -1 when the user first saves the project, then adds new breakpoints */
            if (item->getPointID(i) == -1)
            {
                /* Add the new breakpoints into db */
                query.prepare("INSERT OR IGNORE INTO RelationPoints(rID, x, y, DrawingOrder)"
                              "VALUES (:rID, :x, :y, :DrawingOrder)");
                query.bindValue(":rID", item->id());
                query.bindValue(":x", item->points()[i].x());
                query.bindValue(":y", item->points()[i].y());
                query.bindValue(":DrawingOrder", i);
                if (!query.exec())
                {
                    sqlDatabase.close();
                    /* Send the info to monitor - convert QString to char * */
                    MonitorManager::getInstance()->logMsg("ERROR : Can not add the new association item into DB! (1) : %0", 1,
                                                          query.lastError().text().toLocal8Bit().data());
                    return false;
                }
                
                /* Store the id for update */
                int ptID = query.lastInsertId().toInt();
                item->updatePointID(i, ptID);
            }
            else
            {
                query.prepare("UPDATE RelationPoints SET rID=:rID, x=:x, y=:y, DrawingOrder=:DrawingOrder "
                              "WHERE ID=:ptID AND rID=:rID");
                query.bindValue(":ptID", item->getPointID(i));
                query.bindValue(":rID", item->id());
                query.bindValue(":x", item->points()[i].x());
                query.bindValue(":y", item->points()[i].y());
                query.bindValue(":DrawingOrder", i);
                
                if (!query.exec())
                {
                    sqlDatabase.close();
                    /* Send the info to monitor - convert QString to char * */
                    MonitorManager::getInstance()->logMsg("ERROR : Can not update the association item in DB! (1) : %0", 1,
                                                          query.lastError().text().toLocal8Bit().data());
                    return false;
                }
            }
        }
        
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("INFO : Class item info updated successfully", 0);
        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while updating a new item", 0);
        return false;
    }
    
    return true;
}

/*!
 * \brief This function updates the item information in database, if the item is in database added.*/
bool SqlDatabase::updateClassItem(ClassItem *pClassItem, bool bUndo)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while adding a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());  
            return false;
        }
        
        /* Update the table classitem and generalproperties in the db */
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        
        /* Set up the query */
        query.prepare("UPDATE ClassItem SET Name=:Name, xScene=:xScene, yScene=:yScene, x=:x, y=:y, Width=:Width, Height=:Height, "
                      "HeaderHeight=:HeaderHeight, MembersHeight=:MembersHeight, FunctionsHeight=:FunctionsHeight, Font=:Font WHERE ID = :id");
        query.bindValue(":id", pClassItem->id());
        query.bindValue(":Name", pClassItem->getGeneralProperties()->name);
        query.bindValue(":xScene", pClassItem->getGUIProperties()->positionInScene.x());
        query.bindValue(":yScene", pClassItem->getGUIProperties()->positionInScene.y());
        query.bindValue(":x", pClassItem->getGUIProperties()->boundaryRect.x());
        query.bindValue(":y", pClassItem->getGUIProperties()->boundaryRect.y());
        query.bindValue(":Width", pClassItem->getGUIProperties()->boundaryRect.width());
        query.bindValue(":Height", pClassItem->getGUIProperties()->boundaryRect.height());
        query.bindValue(":HeaderHeight", pClassItem->getGUIProperties()->headerHeight);
        query.bindValue(":MembersHeight", pClassItem->getGUIProperties()->membersHeight);
        query.bindValue(":FunctionsHeight", pClassItem->getGUIProperties()->functionsHeight);
        query.bindValue(":Font", pClassItem->getGUIProperties()->font.toString());
        
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the new class item info %0", 
                                                  1, query.lastError().text().toLocal8Bit().data());
            return false;
        }

        /* Update the general properties */
        QSqlQuery queryGen(QSqlDatabase::database("DBConnectionName"));
        queryGen.setForwardOnly(true);
        /* cID may not be updated, it will be set at beginning once */
        queryGen.prepare("UPDATE GeneralProperties SET Stereotype=:stereotype, Package=:Package, isAbstract=:isAbstract,"
                         "Visibility=:visibility WHERE ID=:gid");
        queryGen.bindValue(":stereotype", pClassItem->getGeneralProperties()->stereotype);
        queryGen.bindValue(":Package", pClassItem->getGeneralProperties()->nameSpace);
        queryGen.bindValue(":isAbstract", pClassItem->getGeneralProperties()->isAbstract);
        queryGen.bindValue(":visibility", (int)(pClassItem->getGeneralProperties()->visibility));
        queryGen.bindValue(":gid", pClassItem->getGeneralProperties()->id);
        
        if (!queryGen.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the new class item general info %0", 
                                                  1, queryGen.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* If there are members to delete : in case -> wenn user removes a member from the gui and saves the project */
        if (!pClassItem->getListMembersToDelete().isEmpty())
        {
            foreach (MemberProperties *member, pClassItem->getListMembersToDelete())
            {
                removeMemberFromDB(pClassItem, member);
                /* Remove the member from the memory */
                delete member;
                member = 0;
            }
            /* Clear the list */
            pClassItem->getListMembersToDelete().clear();
        }
        
        /* If there are members to insert or update them */
        foreach(MemberProperties *member, pClassItem->getListMemberProperties())
        {
            if (!insertUpdateMember(pClassItem, member, pClassItem->id(), bUndo))
            {
                sqlDatabase.close();
                return false;
            }
        }
        
        /* If there are functions to delete : in case -> wenn user removes a member from the gui and saves the project */
        if (!pClassItem->getListFunctionsToDelete().isEmpty())
        {
            foreach (FunctionProperties *function, pClassItem->getListFunctionsToDelete())
            {
                removeFunctionFromDB(pClassItem, function);
                /* Remove the member from the memory */
                delete function;
                function = 0;
            }
            /* Clear the list */
            pClassItem->getListFunctionsToDelete().clear();
        }
        
        /* If there are parameters to delete : in case -> wenn user removes a member from the gui and saves the project */
        if (!pClassItem->getListParametersToDelete().isEmpty())
        {
            foreach (ParameterProperties *parameter, pClassItem->getListParametersToDelete())
            {
                removeParameterFromDB(parameter);
                /* Remove the member from the memory */
                delete parameter;
                parameter = 0;
            }
            /* Clear the list */
            pClassItem->getListMembersToDelete().clear();
        }
        
        /* If there are functions, insert or update them now */
        foreach(FunctionProperties *function, pClassItem->getListFunctionProperties())
        {
            if (!insertUpdateFunction(pClassItem, function, pClassItem->id(), bUndo))
            {
                sqlDatabase.close();
                return false;
            }
        }
        
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("INFO : Class item info updated successfully", 0);
        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while updating a new item", 0);
        return false;
    }
    
    return true;
}

/*!
 * \brief This function inserts a member item into db or updates its status. */
bool SqlDatabase::insertUpdateMember(ClassItem *item, MemberProperties *member, int cID, bool bUndo)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);

    /* -1 means, this member has not been stored into db yet, insert it.
     * bUndo -> true means, user has removed the item from scene, then saved it.
     * Then undo the class item and tries to save it again. So its id is not -1 and is not in database : must be added again. */
    if (member->id == -1 || bUndo)
    {
        query.prepare("INSERT OR IGNORE INTO MemberProperties(cID, Type, Name, InitialValue, Stereotype, "
                      "isStatic, Visibility, Description)"
                      "VALUES (:cID, :Type, :Name, :InitialValue, :Stereotype, :isStatic, :Visibility, :Description)");
        query.bindValue(":cID", cID);
        query.bindValue(":Type", member->type);
        query.bindValue(":Name", member->name);
        query.bindValue(":InitialValue", member->initValue);
        query.bindValue(":Stereotype", member->stereotype);
        query.bindValue(":isStatic", member->isStatic);
        query.bindValue(":Visibility", (int)(member->visibility));
        query.bindValue(":Description", member->description);
        
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not add the member info %0", 1, 
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* Update the id */
        member->id = query.lastInsertId().toInt();

        /* Class Id must be saved into the item too, in order to decide if the item will be inserted or updated */
        ProjectManager::getInstance()->getActiveProject()->setMemberIdAsDbId(item->getGeneralProperties()->name, member->name, member->id);
    }
    else 
    {
        /* This item is in the db, just update it. */
        /* No cID update possible, it shall be inserted once */
        query.prepare("UPDATE MemberProperties SET Type=:Type, Name=:Name, InitialValue=:InitialValue, "
                      "Stereotype=:Stereotype, isStatic=:isStatic, Visibility=:Visibility, Description=:Description WHERE ID=:mid");
        query.bindValue(":mid", member->id);
        query.bindValue(":Type", member->type);
        query.bindValue(":Name", member->name);
        query.bindValue(":InitialValue", member->initValue);
        query.bindValue(":Stereotype", member->stereotype);
        query.bindValue(":isStatic", member->isStatic);
        query.bindValue(":Visibility", (int)(member->visibility));
        query.bindValue(":Description", member->description);
        
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the member info %0", 1, 
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
    }
    
    return true;
}

/*!
 * \brief This function removes a member item from the db. */
bool SqlDatabase::removeMemberFromDB(ClassItem *item, MemberProperties *member)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);

    query.prepare("DELETE FROM MemberProperties WHERE ID=:mID AND cID=:cID");
    query.bindValue(":mID", member->id);
    query.bindValue(":cID", item->id());
            
    if (!query.exec())
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Can not remove the member info %0 from db - %1", 2,
                                              member->name.toLocal8Bit().data(),
                                              query.lastError().text().toLocal8Bit().data());
        return false;
    }
    
    return true;
}

/*!
 * \brief This function inserts a function item into db or updates its status. */
bool SqlDatabase::insertUpdateFunction(ClassItem *item, FunctionProperties *function, int cID, bool bUndo)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);

    /* This means, this function has not been stored into db yet, insert it
     * bUndo -> true means, user has removed the item from scene, then saved it.
     * Then undo the class item and tries to save it again. So its id is not -1 and is not in database : must be added again. */
    if (function->id == -1 || bUndo)
    {
        query.prepare("INSERT OR IGNORE INTO FunctionProperties(cID, ReturnType, Name, Stereotype, "
                      "isStatic, isVirtual, Visibility, Description, Code)"
                      "VALUES (:cID, :ReturnType, :Name, :Stereotype, :isStatic, :isVirtual, :Visibility, :Description, :Code)");
        query.bindValue(":cID", cID);
        query.bindValue(":ReturnType", function->returnType);
        query.bindValue(":Name", function->name);
        query.bindValue(":Stereotype", function->stereotype);
        query.bindValue(":isStatic", function->isStatic);
        query.bindValue(":isVirtual", function->isVirtual);
        query.bindValue(":Visibility", (int)(function->visibility));
        query.bindValue(":Description", function->description);
        query.bindValue(":Code", function->code);
        
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not add the function info %0 - %1", 2, 
                                                  function->name.toLocal8Bit().data(),
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }

        /* Update the id */
        function->id = query.lastInsertId().toInt();
        // sindi sirada fonk id var, sonra coklu eleman ekleme ve update testleri lazum
        
        /* Insert the parameters */
        foreach (ParameterProperties *parameter, function->parameters)
            if (!insertParameterIntoDB(item, parameter, function->id))
                return false;

        /* Class Id must be saved into the item too, in order to decide if the item will be inserted or updated */
        ProjectManager::getInstance()->getActiveProject()->setFunctionIdAsDbId(item->getGeneralProperties()->name, function->name, 
                                                                               function->returnType, function->parameters, function->id);
    }
    else /* This item is in the db, update it */
    { 
        /* No cID update possible, it shall be inserted once */
        query.prepare("UPDATE FunctionProperties SET ReturnType=:ReturnType, Name=:Name, Stereotype=:Stereotype, "
                      "isStatic=:isStatic, isVirtual=:isVirtual, Visibility=:Visibility, Description=:Description, "
                      "Code=:Code WHERE ID=:fid");
        query.bindValue(":fid", function->id);
        query.bindValue(":ReturnType", function->returnType);
        query.bindValue(":Name", function->name);
        query.bindValue(":Stereotype", function->stereotype);
        query.bindValue(":isStatic", function->isStatic);
        query.bindValue(":isVirtual", function->isVirtual);
        query.bindValue(":Visibility", (int)(function->visibility));
        query.bindValue(":Description", function->description);
        query.bindValue(":Code", function->code);
        
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the function info %0", 1, 
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* Update the parameters */
        foreach (ParameterProperties *parameter, function->parameters)
            if (!insertParameterIntoDB(item, parameter, function->id))
                return false;
    }

    return true;
}

/*!
 * \brief This function checks if the item is in database. */
bool SqlDatabase::isAssoItemInDB(AssociationItem *item)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while adding a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());  
            return false;
        }
        
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        query.prepare("SELECT Count(*) FROM RelationItem WHERE ID=:ID");
        query.bindValue(":ID", item->id());
                
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update association item db - %0", 1,
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        int retVal = 0;
        if (query.next()) 
        {
            retVal = query.value(0).toInt();
        }
        else
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update association item db - %0", 1,
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* No item found */
        if (retVal == 0) 
            return false;
    
        /* There is one item found */
        return true;
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while updating an association item", 0);
        return false;
    }
}


/*!
 * \brief This function checks if the item is in database. */
bool SqlDatabase::isClassItemInDB(ClassItem *item)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while adding a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());  
            return false;
        }
        
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        query.prepare("SELECT Count(*) FROM ClassItem WHERE ID=:ID");
        query.bindValue(":ID", item->id());
                
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update class item info %0 in db - %1", 2,
                                                  item->className().toLocal8Bit().data(),
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        int retVal = 0;
        if (query.next()) 
        {
            retVal = query.value(0).toInt();
            qDebug() << "ret val " << retVal;
        }
        else
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update class item info %0 in db - %1", 2,
                                                  item->className().toLocal8Bit().data(),
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* No item found */
        if (retVal == 0) 
            return false;
    
        /* There is one item found */
        return true;
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while updating a class item", 0);
        return false;
    }
}

/*!
 * \brief This function removes a function item from the db. */
bool SqlDatabase::removeFunctionFromDB(ClassItem *item, FunctionProperties *function)
{
    /* Delete first all parameters. */
    QSqlQuery queryParam(QSqlDatabase::database("DBConnectionName"));
    queryParam.setForwardOnly(true);
    
    foreach (ParameterProperties *parameter, function->parameters)
    {
        queryParam.prepare("DELETE FROM ParameterProperties WHERE ID=:pID AND fID=:fID");
        queryParam.bindValue(":pID", parameter->id);
        queryParam.bindValue(":fID", function->id);
        
        if (!queryParam.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not remove the parameter %0 of the function %1 from db - %2", 3,
                                                  parameter->name.toLocal8Bit().data(),
                                                  function->name.toLocal8Bit().data(),
                                                  queryParam.lastError().text().toLocal8Bit().data());
            return false;
        }
    }

    /* Now delete the function. */
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);
    
    query.prepare("DELETE FROM FunctionProperties WHERE ID=:fID AND cID=:cID");
    query.bindValue(":fID", function->id);
    query.bindValue(":cID", item->id());
            
    if (!query.exec())
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Can not remove the function info %0 from db - %1", 2,
                                              function->name.toLocal8Bit().data(),
                                              query.lastError().text().toLocal8Bit().data());
        return false;
    }
    
    return true;
}

/*!
 * \brief This function removes a parameter item from the db. */
bool SqlDatabase::removeParameterFromDB(ParameterProperties *parameter)
{
    QSqlQuery queryParam(QSqlDatabase::database("DBConnectionName"));
    queryParam.setForwardOnly(true);
    
    queryParam.prepare("DELETE FROM ParameterProperties WHERE ID=:pID");
    queryParam.bindValue(":pID", parameter->id);
        
    if (!queryParam.exec())
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Can not remove the parameter %0 from db - %1", 2,
                                              parameter->name.toLocal8Bit().data(),
                                              queryParam.lastError().text().toLocal8Bit().data());
        return false;
    }

    
    return true;
}

/*!
 * \brief This function inserts the parameters of a function into DB. */
bool SqlDatabase::insertParameterIntoDB(ClassItem *item, ParameterProperties *parameter, int fID)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);
    
    /* Insert the new parameter */
    if (parameter->id == -1)
    {
        query.prepare("INSERT OR IGNORE INTO ParameterProperties(fID, Type, Name, InitialValue, Stereotype, Direction)"
                      "VALUES (:fID, :Type, :Name, :InitialValue, :Stereotype, :Direction)");
        query.bindValue(":fID", fID);
        query.bindValue(":Type", parameter->type);
        query.bindValue(":Name", parameter->name);
        query.bindValue(":InitialValue", parameter->initValue);
        query.bindValue(":Stereotype", parameter->stereotype);
        query.bindValue(":Direction", parameter->direction);
        
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not add the parameter %0 - %1", 2,
                                                  parameter->name.toLocal8Bit().data(),
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* Update the id */
        parameter->id = query.lastInsertId().toInt();

        /* Class Id must be saved into the item too, in order to decide if the item will be inserted or updated */
        ProjectManager::getInstance()->getActiveProject()->setParameterIdAsDbId(item->getGeneralProperties()->name, parameter->name, fID, parameter->id);
    }
    /* Or update the info */
    else
    {
        /* fID may not be updated */
        query.prepare("UPDATE ParameterProperties SET Type=:Type, Name=:Name, InitialValue=:InitialValue," 
                      "Stereotype=:Stereotype, Direction=:Direction WHERE fID=:fID AND ID=:pID");
        
        query.bindValue(":fID", fID);
        query.bindValue(":pID", parameter->id);
        query.bindValue(":Type", parameter->type);
        query.bindValue(":Name", parameter->name);
        query.bindValue(":InitialValue", parameter->initValue);
        query.bindValue(":Stereotype", parameter->stereotype);
        query.bindValue(":Direction", parameter->direction);
        
        if (!query.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not update the parameter %0 - %1", 2,
                                                  parameter->name.toLocal8Bit().data(),
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
    }
    
    /* Success */
    return true;
}

/*!
 * \brief This function finds the association items, which are no more in scene but in database and removes them all.
 * \param List of class items, which are still in the scene and stored in database too. New class items are not in the list. */
bool SqlDatabase::removeDeletedClassItems(const QList<ClassItem*> &classItems)
{
    /* Get all the class items from the database */
    QList<int> dbIDs;
    
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while removing a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());             
            return false;
        }
        
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        
        query.prepare("SELECT * FROM ClassItem");
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not remove the class items from DB! (1) : %0", 1,
                                                  query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* Get the ids */
        while (query.next())
            dbIDs.append(query.value(eClassItem::ID).toInt());
        
        /* Find the deleted classes */
        QList<int> removedItemIDs;
        for (int j = 0; j < dbIDs.count(); ++j)
        {
            bool found = false;
    
            for (int i = 0; i < classItems.count(); ++i)
                if (classItems[i]->id() == dbIDs[j])
                    found = true;
            
            if (!found)
                removedItemIDs.append(dbIDs[j]);
        }
        
        /* Now delete these id's from database */
        for (int i = 0; i < removedItemIDs.count(); ++i)
        {
            // project.cpp de zaten assolar bir kere siliniyor, buraya gerek var mi ki ne?
            /* Get the IDs of the association items, which must be removed from the database too, because they are bounded with this class item */
//            QList<int> removedAssoIDs;
//            query.prepare("SELECT * FROM RelationItem WHERE cIDBegin=:cIDBegin OR cIDEnd=:cIDEnd");
//            query.bindValue(":cIDBegin", removedItemIDs[i]);
//            query.bindValue(":cIDEnd", removedItemIDs[i]);
//            if (!query.exec())
//            {
//                sqlDatabase.close();
//                /* Send the info to monitor - convert QString to char * */
//                MonitorManager::getInstance()->logMsg("ERROR : Can not remove the class items from DB! (1) : %0", 1,
//                                                      query.lastError().text().toLocal8Bit().data());
//                return false;
//            }
            
//            /* Get the ids */
//            while (query.next())
//                removedAssoIDs.append(query.value(eRelationItem::ID).toInt());
            
//            /* Remove the association items */
//            foreach (int delID, removedAssoIDs)
//                removeAssociationItem(delID);
                    
            /* Now remove the class item */       
            removeClassItem(removedItemIDs[i]);
        }
        
        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while deleting a class item", 0);
        return false;
    }
   
    return true;
}

/*!
 * \brief This function removes a class item from database.
 * \param ID of the class item
*/
bool SqlDatabase::removeClassItem(int classID)
{
    
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while removing a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());             
            return false;
        }
        
        /* 1-) Remove the members of this class item */
        if (!removeMembersFromDB(classID))
        {
            sqlDatabase.close();
            return false;
        }

        /* 2-) Remove the functions of this item, if exists. */
        if (!removeFunctionsFromDB(classID))
        {
            sqlDatabase.close();
            return false;
        }
        
        /* 3-) Remove the general information of this class item from the db */
        QSqlQuery queryGen(QSqlDatabase::database("DBConnectionName"));
        queryGen.setForwardOnly(true);
        
        queryGen.prepare("DELETE FROM GeneralProperties WHERE cID=:cID");
        queryGen.bindValue(":cID", classID);
        if (!queryGen.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not delete the general properties of the class item : %0", 1, 
                                                  queryGen.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* 4-) Remove the class item from the db */
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        
        query.prepare("DELETE FROM ClassItem WHERE ID=:cID");
        query.bindValue(":cID", classID);
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not delete the class item : %0", 1, query.lastError().text().toLocal8Bit().data());
            return false;
        }

        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("INFO : Class item deleted", 0);

        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while deleting a class item", 0);
        return false;
    }
    
    return true;
}

/*! 
 * \brief This functions removes the members from DB.
 * \param Class ID */
bool SqlDatabase::removeMembersFromDB(int classID)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);

    query.prepare("DELETE FROM MemberProperties WHERE cID=:cID");
    query.bindValue(":cID", classID);
            
    if (!query.exec())
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Can not remove the member info %0", 1,
                                              query.lastError().text().toLocal8Bit().data());
        return false;
    }
    
    return true;
}

/*! 
 * \brief This functions removes the members from DB.
 * \param Class ID */
bool SqlDatabase::removeFunctionsFromDB(int classID)
{
    QSqlQuery queryFunc(QSqlDatabase::database("DBConnectionName"));
    QSqlQuery queryParam(QSqlDatabase::database("DBConnectionName"));
    queryFunc.setForwardOnly(true);
    queryParam.setForwardOnly(true);
    
    /* Get all the function ids from db, which will be removed with this class item */
    queryFunc.prepare("SELECT * FROM FunctionProperties WHERE cID=:cID");
    queryFunc.bindValue(":cID", classID);
    if (!queryFunc.exec())
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Can not remove parameter from DB! (1) : %0", 1,
                                              queryFunc.lastError().text().toLocal8Bit().data());
        return false;
    }
    
    /* Delete the parameters which are bonded with these functions */
    while (queryFunc.next())
    {
        int fuID = queryFunc.value(eFunction::ID).toInt();
        
        queryParam.prepare("DELETE FROM ParameterProperties WHERE fID=:fID");
        queryParam.bindValue(":fID", fuID);
    
        if (!queryParam.exec())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not remove the parameter %0", 1,
                                                  queryParam.lastError().text().toLocal8Bit().data());
            return false;
        }
    }
    
    /* Now delete the functions */
    queryFunc.prepare("DELETE FROM FunctionProperties WHERE cID=:cID");
    queryFunc.bindValue(":cID", classID);
            
    if (!queryFunc.exec())
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Can not remove the function info %0", 1,
                                              queryFunc.lastError().text().toLocal8Bit().data());
        return false;
    }
    
    return true;
}


/*!
 * \brief This function removes a class item from database. */
bool SqlDatabase::removeClassItem(ClassItem *item)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while removing a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());             
            return false;
        }
        
        /* 1-) Remove the members of this item, if exists. */
        foreach(MemberProperties *member, item->getListMemberProperties())
        {
            if (!removeMemberFromDB(item, member))
            {
                sqlDatabase.close();
                return false;
            }
        }

        /* 2-) Remove the functions of this item, if exists. */
        foreach(FunctionProperties *function, item->getListFunctionProperties())
        {
            if (!removeFunctionFromDB(item, function))
            {
                sqlDatabase.close();
                return false;
            }
        }
        
        /* 3-) Remove the general information of this class item from the db */
        QSqlQuery queryGen(QSqlDatabase::database("DBConnectionName"));
        queryGen.setForwardOnly(true);
        
        queryGen.prepare("DELETE FROM GeneralProperties WHERE cID=:cID");
        queryGen.bindValue(":cID", item->id());
        if (!queryGen.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not delete the general properties of the class item : %0", 1, 
                                                  queryGen.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        /* 4-) Remove the class item from the db */
        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        
        query.prepare("DELETE FROM ClassItem WHERE ID=:cID");
        query.bindValue(":cID", item->id());
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not delete the class item : %0", 1, query.lastError().text().toLocal8Bit().data());
            return false;
        }

        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("INFO : Class item %0 deleted", 1, item->className().toLocal8Bit().data());

        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while deleting a class item", 0);
        return false;
    }
    
    return true;
}

/*!
 * \brief This function finds the association items, which are no more in scene but in database and removes them all. */
bool SqlDatabase::removeAssociationItems(const QList<int> &associationItemsInScene)
{
    QList<int> associationItemsInDB;
    QList<int> associationItemsToBeRemoved;
    
    /* Get all association items from the db */
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");

        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while loading a project :  : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());
            return false;
        }

        QSqlQuery itemQuery(QSqlDatabase::database("DBConnectionName"));
        itemQuery.setForwardOnly(true);
        itemQuery.exec("SELECT * FROM RelationItem");
        
        /* For each asso item */
        while (itemQuery.next())
            associationItemsInDB.append(itemQuery.value(eRelationItem::ID).toInt());
    }
    else
    {
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while loading the project", 0);
    }
    
    /* Find the items which must be removed */
    foreach (int idTmp, associationItemsInDB)
    {
        bool delItem = true;
        foreach (int idTmp2, associationItemsInScene)
            if (idTmp == idTmp2)
                delItem = false;

        if (delItem)
            associationItemsToBeRemoved.append(idTmp);
    }
    
    /* Remove all the items from the db */
    foreach (int delID, associationItemsToBeRemoved)
        removeAssociationItem(delID);
    
    
    return true;
}

/*!
 * \brief This function removes an association item from database.
 * There are two possibilities to remove an association item from the database : 
 *  1- user removes the association item
 *  2- user removes a class item, so all the association items must be removed too. */ 
bool SqlDatabase::removeAssociationItem(int id)
{
    if (QSqlDatabase::contains("DBConnectionName"))
    {
        QSqlDatabase sqlDatabase = QSqlDatabase::database("DBConnectionName");
        /* This line creates a new db file or open the existing one */
        //QString dbFile(m_strDBFullPath + ".db");
        QString dbFile(m_strDBFullPath + ".sqlite");
        sqlDatabase.setDatabaseName(dbFile);
        if (!sqlDatabase.open())
        {
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : DB-Connection to %0 failed while removing a new item : %1", 2, 
                                                  sqlDatabase.databaseName().toLocal8Bit().data(),
                                                  sqlDatabase.lastError().text().toLocal8Bit().data());             
            return false;
        }

        QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
        query.setForwardOnly(true);
        
        /* Remove points */
        query.prepare("DELETE FROM RelationPoints WHERE rID=:rID");
        query.bindValue(":rID", id);
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not delete the association item : %0", 1, query.lastError().text().toLocal8Bit().data());
            return false;
        }
        
        
        /* Remove item */
        query.prepare("DELETE FROM RelationItem WHERE ID=:ID");
        query.bindValue(":ID", id);
        if (!query.exec())
        {
            sqlDatabase.close();
            /* Send the info to monitor - convert QString to char * */
            MonitorManager::getInstance()->logMsg("ERROR : Can not delete the association item : %0", 1, query.lastError().text().toLocal8Bit().data());
            return false;
        }

        sqlDatabase.close();
    }
    else
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Problem with DB connection while deleting an association item", 0);
        return false;
    }
    
    return true;
}


/* ----------------------------------------------- */
/* All the functions below are setters and getters */
/* ----------------------------------------------- */
QString SqlDatabase::databaseName() const
{
    return m_strDBName;
}

void SqlDatabase::setDatabaseName(const QString &strDatabaseName)
{
    m_strDBName = strDatabaseName;
}

/* ----------------------------------------------- */

/*
After the exec() call, we can navigate through the query’s result set:
while (query.next()) {
    QString title = query.value(0).toString();
    int year = query.value(1).toInt();
    cerr << qPrintable(title) << ": " << year << endl;
}
*/
/*
 *    if (query.exec()) {
        int count = 0;
        while(query.next()) {
            QSqlRecord record = query.record();
            qDebug() << record;
            count++;
        }
    }
*/
