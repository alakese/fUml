#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QString>
#include <QFont>
#include <QRect>
#include <QPoint>

#include <QDebug>


enum Visibility
{
    PUBLIC = 0, PRIVATE, PROTECTED
};

enum Direction
{
    IN = 0, OUT, INOUT
};

class GeneralProperties
{
public:
    GeneralProperties()
    { }
    GeneralProperties(int _id, const QString _name, const QString _stereotype, const QString _nameSpace, bool _isAbstract, Visibility _visibility) :
        id(_id), name(_name), stereotype(_stereotype), nameSpace(_nameSpace), isAbstract(_isAbstract), visibility(_visibility)
    { }
    
    int id; // TODO : use here uuid() for renamecommand for example
    QString name;
    QString stereotype;
    QString nameSpace;
    bool isAbstract;
    Visibility visibility;
};

class MemberProperties
{
public:
    int id; // id necessary to decide by saving : adding or updating
    QString type;
    QString name;
    QString initValue;
    QString stereotype;
    bool isStatic;
    Visibility visibility;
    QString description;
    /* C'tor */
    MemberProperties()
    { }

    MemberProperties(int _id, const QString &_type, const QString & _name, const QString &_initValue, const QString &_stereotype, 
                     bool _isStatic, Visibility _visibility, const QString &_description) :
                     id(_id), type(_type), name(_name), initValue(_initValue),
                     stereotype(_stereotype), isStatic(_isStatic), visibility(_visibility), description(_description)
    { }
    /* Copy c'tor */
    MemberProperties(MemberProperties *member) : id(member->id), type(member->type), name(member->name), initValue(member->initValue),
        stereotype(member->stereotype), isStatic(member->isStatic), visibility(member->visibility), description(member->description)
    { }
    
    QString string()
    {
        QString visiChar("+");
        if (visibility == PUBLIC)
            visiChar = "+";
        else if (visibility == PRIVATE)
            visiChar = "-";
        else
            visiChar = "#";
            
        if (!initValue.isEmpty())
            return QString("%1 %2:%3=%4").arg(visiChar).arg(name).arg(type).arg(initValue);
        else
            return QString("%1 %2:%3").arg(visiChar).arg(name).arg(type);
    }
};

class ParameterProperties
{
public:
    int id;
    QString type;
    QString name;
    QString initValue;
    QString stereotype;
    Direction direction;
    /* C'tor */
    ParameterProperties()
    { }
    ParameterProperties(int _id, const QString &_type, const QString & _name, const QString &_initValue, 
                        const QString &_stereotype, Direction _direction) :
                        id(_id), type(_type), name(_name), initValue(_initValue), stereotype(_stereotype), direction(_direction)
    { }
    /* Copy c'tor */
    ParameterProperties(ParameterProperties *parameter) : id(parameter->id), type(parameter->type), name(parameter->name), 
                        initValue(parameter->initValue), stereotype(parameter->stereotype), direction(parameter->direction)
    { }
    
    QString string()
    {
        QString dir;
        if (direction == OUT)
            dir = "out";
        else if (direction == INOUT)
            dir = "inout";

        if (direction == IN)
        {
            if (initValue.isEmpty())
                return QString("%1:%2").arg(name).arg(type);
            else
                return QString("%1:%2=%3").arg(name).arg(type).arg(initValue);
        }
        else
        {
            if (initValue.isEmpty())
                return QString("%1 %2:%3").arg(dir).arg(name).arg(type);
            else
                return QString("%1 %2:%3=%4").arg(dir).arg(name).arg(type).arg(initValue);
        }
    }
};

class FunctionProperties
{
public:
    int id;
    QString returnType;
    QString name;
    QString stereotype;
    bool isStatic;
    bool isVirtual;
    Visibility visibility;
    QList<ParameterProperties *> parameters;
    QString description;
    QString code;
    
    /* C'tor */
    FunctionProperties()
    { }
    FunctionProperties(int _id, const QString &_retType, const QString &_name, const QString &_stereotype, bool _isStatic, bool _isVirtual,
                     Visibility _visibility, const QList<ParameterProperties *> &_parameters, const QString &_description, 
                     const QString &_code) : 
                     id(_id), returnType(_retType), name(_name), stereotype(_stereotype), isStatic(_isStatic), isVirtual(_isVirtual),
                     visibility(_visibility), description(_description), code(_code)
    {
        /* The parameters must be saved as new elements */
        foreach (ParameterProperties *parameter, _parameters)
        {
            ParameterProperties *par = new ParameterProperties(parameter);
            parameters.append(par);
        }
    }
    
    /* Copy c'tor */
    FunctionProperties(FunctionProperties *func) : id(func->id), returnType(func->returnType), name(func->name), stereotype(func->stereotype), 
        isStatic(func->isStatic), isVirtual(func->isVirtual), visibility(func->visibility),
        description(func->description), code(func->code)
    {
        /* The parameters must be saved as new elements */
        foreach (ParameterProperties *parameter, func->parameters)
        {
            ParameterProperties *par = new ParameterProperties(parameter);
            parameters.append(par);
        }
    }
    
    bool addParameter(const QString &type, const QString &name, const QList<FunctionProperties *> &functions)
    {
        /* Check if there is same parameter */
        foreach (ParameterProperties *tmp, parameters)
            if (tmp->name == name)
                return false;

        ParameterProperties *par = new ParameterProperties;
        par->type = type;
        par->name = name;
        par->direction = IN; // default value
        
        /* Check if there is same function */
        QList<ParameterProperties *> paramList(parameters);
        paramList.append(par);

        foreach (FunctionProperties *function, functions)
        {
            if (function->name == this->name && function->returnType == this->returnType && 
                    function->parameters.count() == paramList.count())
            {
                int matchCount = 0;
                for (int i = 0; i < function->parameters.count(); ++i)
                    if (function->parameters[i]->type == paramList[i]->type)
                        matchCount++;
                
                /* Then found */
                if (matchCount == paramList.count())
                    return false;
            }
        }

        /* Ok, can add */
        parameters.append(par);
        
        return true;
    }

    /*!
     * \brief This function removes the parameter, if the function without this parameter not exits.
     * False means user cant remove the parameter, because there is a function with that info. */
    bool removeParameter(ParameterProperties *parameter, const QList<FunctionProperties *> &functions)
    {
        int index = -1;
        for (int i = 0; i < parameters.count(); ++i)
            if (parameters[i]->type == parameter->type)
            {
                index = i;
                break;
            }
        
        if (index == -1) // This shouldnt happen
            return false;
        
        /* Store the list in a temp folder */
        QList<ParameterProperties *> paramList(parameters);
        paramList.removeAt(index);
        
        foreach (FunctionProperties *function, functions)
        {
            if (function->name == name && function->returnType == returnType && function->parameters.count() == paramList.count())
            {
                int matchCount = 0;
                for (int i = 0; i < function->parameters.count(); ++i)
                    if (function->parameters[i]->type == paramList[i]->type)
                        matchCount++;
                
                /* Then found */
                if (matchCount == function->parameters.count())
                    return false;
            }
        }
        
        /* Ok, we can remove it */
        parameters.removeAt(index);

        return true;
    }
    
    QString string()
    {  
        QString visiChar("+");
        if (visibility == PUBLIC)
            visiChar = "+";
        else if (visibility == PRIVATE)
            visiChar = "-";
        else
            visiChar = "#";

        QString funcName = QString("%1 %2(").arg(visiChar).arg(name);
        
        if (!parameters.isEmpty())
        {
            foreach (ParameterProperties *parameter, parameters)
            {
                funcName.append(parameter->string());
                funcName.append(",");
            }
            /* Remove the last comma and space after the last param */
            funcName = funcName.remove(funcName.count()-1, 1);
        }
        funcName.append("):%1");
        funcName = funcName.arg(returnType);
        
        return funcName;
    }
};

class GUIProperties
{
public:
    GUIProperties()
    { }
    GUIProperties(const QPoint _positionInScene, const QRect _boundaryRect, int header, int member, int function, const QFont _font) :
        positionInScene(_positionInScene), boundaryRect(_boundaryRect), font(_font)
    {
        headerHeight = header;
        membersHeight = member;
        functionsHeight = function;
    }
    
    /* Position in scene coordinates */
    QPoint positionInScene;
    /* Position and size of the item in item coordinate */
    QRect boundaryRect;
    int headerHeight;
    int membersHeight;
    int functionsHeight;
    QFont font;
};

#endif // PROPERTIES_H
