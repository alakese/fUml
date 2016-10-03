/***
 * 24 November 2014
 * 
 * INFO : To test database : source after second iteration copied and modified. Monitor and ClassItem bonds are removed.
 * 
 * 27 November 2014
 * 
 * To add more items with more properties the information will be generated automated.
 * See : test_database_addRandomItem_returnsTrue_data()
 * 
 * Test results : unit test runs successfully. Creates random number of class items with random numbers of members and functions.
 * Adding them and updating a random element is successfully. Removing an item is successfully.
 * */
#include <QtTest>
#include <QString>
#include <QRect>
#include <QFont>
#include <QFile>
#include "db/sqldatabase.h"
#include "globals/properties.h"
#include "graphics/items/classitem.h"
#include <QFontDatabase>

#include <QDebug>


#define MAX_NUM_LOOP            10
#define UPPERCASE_EDGE          25 // 'Z' - 'A'
#define MAX_NUM_CHAR            62 // See valid char table
#define MAX_NUM_CHAR_NO_SPACE   61 // See valid char table
#define STRING_LENGTH           10
#define MAX_ITEM_COUNT          10


/*!
 * \brief The TestDatabaseTest class
 */
class TestDatabaseTest : public QObject
{
    Q_OBJECT
    
    QList<char> charTable;
    
public:
    TestDatabaseTest();
    
private Q_SLOTS:
    /* Tests : add one item */
    void test_database_addOneItem_returnsTrue_data();
    void test_database_addOneItem_returnsTrue();
 
    /* Tests : remove one item */
    void test_database_removeOneItem_returnsTrue();
    
    /* Tests : add more random item */
    void test_database_addRandomItem_returnsTrue_data();
    void test_database_addRandomItem_returnsTrue();
    
    /* Tests : add more random item then update one of them */
    void test_database_addRandomItemThenUpdateOneItem_returnsTrue_data();
    void test_database_addRandomItemThenUpdateOneItem_returnsTrue();

private:
    void initiateCharTable();
    QString getRandomString(int max);
    int getRandomInt(int max);
    bool getRandomBool();
    void printClassItem(ClassItem *item);
    
private:
    QList<ClassItem*> items;
    QString strProjectPath;
    QString strProjectName1;
    QString strProjectName2;
    int ctLoop;
};

TestDatabaseTest::TestDatabaseTest()
{   
    qsrand(QDateTime::currentDateTime().toTime_t());
    
    initiateCharTable();
    
    /* Variable values */
    strProjectPath = "D:\\Temp\\test";
    strProjectName1 = "testDB1"; // for one item
    strProjectName2 = "testDB2"; // for more (random) item
    
    /* Set driver */
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "DBConnectionName"); 
    if (!db.isValid())
        qDebug() << "DB Connection not successfull";
}

/*
 * Add one class item to database and give it as expected to test_data. */
void TestDatabaseTest::test_database_addOneItem_returnsTrue_data()
{
    // - Since each item will be saved once, the db-id of the properties must be 1
    int id = 1;
    
    /* Remove the .db File, if exists. Otherwise the test will fail. */
    QString fileName = QString("%1\\%2.sqlite").arg(strProjectPath).arg(strProjectName1);
    if (QFile::exists(fileName))
        QVERIFY(QFile::remove(fileName));
    
    SqlDatabase *SQLDatabase = new SqlDatabase(strProjectPath, strProjectName1);
    SQLDatabase->createTables();
    
    // - General propeties-info
    QString classname = "TestClassItem";
    QString classStereotype = "TestStereotype";
    QString classNamespace = "Test.Name.Space";
    bool isClassAbs = false;
    Visibility classVisi = PUBLIC;
    GeneralProperties *genProp = new GeneralProperties(-1, classname, classStereotype, classNamespace, isClassAbs, classVisi);
    
    // - GUI properties
    int minHeight = 28;
    QPoint posInfo(100, 100); 
    QRect rectInfo(0, 0, 100, 3*minHeight); // For each part min height is 28
    QFont fontInfo("Helvetica", 12);
    GUIProperties *guiProp = new GUIProperties(posInfo, rectInfo, minHeight, minHeight, minHeight, fontInfo);

    // - Member properties
    QString memberType = "unsigned short int";
    QString memberName = "testMember";
    QString memberInitVal = "null";
    QString memberStereotype = "test mem stereotype";
    QString memberDesc = "This is a unit test member";
    bool isMemStatic = true;
    Visibility memberVisibility = PROTECTED;
    QList<MemberProperties *> memProp;
    MemberProperties *member = new MemberProperties(-1, memberType, memberName, memberInitVal, memberStereotype, isMemStatic, 
                                                    memberVisibility, memberDesc);
    memProp.append(member);
    
    // - Function properties
    QList<FunctionProperties *> funcProp;
    QList<ParameterProperties *> paramProp;
    QString paramName = "testParameter";
    QString paramType = "int";
    QString paramInitVal = "-1";
    QString paramStereo = "paramStereo";
    Direction paramDir = INOUT;
    
    ParameterProperties *param = new ParameterProperties(-1, paramType, paramName, paramInitVal, paramStereo, paramDir);
    paramProp.append(param);
    
    QString functionName = "testFunction";
    QString functionType = "bool";
    QString functionStereotype = "functionStereotype";
    Visibility funcVisibility = PUBLIC;
    QString functionDesc = "this is a unit test function";
    QString functionCode = "int a = 100; int b = 200; return a + b;";
    FunctionProperties *func = new FunctionProperties(-1, functionType, functionName, functionStereotype, false, false, funcVisibility, 
                                                      paramProp, functionDesc, functionCode);
    funcProp.append(func);
    
    // - Create the class item
    ClassItem *item = new ClassItem(genProp, guiProp, memProp, funcProp);
    SQLDatabase->addNewClassItem(item);

    // - Define unittest rows for GeneralProperties
    QTest::addColumn<int>("classID"); 
    QTest::addColumn<QString>("className"); 
    QTest::addColumn<QString>("classStereotype");
    QTest::addColumn<QString>("classNamespace"); 
    QTest::addColumn<bool>("isAbstract"); 
    QTest::addColumn<int>("visibility");
    
    // - Define unittest rows for GuiProperties INFO : no need to test heights : the columns will be removed in future
    QTest::addColumn<QPoint>("posInfo"); 
    QTest::addColumn<QRect>("rectInfo"); 
    QTest::addColumn<QFont>("fontInfo");
    
    // - Member
    QTest::addColumn<QString>("memberType"); 
    QTest::addColumn<QString>("memberName");
    QTest::addColumn<QString>("memberInitVal");
    QTest::addColumn<QString>("memberStereotype"); 
    QTest::addColumn<bool>("isMemStatic"); 
    QTest::addColumn<int>("memberVisibility");
    QTest::addColumn<QString>("memberDesc"); 
    
    // - Function
    QTest::addColumn<QString>("functionType");
    QTest::addColumn<QString>("functionName");
    QTest::addColumn<QString>("functionStereotype");
    QTest::addColumn<bool>("isFuncStatic");
    QTest::addColumn<bool>("isFuncVirtual");
    QTest::addColumn<int>("funcVisibility");
    QTest::addColumn<QString>("functionDesc");
    QTest::addColumn<QString>("functionCode");
    
    // - Parameter
    QTest::addColumn<QString>("paramType");
    QTest::addColumn<QString>("paramName");
    QTest::addColumn<QString>("paramInitVal");
    QTest::addColumn<QString>("paramStereo");
    QTest::addColumn<int>("paramDirection");
        
    // - Create the test row
    QTest::newRow("ADD ITEM") << id << classname << classStereotype << classNamespace << isClassAbs << (int)classVisi
                              << posInfo << rectInfo << fontInfo
                              << memberType << memberName << memberInitVal << memberStereotype << isMemStatic << (int)memberVisibility << memberDesc
                              << functionType << functionName << functionStereotype << false << false << (int)funcVisibility << functionDesc << functionCode
                              << paramType << paramName << paramInitVal << paramStereo << (int)paramDir;

}

/*
 * Connect to the same database as in data and compare with the expected info. */
void TestDatabaseTest::test_database_addOneItem_returnsTrue()
{
    QString fileName = QString("%1\\%2.sqlite").arg(strProjectPath).arg(strProjectName1);
    QVERIFY(QFile::exists(fileName));
            
    /* Open the test database */
    SqlDatabase *SQLDatabase = new SqlDatabase(strProjectPath, strProjectName1);
    
    // - Fetch class
    QFETCH(int, classID);
    QFETCH(QString, className);
    QFETCH(QString, classStereotype);
    QFETCH(QString, classNamespace);
    QFETCH(bool, isAbstract);
    QFETCH(int, visibility);
    // - GUI
    QFETCH(QPoint, posInfo);
    QFETCH(QRect, rectInfo);
    QFETCH(QFont, fontInfo);
    // - Member
    QFETCH(QString, memberType);
    QFETCH(QString, memberName);
    QFETCH(QString, memberInitVal);
    QFETCH(QString, memberStereotype);
    QFETCH(bool, isMemStatic);
    QFETCH(int, memberVisibility);
    QFETCH(QString, memberDesc);
    // - Function
    QFETCH(QString, functionType);
    QFETCH(QString, functionName);
    QFETCH(QString, functionStereotype);
    QFETCH(bool, isFuncStatic);
    QFETCH(bool, isFuncVirtual);
    QFETCH(int, funcVisibility);
    QFETCH(QString, functionDesc);
    QFETCH(QString, functionCode);     
    // - Parameter
    QFETCH(QString, paramType);
    QFETCH(QString, paramName);
    QFETCH(QString, paramInitVal);
    QFETCH(QString, paramStereo);
    QFETCH(int, paramDirection);
    
    /* Get class item info from the db */
    QList<ClassItem *> items;
    SQLDatabase->getClassItems(items);

    QVERIFY(items.count() == 1);
    
    // We sure know there is only one item at this point in database
    ClassItem *itemFromDB = items[0];
    GeneralProperties *genProp = itemFromDB->getGeneralProperties();
    GUIProperties *guiProp = itemFromDB->getGUIProperties();
    QList<MemberProperties *> memProp = itemFromDB->getListMemberProperties();
    QList<FunctionProperties *> funcProp = itemFromDB->getListFunctionProperties();
    
    /* Compare db info with fetch info */
    QCOMPARE(classID, classID); //since there is only one item, it must be 1
    QCOMPARE(className, genProp->name);
    QCOMPARE(classStereotype, genProp->stereotype);
    QCOMPARE(classNamespace, genProp->nameSpace);
    QCOMPARE(isAbstract, genProp->isAbstract);
    QCOMPARE(visibility, (int)genProp->visibility);
    QCOMPARE(posInfo, guiProp->positionInScene);
    QCOMPARE(rectInfo, guiProp->boundaryRect);
    QCOMPARE(fontInfo, guiProp->font);
    QCOMPARE(memberType, memProp[0]->type);
    QCOMPARE(memberName, memProp[0]->name);
    QCOMPARE(memberInitVal, memProp[0]->initValue);
    QCOMPARE(memberStereotype, memProp[0]->stereotype);
    QCOMPARE(isMemStatic, memProp[0]->isStatic);
    QCOMPARE(memberVisibility, (int)memProp[0]->visibility);
    QCOMPARE(memberDesc, memProp[0]->description);
    QCOMPARE(functionType, funcProp[0]->returnType);
    QCOMPARE(functionName, funcProp[0]->name);
    QCOMPARE(functionStereotype, funcProp[0]->stereotype);
    QCOMPARE(isFuncStatic, funcProp[0]->isStatic);
    QCOMPARE(isFuncVirtual, funcProp[0]->isVirtual);
    QCOMPARE(funcVisibility, (int)funcProp[0]->visibility);
    QCOMPARE(functionDesc, funcProp[0]->description);
    QCOMPARE(functionCode, funcProp[0]->code);
    QCOMPARE(paramType, funcProp[0]->parameters[0]->type);
    QCOMPARE(paramName, funcProp[0]->parameters[0]->name);
    QCOMPARE(paramInitVal, funcProp[0]->parameters[0]->initValue);
    QCOMPARE(paramStereo, funcProp[0]->parameters[0]->stereotype);
    QCOMPARE(paramDirection, (int)funcProp[0]->parameters[0]->direction);
}

/*
 * Removes one item */
void TestDatabaseTest::test_database_removeOneItem_returnsTrue()
{
    SqlDatabase *SQLDatabase = new SqlDatabase(strProjectPath, strProjectName1);

    // We sure know there is only one item at this point in database    
    QList<ClassItem *> items;
    SQLDatabase->getClassItems(items);
    ClassItem *itemFromDB = items[0];
    SQLDatabase->removeItem(itemFromDB);
    
    QVERIFY(SQLDatabase->isDBEmpty());
}

/* Tests : add more random item */
void TestDatabaseTest::test_database_addRandomItem_returnsTrue_data()
{
    /* Remove the .db File, if exists. Otherwise the test will fail. */
    QString fileName = QString("%1\\%2.sqlite").arg(strProjectPath).arg(strProjectName2);
    if (QFile::exists(fileName))
      QVERIFY(QFile::remove(fileName));

    SqlDatabase *SQLDatabase = new SqlDatabase(strProjectPath, strProjectName2);
    SQLDatabase->createTables();
    
    /* Data for test */
    QStringList fonts = QStringList() << "Helvetica" << "Arial" << "Times new roman" << "ComicSans";
    int fontSizes[4] = {10, 12, 14, 16};
    QStringList types = QStringList() << "char" << "char16_t" << "char32_t" << "wchar_t" << "signed short int"  << "signed int"  <<
                          "signed long int" << "signed long long int" << "unsigned char" << "unsigned short int" <<
                          "unsigned int" << "unsigned long int" << "unsigned long long int" << "float" << "double" <<
                          "long double" << "bool";
    
    /* Random number of elements to test */
    ctLoop = getRandomInt(MAX_NUM_LOOP) + 1;
    
    // - Define unittest rows for GeneralProperties
    QTest::addColumn<int>("classID"); 
    QTest::addColumn<QString>("className"); 
    QTest::addColumn<QString>("classStereotype");
    QTest::addColumn<QString>("classNamespace"); 
    QTest::addColumn<bool>("isAbstract"); 
    QTest::addColumn<int>("visibility");
    
    // - Define unittest rows for GuiProperties INFO : no need to test heights : the columns will be removed in future
    QTest::addColumn<QPoint>("posInfo"); 
    QTest::addColumn<QRect>("rectInfo"); 
    QTest::addColumn<QFont>("fontInfo");
    
    GeneralProperties *genProp;
    GUIProperties *guiProp;
    QList<MemberProperties *> memProp;
    QList<FunctionProperties *> funcProp;
    for (int i = 0; i < ctLoop; ++i)
    {
        memProp.clear();
        funcProp.clear();
        
        // - General propeties-info
        QString className = getRandomString(MAX_NUM_CHAR_NO_SPACE);
        QString classStereotype = getRandomString(MAX_NUM_CHAR);
        QString classNamespace = getRandomString(MAX_NUM_CHAR);
        bool isClassAbstract = getRandomBool();
        Visibility classVisibility = (Visibility)getRandomInt(3);
        GeneralProperties *genPropTmp = new GeneralProperties(-1, className, classStereotype, classNamespace, isClassAbstract, classVisibility);
        genProp = genPropTmp;
        
        // - GUI properties
        int minHeight = 28;
        QPoint posInfo(getRandomInt(500), getRandomInt(500)); 
        QRect rectInfo(0, 0, getRandomInt(100), 3*minHeight); // Height will be calculated automatically
        QString font = fonts[getRandomInt(fonts.count())];
        int fontSize = fontSizes[getRandomInt(4)]; // 4 is count of the array
        QFont fontInfo(font, fontSize);
        GUIProperties *guiPropTmp = new GUIProperties(posInfo, rectInfo, minHeight, minHeight, minHeight, fontInfo);
        guiProp = guiPropTmp;
        
        // - Member properties
        int memberCount = getRandomInt(MAX_ITEM_COUNT);
        qDebug() << i+1 << " - Member count random " << memberCount;
        for (int j = 0; j < memberCount; ++j)
        {
            QString memberType = types[getRandomInt(types.count())];
            QString memberName = getRandomString(MAX_NUM_CHAR_NO_SPACE);
            QString memberInitVal = getRandomString(MAX_NUM_CHAR_NO_SPACE);
            QString memberStereotype = getRandomString(MAX_NUM_CHAR);
            QString memberDescription = getRandomString(MAX_NUM_CHAR);
            bool isMemberStatic = getRandomBool();
            Visibility memberVisibility = (Visibility)getRandomInt(3);
            MemberProperties *member = new MemberProperties(-1, memberType, memberName, memberInitVal, memberStereotype, 
                                                            isMemberStatic, memberVisibility, memberDescription);
            memProp.append(member);
        }
        
        foreach (MemberProperties *mem, memProp)
        {
            qDebug() << " " << mem->string();
        }
        
        // - Function properties
        int functionCount = getRandomInt(MAX_ITEM_COUNT);
        for (int j = 0; j < functionCount; ++j)
        {
            QList<ParameterProperties *> paramProp;
            int parameterCount = getRandomInt(MAX_ITEM_COUNT);
            for (int k = 0; k < parameterCount; ++k)
            {
                QString paramName = getRandomString(MAX_NUM_CHAR_NO_SPACE);
                QString paramType = types[getRandomInt(types.count())];
                QString paramInitVal = getRandomString(MAX_NUM_CHAR_NO_SPACE);
                QString paramStereo = getRandomString(MAX_NUM_CHAR);
                Direction paramDir = (Direction)getRandomInt(3);;
                ParameterProperties *param = new ParameterProperties(-1, paramType, paramName, paramInitVal, paramStereo, paramDir);
                paramProp.append(param);
            }
                
            QString functionName = getRandomString(MAX_NUM_CHAR_NO_SPACE);
            QString functionType = types[getRandomInt(types.count())];
            QString functionStereotype = getRandomString(MAX_NUM_CHAR_NO_SPACE);
            Visibility funcVisibility = (Visibility)getRandomInt(3);
            bool isFuncStatic = getRandomBool();
            bool isFuncVirtual = getRandomBool();
            QString functionDesc = getRandomString(MAX_NUM_CHAR);
            QString functionCode = getRandomString(MAX_NUM_CHAR);
            FunctionProperties *func = new FunctionProperties(-1, functionType, functionName, functionStereotype, 
                                                              isFuncStatic, isFuncVirtual, funcVisibility, 
                                                              paramProp, functionDesc, functionCode);
            funcProp.append(func);
        }
        
        // - Create the class item
        ClassItem *item = new ClassItem(genProp, guiProp, memProp, funcProp);
        
        // - Store into DB
        SQLDatabase->addNewClassItem(item);
        
        // - Store local
        items.append(item);
    }
}

void TestDatabaseTest::test_database_addRandomItem_returnsTrue()
{
    QString fileName = QString("%1\\%2.sqlite").arg(strProjectPath).arg(strProjectName2);
    QVERIFY(QFile::exists(fileName));

    // Open the test database
    SqlDatabase *SQLDatabase = new SqlDatabase(strProjectPath, strProjectName2);
                                     
    // - Get class item info from the db
    QList<ClassItem *> itemsFromDB;
    SQLDatabase->getClassItems(itemsFromDB);
   
    QCOMPARE(items.count(), itemsFromDB.count());
    for (int i = 0; i < items.count(); ++i)
    {
        qDebug() << "Testing item " << i+1;
        
        // - Compare General propeties-info
        QCOMPARE(items[i]->getGeneralProperties()->id, itemsFromDB[i]->getGeneralProperties()->id);
        QCOMPARE(items[i]->getGeneralProperties()->name, itemsFromDB[i]->getGeneralProperties()->name);
        QCOMPARE(items[i]->getGeneralProperties()->stereotype, itemsFromDB[i]->getGeneralProperties()->stereotype);
        QCOMPARE(items[i]->getGeneralProperties()->nameSpace, itemsFromDB[i]->getGeneralProperties()->nameSpace);
        QCOMPARE(items[i]->getGeneralProperties()->isAbstract, itemsFromDB[i]->getGeneralProperties()->isAbstract);
        QCOMPARE((int)items[i]->getGeneralProperties()->visibility, (int)itemsFromDB[i]->getGeneralProperties()->visibility);
        // - Compare GUI propeties-info
        QCOMPARE(items[i]->getGUIProperties()->positionInScene, itemsFromDB[i]->getGUIProperties()->positionInScene);
        QCOMPARE(items[i]->getGUIProperties()->boundaryRect, itemsFromDB[i]->getGUIProperties()->boundaryRect);
        QCOMPARE(items[i]->getGUIProperties()->font, itemsFromDB[i]->getGUIProperties()->font);
        // - Compare members
        QCOMPARE(items[i]->getListMemberProperties().count(), itemsFromDB[i]->getListMemberProperties().count());
        qDebug() << "  Member counts " << items[i]->getListMemberProperties().count(); 
        for (int j = 0; j < items[i]->getListMemberProperties().count(); ++j)
        {
            QCOMPARE(items[i]->getListMemberProperties()[j]->id, itemsFromDB[i]->getListMemberProperties()[j]->id);
            QCOMPARE(items[i]->getListMemberProperties()[j]->type, itemsFromDB[i]->getListMemberProperties()[j]->type);
            QCOMPARE(items[i]->getListMemberProperties()[j]->name, itemsFromDB[i]->getListMemberProperties()[j]->name);
            QCOMPARE(items[i]->getListMemberProperties()[j]->initValue, itemsFromDB[i]->getListMemberProperties()[j]->initValue);
            QCOMPARE(items[i]->getListMemberProperties()[j]->stereotype, itemsFromDB[i]->getListMemberProperties()[j]->stereotype);
            QCOMPARE(items[i]->getListMemberProperties()[j]->isStatic, itemsFromDB[i]->getListMemberProperties()[j]->isStatic);
            QCOMPARE((int)items[i]->getListMemberProperties()[j]->visibility, (int)itemsFromDB[i]->getListMemberProperties()[j]->visibility);
            QCOMPARE(items[i]->getListMemberProperties()[j]->description, itemsFromDB[i]->getListMemberProperties()[j]->description);
        }
        // - Compare functions
        QCOMPARE(items[i]->getListFunctionProperties().count(), itemsFromDB[i]->getListFunctionProperties().count());
        qDebug() << "  Function counts " << items[i]->getListFunctionProperties().count();
        for (int j = 0; j < items[i]->getListFunctionProperties().count(); ++j)
        {
            QCOMPARE(items[i]->getListFunctionProperties()[j]->id, itemsFromDB[i]->getListFunctionProperties()[j]->id);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->returnType, itemsFromDB[i]->getListFunctionProperties()[j]->returnType);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->name, itemsFromDB[i]->getListFunctionProperties()[j]->name);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->stereotype, itemsFromDB[i]->getListFunctionProperties()[j]->stereotype);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->isStatic, itemsFromDB[i]->getListFunctionProperties()[j]->isStatic);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->isVirtual, itemsFromDB[i]->getListFunctionProperties()[j]->isVirtual);
            QCOMPARE((int)items[i]->getListFunctionProperties()[j]->visibility, (int)itemsFromDB[i]->getListFunctionProperties()[j]->visibility);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->description, itemsFromDB[i]->getListFunctionProperties()[j]->description);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->code, itemsFromDB[i]->getListFunctionProperties()[j]->code);
            
            // - Parameters
            QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters.count(), itemsFromDB[i]->getListFunctionProperties()[j]->parameters.count());
            qDebug() << "    Parameter counts " << items[i]->getListFunctionProperties()[j]->parameters.count();
            for (int k = 0; k < items[i]->getListFunctionProperties()[j]->parameters.count(); ++k)
            {
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->id, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->id);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->name, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->name);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->type, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->type);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->initValue, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->initValue);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->stereotype, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->stereotype);
                QCOMPARE((int)items[i]->getListFunctionProperties()[j]->parameters[k]->direction, 
                         (int)itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->direction);
            }
        }
    }
}

void TestDatabaseTest::initiateCharTable()
{
    int i;
    for (i = 'A'; i <= 'Z'; ++ i) // Upper case
        charTable.append(i);
    
    for (i = 'a'; i <= 'z'; ++ i) // Lower case 
        charTable.append(i);

    for (i = '0'; i <= '9'; ++ i) // Digits
        charTable.append(i);
    
    charTable.append('_');
    charTable.append(32); // Space
}

QString TestDatabaseTest::getRandomString(int max)
{
    /* First letter between A - Z */
    char firstLetter = charTable[getRandomInt(UPPERCASE_EDGE)];
    QString string(firstLetter);
    
    /* Rest is alpha-numeric */
    int stringLength = getRandomInt(STRING_LENGTH);
    for (int i = 0; i < stringLength; ++i)
        string.append(charTable[getRandomInt(max)]);
    
    return string;
}

int TestDatabaseTest::getRandomInt(int max)
{
    return qrand() % max;
}

bool TestDatabaseTest::getRandomBool()
{
    return qrand() % 2 == 0;
}

/* Tests : add more random item and then update one item */
void TestDatabaseTest::test_database_addRandomItemThenUpdateOneItem_returnsTrue_data()
{
    QString fileName = QString("%1\\%2.sqlite").arg(strProjectPath).arg(strProjectName2);
    QVERIFY(QFile::exists(fileName));

    SqlDatabase *SQLDatabase = new SqlDatabase(strProjectPath, strProjectName2);
    
    // - Item to be updated - select random one
    int index = getRandomInt(items.count());
    qDebug() << "Item to update with index " << index;
    //printClassItem(items[index]);
    
    /* Data for test */
    QStringList fonts = QStringList() << "Helvetica" << "Arial" << "Times new roman" << "ComicSans";
    int fontSizes[4] = {10, 12, 14, 16};
    QStringList types = QStringList() << "char" << "char16_t" << "char32_t" << "wchar_t" << "signed short int"  << "signed int"  <<
                          "signed long int" << "signed long long int" << "unsigned char" << "unsigned short int" <<
                          "unsigned int" << "unsigned long int" << "unsigned long long int" << "float" << "double" <<
                          "long double" << "bool";
    
    GeneralProperties *genProp;
    GUIProperties *guiProp;
    QList<MemberProperties *> memProp;
    QList<FunctionProperties *> funcProp;
        
    // - General propeties-info
    int gID = items[index]->getGeneralProperties()->id;
    QString className = getRandomString(MAX_NUM_CHAR_NO_SPACE);
    QString classStereotype = getRandomString(MAX_NUM_CHAR);
    QString classNamespace = getRandomString(MAX_NUM_CHAR);
    bool isClassAbstract = getRandomBool();
    Visibility classVisibility = (Visibility)getRandomInt(3);
    GeneralProperties *genPropTmp = new GeneralProperties(gID, className, classStereotype, classNamespace, isClassAbstract, classVisibility);
    genProp = genPropTmp;
        
    // - GUI properties
    int minHeight = 28;
    QPoint posInfo(getRandomInt(500), getRandomInt(500)); 
    QRect rectInfo(0, 0, getRandomInt(100), 3*minHeight); // Height will be calculated automatically
    QString font = fonts[getRandomInt(fonts.count())];
    int fontSize = fontSizes[getRandomInt(4)]; // 4 is count of the array
    QFont fontInfo(font, fontSize);
    GUIProperties *guiPropTmp = new GUIProperties(posInfo, rectInfo, minHeight, minHeight, minHeight, fontInfo);
    guiProp = guiPropTmp;
        
    // - Member properties
    int memberCount = items[index]->getListMemberProperties().count();

    for (int j = 0; j < memberCount; ++j)
    {
        int mID = items[index]->getListMemberProperties()[j]->id;
        QString memberType = types[getRandomInt(types.count())];
        QString memberName = getRandomString(MAX_NUM_CHAR_NO_SPACE);
        QString memberInitVal = getRandomString(MAX_NUM_CHAR_NO_SPACE);
        QString memberStereotype = getRandomString(MAX_NUM_CHAR);
        QString memberDescription = getRandomString(MAX_NUM_CHAR);
        bool isMemberStatic = getRandomBool();
        Visibility memberVisibility = (Visibility)getRandomInt(3);
        MemberProperties *member = new MemberProperties(mID, memberType, memberName, memberInitVal, memberStereotype, 
                                                        isMemberStatic, memberVisibility, memberDescription);
        memProp.append(member);
    }
        
    // - Function properties
    int functionCount = items[index]->getListFunctionProperties().count();
    for (int j = 0; j < functionCount; ++j)
    {
        QList<ParameterProperties *> paramProp;
        int parameterCount = items[index]->getListFunctionProperties()[j]->parameters.count();
        for (int k = 0; k < parameterCount; ++k)
        {
            int pID = items[index]->getListFunctionProperties()[j]->parameters[k]->id;
            QString paramName = getRandomString(MAX_NUM_CHAR_NO_SPACE);
            QString paramType = types[getRandomInt(types.count())];
            QString paramInitVal = getRandomString(MAX_NUM_CHAR_NO_SPACE);
            QString paramStereo = getRandomString(MAX_NUM_CHAR);
            Direction paramDir = (Direction)getRandomInt(3);;
            ParameterProperties *param = new ParameterProperties(pID, paramType, paramName, paramInitVal, paramStereo, paramDir);
            paramProp.append(param);
        }

        int fID = items[index]->getListFunctionProperties()[j]->id;
        QString functionName = getRandomString(MAX_NUM_CHAR_NO_SPACE);
        QString functionType = types[getRandomInt(types.count())];
        QString functionStereotype = getRandomString(MAX_NUM_CHAR_NO_SPACE);
        Visibility funcVisibility = (Visibility)getRandomInt(3);
        bool isFuncStatic = getRandomBool();
        bool isFuncVirtual = getRandomBool();
        QString functionDesc = getRandomString(MAX_NUM_CHAR);
        QString functionCode = getRandomString(MAX_NUM_CHAR);
        FunctionProperties *func = new FunctionProperties(fID, functionType, functionName, functionStereotype, 
                                                          isFuncStatic, isFuncVirtual, funcVisibility, 
                                                          paramProp, functionDesc, functionCode);
        funcProp.append(func);
    }
        
    // - Create the class item
    ClassItem *item = new ClassItem(genProp, guiProp, memProp, funcProp);
    qDebug() << "Updated item";
    //printClassItem(items[index]);
    
    // Copy the old file for comparision
    QString oldFileName = QString("%1_old").arg(fileName);
    QFile::copy(fileName, oldFileName);
    // - Update in DB
    SQLDatabase->updateClassItem(item);
    
    // - Update local
    ClassItem *toRemove = items[index];
    items[index] = item;
    
    delete toRemove;
    toRemove = NULL;
}

void TestDatabaseTest::test_database_addRandomItemThenUpdateOneItem_returnsTrue()
{
    QString fileName = QString("%1\\%2.sqlite").arg(strProjectPath).arg(strProjectName2);
    QVERIFY(QFile::exists(fileName));

    // Open the test database
    SqlDatabase *SQLDatabase = new SqlDatabase(strProjectPath, strProjectName2);
                                     
    // - Get class item info from the db
    QList<ClassItem *> itemsFromDB;
    SQLDatabase->getClassItems(itemsFromDB);
   
    QCOMPARE(items.count(), itemsFromDB.count());
    for (int i = 0; i < items.count(); ++i)
    {
        qDebug() << "Testing item " << i+1;
        // - Compare General propeties-info
        QCOMPARE(items[i]->getGeneralProperties()->id, itemsFromDB[i]->getGeneralProperties()->id);
        QCOMPARE(items[i]->getGeneralProperties()->name, itemsFromDB[i]->getGeneralProperties()->name);
        QCOMPARE(items[i]->getGeneralProperties()->stereotype, itemsFromDB[i]->getGeneralProperties()->stereotype);
        QCOMPARE(items[i]->getGeneralProperties()->nameSpace, itemsFromDB[i]->getGeneralProperties()->nameSpace);
        QCOMPARE(items[i]->getGeneralProperties()->isAbstract, itemsFromDB[i]->getGeneralProperties()->isAbstract);
        QCOMPARE((int)items[i]->getGeneralProperties()->visibility, (int)itemsFromDB[i]->getGeneralProperties()->visibility);
        // - Compare GUI propeties-info
        QCOMPARE(items[i]->getGUIProperties()->positionInScene, itemsFromDB[i]->getGUIProperties()->positionInScene);
        QCOMPARE(items[i]->getGUIProperties()->boundaryRect, itemsFromDB[i]->getGUIProperties()->boundaryRect);
        QCOMPARE(items[i]->getGUIProperties()->font, itemsFromDB[i]->getGUIProperties()->font);
        // - Compare members
        QCOMPARE(items[i]->getListMemberProperties().count(), itemsFromDB[i]->getListMemberProperties().count());
        qDebug() << "  Member counts " << items[i]->getListMemberProperties().count(); 
        for (int j = 0; j < items[i]->getListMemberProperties().count(); ++j)
        {
            QCOMPARE(items[i]->getListMemberProperties()[j]->id, itemsFromDB[i]->getListMemberProperties()[j]->id);
            QCOMPARE(items[i]->getListMemberProperties()[j]->type, itemsFromDB[i]->getListMemberProperties()[j]->type);
            QCOMPARE(items[i]->getListMemberProperties()[j]->name, itemsFromDB[i]->getListMemberProperties()[j]->name);
            QCOMPARE(items[i]->getListMemberProperties()[j]->initValue, itemsFromDB[i]->getListMemberProperties()[j]->initValue);
            QCOMPARE(items[i]->getListMemberProperties()[j]->stereotype, itemsFromDB[i]->getListMemberProperties()[j]->stereotype);
            QCOMPARE(items[i]->getListMemberProperties()[j]->isStatic, itemsFromDB[i]->getListMemberProperties()[j]->isStatic);
            QCOMPARE((int)items[i]->getListMemberProperties()[j]->visibility, (int)itemsFromDB[i]->getListMemberProperties()[j]->visibility);
            QCOMPARE(items[i]->getListMemberProperties()[j]->description, itemsFromDB[i]->getListMemberProperties()[j]->description);
        }
        // - Compare functions
        QCOMPARE(items[i]->getListFunctionProperties().count(), itemsFromDB[i]->getListFunctionProperties().count());
        qDebug() << "  Function counts " << items[i]->getListFunctionProperties().count();
        for (int j = 0; j < items[i]->getListFunctionProperties().count(); ++j)
        {
            QCOMPARE(items[i]->getListFunctionProperties()[j]->id, itemsFromDB[i]->getListFunctionProperties()[j]->id);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->returnType, itemsFromDB[i]->getListFunctionProperties()[j]->returnType);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->name, itemsFromDB[i]->getListFunctionProperties()[j]->name);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->stereotype, itemsFromDB[i]->getListFunctionProperties()[j]->stereotype);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->isStatic, itemsFromDB[i]->getListFunctionProperties()[j]->isStatic);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->isVirtual, itemsFromDB[i]->getListFunctionProperties()[j]->isVirtual);
            QCOMPARE((int)items[i]->getListFunctionProperties()[j]->visibility, (int)itemsFromDB[i]->getListFunctionProperties()[j]->visibility);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->description, itemsFromDB[i]->getListFunctionProperties()[j]->description);
            QCOMPARE(items[i]->getListFunctionProperties()[j]->code, itemsFromDB[i]->getListFunctionProperties()[j]->code);
            
            // - Parameters
            QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters.count(), itemsFromDB[i]->getListFunctionProperties()[j]->parameters.count());
            qDebug() << "    Parameter counts " << items[i]->getListFunctionProperties()[j]->parameters.count();
            for (int k = 0; k < items[i]->getListFunctionProperties()[j]->parameters.count(); ++k)
            {
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->id, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->id);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->name, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->name);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->type, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->type);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->initValue, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->initValue);
                QCOMPARE(items[i]->getListFunctionProperties()[j]->parameters[k]->stereotype, itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->stereotype);
                QCOMPARE((int)items[i]->getListFunctionProperties()[j]->parameters[k]->direction, 
                         (int)itemsFromDB[i]->getListFunctionProperties()[j]->parameters[k]->direction);
            }
        }
    }
}

void TestDatabaseTest::printClassItem(ClassItem *item)
{
    // - General propeties-info
    qDebug() << "--- item ---";
    qDebug() << item->getGeneralProperties()->id << item->getGeneralProperties()->name << item->getGeneralProperties()->stereotype << 
                item->getGeneralProperties()->nameSpace << item->getGeneralProperties()->isAbstract << (int)item->getGeneralProperties()->visibility;
    // - GUI propeties-info
    qDebug() << item->getGUIProperties()->positionInScene << item->getGUIProperties()->boundaryRect << item->getGUIProperties()->font;
    // - members
    for (int j = 0; j < item->getListMemberProperties().count(); ++j)
    {
        qDebug() << item->getListMemberProperties()[j]->id << item->getListMemberProperties()[j]->type << item->getListMemberProperties()[j]->name <<
                    item->getListMemberProperties()[j]->initValue << item->getListMemberProperties()[j]->stereotype << item->getListMemberProperties()[j]->isStatic <<
                    (int)item->getListMemberProperties()[j]->visibility << item->getListMemberProperties()[j]->description;
    }
    // - Compare functions
    for (int j = 0; j < item->getListFunctionProperties().count(); ++j)
    {
        qDebug() << item->getListFunctionProperties()[j]->id << item->getListFunctionProperties()[j]->returnType << item->getListFunctionProperties()[j]->name <<
                    item->getListFunctionProperties()[j]->stereotype << item->getListFunctionProperties()[j]->isStatic << item->getListFunctionProperties()[j]->isVirtual <<
                    (int)item->getListFunctionProperties()[j]->visibility << item->getListFunctionProperties()[j]->description << item->getListFunctionProperties()[j]->code;
        
        // - Parameters
        for (int k = 0; k < item->getListFunctionProperties()[j]->parameters.count(); ++k)
        {
            qDebug() << item->getListFunctionProperties()[j]->parameters[k]->id << item->getListFunctionProperties()[j]->parameters[k]->name << 
                        item->getListFunctionProperties()[j]->parameters[k]->type << item->getListFunctionProperties()[j]->parameters[k]->initValue << 
                        item->getListFunctionProperties()[j]->parameters[k]->stereotype << (int)item->getListFunctionProperties()[j]->parameters[k]->direction;
        }
    }
    qDebug() << "-----------";
}

QTEST_APPLESS_MAIN(TestDatabaseTest)

#include "tst_testdatabasetest.moc"
