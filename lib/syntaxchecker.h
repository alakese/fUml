#ifndef SYNTAXCHECKER_H
#define SYNTAXCHECKER_H

#include "syntaxchecker_global.h"

class SYNTAXCHECKERSHARED_EXPORT SyntaxChecker
{
public:
    bool isClassNameAllowed(const QString &);
    bool isDigit(const QString &);
    bool isParameterNameAllowed(const QString &);
    bool parameterDotFree(const QString &);
};

#endif // SYNTAXCHECKER_H
