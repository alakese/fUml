#include "fontdialog.h"
#include "../pages/fontpage.h"
#include <QFontDialog>
#include <QKeyEvent>
#include <QDebug>

FontDialog::FontDialog(FontPage *parentPage, const QFont &initialFont, QWidget *parent) : 
    QFontDialog(initialFont, parent)
{
   /* Save the font */
    oldFont = initialFont;

    /* For the ok-cancel buttons on the dialog */
    page = parentPage;
    
    /* Dont show the ok-cancel buttons because the main dialog has some */
    setOption(QFontDialog::NoButtons);
    
    /* If the font changes tell this to parent */
    connect (this, &FontDialog::signalFontChanged, page, &FontPage::currentFontChanged);
    connect (this, &FontDialog::currentFontChanged, this, &FontDialog::fontChanged);
}

/*!
 * \brief This function prevents font-dialog in properties window to be closed when the escape will be pressed. */
void FontDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape)
        return;

    QDialog::keyPressEvent(event);
}

void FontDialog::fontChanged(const QFont &newFont)
{
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit signalFontChanged(oldFont, newFont);
}
