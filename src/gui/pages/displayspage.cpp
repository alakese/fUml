#include "displayspage.h"
#include "../../globals/globals.h"
#include "../../gui/dialogs/classitemdialog.h"
#include <QGroupBox>
#include <QCheckBox>
#include <QVBoxLayout>


DisplaysPage::DisplaysPage(qint8 states, ClassItemDialog *parent) 
{
    checksStatesInitial = states;
            
    showMembers = new QCheckBox(tr("Show members"));
    showMembers->setChecked(GET_BIT(checksStatesInitial, BIT1) == 1);
    
    showOnlyMemberNames = new QCheckBox(tr("Show only member names"));
    showOnlyMemberNames ->setChecked(GET_BIT(checksStatesInitial, BIT2) == 1);
    if (GET_BIT(checksStatesInitial, BIT1) == 0)
        showOnlyMemberNames->setEnabled(false);
    
    showMethods = new QCheckBox(tr("Show methods"));
    showMethods->setChecked(GET_BIT(checksStatesInitial, BIT3) == 1);
    
    showOnlyMethodNames = new QCheckBox(tr("Show only method names"));
    showOnlyMethodNames ->setChecked(GET_BIT(checksStatesInitial, BIT4) == 1);
    if (GET_BIT(checksStatesInitial, BIT3) == 0)
        showOnlyMethodNames->setEnabled(false);
    
    showOnlyPublic = new QCheckBox(tr("Show only public"));
    showOnlyPublic->setChecked(GET_BIT(checksStatesInitial, BIT5) == 1);
    
    groupBox = new QGroupBox(tr("Display options"));
    
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(showMembers);
    vbox->addWidget(showOnlyMemberNames);
    vbox->addWidget(showMethods);
    vbox->addWidget(showOnlyMethodNames);
    vbox->addWidget(showOnlyPublic);
    groupBox->setLayout(vbox);
    
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);
    mainLayout->addSpacing(1);
    
    setLayout(mainLayout);
    
    connect(showMembers, &QCheckBox::stateChanged, this, &DisplaysPage::stateShowMembersChanged);
    connect(showOnlyMemberNames, &QCheckBox::stateChanged, this, &DisplaysPage::anyStateChanged);
    connect(showMethods, &QCheckBox::stateChanged, this, &DisplaysPage::stateShowMethodsChanged);
    connect(showOnlyMethodNames, &QCheckBox::stateChanged, this, &DisplaysPage::anyStateChanged);
    connect(showOnlyPublic, &QCheckBox::stateChanged, this, &DisplaysPage::anyStateChanged);
    connect(this, &DisplaysPage::checkOKState, parent, &ClassItemDialog::checkOKState);
}

/*!
 * \brief This function will be called, if the user changes the state of the first check box : "show members" */
void DisplaysPage::stateShowMembersChanged(int state)
{
    showOnlyMemberNames->setEnabled(state == 0 ? false : true);
    
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

/*!
 * \brief This function will be called, if the user changes the state of the thirs check box : "show methods" */
void DisplaysPage::stateShowMethodsChanged(int state)
{
    showOnlyMethodNames->setEnabled(state == 0 ? false : true);
    
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

/*!
 * \brief This function will be called, if any other states will be changed */
void DisplaysPage::anyStateChanged(int state)
{
    Q_UNUSED(state);
    
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

/*!
 * \brief This function checks the states of checkboxes and returns true, if any of the checks has been changed. */
bool DisplaysPage::checkTheStates()
{
    return checksStatesInitial != getCurrentStates();
}

/*!
 * \brief This function checks the states and returns them in a 8-Bit variable. */
qint8 DisplaysPage::getCurrentStates()
{
    qint8 currentStates = 0;

    if (showMembers->isChecked())
    {
        SET_BIT(currentStates, BIT1);

        /* Show only member names may be checked only then if show members is checked */
        if (showOnlyMemberNames->isChecked())
            SET_BIT(currentStates, BIT2);
    }

    if (showMethods->isChecked())
    {
        SET_BIT(currentStates, BIT3);

        /* Show only Method names may be checked only then if show Methods is checked */
        if (showOnlyMethodNames->isChecked())
            SET_BIT(currentStates, BIT4);
    }
    
    if (showOnlyPublic->isChecked())
        SET_BIT(currentStates, BIT5);

    return currentStates;
}

