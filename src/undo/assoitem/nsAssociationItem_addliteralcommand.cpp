#include "nsAssociationItem_addliteralcommand.h"
#include "../../graphics/items/literalitem.h"
#include <QGraphicsScene>


/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{
    nsAssociationItem::AddLiteralCommand::AddLiteralCommand(LiteralItem *item, const QString &text)
        : m_pItem(item), m_text(text)
    {
        
    }
    
    void nsAssociationItem::AddLiteralCommand::undo()
    {
        m_pItem->setText(""); // Remove text
        m_pItem->parentItem()->scene()->update();
        /* Set the text for undo-view */
        setText(QObject::tr("Literal removed"));
    }
    
    void nsAssociationItem::AddLiteralCommand::redo()
    {
        m_pItem->setText(m_text);
        m_pItem->parentItem()->scene()->update();
        /* Set the text for undo-view */
        setText(QObject::tr("Literal added"));
    }
}
