#include "classitemdialog.h"
#include "../pages/generalpage.h"
#include "../pages/memberspage.h"
#include "../pages/functionspage.h"
#include "../pages/displayspage.h"
#include "../pages/propertiespage.h"
#include "../pages/fontpage.h"
#include "../../globals/globals.h"
#include "../../globals/properties.h"
#include <QListWidget>
#include <QStackedWidget>
#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>


ClassItemDialog::ClassItemDialog(GeneralProperties *genProp, const QList<MemberProperties *> &memProp, const QList<FunctionProperties *> &funcProp, 
                                 int currentPage, const QFont &initialFont, qint8 displayOptions)
{   
    /* Create the left side */
    m_pContentsList = new QListWidget;
    m_pContentsList->setViewMode(QListView::IconMode);
    m_pContentsList->setIconSize(QSize(96, 84));
    m_pContentsList->setMovement(QListView::Static);
    m_pContentsList->setMaximumWidth(128);
    m_pContentsList->setSpacing(12);
 
    /* Create the right side */
    m_pPagesWidget = new QStackedWidget;
    m_pPagesWidget->addWidget(new GeneralPage(genProp, this));
    m_pPagesWidget->addWidget(new MembersPage(memProp, this));
    m_pPagesWidget->addWidget(new FunctionsPage(funcProp, this));
    m_pPagesWidget->addWidget(new DisplaysPage(displayOptions, this));
    m_pPagesWidget->addWidget(new FontPage(initialFont, this));
    /* Create the buttons and connections */
    m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    /* Button will be disabled at first, and chage its state if any property will be edited */
    m_pButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    
    connect(m_pButtonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(m_pButtonBox, SIGNAL(rejected()), this, SLOT(reject()));
      
    /* Create the contents on the left side with icons */
    createContents();
    
    /* Open the selected tab; e.g general page, members or functions */
    m_pContentsList->setCurrentRow(currentPage);
    
    /* Set layouts */
    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(m_pContentsList);
    horizontalLayout->addWidget(m_pPagesWidget, 1);
    
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(m_pButtonBox);
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);
    
    setWindowTitle(tr("Properties"));
    
    resize(600, 600);
}

/*!
 * \brief This function creates the items in the list widget. */
void ClassItemDialog::createContents()
{
    /* Page : General */
    QListWidgetItem *general = new QListWidgetItem(m_pContentsList);
    general->setIcon(QIcon(":/images/config.png"));
    general->setText(tr("General"));
    general->setTextAlignment(Qt::AlignHCenter);
    general->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    /* Page : Members */
    QListWidgetItem *members = new QListWidgetItem(m_pContentsList);
    members->setIcon(QIcon(":/images/config.png"));
    members->setText(tr("Members"));
    members->setTextAlignment(Qt::AlignHCenter);
    members->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    /* Page : Functions */
    QListWidgetItem *functions = new QListWidgetItem(m_pContentsList);
    functions->setIcon(QIcon(":/images/config.png"));
    functions->setText(tr("Functions"));
    functions->setTextAlignment(Qt::AlignHCenter);
    functions->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
  
    /* Page : Display options */
    QListWidgetItem *disps = new QListWidgetItem(m_pContentsList);
    disps->setIcon(QIcon(":/images/config.png"));
    disps->setText(tr("Display"));
    disps->setTextAlignment(Qt::AlignHCenter);
    disps->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    /* Page : Font */
    QListWidgetItem *font = new QListWidgetItem(m_pContentsList);
    font->setIcon(QIcon(":/images/config.png"));
    font->setText(tr("Font"));
    font->setTextAlignment(Qt::AlignHCenter);
    font->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    
    /* Create the connection for the left side */
    connect(m_pContentsList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), 
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}

void ClassItemDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{    
    if (!current)
        current = previous;

    m_pPagesWidget->setCurrentIndex(m_pContentsList->row(current));
}

void ClassItemDialog::checkOKState()
{
    GeneralPage *genPage = (GeneralPage*)(m_pPagesWidget->widget(GENERALPAGE));
    MembersPage *memPage = (MembersPage*)(m_pPagesWidget->widget(MEMBERSPAGE));
    FunctionsPage *funcPage = (FunctionsPage*)(m_pPagesWidget->widget(FUNCTIONSPAGE));
    DisplaysPage *dispPage = (DisplaysPage*)(m_pPagesWidget->widget(DISPLAYPAGE));
    FontPage *fontPage = (FontPage*)(m_pPagesWidget->widget(FONTPAGE));
    
    if (genPage->getClassName().isEmpty())
        m_pButtonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
    else    
        m_pButtonBox->button(QDialogButtonBox::Ok)->setEnabled(genPage->checkTheListNow() || memPage->checkTheListNow() ||
                                                               funcPage->checkTheListNow() || fontPage->checkTheFontState() ||
                                                               dispPage->checkTheStates());
}

GeneralProperties *ClassItemDialog::getGeneralInformation()
{
    GeneralPage *genPage = (GeneralPage*)(m_pPagesWidget->widget(GENERALPAGE));
    return genPage->getInformation();
}

QList<MemberProperties*> ClassItemDialog::getMemberInformation() const
{
    MembersPage *memPage = (MembersPage*)(m_pPagesWidget->widget(MEMBERSPAGE));
    return memPage->getInformation();
}

QList<FunctionProperties *> ClassItemDialog::getFunctionInformation() const
{
    FunctionsPage *funcPage = (FunctionsPage*)(m_pPagesWidget->widget(FUNCTIONSPAGE));
    return funcPage->getInformation();
}

QList<MemberProperties*> ClassItemDialog::getDeleteMemberInformation() const
{
    MembersPage *memPage = (MembersPage*)(m_pPagesWidget->widget(MEMBERSPAGE));
    return memPage->getDeleteInformation();
}

QList<FunctionProperties*> ClassItemDialog::getDeleteFunctionInformation() const
{
    FunctionsPage *funcPage = (FunctionsPage*)(m_pPagesWidget->widget(FUNCTIONSPAGE));
    return funcPage->getDeleteInformation();
}

QList<ParameterProperties*> ClassItemDialog::getDeleteParameterInformation() const
{
    FunctionsPage *funcPage = (FunctionsPage*)(m_pPagesWidget->widget(FUNCTIONSPAGE));
    return funcPage->getParametersDeleteInformation();
}

QFont ClassItemDialog::getNewFont() const
{
    FontPage *fontPage = (FontPage*)(m_pPagesWidget->widget(FONTPAGE));
    return fontPage->getFont();
}

qint8 ClassItemDialog::getDisplayOptions() const
{
    DisplaysPage *dispPage = (DisplaysPage*)(m_pPagesWidget->widget(DISPLAYPAGE));
    return dispPage->getCurrentStates();
}
