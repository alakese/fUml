#ifndef SQLDB_H
#define SQLDB_H

#include <QSqlDatabase>
#include <QtSql/QSqlQueryModel>

class ClassItem;
class MemberProperties;
class FunctionProperties;
class ParameterProperties;


/*!
 * \class SqlDatabase 
 * \brief This class is an interface between the application classes and local database.
 * The Database is a sqlite database, which does not need any server. Therefore it is possible 
 * to store items in this databank and load them.
 * 
 * In this class, each function must create the database connection and do their stuff. A class member
 * is not a good idea, because the addDatabase() can be called once and each Project-Object creates its
 * own connection. Using a member element (like m_sqlDatabase), closes the other connections and therefore
 * makes it unpossible fpr each project to communicate with database. Easy way : create driver and tables once
 * and if you call a function to do some stuff with database, open the connection with a given name, do 
 * you stuff and at the and the object will be destroyed, because it will be defined local in a function. */
class SqlDatabase
{
    /* xScene and yScene are the position of the item in the scene */
    /* x and y are for item's local coordinates */
    /* Table ClassItem */
    struct eClassItem { enum { ID = 0, CLASSNAME, XSCENE, YSCENE, X, Y, WIDTH, HEIGHT, HEADERH, MEMBERSH, FUNCTIONSH, FONT }; };
    /* Table GeneralProperties */
    struct eGeneral { enum { ID = 0, CLASSID, STEREOTYPE, NAMESPACE, IS_ABSTRACT, VISIBILITY}; };
    /* Table MemberProperties */
    struct eMember { enum { ID = 0, CLASSID, TYPE, NAME, INITIAL_VALUE, STEREOTYPE, IS_STATIC, VISIBILITY, DESCRIPTION}; };
    /* Table FunctionProperties */
    struct eFunction { enum { ID = 0, CLASSID, RETURN_TYPE, NAME, STEREOTYPE, IS_STATIC, IS_VIRTUAL, VISIBILITY, DESCRIPTION, CODE}; };
    /* Table ParameterProperties */
    struct eParameter { enum { ID = 0, FUNCTIONID, TYPE, NAME, INITIAL_VALUE, STEREOTYPE, DIRECTION}; };
    
    QString m_strDBName;
    QString m_strDBPath;
    QString m_strDBFullPath;
    
public:
    explicit SqlDatabase(const QString &, const QString &);
    void createTables();
    bool addNewClassItem(ClassItem *);
    QString databaseName() const;
    void setDatabaseName(const QString &);
    bool getClassItems(QList<ClassItem *> &);
    bool updateClassItem(ClassItem *);
    bool removeItem(ClassItem *);
    
    bool isDBEmpty(); // Only for unittest
    
private:    
    bool executeQuery(const QString &, const QString &);
    bool insertUpdateMember(ClassItem *, MemberProperties *, int);
    bool insertUpdateFunction(ClassItem *, FunctionProperties *, int);
    bool insertParameterIntoDB(ClassItem *, ParameterProperties *, int);
    bool removeMemberFromDB(ClassItem *, MemberProperties *);
    bool removeFunctionFromDB(ClassItem *, FunctionProperties *);
    bool removeParameterFromDB(ParameterProperties *);
};

#endif // SQLDB_H
