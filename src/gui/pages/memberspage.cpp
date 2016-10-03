#include "memberspage.h"
#include "../dialogs/addeditmemberdialog.h"
#include "../../gui/dialogs/classitemdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QPushbutton>
#include <QGroupBox>
#include <QListWidget>
#include <QIcon>
#include <QPlainTextEdit>
#include <QBitmap>
#include <QListWidgetItem>


MembersPage::MembersPage(const QList<MemberProperties *> &memberProperties, ClassItemDialog *parent)
{
    configGroup = new QGroupBox(tr("Members settings"));
    
    /* Create the items */
    membersList = new QListWidget;
    /* Only one item can be selected */
    membersList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    /* Add the items to the list */
    foreach(MemberProperties *member, memberProperties)
    {
        /* Add the name to list widget */
        membersList->addItem(member->string(false));
        /* Add the info to qlist */
        members.append(member);
        /* Create a copy of this item and store it for compare reference */
        MemberProperties *memberRef = new MemberProperties(member);
        membersAtBeginning.append(memberRef);
    }
    
    /* To make the image on the button transparent use mask */    
    QPixmap pixmapUp(":/images/up.jpg");
    QBitmap bitmapUp(":/images/up.jpg");
    pixmapUp.setMask(bitmapUp);
    QIcon iconUp(pixmapUp);
    buttonUp = new QPushButton;
    buttonUp->setIcon(iconUp);
    buttonUp->setIconSize(QSize(20, 20));
    
    /* Second mask for down */
    QPixmap pixmapDo(":/images/down.jpg");
    QBitmap bitmapDo(":/images/down.jpg");
    pixmapDo.setMask(bitmapDo);
    QIcon iconDo(pixmapDo);    
    buttonDown = new QPushButton;
    buttonDown->setIcon(iconDo);
    buttonDown->setIconSize(QSize(20, 20));
    
    buttonAdd = new QPushButton(tr("Add"));
    buttonDel = new QPushButton(tr("Delete"));
    buttonEdit = new QPushButton(tr("Edit"));

    tabWidget = new QTabWidget;
    description = new QPlainTextEdit;
    tabWidget->addTab(description, tr("Description"));

    /* Set the layouts */
    QVBoxLayout *arrowsLayout = new QVBoxLayout;
    arrowsLayout->addWidget(buttonUp);
    arrowsLayout->addWidget(buttonDown);
    arrowsLayout->addStretch(1);
    
    QHBoxLayout *listLayout = new QHBoxLayout;
    listLayout->addWidget(membersList);
    listLayout->addLayout(arrowsLayout);
    
    QHBoxLayout *buttsLayout = new QHBoxLayout;
    buttsLayout->addWidget(buttonAdd);
    buttsLayout->addWidget(buttonDel);
    buttsLayout->addWidget(buttonEdit);
    
    QVBoxLayout *allLayout = new QVBoxLayout;
    allLayout->addLayout(listLayout);
    allLayout->addSpacing(15);
    allLayout->addLayout(buttsLayout);
    allLayout->addSpacing(15);
    allLayout->addWidget(tabWidget);
    allLayout->addStretch(1);

    configGroup->setLayout(allLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(configGroup);
    mainLayout->addStretch(1);
    
    setLayout(mainLayout);

    /* Create the connections */
    connect(membersList, &QListWidget::itemPressed, this, &MembersPage::updateUI);
    connect(membersList, &QListWidget::itemDoubleClicked, this, &MembersPage::editMember);
    connect(buttonAdd, &QPushButton::pressed, this, &MembersPage::addMember);
    connect(buttonDel, &QPushButton::pressed, this, &MembersPage::deleteMember);
    connect(buttonEdit, &QPushButton::pressed, this, &MembersPage::editMember);
    connect(buttonUp, &QPushButton::pressed, this, &MembersPage::upPressed);
    connect(buttonDown, &QPushButton::pressed, this, &MembersPage::downPressed);
    connect(description, &QPlainTextEdit::textChanged, this, &MembersPage::updateDescription);
    connect(this, &MembersPage::checkOKState, parent, &ClassItemDialog::checkOKState);
    
    clearingDescription = false;
    
    /* Disable the buttons */
    updateUI();
}

MembersPage::~MembersPage()
{
    /* TODO clear here the lists */
}

/*!
 * \brief This function enables or disables some buttons. */
void MembersPage::updateUI()
{
    bool itemSelected = membersList->selectedItems().count() > 0;
    /* Delete and edit buttons are enabled, if any item is selected */
    buttonDel->setEnabled(itemSelected);
    buttonEdit->setEnabled(itemSelected);
    /* Put the description into dialog */
    description->setEnabled(itemSelected);
    if (itemSelected)
    {
        description->setPlainText(members[membersList->currentRow()]->description);
    }
    else
    {
        /* If nothing selected, then dont show anything */
        clearingDescription = true;
        description->clear();
    }
    /* Up enabled if not the first item is selected */
    buttonUp->setEnabled(itemSelected && membersList->currentRow() != 0);
    /* Down enabled if not the last item is selected */
    buttonDown->setEnabled(itemSelected && membersList->currentRow() != membersList->count() - 1);
}

/*!
 * \brief This function adds an element into the widget list. */
void MembersPage::addMember()
{
    /* Call the dialog for adding a new item : false */
    AddEditMemberDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) 
    {
        /* Get the info from the dialog */
        MemberProperties *info = dlg.getInformation();
        /* Add the name to list widget */
        membersList->addItem(info->string(false));
        /* Add the info to qlist */
        members.append(info);
        /* Update buttons */
        updateUI();
    }
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

void MembersPage::deleteMember()
{
    /* Remove the text in description */
    description->clear();
    /* 0 : can delete only one item */
    int index = membersList->currentRow();
    QListWidgetItem *item = membersList->takeItem(index);
    delete item;
    item = NULL;
    /* Get the element before removing it */
    MemberProperties *member = members[index];
    /* Remove the element from the list */
    members.removeAt(index);
    /* Update buttons */
    updateUI();
    /* If -1, means not in db yet */
    if (member->id == -1)
    {
        /* Just remove the member from the memory */
        delete member;
        member = NULL;
    }
    else
    {
        /* This member is in db, add to "todelete-list" to remove it from the db */
        membersToDelete.append(member);
    }
    
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

void MembersPage::editMember()
{
    /* 0 : can edit only one item */
    int index = membersList->currentRow();
    
    MemberProperties *editInfo = members.at(index);
    /* Call the dialog */
    AddEditMemberDialog dlg(this, editInfo);
    if (dlg.exec() == QDialog::Accepted) 
    {
        /* Save the id of the member, because the dialog will return for id -1 back, because there is no widget fot the id */
        int currID = editInfo->id;
        /* First remove old entry from members to avoid memory leak */
        delete editInfo;
        editInfo = NULL;
        /* Get the info from the dialog */
        MemberProperties *info = dlg.getInformation();
        /* Store the id back */
        info->id = currID;
        /* Edit the info to qlist */
        members[index] = info;
        /* Change the name in the list window */
        QListWidgetItem *oldItem = membersList->takeItem(index);
        //QListWidgetItem *item = new QListWidgetItem(QString("%1 : %2").arg(info->name).arg(info->type));
        QListWidgetItem *item = new QListWidgetItem(info->string(false));
        membersList->insertItem(index, item);
        /* Remove the old item from the memory */
        delete oldItem;
        oldItem = NULL;
        /* Tell parent to check for the ok-button - smtg maybe changed */
        emit checkOKState(); 
    }
    /* Update buttons */
    updateUI();
}

void MembersPage::upPressed()
{
    int index = membersList->currentRow();
    
    /* Item up in the list widget */
    QListWidgetItem *item = membersList->takeItem(index);
    membersList->insertItem(index-1, item);
    membersList->setCurrentRow(index-1);
    
    /* Item up in the qlist */
    MemberProperties *info = members.at(index-1);
    members[index-1] = members[index];
    members[index] = info;
    
    /* Update buttons */
    updateUI();
}

void MembersPage::downPressed()
{
    int index = membersList->currentRow();
    
    /* Item down in the list widget */
    QListWidgetItem *item = membersList->takeItem(index);
    membersList->insertItem(index+1, item);
    membersList->setCurrentRow(index+1);
    
    /* Item up in the qlist */
    MemberProperties *info = members.at(index+1);
    members[index+1] = members[index];
    members[index] = info;
    
    /* Update buttons */
    updateUI();
}

QList<MemberProperties*> MembersPage::getInformation() const
{
    return members;
}

QList<MemberProperties*> MembersPage::getDeleteInformation() const
{
    return membersToDelete;
}

/*!
 * \brief This function will be called, when an item will be inserted into or deleted from the widget list. */
bool MembersPage::checkTheListNow()
{
    /* If the counts not equal, then enable the ok-button */
    if (membersAtBeginning.count() != members.count())
        return true;
        
    /* If both has no items */
    if (membersAtBeginning.count() == 0 && members.count() == 0)
        return false;
        
    /* Else compare the elements */
    for (int index = 0; index < members.count(); ++index)
    {
        bool state = membersAtBeginning[index]->name != members[index]->name || membersAtBeginning[index]->type != members[index]->type ||
                membersAtBeginning[index]->stereotype != members[index]->stereotype || membersAtBeginning[index]->isStatic != members[index]->isStatic ||
                membersAtBeginning[index]->initValue != members[index]->initValue || membersAtBeginning[index]->visibility != members[index]->visibility ||
                membersAtBeginning[index]->description != members[index]->description;
        if (state)
            return true;
    }
    
    /* At this point all elements compared and they all equal */
    return false;
}

/*!
 * \brief This function checks, if the given member already exists for namecheck. */
bool MembersPage::memberExists(const QString &memberName)
{
    bool ret = false;
    foreach (MemberProperties *member, members)
        if (member->name == memberName)
            ret = true;

    return ret;
}

/*!
 * \brief This function updates the description of a member, whenever it will be updated. */
void MembersPage::updateDescription()
{
    /* If QPlainTextEdit will be cleared, then don't update the list.
       This clear will be sent, if no item is selected in the list. Otherwise
       if the description info will be cleared by the user, no check is necessary. */
    if (clearingDescription)
    {
        clearingDescription = false;
        return;
    }
    
    int index = membersList->currentRow();
    if (index > -1)
    {
        members[index]->description = description->toPlainText();
        /* Tell parent to check for the ok-button - smtg maybe changed */
        emit checkOKState();
    }
}
