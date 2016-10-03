#include "generatecode.h"
#include "project.h"
#include "projectmanagement.h"
#include "../graphics/graphicsscene.h"
#include "../graphics/items/classitem.h"
#include "../gui/monitor/monitor.h"
#include <QDir>
#include <QFile>
#include <QTextStream>


/* Template file infos for a class item */
namespace TEMPLATE_FILE {
    const qint32 MagicNumber = 0x544D50; //for TMP
    const qreal VersionNumber = 1.0;
}

/*!
 * \brief This function generates the code for the active project. */
void GenerateCode::generate(const QString &fileName, const QString &projectPath)
{
    /* Open the template file to generate the source files */
    QFile templateFile(fileName);     // TODO : this file should be in installation path
    if (!templateFile.open(QFile::ReadOnly)) {
      MonitorManager::getInstance()->logMsg("ERROR : Could not open file %0 for reading", 1, "ClassTemplate.tmp");
      return;
    }
    
    QDataStream inTemplate(&templateFile);
    inTemplate.setVersion(QDataStream::Qt_5_3);
    inTemplate.setDevice(&templateFile);
    
    /* Check file extension */
    qint32 magicNumber;
    inTemplate >> magicNumber;
    
    if (TEMPLATE_FILE::MagicNumber != magicNumber)
    {
        MonitorManager::getInstance()->logMsg("ERROR : Wrong template file found. Please contact to your admin!", 0);
        return;
    }
    
    /* Check version number */
    qreal versionNumber;
    inTemplate >> versionNumber;
    if (TEMPLATE_FILE::VersionNumber != versionNumber)
    {
        MonitorManager::getInstance()->logMsg("ERROR : Wrong template file found. Please contact to your admin!", 0);
        return;
    }
    
    QList<QString> hFileTemplate;
    QList<QString> cFileTemplate;
    QString item;
    bool cPartBegin = false;
    
    /* Read template items in an array. PS : For now there are not much items, it can be read in an array */
    while (!inTemplate.atEnd())
    {
        /* Get the line */
        inTemplate >> item;
        
        if (item == "C_Begins_Here")
        {
            /* After this point the lines (read from the file) belong to c file */
            cPartBegin = true;
            /* Dont write "C_Begins_Here" into the file */
            continue;
        }
        
        if (cPartBegin)
            /* Get c file info */
            cFileTemplate.append(item);
        else
            /* Get h file info */
            hFileTemplate.append(item);
    }
    templateFile.close();

    /* Create the folder /SGen for source generation */
    QDir dir(projectPath + "\\SGen");
    if (!dir.exists())
    {
        if (!dir.mkpath("."))
        {
            MonitorManager::getInstance()->logMsg("ERROR : Can not create folder /SGen", 0);
            return;
        }
    }

    /* Get all classitems from the scene */
    QList<QGraphicsItem*> items = ProjectManager::getInstance()->getActiveProject()->scene()->items();
    QListIterator<QGraphicsItem*> i(items);
    
    /* Generate the files for each class item */
    while (i.hasNext())
    {
        if (ClassItem *item = dynamic_cast<ClassItem*>(i.next()))
        {
            /* For readability */
            QString className = item->className();
            
            /* Header definition word */
            QString classNameCapLet(item->className().toUpper() + "_H");
            
            /* Generate code directory and filename */
            QString genFilesDir = dir.absolutePath() + "\\";
            
            /* If there is a namespace, create the subfolders */
            if (!item->getGeneralProperties()->nameSpace.isEmpty())
            {
                QStringList dirList = item->getGeneralProperties()->nameSpace.split("::");
                foreach (QString folder, dirList)
                {
                    genFilesDir.append(folder);
                    genFilesDir.append("\\");
                    
                    /* Create the folder */
                    QDir dirInside(genFilesDir);
                    if (!dirInside.exists())
                    {
                        if (!dirInside.mkpath("."))
                        {
                            MonitorManager::getInstance()->logMsg("ERROR : Can not create folder %1", 1, 
                                                                  genFilesDir.toLocal8Bit().data());
                            return;
                        }
                    }
                }
            }
            genFilesDir.append(className);
            
            /* Create h file */
            QFile hFile(genFilesDir + ".h");
            if (!hFile.open(QFile::WriteOnly | QFile::Text))
            {
              MonitorManager::getInstance()->logMsg("ERROR : Can not create file %0", 1, hFile.fileName().toLocal8Bit().data());
              return;
            }
        
            /* Create c file */
            QFile cFile(genFilesDir + ".c");
            if (!cFile.open(QFile::WriteOnly | QFile::Text))
            {
              MonitorManager::getInstance()->logMsg("ERROR : Can not create file %0", 1, cFile.fileName().toLocal8Bit().data());
              return;
            }
            
            /* Create streams */
            QTextStream outHFile(&hFile);
            QTextStream outCFile(&cFile);
            
            /* Generate h file */
            MonitorManager::getInstance()->logMsg("INFO : Generating header file for %0", 1, className.toLocal8Bit().data());
            foreach(QString tempItem, hFileTemplate)
            {
                if (tempItem.contains("%CLASSNAME_H%"))
                {
                    tempItem.replace("%CLASSNAME_H%", classNameCapLet);
                } 
                else if (tempItem.contains("%NAMESPACE%"))
                {
                    /* Check if namespace is given */
                    if (!item->getGeneralProperties()->nameSpace.isEmpty())
                    {
                        tempItem.replace("%NAMESPACE%", item->getGeneralProperties()->nameSpace);
                        tempItem.append(" {");
                    }
                    else
                    {
                        /* If no namespace defined, then remove the line in the template file */
                        tempItem.clear();
                    }
                }
                else if (tempItem.contains("%ENDNAMESPACE%"))
                {
                    /* Check if namespace is given, then close it */
                    if (!item->getGeneralProperties()->nameSpace.isEmpty())
                    {
                        tempItem.replace("%ENDNAMESPACE%", "}");
                    }
                    else
                    {
                        /* If no namespace defined, then remove the line in the template file */
                        tempItem.clear();
                    }
                }
                else if (tempItem.contains("%CLASSNAME%"))
                {
                    tempItem.replace("%CLASSNAME%", className);
                }
                else if (tempItem.contains("%PRIVATE_MEMBERS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PRIVATE_MEMBERS%");
                    tempItem = getMemberDefinitionsAsString(item, PRIVATE);
                }
                else if (tempItem.contains("%PUBLIC_MEMBERS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PUBLIC_MEMBERS%");
                    tempItem = getMemberDefinitionsAsString(item, PUBLIC);
                }
                else if (tempItem.contains("%PROTECTED_MEMBERS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PROTECTED_MEMBERS%");
                    tempItem = getMemberDefinitionsAsString(item, PROTECTED);
                }
                else if (tempItem.contains("%PUBLIC_FUNCTIONS%"))
                {                    
                    /* First remove the keyword */
                    tempItem.remove("%PUBLIC_FUNCTIONS%");
                    tempItem = getFunctionDefinitionsAsString(item, PUBLIC);
                }
                else if (tempItem.contains("%PRIVATE_FUNCTIONS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PRIVATE_FUNCTIONS%");
                    tempItem = getFunctionDefinitionsAsString(item, PRIVATE);
                }
                else if (tempItem.contains("%PROTECTED_FUNCTIONS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PROTECTED_FUNCTIONS%");
                    tempItem = getFunctionDefinitionsAsString(item, PROTECTED);
                }

                /* Write the new item */
                if (!tempItem.isEmpty())
                    outHFile << tempItem << "\n";
            }

            /* Generate c file */
            MonitorManager::getInstance()->logMsg("INFO : Generating source file for %0", 1, item->className().toLocal8Bit().data());
            foreach(QString tempItem, cFileTemplate)
            {
                if (tempItem.contains("%ClassName%"))
                {
                    /* #include".." */
                    tempItem.replace("%ClassName%", className);
                    tempItem.append("\n\n"); // Add some \n after #include-part
                }
                else if (tempItem.contains("%PUBLIC_FUNCTIONS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PUBLIC_FUNCTIONS%");
                    tempItem = getFunctionDeclarationsAsString(item, PUBLIC);
                }
                else if (tempItem.contains("%PRIVATE_FUNCTIONS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PRIVATE_FUNCTIONS%");
                    tempItem = getFunctionDeclarationsAsString(item, PRIVATE);
                }
                else if (tempItem.contains("%PROTECTED_FUNCTIONS%"))
                {
                    /* First remove the keyword */
                    tempItem.remove("%PROTECTED_FUNCTIONS%");
                    tempItem = getFunctionDeclarationsAsString(item, PROTECTED);
                }
                
                /* Write the new item */
                outCFile << tempItem << "\n";
            }
            
            /* Close files */
            hFile.close();
            cFile.close();
        }
    }
}


/*!
 * \brief This function returns the members as a string in the form as in a class definition. */
QString GenerateCode::getMemberDefinitionsAsString(ClassItem *item, Visibility visibility)
{
    QString retVal = "";
    
    /* See if there are any members, if not dont write the visibility keyword */
    foreach (MemberProperties *member, item->getListMemberProperties())
        if (member->visibility == visibility)
        {
            retVal = QString("%1:\n").arg(visibility == PUBLIC ? "public" : (visibility == PRIVATE ? "private" : "protected"));
            break;
        }

    /* Write each member into file */
    foreach (MemberProperties *member, item->getListMemberProperties())
    {
        if (member->visibility == visibility)
        {
           
            QString line = QString("%1 %2;\n").arg(member->type).arg(member->name);
            
            if (member->isStatic)
                line.prepend("static ");
            
            line.prepend("\t");
            
            /* Look, if there is a description */
            if (!member->description.isEmpty())
            {
                QString desc;
                QString lineDesc;
                QStringList lineList = member->description.split("\n");
                foreach (QString linePart, lineList)
                {
                    desc = "\n";
                    desc.prepend(linePart);
                    desc.prepend("\t//");
                    lineDesc.append(desc);
                }
                line.prepend(lineDesc);
            }
                
            retVal.append(line);
        }
    }
    
    return retVal;
}

/*!
 * \brief This function returns the functions as a string in the form as in a class definition. */
QString GenerateCode::getFunctionDefinitionsAsString(ClassItem *item, Visibility visibility)
{
    QString retVal = "";
    
    /* See if there are any functions, if not dont write the visibility keyword */
    foreach (FunctionProperties *function, item->getListFunctionProperties())
        if (function->visibility == visibility)
        {
            retVal = QString("%1:\n").arg(visibility == PUBLIC ? "public" : (visibility == PRIVATE ? "private" : "protected"));
            break;
        }

    /* If there are no c'tors and there are members to initialize*/
    if (visibility == PUBLIC && !item->hasCtor())
        /* Write the default c'tor in public part */
        foreach (MemberProperties *member, item->getListMemberProperties())
            if (!member->initValue.isEmpty())
            {
                /* Write the function core */
                QString line = QString("\t%1();\n").arg(item->className());
                retVal.append(line);
                break;
            }
    
    /* Write each function into file */
    foreach (FunctionProperties *function, item->getListFunctionProperties())
    {
        if (function->visibility == visibility)
        {
            /* Write the function core */
            QString line = QString("%1 %2(").arg(function->returnType, function->name);
            /* See if there are any parameters */
            if (!function->parameters.isEmpty())
            {
                foreach(ParameterProperties *parameter, function->parameters)
                {
                    QString strPar;
                    if (!parameter->initValue.isEmpty())
                    {
                        if (parameter->direction == IN)
                            strPar = QString("%1 %2 = %3, ").arg(parameter->type).arg(parameter->name).arg(parameter->initValue);
                        else if (parameter->direction == OUT)
                            strPar = QString("/*out*/ %1 *%2 = %3, ").arg(parameter->type).arg(parameter->name).arg(parameter->initValue);
                        else if (parameter->direction == INOUT)
                            strPar = QString("/*inout*/ %1 *%2 = %3, ").arg(parameter->type).arg(parameter->name).arg(parameter->initValue);
                    }
                    else
                    {
                        if (parameter->direction == IN)
                            strPar = QString("%1 %2, ").arg(parameter->type).arg(parameter->name);
                        else if (parameter->direction == OUT)
                            strPar = QString("/*out*/ %1 *%2, ").arg(parameter->type).arg(parameter->name);
                        else if (parameter->direction == INOUT)
                            strPar = QString("/*inout*/ %1 *%2, ").arg(parameter->type).arg(parameter->name);
                    }
                    line.append(strPar);
                }
                /* Remove last comma and space */
                line.remove(line.size()-2, 2);
            }
            /* Add the closing */
            line.append(");\n");
            
            /* If static */
            if (function->isStatic)
                line.prepend("static ");

            if (function->isVirtual)
                line.prepend("virtual ");

            line.prepend("\t");

            retVal.append(line);
        }
    }
    
    return retVal;
}

/*!
 * \brief This function returns the functions as a string in the form as in a source code. */
QString GenerateCode::getFunctionDeclarationsAsString(ClassItem *item, Visibility visibility)
{
    QString retVal = "";
    
    /* If there are no c'tors and there are members to initialize*/
    if (visibility == PUBLIC && !item->hasCtor())
    {
        /* Are there any members to initialize */
        bool hasInits = false; 
        foreach (MemberProperties *member, item->getListMemberProperties())
            if (!member->initValue.isEmpty())
                hasInits = true;
        
        if (hasInits)
        {
            QString root = QString("%1").arg(item->className());
            if (!item->getGeneralProperties()->nameSpace.isEmpty())
            {
                QString nSpace = QString("%1::").arg(item->getGeneralProperties()->nameSpace);
                root.prepend(nSpace);
            }
            
            QString line = QString("%1::%2()\n{\n").arg(root).arg(item->className());
            retVal.append(line);
            
            /* Write the default c'tor in public part */
            foreach (MemberProperties *member, item->getListMemberProperties())
                if (!member->initValue.isEmpty())
                {
                    /* Write the function core */
                    QString lineInit = QString("\t%1 = %2;\n").arg(member->name).arg(member->initValue);
                    retVal.append(lineInit);
                }
            
            retVal.append("}\n\n");
        }
    }
    
    /* Write each function into file */
    foreach (FunctionProperties *function, item->getListFunctionProperties())
    {
        if (function->visibility == visibility)
        {
            QString root = QString("%1").arg(item->className());
            if (!item->getGeneralProperties()->nameSpace.isEmpty())
            {
                QString nSpace = QString("%1::").arg(item->getGeneralProperties()->nameSpace);
                root.prepend(nSpace);
            }
            
            QString line = QString("%1 %2::%3(").arg(function->returnType).arg(root).arg(function->name);
            /* See if there are any parameters */
            if (!function->parameters.isEmpty())
            {
                foreach(ParameterProperties *parameter, function->parameters)
                {
                    QString strPar;
                    if (parameter->direction == IN)
                        strPar = QString("%1 %2, ").arg(parameter->type).arg(parameter->name);
                    else if (parameter->direction == OUT)
                        strPar = QString("%1 *%2, ").arg(parameter->type).arg(parameter->name);
                    else if (parameter->direction == INOUT)
                        strPar = QString("%1 *%2, ").arg(parameter->type).arg(parameter->name);

                    line.append(strPar);
                }
                /* Remove last comma and space */
                line.remove(line.size()-2, 2);
            }
            
            line.append(")\n{\n");
            
            /* If c'tor exists, initialize the members */
            if (function->name == item->className())
            {
                QString defDesc = QString("/**\n");
                
                /* Add the default description */
                if (function->description.isEmpty())
                    defDesc.append(" * Constructor\n");
                else
                {
                    /* Or the existing one */
                    QString desc;
                    QString lineDesc;
                    QStringList lineList = function->description.split("\n");
                    foreach (QString linePart, lineList)
                    {
                        desc = "\n";
                        desc.prepend(linePart);
                        desc.prepend(" * ");
                        lineDesc.append(desc);
                    }
                    defDesc.append(lineDesc);
                }
                
                /* Add the parameters, if exists */
                if (!function->parameters.isEmpty())
                {
                    foreach (ParameterProperties *param, function->parameters)
                    {
                        defDesc.append(" * @param ");
                        defDesc.append(param->name);
                        defDesc.append("\n");
                    }
                }
                defDesc.append(" */\n");
                
                line.prepend(defDesc);
                
                /* Are there any members to initialize */
                foreach (MemberProperties *member, item->getListMemberProperties())
                {
                    if (!member->initValue.isEmpty())
                    {
                        QString initVal = QString("\t%1 = %2\n").arg(member->name).arg(member->initValue);
                        line.append(initVal);
                    }
                }
            }
            else
            {
                /* If this is not a constructor, then add the description, parameters and return value as comment */
                QString defDesc = QString("/**\n");
                
                /* Add the description */
                if (!function->description.isEmpty())
                {
                    QString desc;
                    QString lineDesc;
                    QStringList lineList = function->description.split("\n");
                    foreach (QString linePart, lineList)
                    {
                        desc = "\n";
                        desc.prepend(linePart);
                        desc.prepend(" * ");
                        lineDesc.append(desc);
                    }
                    defDesc.append(lineDesc);
                    /* Add a space between the description and information */
                    defDesc.append(" *\n");
                }

                /* Add the return value */
                defDesc.append(" * @return ");
                defDesc.append(function->returnType);
                defDesc.append("\n");
                
                /* Add the parameters, if exists */
                if (!function->parameters.isEmpty())
                {
                    foreach (ParameterProperties *param, function->parameters)
                    {
                        defDesc.append(" * @param ");
                        defDesc.append(param->name);
                        defDesc.append("\n");
                    }
                }
                defDesc.append(" */\n");
                
                line.prepend(defDesc);
            }
            
            
            /* If there is a code given by the user, write it */
            if (!function->code.isEmpty())
            {
                QString desc;
                QString lineDesc;
                QStringList lineList = function->code.split("\n");
                foreach (QString linePart, lineList)
                {
                    desc = "\n";
                    desc.prepend(linePart);
                    desc.prepend("\t");
                    lineDesc.append(desc);
                }
                line.append(lineDesc);
            }
            /* Or else, return some default values for some default types, if not a constructor */
            else if (function->returnType.contains("float"))
            {
                line.append("\t// TODO generated code\n\treturn 0.0f;\n");
            }
            else if (function->returnType.contains("double"))
            {
                line.append("\t// TODO generated code\n\treturn 0.0;\n");
            }
            else if (function->returnType.contains("short"))
            {
                line.append("\t// TODO generated code\n\treturn 0;\n");
            }
            else if (function->returnType.contains("int"))
            {
                line.append("\t// TODO generated code\n\treturn 0;\n");
            }
            else if (function->returnType.contains("char"))
            {
                line.append("\t// TODO generated code\n\treturn '\0';\n");
            }
            else if (function->returnType.contains("bool"))
            {
                line.append("\t// TODO generated code\n\treturn false;\n");
            }
            else
            {
                /* If a constructor */
                if (function->name == item->className())
                    line.append("\t// TODO generated code\n");
                else
                    line.append("\t// TODO generated code\n\t// TODO return a value\n");
            }

            line.append("}\n\n");

            retVal.append(line);
        }
    }
    /* Remove the last \n */
    retVal.remove(retVal.size()-1, 1);
    
    return retVal;
}
