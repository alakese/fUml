#ifndef NSCLASSITEM_CHANGEFONTCOMMAND_H
#define NSCLASSITEM_CHANGEFONTCOMMAND_H

#include <QUndoCommand>
#include <QFont>

class ClassItem;



/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{

/*!
 * \brief The ChangeFontCommand class
 */
class ChangeFontCommand : public QUndoCommand
{
    enum { ID = 3 };
    ClassItem *m_pClassItem;
    QFont m_oldFont;
    QFont m_newFont;
    
public:
    ChangeFontCommand(ClassItem *, QUndoCommand *parent = 0);
    ~ChangeFontCommand();
    int id() const;
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *);
    QFont font() const;
    ClassItem *classItem() const;
};

} // Namespace

#endif // NSCLASSITEM_CHANGEFONTCOMMAND_H
