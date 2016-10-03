#ifndef GENERATECODE_H
#define GENERATECODE_H

#include <QString>
#include "../globals/properties.h"

class ClassItem;

/*!
 * \class The GenerateCode class
 */
class GenerateCode
{
public:
    void generate(const QString &,  const QString &);

private:
    QString getMemberDefinitionsAsString(ClassItem *, Visibility);
    QString getFunctionDefinitionsAsString(ClassItem *, Visibility);
    QString getFunctionDeclarationsAsString(ClassItem *, Visibility);
};

#endif // GENERATECODE_H
