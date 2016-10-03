#include "sqldatabase.h"
#include "graphics/items/classitem.h"
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>

#include <QDebug>


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
        
        /* Close the connection to db */
        sqlDatabase.close();
        
    }
    else
    {
        /* Send the info to monitor */
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
        return false;
    }
    
    return true;
}

bool SqlDatabase::isDBEmpty()
{
    QSqlQuery itemQuery(QSqlDatabase::database("DBConnectionName"));
    itemQuery.setForwardOnly(true);
    
    itemQuery.exec("SELECT count(*) FROM ClassItem");
    if (itemQuery.next())
        if (itemQuery.value(0).toInt() != 0)
            return false;
    
    itemQuery.exec("SELECT count(*) FROM FunctionProperties");
    if (itemQuery.next())
        if (itemQuery.value(0).toInt() != 0)
            return false;

    itemQuery.exec("SELECT count(*) FROM GeneralProperties");
    if (itemQuery.next())
        if (itemQuery.value(0).toInt() != 0)
            return false;

    itemQuery.exec("SELECT count(*) FROM MemberProperties");
    if (itemQuery.next())
        if (itemQuery.value(0).toInt() != 0)
            return false;

    itemQuery.exec("SELECT count(*) FROM ParameterProperties");
    if (itemQuery.next())
        if (itemQuery.value(0).toInt() != 0)
            return false;
    
    
    return true;
}
#include <QTest>
/*!
 * \brief This functions adds a new class item into the db. */
bool SqlDatabase::addNewClassItem(ClassItem *item)
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
            return false;
        }

        /* Get ClassItem ID from the db */
        int classID = query.lastInsertId().toInt();
        item->setID(classID);
        
        /* Class Id must be saved into the item too, in order to decide if the item will be inserted or updated */
//        ProjectManager::getInstance()->getActiveProject()->setItemIdAsDbId(item->getGeneralProperties()->name, classID);
        
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
            return false;
        }
        
        /* Add members */
        foreach(MemberProperties *member, item->getListMemberProperties())
        {
            if (!insertUpdateMember(item, member, classID))
            {
                sqlDatabase.close();
                return false;
            }
        }

        /* Add functions */
        foreach(FunctionProperties *function, item->getListFunctionProperties())
        {
            if (!insertUpdateFunction(item, function, classID))
            {
                sqlDatabase.close();
                return false;
            }
        }

        sqlDatabase.close();
    }
    else
    {
        return false;
    }
    
    return true;
}

/*!
 * \brief This function returns the complete db information for the scene. When the user loads a project, the db information
 * will be loaded to the scene. */
bool SqlDatabase::getClassItems(QList<ClassItem *> &classes)
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
            qDebug() << "Can not open database";
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
                qDebug() << "Can not execute query from generalproperties";
                sqlDatabase.close();
                return false;
            }
            
            /* Store general properties */
            while (query.next())
            {
                genProp->stereotype  = query.value(2).toString();
                genProp->nameSpace = query.value(3).toString();
                genProp->isAbstract = query.value(4).toBool();
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
        qDebug() << "No Connection";
    }
    
    return true;
}

/*!
 * \brief This function updates the item information in database, if the item is in database added.*/
bool SqlDatabase::updateClassItem(ClassItem *pClassItem)
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
            if (!insertUpdateMember(pClassItem, member, pClassItem->id()))
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
            if (!insertUpdateFunction(pClassItem, function, pClassItem->id()))
            {
                sqlDatabase.close();
                return false;
            }
        }
        
        sqlDatabase.close();
    }
    else
    {
        return false;
    }
    
    return true;
}

/*!
 * \brief This function inserts a member item into db or updates its status. */
bool SqlDatabase::insertUpdateMember(ClassItem *item, MemberProperties *member, int cID)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);
    
    if (member->id == -1) // This means, this member has not been stored into db yet, insert it.
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
            qDebug() << "  insertUpdateMember() : ERROR Adding -> " << member->string();
            return false;
        }
        
        /* Update the id */
        member->id = query.lastInsertId().toInt();
    }
    else // This item is in the db, update it.
    {
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
            qDebug() << "  insertUpdateMember() : ERROR Updating -> " << member->string();
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
        qDebug() << "  removeMemberFromDB() : ERROR Removing -> " << member->string();
        return false;
    }
    
    return true;
}

/*!
 * \brief This function inserts a function item into db or updates its status. */
bool SqlDatabase::insertUpdateFunction(ClassItem *item, FunctionProperties *function, int cID)
{
    QSqlQuery query(QSqlDatabase::database("DBConnectionName"));
    query.setForwardOnly(true);

    if (function->id == -1) /* This means, this function has not been stored into db yet, insert it */
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
//        ProjectManager::getInstance()->getActiveProject()->setFunctionIdAsDbId(item->getGeneralProperties()->name, function->name, 
//                                                                               function->returnType, function->parameters, function->id);
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
            return false;
        }
        
        /* Update the id */
        parameter->id = query.lastInsertId().toInt();

        /* Class Id must be saved into the item too, in order to decide if the item will be inserted or updated */
//        ProjectManager::getInstance()->getActiveProject()->setParameterIdAsDbId(item->getGeneralProperties()->name, parameter->name, fID, parameter->id);
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
            return false;
        }
    }
    
    /* Success */
    return true;
}

/*!
 * \brief This function removes an item from database. */
bool SqlDatabase::removeItem(ClassItem *item)
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
            return false;
        }


        sqlDatabase.close();
    }
    else
    {
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
