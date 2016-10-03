#include "nsAssociationItem_deleteliteralcommand.h"
#include "../../graphics/items/literalitem.h"
#include <QGraphicsScene>

/*!
* \namespace nsAssociationItem
*/
namespace nsAssociationItem
{
    nsAssociationItem::DeleteLiteralCommand::DeleteLiteralCommand(LiteralItem *item)
        : m_pItem(item)
    {
        m_text = item->text();
    }
    
    void nsAssociationItem::DeleteLiteralCommand::undo()
    {
        m_pItem->setText(m_text);
        m_pItem->parentItem()->scene()->update();
        /* Set the text for undo-view */
        setText(QObject::tr("Literal added back"));
    }
    
    void nsAssociationItem::DeleteLiteralCommand::redo()
    {
        m_pItem->setText(""); // Remove text
        m_pItem->parentItem()->scene()->update();
        /* Set the text for undo-view */
        setText(QObject::tr("Literal removed"));
    }
}
