#ifndef NSCLASSITEM_RESIZECOMMAND_H
#define NSCLASSITEM_RESIZECOMMAND_H

#include <QUndoCommand>

class ClassItem;



/*!
 * \namespace nClassItem
 */
namespace nsClassItem
{

/*!
 * \brief The ResizeCommand class
 */
class ResizeCommand : public QUndoCommand
{
    enum { ID = 2 };
    ClassItem *m_pClassItem;
    int m_newWidth;
    int m_newHeightHeader;
    int m_newHeightFunctions;
    int m_newHeightMembers;
    int m_oldWidth;
    int m_oldHeightHeader;
    int m_oldHeightFunctions;
    int m_oldHeightMembers;
    
public:
    ResizeCommand(ClassItem *, QUndoCommand *parent = 0);
    ~ResizeCommand();
    int id() const;
    void undo();
    void redo();
    bool mergeWith(const QUndoCommand *);
    ClassItem *classItem() const;    
    int newWidth() const { return m_newWidth; }
    int newHeightHeader() const { return m_newHeightHeader; }
    int newHeightMembers() const { return m_newHeightMembers; }
    int newHeightFunctions() const { return m_newHeightFunctions; }
};

} // Namespace

#endif // NSCLASSITEM_RESIZECOMMAND_H
