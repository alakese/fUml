#ifndef CODEWINDOW_H
#define CODEWINDOW_H

#include <QPlainTextEdit>

QT_BEGIN_NAMESPACE
class QCompleter;
class QStringListModel;
class QStandardItemModel;
QT_END_NAMESPACE

/*!
 * \class CodeWindow
 * \brief Window for command entries. Some actions can be done by using predefined commands.
 * A command has this body: <command> <item> (user argument) <subItem> (user argument)
 * * A command is one of these : create, delete, move, rename, resize
 * * An item is one of these : class
 * * A subItem is one of these : position, to, size
 * These are the keywords. User must give his own values as arguments :
 * A user argument is one of these : a class name, X and Y position of a class, W:width and H:height of a class item
 * 
 * Current commands are:
 * * create class (name) position (X) (Y) [ example : create class Student position 100 200 ]
 * * delete class (name) [ example : delete class Student ]
 * * move   class (name) to (X) (Y) [ example : move class Student to 300 300 ]
 * * rename class (name) to newName [ example : rename class Student to NewStudent ]
 * * resize class (name) size (W) (H) [ example : resize class Student to 200 400 ]
 * 
 * To add/change the properties of a class item, the user can press the .(dot) key and select methods or items (members or functions).
 * This is how it works:
 * 
 * * ClassName.method()
 * * ClassName.member.method()
 * * ClassName.function.method()
 * * ClassName.function.parameter.method()
 * 
 * method() is a set-function for a specific property:
 * * For class : setName(string), setStereotype(string), setNamespace(string), setAbstract(bool), setVisibility(enum), 
 * addMember(type as string, name as string) or addFunction(returnType as string, name as string)
 * * For member : setType(string), setName(string), setInitialValue(string), setStereotype(string), setStatic(bool), setVisibility(enum) or
 * setDescription(string)
 * * For function : setReturnType(string), setName(string), setStereotype(string), setStatic(bool), setVirtual(bool), setVisibility(enum),
 * setDescription(string), setCode(string) or addParameter(type as string, name as string)
 * * For parameter : setType(string), setName(string), setInitialValue(string), setStereotype(string) or setDirection(enum)
 */
class CodeWindow : public QPlainTextEdit
{
    Q_OBJECT

    enum COMMAND_PART { COMMAND = 0, ITEM, ARG1, SUB_COMMAND, ARG2, ARG3 };
    enum LIST_TYPE { CLASSLIST = 0, MEMBERLIST, FUNCTIONLIST, PARAMETERLIST };
    
    int cmdSignIndex;
    QCompleter *m_pCompleter;
    QStringListModel *m_pModel;
    /* Commands : create, move, ...*/
    QStringList m_listCommands;
    /* Items : class, ...*/
    QStringList m_listItems;
    /* Optional items : position, to, ... */
    QStringList m_listSubCommands;
    /* Syntax-info read from a template file */
    QStringList m_listSyntaxInfo;
    /* Methods of a class item */
    QStringList m_listClassMethods;
    QStringList m_listMemberMethods;
    QStringList m_listFunctionMethods;
    QStringList m_listParameterMethods;
    /* Info, which will be shown in tooltip */
    QStringList m_listTooltipDetails;
    bool m_bPressedParenLeft;
    
private:
    bool checkSyntax(const QString &);
    void performCommand(const QString &);
    bool getSyntaxInfoFromTemplate();
    void popupCompleter();
    void setModelStringForProperty(const QStringList &);
    void showTooltip(const QString &, bool);
    void mousePositionCheck();
    bool checkMethod(const QString &, LIST_TYPE);
    
public:
    explicit CodeWindow(QWidget *parent = 0);
    QSize sizeHint() const;
    
protected:
    void keyPressEvent(QKeyEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    
private slots:
    void insertCompletion(const QString &completion);
    void performCompletion();
    void dotPressed(bool);
};

#endif // CODEWINDOW_H
