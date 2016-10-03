#include "fontpage.h"
#include "../dialogs/fontdialog.h"
#include "../../gui/dialogs/classitemdialog.h"
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QVBoxLayout>
#include <QDebug>

FontPage::FontPage(const QFont &initialFont, ClassItemDialog *parent)
{
    mdiArea = new QMdiArea(this);
    
    QMdiSubWindow *w = mdiArea->addSubWindow(new FontDialog(this, initialFont));
    w->setWindowFlags(Qt::FramelessWindowHint);
    w->resize(100, 100);
    w->showMaximized();
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(mdiArea);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
    
    connect(this, &FontPage::checkOKState, parent, &ClassItemDialog::checkOKState);
}

/*!
 * \brief This function will be called whenever the user changes a font property. */
void FontPage::currentFontChanged(const QFont &oldFont, const QFont &newFont)
{
    this->oldFont = oldFont;
    this->newFont = newFont;
    
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

bool FontPage::checkTheFontState()
{
    return oldFont != newFont;
}

QFont FontPage::getFont() const
{
    return newFont;
}
