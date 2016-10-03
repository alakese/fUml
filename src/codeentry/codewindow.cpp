#include "codewindow.h"
#include "wordhighlighter.h"
#include "../gui/mainwindow/mainwindow.h"
#include "../project/project.h"
#include "../project/projectmanagement.h"
#include "../graphics/items/classitem.h"
#include "../gui/mainwindow/treewidget.h"
#include "../includes/syntaxchecker.h"
#include <QCoreApplication>
#include <QCompleter>
#include <QStringListModel>
#include <QShortcut>
#include <QAbstractItemView>
#include <QScrollBar>
#include <QTextBlock>
#include <QPainter>
#include <QToolTip>
#include <QGraphicsScene>

#include <QDebug>


/* Template file infos for a class item */
namespace TEMPLATE_FILE {
    const qint32 MagicNumber = 0x544D50; //for TMP1
    const qreal VersionNumber = 1.0;
}

namespace {
    const QString readySign = "#"; // TODO eskisi gibi > mi kullan (sorun stereotype), yoksa icsel olarak degisecek mi?
}

CodeWindow::CodeWindow(QWidget *parent) :
    QPlainTextEdit(parent)
{
    /* Text is wrapped at word boundaries */
    setWordWrapMode(QTextOption::WordWrap);
    
    // TODO - highlight must do only highlight, no more setPopupType
    // WordHighlighter *wh = new WordHighlighter(document());
    // connect(wh, &WordHighlighter::setPopupType, this, &CodeWindow::setPopupType);
    // 1 - ayrica girilen bir degeri sifirlma nasil olacaka
    /* Initialize lists : sorted order is neccessary because of setModelSorting(case insens) */
    m_listCommands << "create" << "delete" << "move" << "rename" << "resize";
    m_listItems << "class";
    m_listSubCommands << "position" << "size" << "to";

    m_listClassMethods << "setName()" << "setStereotype()" << "setNamespace()" << "setAbstract()" << "setVisibility()" <<
                          "addMember()" << "addFunction()" << "clearStereotype()" << "clearNamespace()";
    m_listMemberMethods << "setType()" << "setName()" << "setInitialValue()" << "setStereotype()" << "setStatic()" << "setVisibility()" << 
                           "setDescription()" << "clearInitialValue()" << "clearStereotype()" << "clearDescription()" << "removeMember()";
    m_listFunctionMethods << "setReturnType()" << "setName()" << "setStereotype()" << "setStatic()" << "setVirtual()" << 
                             "setVisibility()" << "setDescription()" << "setCode()" << "addParameter()" << 
                             "clearStereotype()" << "clearDescription()" << "clearCode()" << "removeFunction()"; 
    m_listParameterMethods << "setType()" << "setName()" << "setInitialValue()" << "setStereotype()" << "setDirection()" << 
                              "clearInitialValue()" << "clearStereotype()" << "removeParameter()";
    
    m_listTooltipDetails << "addFunction(returnType as string, name as string, [optional] parameterType as string, [optional] parameterName as string, ...)" << 
                            "addMember(type as string, name as string)" << 
                            "addParameter(type as string, name as string)" << "setAbstract(value as \"true\" or \"false\")" << 
                            "setCode(code as string)" << "setDescription(description as string)" << "setDirection(as \"in\", \"out\" or \"inout\")" <<
                            "setInitialValue(value as string)" << "setStereotype(stereotype as string)" << "setName(name as string)" << 
                            "setNamespace(namespace as string)" << "setReturnType(returnType as string)" << "setStatic(value \"true\" or \"false\")" << 
                            "setVirtual(value \"true\" or \"false\")" << "setVisibility(value as \"public\", \"private\" or \"protected\")" << 
                            "setType(type as string)";
    /* Model settings */
    m_pModel = new QStringListModel(this);
    
    /* Completer settings */
    m_pCompleter = new QCompleter(this);
    m_pCompleter->setWidget(this);
    m_pCompleter->setCompletionMode(QCompleter::PopupCompletion);
    m_pCompleter->setModel(m_pModel);
    m_pCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    m_pCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    
    /* Connections */
    connect(m_pCompleter, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));
    /* For create, delete, move, resize, rename */
    (void) new QShortcut(QKeySequence(tr("Ctrl+SPACE", "Complete")), this, SLOT(performCompletion()));
    
    /* Show entry */
    insertPlainText(readySign);
    
    /* Get the syntax info from the template file */
    getSyntaxInfoFromTemplate();
}

/*!
 * \brief This function checks the syntax before the command will be performed. */
bool CodeWindow::checkSyntax(const QString &line)
{
    /* Split the command line */
    QStringList commandParts = line.split(QRegularExpression("\\s"));
    /* Get the first word */
    QString command = commandParts.at(COMMAND);
    /* If this is a command */
    if (m_listCommands.contains(command))
    {
        /* Search the command in the list of syntax-info*/
        QString strExp = QString("%1(.*)").arg(command);
        /* Find the index of the command in the list */
        int index = m_listSyntaxInfo.indexOf(QRegularExpression(strExp), 0);
        /* Get the template for the given command */
        QStringList syntaxTemplateParts = m_listSyntaxInfo.at(index).split(QRegularExpression("\\s"));
        /* Did the user give the correct number of words */
        if (commandParts.size() != syntaxTemplateParts.size())
        {
            insertPlainText("\n-Error : wrong number of elements");
            return false;
        }
        
        /* Compare the given command structure with the template one */
        for (int i = 1; i < commandParts.size(); ++i)
        {
            /* Did the user give a valid item-name */
            if (syntaxTemplateParts.at(i) == "item")
            {
                if (!m_listItems.contains(commandParts.at(i)))
                {
                    insertPlainText(QString("\n-Error : %1 is not a valid item. Type \"items\" to see a list of the items!").arg(commandParts.at(i)));
                    return false;
                }
            }
            /* Is the classname a valid classname? */
            else if (syntaxTemplateParts.at(i) == "classname")
            {
                SyntaxChecker namechecker;
                if (!namechecker.isClassNameAllowed(commandParts.at(i)))
                {
                    insertPlainText("\n-Error : Name syntax is not allowed!");
                    return false;
                }
            }
            else if (syntaxTemplateParts.at(i).contains("subitem"))
            {
                int ind = syntaxTemplateParts.at(i).indexOf(":");
                int count = syntaxTemplateParts.at(i).size() - ind;
                QString val = syntaxTemplateParts.at(i).right(count);
                val.remove(":");
                if (commandParts.at(i) != val)
                {
                    insertPlainText(QString("\n-Error : %1 is not a valid subitem. Type \"subitems\" to see a list of the subitem!").arg(command));
                    return false;
                }
            }
            else if (syntaxTemplateParts.at(i) == "x")
            {
                /* Check if a valid number */
                SyntaxChecker numChecker;
                if (!numChecker.isDigit(commandParts.at(i)))
                {
                    insertPlainText("\n-Error : X is not a digit!");
                    return false;
                }
            }
            else if (syntaxTemplateParts.at(i) == "y")
            {
                /* Check if a valid number */
                SyntaxChecker numChecker;
                if (!numChecker.isDigit(commandParts.at(i)))
                {
                    insertPlainText("\n-Error : Y is not a digit!");
                    return false;
                }
            }
            else if (syntaxTemplateParts.at(i) == "w")
            {
                /* Check if a valid number */
                SyntaxChecker numChecker;
                if (!numChecker.isDigit(commandParts.at(i)))
                {
                    insertPlainText("\n-Error : Width is not a digit!");
                    return false;
                }
            }
            else if (syntaxTemplateParts.at(i) == "h")
            {
                /* Check if a valid number */
                SyntaxChecker numChecker;
                if (!numChecker.isDigit(commandParts.at(i)))
                {
                    insertPlainText("\n-Error : Height is not a digit!");
                    return false;
                }
            }
        }
    }
    else
    {
        /* Check if the user closed the parantheses */
        if (line[line.length()-1] != ')')
        {
            insertPlainText("\n-Error : Missing right parenthesis!");
            return false;
        }
        
        /* Are there any dots between ( and ), unless it is setNamespace with dot(s) */
        if (line.contains("setNamespace") && line.contains("."))
        {
            /* Namespace may have dots in it, so split it in other way */
            QString tmpLine = line;
            QString strParam = tmpLine.mid(line.indexOf("("));
            QString strMain = tmpLine.remove(strParam);
            commandParts = strMain.split(".");
            commandParts[commandParts.count()-1] += strParam;
        }
        else
        {
            SyntaxChecker namechecker;
            if (!namechecker.parameterDotFree(line))
            {
                insertPlainText("\n-Error : Dot found between ( and )!");
                return false;
            }
            
            /* This may be a method of a class to set its property or elements property : split again with dot */
            commandParts = line.split(".");
        }
        
        /* 2 means only methods of classitem */
        if (commandParts.count() == 2)
        {
            /* Get all class items */
            QList<ClassItem*> *items = ProjectManager::getInstance()->getActiveProject()->getClassItems();
            
            foreach (ClassItem *item, *items)
            {
                /* If the command is a valid classname */
                if (item->className() == commandParts[0])
                {
                    /* Get the method name and parameter(s) if exists */
                    QString method = commandParts[1];
                    return checkMethod(method, CLASSLIST);
                }
            }
            
            insertPlainText("\n-Error : Wrong class name!");
            return false;
        }
        /* 3 means a member or a function */
        else if (commandParts.count() == 3)
        {
            /* Get all class items */
            QList<ClassItem*> *items = ProjectManager::getInstance()->getActiveProject()->getClassItems();
            
            bool classFound = false;
            bool memberFound = false;
            bool functionFound = false;
            
            /* A valid class name? */ 
            ClassItem *item;
            foreach (item, *items)
                if (item->className() == commandParts[0])
                {
                    classFound = true;
                    break;
                }
            
            /* If there is no valid class name in command then return */
            if (!classFound)
            {
                insertPlainText("\n-Error : Wrong class name!");
                return false;
            }
    
            /* So, now see see if the second part of the command is a member or a function */
            foreach (MemberProperties *member, item->getListMemberProperties())
                if (member->name == commandParts[1])
                {
                    memberFound = true;
                    break;
                }
            
            /* If true, then a member */
            if (memberFound)
            {
                /* Check method */
                QString method = commandParts[2];
                return checkMethod(method, MEMBERLIST);
            }

            /* If not, then look if it is a function */
            FunctionProperties *function;
            foreach (function, item->getListFunctionProperties())
            {
                /* Remove the visibility : first char visibility, second is space */
                QString funcFullName(function->string(false));
                funcFullName = funcFullName.remove(0, 2);
                if (funcFullName == commandParts[1])
                {
                    functionFound = true;
                    break;
                }
            }
            
            /* If true, then a function */
            if (functionFound)
            {
                /* Check method name */
                QString method = commandParts[2];
                return checkMethod(method, FUNCTIONLIST);
            }
            else
            {
                insertPlainText("\n-Error : Wrong member or function name!");
                return false;
            }
        }
        /* 4 means a parameter */
        else if (commandParts.count() == 4)
        {
            /* Get all class items */
            QList<ClassItem*> *items = ProjectManager::getInstance()->getActiveProject()->getClassItems();
            
            bool classFound = false;
            bool functionFound = false;
            bool parameterFound = false;
            
            /* A valid class name? */ 
            ClassItem *item;
            foreach (item, *items)
                if (item->className() == commandParts[0])
                {
                    classFound = true;
                    break;
                }
            
            /* If there is no valid class name in command then return */
            if (!classFound)
            {
                insertPlainText("\n-Error : Wrong class name!");
                return false;
            }
    
            /* Check if this a valid function */
            FunctionProperties *function;
            foreach (function, item->getListFunctionProperties())
            {
                /* Remove the visibility : first char visibility, second is space */
                QString funcFullName(function->string(false));
                funcFullName = funcFullName.remove(0, 2);
                if (funcFullName == commandParts[1])
                {
                    functionFound = true;
                    break;
                }
            }
            
            /* If true, then a function */
            if (functionFound)
            {
                /* And the third one must be a parameter */
                ParameterProperties *parameter;
                foreach (parameter, function->parameters)
                    if (parameter->name == commandParts[2])
                    {
                        parameterFound = true;
                        break;
                    }
                        
                /* If true, then show the popup for this parameter */
                if (parameterFound)
                {
                    /* Check method name */
                    QString method = commandParts[3];
                    return checkMethod(method, PARAMETERLIST);
                }
                else
                {
                    insertPlainText("\n-Error : Wrong parameter name!");
                    return false; 
                }
            }
            else
            {
                insertPlainText("\n-Error : Wrong function name!");
                return false;
            }
        }
    }

    return true;
}

/*! This function returns true, if the given parameter a valid method name is and has the right parameter
 * names or value. */
bool CodeWindow::checkMethod(const QString &_method, LIST_TYPE listType)
{
    QString method = _method;
    
    /* For which item shall the method be checked */
    QStringList methodLists;
    switch (listType)
    {
        case CLASSLIST:
            methodLists = m_listClassMethods;
            break;
        case MEMBERLIST:
            methodLists = m_listMemberMethods;
            break;
        case FUNCTIONLIST:
            methodLists = m_listFunctionMethods;
            break;
        case PARAMETERLIST:
            methodLists = m_listParameterMethods;
            break;
        default:
            /* Shouldnt happen */
            return false;
    }
    
    if (method.contains("()"))
    {
        /* Check if the method name is valid */
        if (!methodLists.contains(method))
        {
            insertPlainText("\n-Error : Wrong method name!");
            return false;
        }
        
        /* Check, if this a clear-function or remove-function */
        if (method.contains("clear") || method.contains("remove"))
            return true;

        /* Check if the method has to have some parameters and the user forgot them */
        QString methodNameOnly = method.remove("()");
        /* Get the syntax info */
        QString strExp = QString("%1(.*)").arg(methodNameOnly);
        /* Find the index of the method in the syntaxinfo list */
        int index = m_listSyntaxInfo.indexOf(QRegularExpression(strExp), 0);
        /* Get the template for the given method */
        QString methodInfo = m_listSyntaxInfo.at(index);
        
        /* There are two kind of functions : a) parameters as strings b) parameters as enums
         * enums must have the complete name like true, false or inout */
        if (methodInfo.contains("value"))
        {
            /* Then this function takes one parameter with an enum value, user forgot this */
            insertPlainText("\n-Error : Wrong number of parameters!");
            return false;
        }
        else
        {
            /* Get the parameter info */
            QString parameterInfoOnly = methodInfo.mid(methodInfo.indexOf("("));
            
            /* Remove ( and ) from parameter info and get actual the parameter count */
            parameterInfoOnly = parameterInfoOnly.remove("(");
            parameterInfoOnly = parameterInfoOnly.remove(")");
            QStringList parameterInfoList = parameterInfoOnly.split(",");
            
            if (parameterInfoList.count() > 0)
            {
                insertPlainText("\n-Error : Wrong number of parameters!");
                return false;
            }
        }
    }
    else
    {
        /* Seperate the method name and parameter(s) */
        QString parameterOnly = method.mid(method.indexOf("("));
        QString methodNameOnly = method.remove(parameterOnly);
        /* The methods are saved like method() */
        QString searchText = methodNameOnly + "()"; 
        /* Is methodname valid? */
        if (!methodLists.contains(searchText))
        {
            insertPlainText("\n-Error : Wrong method name!");
            return false;
        }
        
        /* Then remove first ( and ) from parameter */
        parameterOnly = parameterOnly.remove("(");
        parameterOnly = parameterOnly.remove(")");
        QStringList parameterList = parameterOnly.split(",");
        /* Trim beginning and end of strings against spaces */
        parameterList.replaceInStrings(QRegularExpression("^\\s*|\\s*$"), "");
        
        /* Namespace may have . and : in its parameter */
        if (methodNameOnly.contains("setNamespace"))
        {
            QRegularExpression re("^[a-zA-Z_][a-zA-Z0-9_.:]*$");
            QRegularExpressionMatch match = re.match(parameterOnly);
            if (!match.hasMatch())
            {
                QString err = QString("\n-Error : Parameter name \"%1\" is not allowed!").arg(parameterOnly);;
                insertPlainText(err);
                return false;    
            } 
        }
        else if (methodNameOnly.contains("setInitialValue") || methodNameOnly.contains("setDescription") || 
                 methodNameOnly.contains("setCode") || methodNameOnly.contains("setStereotype"))
        {
            ; // dont check these parameter; can be numeric (int) or alpha-numeric (QString) or text
        }
        else if (methodNameOnly.contains("setType") || methodNameOnly.contains("setReturnType"))
        {
            QRegularExpression re("^[a-zA-Z_][a-zA-Z0-9_ ]*$");
            QRegularExpressionMatch match = re.match(parameterOnly);
            if (!match.hasMatch())
            {
                QString err = QString("\n-Error : Parameter name \"%1\" is not allowed!").arg(parameterOnly);;
                insertPlainText(err);
                return false;    
            }
        }
        else
        {
            /* Check parameter name syntax */
            foreach (QString paramChecks, parameterList)
            {
                SyntaxChecker namechecker;
                if (!namechecker.isParameterNameAllowed(paramChecks))
                {
                    QString err = QString("\n-Error : Parameter name \"%1\" is not allowed!").arg(paramChecks);;
                    insertPlainText(err);
                    return false;
                }
            }
        }
        
        /* Get the syntax info */
        QString strExp = QString("%1(.*)").arg(methodNameOnly);
        /* Find the index of the method in the syntaxinfo list */
        int index = m_listSyntaxInfo.indexOf(QRegularExpression(strExp), 0);
        /* Get the template for the given command */
        QString methodInfo = m_listSyntaxInfo.at(index);
        
        /* There are two kind of functions : a) parameters as strings b) parameters as enums
         * enums must have the complete name like true, false or inout */
        if (methodInfo.contains("value:"))
        {
            /* Check if the number of parameters is ok */
            if (parameterList.count() != 1)
            {
                /* Then this function takes one parameter with an enum value, user forgot this */
                insertPlainText("\n-Error : Wrong number of parameters!");
                return false;
            }
            
            /* Check if the parameter is correct given? */
            QString parameterInfoOnly = methodInfo.mid(methodInfo.indexOf(":"));
            parameterInfoOnly = parameterInfoOnly.remove(":");
            /* Remove only ) from parameter info */
            parameterInfoOnly = parameterInfoOnly.remove(")");
            
            /* Possible values for this method */
            QStringList parameterInfoList = parameterInfoOnly.split(",");
            foreach (QString parInfo, parameterInfoList)
                if (parInfo == parameterList[0])
                    return true;
            
            /* Then this function takes one parameter with an enum value, user forgot this */
            insertPlainText("\n-Error : Parameter value not correct!");
            return false;
        }
        else
        {
            /* Then this is a method with string parameters. Get the parameter info. */
            QString parameterInfoOnly = methodInfo.mid(methodInfo.indexOf("("));
            /* Then remove ( and ) from parameter info too */
            parameterInfoOnly = parameterInfoOnly.remove("(");
            parameterInfoOnly = parameterInfoOnly.remove(")");
            QStringList parameterInfoList = parameterInfoOnly.split(",");
            
            if (method.contains("addFunction"))
            {
                /* addFunction has another syntax as the other functions: it can have more than two parameters, therefore min = 2 parameters */
                /* The number of parameters must be paired */
                if (parameterInfoList.count() % 2 != 0)
                {
                    insertPlainText("\n-Error : Wrong number of parameters!");
                    return false;
                }
            }
            else
            {
                /* Are the parameter complete : not much not less */
                if (parameterInfoList.count() != parameterList.count())
                {
                    insertPlainText("\n-Error : Wrong number of parameters!");
                    return false;
                }
            }
            
            /* Check if parameters are really given, there are no empty parameters */
            foreach (QString elem, parameterList)
                if (elem.isEmpty())
                {
                    /* Then this function takes one parameter with an enum value, user forgot this */
                    insertPlainText("\n-Error : Parameter value empty!");
                    return false;
                }
        }
    }
    
    /* Everything is okay */
    return true;
}

/*!
 * \brief This function checks, is there any valid name given before a point ".". If so, it shows all the methods and objects for that item.
 * This function will be called, if :
 * * the user presses a dot
 * * or the user presses ctrl+space after a dot (closes the window with esc) */
void CodeWindow::dotPressed(bool ctrlSpacePressed)
{
    /* Get all class items */
    QList<ClassItem*> *items = ProjectManager::getInstance()->getActiveProject()->getClassItems();
    
    /* If there are no items, then do nothing */
    if (items->isEmpty())
        return; 

    /* Get the text from the console */
    QTextCursor cursor = textCursor();
    /* The whole line */
    cursor.select(QTextCursor::LineUnderCursor);
    const QString itemConsoleCmd = cursor.selectedText();
    /* Copy because itemConsole is const */
    QString splitting = itemConsoleCmd;

    /* If this function is called with ctrl+space, then there is dot at the end, remove it
     * otherwise this function will be called by pressing the dot. In this case the dot will
     * be removed automatically. */
    if (ctrlSpacePressed)
        splitting = splitting.remove(splitting.size()-1, 1);
    
    /* remove > */
    splitting = splitting.remove(0, 1); 
    /* Split into strings if there are more than one point : e.g class.function.parameter */
    QStringList splittedList = splitting.split(".");

    /* 1 dot means we need to show methods of classitem */
    if (splittedList.count() == 1)
    {
        /* Get all the names */ 
        foreach (ClassItem *item, *items)
        {
            /* If the command is a valid classname */
            if (item->className() == splittedList[0])
            {
                QStringList tmpList(m_listClassMethods);
                /* If this class item has some members or function, then add them to the list */
                if (!item->getListMemberProperties().empty())
                    foreach (MemberProperties *member, item->getListMemberProperties())
                        tmpList.append(member->name);
                /* Functions can not have unique name. Must identify with its parameter and return type */
                if (!item->getListFunctionProperties().empty())
                    foreach (FunctionProperties *function, item->getListFunctionProperties())
                    {
                        /* Remove the visibility : first char visibility, second is space */
                        QString funcFullName(function->string(false));
                        funcFullName = funcFullName.remove(0, 2);
                        tmpList.append(funcFullName);
                    }
                
                /* Update the model - show the methods + members&functions of a class item */
                tmpList.sort(Qt::CaseInsensitive);
                m_pModel->setStringList(tmpList);
                /* Open popup window */
                popupCompleter();
                /* Finish */
                return;
            }
        }
    }
    /* 2 dots mean we need to show methods of a member or function */
    else if (splittedList.count() == 2)
    {
        bool classFound = false;
        bool memberFound = false;
        bool functionFound = false;
        
        /* Look for the class first : must have the format : classname.member or classname.function */ 
        ClassItem *item;
        foreach (item, *items)
            if (item->className() == splittedList[0])
            {
                classFound = true;
                break;
            }
        
        /* If there is no valid class name in command then return */
        if (!classFound)
            return;

        /* So, now see see if the second part of the command is a member or a function */
        foreach (MemberProperties *member, item->getListMemberProperties())
            if (member->name == splittedList[1])
            {
                memberFound = true;
                break;
            }
        
        /* If true, then show the popup for a member */
        if (memberFound)
        {
            /* Show the methods of a class item's member */
            QStringList tmpList(m_listMemberMethods);
            tmpList.sort(Qt::CaseInsensitive);
            m_pModel->setStringList(tmpList);
            /* Open popup window */
            popupCompleter();
            /* Finish */
            return;
        }
        
        /* If not, then look for a function */
        FunctionProperties *function;
        foreach (function, item->getListFunctionProperties())
        {
            /* Remove the visibility : first char visibility, second is space */
            QString funcFullName(function->string(false));
            funcFullName = funcFullName.remove(0, 2);
            if (funcFullName == splittedList[1])
            {
                functionFound = true;
                break;
            }
        }
        
        /* If true, then show the popup for a function */
        if (functionFound)
        {
            QStringList tmpList(m_listFunctionMethods);
            if (!function->parameters.isEmpty())
                foreach (ParameterProperties *parameter, function->parameters)
                    tmpList.append(parameter->name);
            
            /* Show the methods and parameters of a class item's function */
            tmpList.sort(Qt::CaseInsensitive);
            m_pModel->setStringList(tmpList);
            /* Open popup window */
            popupCompleter();
        }
    }
    /* 3 dots mean we need to show methods of a parameter */
    else if (splittedList.count() == 3)
    {
        bool classFound = false;
        bool functionFound = false;
        bool parameterFound = false;
        
        /* Look for the class first : must have the format : classname.member or classname.function */ 
        ClassItem *item;
        foreach (item, *items)
            if (item->className() == splittedList[0])
            {
                classFound = true;
                break;
            }
        
        /* If there is no valid class name in command then return */
        if (!classFound)
            return;

        /* See now if the second part of the command is a function */
        FunctionProperties *function;
        foreach (function, item->getListFunctionProperties())
        {
            QString funcFullName(function->string(false));
            funcFullName = funcFullName.remove(0, 2);
            if (funcFullName == splittedList[1])
            {
                functionFound = true;
                break;
            }
        }
        
        /* If there is no valid class name in command then return */
        if (!functionFound)
            return;

        /* And the third one must be a parameter */
        ParameterProperties *parameter;
        foreach (parameter, function->parameters)
            if (parameter->name == splittedList[2])
            {
                parameterFound = true;
                break;
            }
                
        /* If true, then show the popup for this parameter */
        if (parameterFound)
        {
            QStringList tmpList(m_listParameterMethods);
            /* Show the methods of a function's parameter */
            tmpList.sort(Qt::CaseInsensitive);
            m_pModel->setStringList(tmpList);
            /* Open popup window */
            popupCompleter();
        }
    }
}

/*!
 * \brief This function gets the syntax information from a template file. See BinaryWriter. */
bool CodeWindow::getSyntaxInfoFromTemplate()
{
    /* Get the exe path */
    QString tmpFilePath = QCoreApplication::applicationDirPath() + "/../templates/SyntaxTemplate.tmp";
    
    /* Open the template file to generate the source files */
    QFile templateFile(tmpFilePath);     // TODO : this file should be in installation path
    if (!templateFile.open(QFile::ReadOnly))
    {
      //MonitorManager::getInstance()->logMsg("ERROR : Could not open file %0 for reading", 1, "ClassTemplate.tmp");
        qDebug() << "Could not open file ClassTemplate.tmp on " << tmpFilePath;
        return false;
    }
    
    QDataStream inTemplate(&templateFile);
    inTemplate.setVersion(QDataStream::Qt_5_3);
    inTemplate.setDevice(&templateFile);
    
    /* Check file extension */
    qint32 magicNumber;
    inTemplate >> magicNumber;
    
    if (TEMPLATE_FILE::MagicNumber != magicNumber)
    {
        //MonitorManager::getInstance()->logMsg("ERROR : Wrong template file found. Please contact to your admin!", 0);
        return false;
    }
    
    /* Check version number */
    qreal versionNumber;
    inTemplate >> versionNumber;
    if (TEMPLATE_FILE::VersionNumber != versionNumber)
    {
        //MonitorManager::getInstance()->logMsg("ERROR : Wrong template file found. Please contact to your admin!", 0);
        return false;
    }
    
    /* Read template items in an array. PS : For now there are not much items, it can be read in an array */
    QString item;
    while (!inTemplate.atEnd())
    {
        /* Get the line */
        inTemplate >> item;
        /* Into the buffer */
        m_listSyntaxInfo << item;
    }
    
    return true;
}

void CodeWindow::keyPressEvent(QKeyEvent *event)
{
    QTextCursor cursor = textCursor();
    int curPosInBlock = cursor.positionInBlock();
    
    /* If somehow the cursor position is beyond >, then move it to right */
    if (curPosInBlock == 0)
        moveCursor(QTextCursor::Right);
    
    /* If the cursor is near the readySign, then there are some events, which the user may not do. Ignore them. */
    else if (curPosInBlock == 1)
    {
        switch (event->key())
        {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Left: /* May go to right */
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape:
            case Qt::Key_Space:
            case Qt::Key_Backspace: 
                event->ignore(); 
                return;
        }
    }
   
    /* If the user opened a paranthesis */
    if (m_bPressedParenLeft)
    {
        switch (event->key())
        {
            case Qt::Key_Home:
            case Qt::Key_Delete:
            case Qt::Key_Backspace:
            case Qt::Key_Left:
            {
                /* If the cursor passes the first ( over, hide the tooltip*/
                cursor.select(QTextCursor::LineUnderCursor);
                QString line = cursor.selectedText();
                int indexOf1stParanth = line.indexOf("(");
                
                if (curPosInBlock < indexOf1stParanth + 2) //+2 because of indexing
                {
                    QToolTip::hideText();
                    m_bPressedParenLeft = false;
                }
                break;
            }
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                break;
            default:
                break;
        }   
    }

    if (m_pCompleter->popup()->isVisible())
    {
        /* If the popup is open, then ignore these key-events or hide the popup */
        switch (event->key())
        {
            case Qt::Key_Up:
            case Qt::Key_Down:
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Escape: 
                event->ignore(); 
                return;
            default: 
                m_pCompleter->popup()->hide(); 
                break;
        }
    }
    else
    {
        /* If popup not visible and pressed dot, then check if there is a classname or propertyname before it */
        switch (event->key())
        {
            case Qt::Key_Period:
                /* Last prefix is still in completer, clear it in order to open the popup for dot-press */
                m_pCompleter->setCompletionPrefix("");
                /* false means dot is pressed, not ctrl+space */
                dotPressed(false);
                break;

            case Qt::Key_Home:
            {
                /* If home pressed, cursor may not go beyond readySign */
                QTextCursor cursor = textCursor();
                cursor.select(QTextCursor::Document);
                /* Get the text */
                const QString docu = cursor.selectedText();
                int indexOfLast = docu.lastIndexOf(readySign) + 1;
                //int indexOfLast = docu.lastIndexOf(QRegularExpression(">[^\\s]")) + 1;
                cursor.setPosition(indexOfLast);
                setTextCursor(cursor);
                /* Don't give this key to QPlainTextEdit::keyPressEvent */
                return;
            }
            case Qt::Key_Space:
            case Qt::Key_Control:
            case Qt::Key_End:
            case Qt::Key_Delete:
            case Qt::Key_Backspace:
            case Qt::Key_Enter:
            case Qt::Key_Return:
            case Qt::Key_Left:
            case Qt::Key_Right: 
                break; /* These are the valid keys, which i can not capture with regular expressions */
            /* If the user presses (, then show tooltip, if there is a valid method-name before it */
            case Qt::Key_ParenLeft:
            {
                QTextCursor cursor = textCursor();
                cursor.select(QTextCursor::WordUnderCursor);
                showTooltip(cursor.selectedText(), false);
            }
                break;
            default:
                /* List of valid characters */
                QString chr = event->text();
                if (!chr.contains((QRegularExpression("[0-9a-zA-Z_.,():]"))))
                    return;
        }
    }

    /* Enter/Return pressed : process the command */
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        /* If tooltip visible, hide it anyway */
        QToolTip::hideText();
        m_bPressedParenLeft = false;
        
        /* Get the line and remove readySign */
        QString line = cursor.block().text().trimmed().remove(0, 1); 
        moveCursor(QTextCursor::End);

        /* Check it */
        if (checkSyntax(line))
           performCommand(line);
        else
           insertPlainText("\nError : Syntax error!");
    }
    
    /* For some events we must call parent event */
    QPlainTextEdit::keyPressEvent(event);
    
    /* Put the readySign again and update the position of command-entry point */
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        insertPlainText(readySign);
}

/*!
 * \brief This function inserts the selected word from the completion popup window into console. */
void CodeWindow::insertCompletion(const QString &completion)
{
    /* Insert the text */
    QTextCursor cursor = textCursor();

    /* Get the line first */
    cursor.select(QTextCursor::LineUnderCursor);
    QString line = cursor.selectedText();
    
    /* At this point there will be two options : user entered classname and pressed completion, so there is no dot yet
     * or he entered a property or method, this means there is at least one dot. */
    if (!line.contains("."))
    {
        cursor.movePosition(QTextCursor::EndOfLine);
        cursor.select(QTextCursor::WordUnderCursor);
    }
    else
    {
        /* Get all document for the real position */
        cursor.select(QTextCursor::Document);
        int index = cursor.selectedText().lastIndexOf(readySign);
        /* Set position in the line but using the position in document */
        int lastIndex = line.lastIndexOf('.') + 1;
        int lineSize = line.size();
        cursor.setPosition(index + lastIndex);
        cursor.setPosition(index + lineSize, QTextCursor::KeepAnchor);
    }

    cursor.insertText(completion);
    
    //int numberOfCharsToComplete = completion.length() - m_pCompleter->completionPrefix().length();
    //cursor.insertText(completion.right(numberOfCharsToComplete));
    setTextCursor(cursor);
    
    /* If a method */
    if (completion.contains("()"))
    {
        /* Get the name */
        QString info = completion;
        info = info.remove("()");
        showTooltip(info, true);
    }
}

void CodeWindow::mousePressEvent(QMouseEvent *event)
{
    QPlainTextEdit::mousePressEvent(event);
    mousePositionCheck();
}

void CodeWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QPlainTextEdit::mouseReleaseEvent(event);
    mousePositionCheck();
}

/*!
 * \brief This function checks, if the position of the cursor is after mouse press/release on the valid line.
 * User may type only in the current line. */
void CodeWindow::mousePositionCheck()
{
    QTextCursor cursor = textCursor();

    /* Cursor may not go behind to > */
    if (cursor.positionInBlock() <= 0)
        moveCursor(QTextCursor::Right);
    else
    {
        int curPos = cursor.position();
        /* TODO selection whole document for qstring max 2gb at once, see if ok */
        cursor.select(QTextCursor::Document);
        int index = cursor.selectedText().lastIndexOf(readySign);
        if (curPos < index)
        {
            cursor.setPosition(index+1);
            setTextCursor(cursor);
        }
    }
}

/*!
 * \brief This function sets the model of the completer and pops a windows with the list of matched-words up, if there are multiple words 
 * with the given prefix given or inserts it directly if there is only one word. */
void CodeWindow::performCompletion()
{
    /* Find out which list must be shown */
    {
        QTextCursor cursor = textCursor();
        /* Get the whole line */
        cursor.select(QTextCursor::LineUnderCursor);
        const QString itemConsole = cursor.selectedText();
        /* Copy because itemConsole is const */
        QString splitting = itemConsole;
        /* remove > */
        splitting = splitting.remove(0, 1);
        /* Split */ 
        QStringList splittedList = splitting.split(QRegularExpression("\\s"));
        
        /* If count == 1, user gave only one word, this can be a command or if there is dot in it, then a classname followed by member or function */
        if (splittedList.count() == 1)
        {
            /* If the command has a point at end, then look if it is a valid name */
            /* If the size is 0, means user typed nothing just pressed ctrl+space */
            /* If the size is 1, means user typed one char which may not be a dot */
            if (splittedList[0].size() > 1)
            {
                /* If ctrl+space is pressed, and there is a dot at the end */
                if (splittedList[0].at(splittedList[0].size()-1) == '.')
                {
                    /* Last prefix is still in completer, clear it in order to open the popup for dot-press */
                    m_pCompleter->setCompletionPrefix("");
                    /* true means there is a dot at the end of the string */
                    dotPressed(true);
                    /* Below shall not be done */
                    return;
                }
                /* If the last char is not a dot but the string has a dot in it, then this is a property call. In this case
                 * when the user presses the ctrl+space, then for the property or method the completion or popup shall be performed. */
                else if (splittedList[0].contains(QRegularExpression("\\.")))
                {
                    /* Split the string with dot */
                    QStringList dotSplitList = splittedList[0].split(QRegularExpression("\\."));
                    /* Perform completion or popup for more than one item */
                    setModelStringForProperty(dotSplitList);
                    /* Dont do below */
                    return;
                }
            }

            /* Add the commands to a temp list */
            QStringList commands(m_listCommands);
            /* Each time a completion will be performed, add the class items as a command to completion */
            QList<ClassItem*> *items = ProjectManager::getInstance()->getActiveProject()->getClassItems();
            /* If there are no items, then do nothing */
            if (!items->isEmpty())
            {
                /* Get all the names */
                foreach (ClassItem *item, *items)
                    commands.append(item->className());
                /* Sort the list, so the completion can work */
                commands.sort(Qt::CaseInsensitive);
                m_pModel->setStringList(commands);
            }
        }
        /* If count == 2, user gave two words, this shall be an item */    
        else if (splittedList.count() == 2)
        {
            m_pModel->setStringList(m_listItems);
        }
        /* If count == 4, user gave two words, this shall be an item */    
        else if (splittedList.count() == 4)
        {
            m_pModel->setStringList(m_listSubCommands);
        }
        /* If count == 3 or more than 4 then this is not the part, where we want to show the window */
        else
        {
            return;
        }
    }
    
    /* Get the current cursor */
    QTextCursor cursor = textCursor();
    /* Select the word under it */
    cursor.select(QTextCursor::WordUnderCursor);
    /* Get the text */
    const QString completionPrefix = cursor.selectedText();
    
    /* Set the prefix only if it changed */
    if (completionPrefix != m_pCompleter->completionPrefix())
    {
        /* Set for completion */
        m_pCompleter->setCompletionPrefix(completionPrefix);
        /* Select the first item in the list */
        m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index(0, 0));
    }
    
    if (m_pCompleter->completionCount() == 1)
    {
        /* If there is only one item, so paste it */
        insertCompletion(m_pCompleter->currentCompletion());
    }
    else
    {
        popupCompleter();
    }
}

/*!
 * \brief This function performs the command. */
void CodeWindow::performCommand(const QString &command)
{
    /* Split the command line */
    QStringList commandParts = command.split(QRegularExpression("\\s"));
    if (commandParts.at(COMMAND) == "create")
    {
        QString className = commandParts.at(ARG1);
        int x = commandParts.at(ARG2).toInt();
        int y = commandParts.at(ARG3).toInt();
        ((MainWindow*)(parent()->parent()))->addClassItem(className, QPoint(x, y));
    }
    else if (commandParts.at(COMMAND) == "delete")
    {
        QString className = commandParts.at(ARG1);
        ((MainWindow*)(parent()->parent()))->projectItems()->removeItem(className);
    }
    else if (commandParts.at(COMMAND) == "move")
    {
        QString className = commandParts.at(ARG1);
        int x = commandParts.at(ARG2).toInt();
        int y = commandParts.at(ARG3).toInt();
        ClassItem *item = ProjectManager::getInstance()->getActiveProject()->getClassItem(className);
        item->storePosition();
        item->setPos(QPoint(x, y));
        ((MainWindow*)(parent()->parent()))->itemMoved(item);
    }
    else if (commandParts.at(COMMAND) == "rename")
    {
        QString classNameOld = commandParts.at(ARG1);
        QString classNameNew = commandParts.at(ARG2);
        ((MainWindow*)(parent()->parent()))->itemRenamed(classNameOld, classNameNew);
    }
    else if (commandParts.at(COMMAND) == "resize")
    {
        QString className = commandParts.at(ARG1);
        int w = commandParts.at(ARG2).toInt();
        int h = commandParts.at(ARG3).toInt();
        ClassItem *item = ProjectManager::getInstance()->getActiveProject()->getClassItem(className);
        item->storeCurrentSize();
        item->resizeItem(w, h);
    }
    else
    {
        /* Else, this is a set-property-method */
        QStringList commandParts;
        
        /* Are there any dots between ( and ), unless it is setNamespace, which may have dots */
        if (command.contains("setNamespace"))
        {
            /* Namespace may have dots in it, so split it in other way */
            QString tmpLine = command;
            QString strParam = tmpLine.mid(tmpLine.indexOf("("));
            QString strMain = tmpLine.remove(strParam);
            commandParts = strMain.split(".");
            commandParts[commandParts.count()-1] += strParam;
        }
        else
        {
            commandParts = command.split(".");
        }
        
        if (commandParts.count() == 2)
        {
            /* A method for class item's general properties */
            ClassItem *classItem = ProjectManager::getInstance()->getActiveProject()->getClassItem(commandParts[0]);
            QString method = commandParts[1];

            /* Get parameter value */
            QString value = method.mid(method.indexOf("("));
            value = value.remove("(");
            value = value.remove(")");
            
            if (method.contains("setNamespace"))
            {
                value.replace(".", "::");
                classItem->getGeneralProperties()->nameSpace = value;
            }
            else if (method.contains("clearNamespace"))
            {
                classItem->getGeneralProperties()->nameSpace = "";
            }            
            else if (method.contains("setName"))
            {
                classItem->getGeneralProperties()->name = value;
            }
            else if (method.contains("setStereotype"))
            {
                classItem->getGeneralProperties()->stereotype = value;
            }
            else if (method.contains("clearStereotype"))
            {
                classItem->getGeneralProperties()->stereotype = "";
            }
            else if (method.contains("setAbstract"))
            {
                classItem->getGeneralProperties()->isAbstract = (value == "true" ? true : false);
            }
            else if (method.contains("setVisibility"))
            {
                classItem->getGeneralProperties()->visibility = (value == "public" ? PUBLIC : (value == "private" ? PRIVATE : PROTECTED));
            }
            else if (method.contains("addMember"))
            {
                /* Here value has two values seperated with comma */
                value = value.replace(QRegularExpression("\\s*"), "");
                QStringList listVals = value.split(",");
                classItem->addMember(listVals[0], listVals[1]);
            }
            else if (method.contains("addFunction"))
            {
                /* Here value has two values seperated with comma */
                value = value.replace(QRegularExpression("\\s*"), "");
                QStringList listVals = value.split(",");

                QList<ParameterProperties *> *params = new QList<ParameterProperties *>();
                int countParameters = (listVals.count() - 2) / 2;
                for (int j = 0; j < countParameters; ++j)
                {
                    ParameterProperties *newParam = new ParameterProperties;
                    newParam->type = listVals[2+j];
                    newParam->name = listVals[3+j];
                    params->append(newParam);
                }
                if (!classItem->addFunction(listVals[0], listVals[1], params))
                    insertPlainText("\n-Error : This function already exists!");
            }
            classItem->scene()->update();
        }
        else if (commandParts.count() == 3)
        {
            /* Method of a member or a function : find out which one */
            ClassItem *classItem = ProjectManager::getInstance()->getActiveProject()->getClassItem(commandParts[0]);
            
            /* A member or a function */
            QString subItem = commandParts[1];

            /* Get the parameter values */
            QString method = commandParts[2];
            QString value = method.mid(method.indexOf("("));
            value = value.remove("(");
            value = value.remove(")");

            bool memberFound = false;            
            MemberProperties *member;
            foreach(member, classItem->getListMemberProperties())
                if (member->name == subItem)
                {
                    memberFound = true;
                    break;
                }
            
            if (memberFound)
            {
                if (method.contains("setName"))
                {
                    member->name = value;
                }
                else if (method.contains("setType"))
                {
                    member->type = value;
                }
                else if (method.contains("setStatic"))
                {
                    member->isStatic = (value == "true" ? true : false);
                }
                else if (method.contains("setInitialValue"))
                {
                    member->initValue = value;
                }
                else if (method.contains("clearInitialValue"))
                {
                    member->initValue = "";
                }
                else if (method.contains("setStereotype"))
                {
                    member->stereotype = value;
                }
                else if (method.contains("clearStereotype"))
                {
                    member->stereotype = "";
                }
                else if (method.contains("setVisibility"))
                {
                    member->visibility = (value == "public" ? PUBLIC : (value == "private" ? PRIVATE : PROTECTED));
                }
                else if (method.contains("setDescription"))
                {
                    member->description = value;
                }
                else if (method.contains("clearDescription"))
                {
                    member->description = "";
                }
                else if (method.contains("removeMember"))
                {
                    classItem->removeMember(member);
                }
                classItem->scene()->update();
            }
            else
            {
                bool functionFound = false;            
                FunctionProperties *function;
                foreach(function, classItem->getListFunctionProperties())
                {
                    QString funcFullName(function->string(false));
                    funcFullName = funcFullName.remove(0, 2);
                    if (funcFullName == subItem)
                    {
                        functionFound = true;
                        break;
                    }
                }
            
                if (functionFound)
                {
                    if (method.contains("setName"))
                    {
                        function->name = value;
                    }
                    else if (method.contains("setReturnType"))
                    {
                        function->returnType = value;
                    }
                    else if (method.contains("setVirtual"))
                    {
                        function->isVirtual = (value == "true" ? true : false);
                    }
                    else if (method.contains("setStatic"))
                    {
                        function->isStatic = (value == "true" ? true : false);
                    }
                    else if (method.contains("setStereotype"))
                    {
                        function->stereotype = value;
                    }
                    else if (method.contains("clearStereotype"))
                    {
                        function->stereotype = "";
                    }                    
                    else if (method.contains("setVisibility"))
                    {
                        function->visibility = (value == "public" ? PUBLIC : (value == "private" ? PRIVATE : PROTECTED));
                    }
                    else if (method.contains("setDescription"))
                    {
                        function->description = value;
                    }
                    else if (method.contains("clearDescription"))
                    {
                        function->description = "";
                    }                    
                    else if (method.contains("setCode"))
                    {
                        function->code = value;
                    }
                    else if (method.contains("clearCode"))
                    {
                        function->code = "";
                    }                    
                    else if (method.contains("addParameter"))
                    {
                        /* Here value has two values seperated with comma */
                        value = value.replace(QRegularExpression("\\s*"), "");
                        QStringList listVals = value.split(",");
                        if (!function->addParameter(listVals[0], listVals[1], classItem->getListFunctionProperties()))
                            insertPlainText("\n-Error : This parameter name already exists!");
                    }
                    else if (method.contains("removeFunction"))
                    {
                        classItem->removeFunction(function);
                    }
                    classItem->scene()->update();;
                }
            }
        }
        else if (commandParts.count() == 4)
        {
            /* Method of a parameter */
            ClassItem *classItem = ProjectManager::getInstance()->getActiveProject()->getClassItem(commandParts[0]);
            
            /* Must be a function */
            QString functionName = commandParts[1];

            /* Must be a parameter */
            QString parameterName = commandParts[2];
            
            /* Get the parameter values */
            QString method = commandParts[3];
            QString value = method.mid(method.indexOf("("));
            value = value.remove("(");
            value = value.remove(")");

            bool functionFound = false;            
            FunctionProperties *function;
            foreach(function, classItem->getListFunctionProperties())
            {
                QString funcFullName(function->string(false));
                funcFullName = funcFullName.remove(0, 2);
                if (funcFullName == functionName)
                {
                    functionFound = true;
                    break;
                }
            }
        
            if (functionFound)
            {
                bool parameterFound = false;
                ParameterProperties *parameter;
                foreach(parameter, function->parameters)
                {
                    if (parameter->name == parameterName)
                    {
                        parameterFound = true;
                        break;
                    }
                }
                
                if (parameterFound)
                {
                    if (method.contains("setName"))
                    {
                        parameter->name = value;
                    }
                    else if (method.contains("setType"))
                    {
                        parameter->type = value;
                    }
                    else if (method.contains("setInitialValue"))
                    {
                        parameter->initValue = value;
                    }
                    else if (method.contains("clearInitialValue"))
                    {
                        parameter->initValue = "";
                    }
                    else if (method.contains("setStereotype"))
                    {
                        parameter->stereotype = value;
                    }
                    else if (method.contains("clearStereotype"))
                    {
                        parameter->stereotype = "";
                    }
                    else if (method.contains("setDirection"))
                    {
                        parameter->direction = (value == "out" ? OUT : (value == "inout" ? INOUT : IN));
                    }
                    else if (method.contains("removeParameter"))
                    {
                        if (!function->removeParameter(parameter, classItem->getListFunctionProperties()))
                            insertPlainText("\n-Error : Can not remove the parameter. There is already another function with these properties!");
                    }
                    classItem->scene()->update();
                }
            }
        }
    }
}

/*!
 * \brief This function opens the popup window with commands, methods or members. The model's setStringList() sets the list of the items
 * which will be shown. */
void CodeWindow::popupCompleter()
{
    int width = m_pCompleter->popup()->sizeHintForColumn(0) + m_pCompleter->popup()->verticalScrollBar()->sizeHint().width();
    QRect rect = cursorRect();
    rect.setWidth(width);
    m_pCompleter->complete(rect);
}

/*!
 * \brief This function sets the model of the completer for a given class property.
 * Scenario : The user enters the classname, member, function or parameter and then presses dot and escape.
 * So he looses the popup window. He enters a prefix and presses ctrl+space; at this time the popup must
 * show the items or perform completion, if there is only one item. In this function, find out which part is
 * given and update the modellist. */
void CodeWindow::setModelStringForProperty(const QStringList &command)
{
    QString toComplete;
    /* Get all class items */
    QList<ClassItem*> *items = ProjectManager::getInstance()->getActiveProject()->getClassItems();
    
    /* If there are no items, then do nothing */
    if (items->isEmpty())
        return; 
        
    if (command.count() == 2) /* classname.prefix*/
    {
        bool classFound = false;
        ClassItem *item;
        foreach (item, *items)
            /* If the command is a valid classname */
            if (item->className() == command[0])
            {
                classFound = true;
                break;
            }
        
        if (!classFound)
            return;
        
        QStringList tmpList(m_listClassMethods);
        
        /* If this class item has some members or function, then add them to the list */
        if (!item->getListMemberProperties().empty())
            foreach (MemberProperties *member, item->getListMemberProperties())
                tmpList.append(member->name);
        
        if (!item->getListFunctionProperties().empty())
            foreach (FunctionProperties *function, item->getListFunctionProperties())
            {
                /* Remove the visibility : first char visibility, second is space */
                QString funcFullName(function->string(false));
                funcFullName = funcFullName.remove(0, 2);
                tmpList.append(funcFullName);
            }
        
        /* Update the model - show the methods + members&functions of a class item */
        tmpList.sort(Qt::CaseInsensitive);
        m_pModel->setStringList(tmpList);
        toComplete = command[1];
            
    }
    else if (command.count() == 3) /* classname.member.prefix or classname.function.prefix*/
    {
        bool classFound = false;
        bool memberFound = false;
        bool functionFound = false;
        
        /* Look for the class first : must have the format : classname.member or classname.function */ 
        ClassItem *item;
        foreach (item, *items)
        if (item->className() == command[0])
        {
            classFound = true;
            break;
        }
        
        /* If there is no valid class name in command then return */
        if (!classFound)
            return;
        
        /* So, now see see if the second part of the command is a member or a function */
        foreach (MemberProperties *member, item->getListMemberProperties())
            if (member->name == command[1])
            {
                memberFound = true;
                break;
            }
        
        /* If true, then set the model list */
        if (memberFound)
        {
            /* Show the methods of a class item's member */
            QStringList tmpList(m_listMemberMethods);
            tmpList.sort(Qt::CaseInsensitive);
            m_pModel->setStringList(tmpList);
            toComplete = command[2];
        }
        else
        {
            /* If not, then look for a function */
            FunctionProperties *function;
            foreach (function, item->getListFunctionProperties())
            {
                /* Remove the visibility : first char visibility, second is space */
                QString funcFullName(function->string(false));
                funcFullName = funcFullName.remove(0, 2);
                if (funcFullName == command[1])
                {
                    functionFound = true;
                    break;
                }
            }
            
            /* If true, then show the popup for a function */
            if (functionFound)
            {
                QStringList tmpList(m_listFunctionMethods);
                if (!function->parameters.isEmpty())
                    foreach (ParameterProperties *parameter, function->parameters)
                        tmpList.append(parameter->name);
                
                /* Show the methods and parameters of a class item's function */
                tmpList.sort(Qt::CaseInsensitive);
                m_pModel->setStringList(tmpList);
                toComplete = command[2];
            }
            else
                /* At this point no member or function found, so do nothing further */
                return;
        }
    }
    else if (command.count() == 4) /* classname.member.function.prefix */
    {
        bool classFound = false;
        bool functionFound = false;
        bool parameterFound = false;
        
        /* Look for the class first : must have the format : classname.member or classname.function */ 
        ClassItem *item;
        foreach (item, *items)
           if (item->className() == command[0])
           {
               classFound = true;
               break;
           }
        
        /* If there is no valid class name in command then return */
        if (!classFound)
           return;
        
        /* See now if the second part of the command is a function */
        FunctionProperties *function;
        foreach (function, item->getListFunctionProperties())
        {
            /* Remove the visibility : first char visibility, second is space */
            QString funcFullName(function->string(false));
            funcFullName = funcFullName.remove(0, 2);
            if (funcFullName == command[1])
            {
                functionFound = true;
                break;
            }
        }
        
        /* If there is no valid class name in command then return */
        if (!functionFound)
           return;
        
        /* And the third one must be a parameter */
        ParameterProperties *parameter;
        foreach (parameter, function->parameters)
           if (parameter->name == command[2])
           {
               parameterFound = true;
               break;
           }
               
        /* If true, then show the popup for this parameter */
        if (parameterFound)
        {
           QStringList tmpList(m_listParameterMethods);
           /* Show the methods of a function's parameter */
           tmpList.sort(Qt::CaseInsensitive);
           m_pModel->setStringList(tmpList);
           toComplete = command[3];
        }
        else
            /* At this point no parameter found, so do nothing further */
            return;
    }
    
    /* Set the prefix only if it changed */
    if (toComplete != m_pCompleter->completionPrefix())
    {
        /* Set for completion */
        m_pCompleter->setCompletionPrefix(toComplete);
        /* Select the first item in the list */
        m_pCompleter->popup()->setCurrentIndex(m_pCompleter->completionModel()->index(0, 0));
    }

    if (m_pCompleter->completionCount() == 1)
        /* If there is only one item, so paste it */
        insertCompletion(m_pCompleter->currentCompletion());
    else
        /* Open popup window */
        popupCompleter();
    
    /* Finish here */
    return;
}

/*!
 * \brief This function resizes the height of the dockwindow in the main window.
 * INFO : dock->resize() is not working. One way to resize a dock window is to reimplement the sizeHint()
 * function of the widget in the dockwindows. */
QSize CodeWindow::sizeHint() const
{
    const QSize size = this->size();
    return QSize(size.width(), 100);
}

/*!
 * \brief This function shows the tooltip for methods, if the given parameter is a valid function name.
 * The cursor will be moved to left, if the method name is selected from the popup window, in order to 
 * give the user the ability write arguments. Otherwise the user opens a paranthesis by himself, in which case
 * the cursor may not be moved. */
void CodeWindow::showTooltip(const QString &info, bool bMoveCursor)
{
    foreach (QString method, m_listTooltipDetails)
    {
        QString methodInfo = method;
        methodInfo = methodInfo.remove(methodInfo.mid(methodInfo.indexOf("(")));
        
        if (methodInfo == info)
        {
            if (bMoveCursor)
                /* Before opening the tooltip, move cursor to between paranths */
                moveCursor(QTextCursor::Left);

            /* Now tooltip can be opened */
            QPoint pt(cursorRect().topLeft());
            pt.setX(pt.x());
            QToolTip::showText(viewport()->mapToGlobal(pt), method, this, QRect(), 15000);

            m_bPressedParenLeft = true;
            return;
        }
    }
}
