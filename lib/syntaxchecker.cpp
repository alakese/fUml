#include "syntaxchecker.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>


bool SyntaxChecker::isClassNameAllowed(const QString &name)
{
    /* Has the new name allowed characters and syntax*/
    QRegularExpression re("^[a-zA-Z_][a-zA-Z0-9_]*$");
    QRegularExpressionMatch match = re.match(name);
    return match.hasMatch();
}

bool SyntaxChecker::isDigit(const QString &word)
{
    /* Is this digit */
    QRegularExpression re("^[0-9]+$");
    QRegularExpressionMatch match = re.match(word);
    return match.hasMatch();
}

bool SyntaxChecker::isParameterNameAllowed(const QString &parameter)
{
    /* Has the parameter/variable allowed characters and syntax*/
    QRegularExpression re("^[a-zA-Z_][a-zA-Z0-9_]*$");
    QRegularExpressionMatch match = re.match(parameter);
    return match.hasMatch();
}

/*!
 * \brief The command given from a user may have dots, if he is calling a method. But inside the method, there 
 * shall be no dots. For example : class.addFunction(bool. isDigit) -> user forgot to enter a comma. */
bool SyntaxChecker::parameterDotFree(const QString &line)
{
    /* Has the parameter/variable allowed characters and syntax*/
    QRegularExpression re(".+\\([a-zA-Z0-9_, ]*\\)");
    QRegularExpressionMatch match = re.match(line);
    return match.hasMatch();
}
