#include "functionspage.h"
#include "../dialogs/addeditfunctiondialog.h"
#include "../../gui/dialogs/classitemdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushbutton>
#include <QGroupBox>
#include <QListWidget>
#include <QIcon>
#include <QPlainTextEdit>
#include <QBitmap>
#include <QListWidgetItem>

#include <QDebug>


FunctionsPage::FunctionsPage(const QList<FunctionProperties *> &functionProperties, ClassItemDialog *parent)
{
    configGroup = new QGroupBox(tr("Function settings"));
    
    /* Create the items */
    functionsList = new QListWidget;
    /* Only one item can be selected */
    functionsList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    /* Add the functions in to list */
    foreach (FunctionProperties *function, functionProperties)
    {
        /* Add the name to list widget */
        //functionsList->addItem(createFunctionName(function));
        functionsList->addItem(function->string(false));
        /* Add the info to qlist */
        functions.append(function);        
        /* Create a copy of this item and store it for compare reference */
        FunctionProperties *funcRef = new FunctionProperties(function);
        functionsAtBeginning.append(funcRef);
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
    code = new QPlainTextEdit;
    tabWidget->addTab(description, tr("Description"));
    tabWidget->addTab(code, tr("Code"));
    
    /* Set the layouts */
    QVBoxLayout *arrowsLayout = new QVBoxLayout;
    arrowsLayout->addWidget(buttonUp);
    arrowsLayout->addWidget(buttonDown);
    arrowsLayout->addStretch(1);
    
    QHBoxLayout *listLayout = new QHBoxLayout;
    listLayout->addWidget(functionsList);
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
    connect(functionsList, &QListWidget::itemPressed, this, &FunctionsPage::updateUI);
    connect(functionsList, &QListWidget::itemDoubleClicked, this, &FunctionsPage::editFunction);
    connect(buttonAdd, &QPushButton::pressed, this, &FunctionsPage::addFunction);
    connect(buttonDel, &QPushButton::pressed, this, &FunctionsPage::deleteFunction);
    connect(buttonEdit, &QPushButton::pressed, this, &FunctionsPage::editFunction);
    connect(buttonUp, &QPushButton::pressed, this, &FunctionsPage::upPressed);
    connect(buttonDown, &QPushButton::pressed, this, &FunctionsPage::downPressed);
    connect(description, &QPlainTextEdit::textChanged, this, &FunctionsPage::updateDescription);
    connect(code, &QPlainTextEdit::textChanged, this, &FunctionsPage::updateCode);
    connect(this, &FunctionsPage::checkOKState, parent, &ClassItemDialog::checkOKState);
    
    clearingDescription = false;
    clearingCode = false;
        
    /* Disable the buttons */
    updateUI();
}

FunctionsPage::~FunctionsPage()
{
    /* TODO clear here the lists */
}

/*!
 * \brief This function enables or disables some buttons. */
void FunctionsPage::updateUI()
{
    bool itemSelected = functionsList->selectedItems().count() > 0;
    /* Delete and edit buttons are enabled, if any item is selected */
    buttonDel->setEnabled(itemSelected);
    buttonEdit->setEnabled(itemSelected);
    /* Put the description and the code into dialog */
    description->setEnabled(itemSelected);
    code->setEnabled(itemSelected);
    if (itemSelected)
    {
        int index = functionsList->currentRow();
        description->setPlainText(functions[index]->description);
        code->setPlainText(functions[index]->code);
    }
    else
    {
        /* If nothing selected, then dont show anything */
        clearingDescription = true;
        clearingCode = true;
        description->clear();
        code->clear();
    }
    /* Up enabled if not the first item is selected */
    buttonUp->setEnabled(itemSelected && functionsList->currentRow() != 0);
    /* Down enabled if not the last item is selected */
    buttonDown->setEnabled(itemSelected && functionsList->currentRow() != functionsList->count() - 1);
}

/*!
 * \brief This function adds an element into the widget list. */
void FunctionsPage::addFunction()
{
    /* Call the dialog for adding a new item : false */
    AddEditFunctionDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) 
    {
        /* Get the info from the dialog */
        FunctionProperties *info = dlg.getInformation();
        /* Add the name to list widget : because the item will be added for the first time, it has no parameters yet */
        /* A function must be identified with its parameters, therefore we need to modify the name of the function first */
        //functionsList->addItem(QString("%1() : %2").arg(info->name).arg(info->returnType));
        QListWidgetItem *item = new QListWidgetItem(info->string(false));
        functionsList->addItem(item);
        /* Add the info to qlist */
        functions.append(info);
        /* Update buttons */
        updateUI();
    }
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

void FunctionsPage::deleteFunction()
{
    /* Remove the text in description and code */
    description->clear();
    code->clear();
    /* 0 : can delete only one item */
    int index = functionsList->currentRow();
    QListWidgetItem *item = functionsList->takeItem(index);
    delete item;
    item = NULL;
    /* Get the element before removing it */
    FunctionProperties *function = functions[index];
    /* Remove the element from the list */    
    functions.removeAt(index);
    /* Update buttons */
    updateUI();
    /* If -1, means not in db yet */
    if (function->id == -1)
    {
        /* Just remove the function from the memory */
        delete function;
        function= NULL;
    }
    else
    {
        /* This function is in db, add to "todelete-list" to remove it from the db */
        functionsToDelete.append(function);
    }
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

void FunctionsPage::editFunction()
{
    /* 0 : can edit only one item */
    int index = functionsList->currentRow();
    FunctionProperties *editInfo = functions.at(index);
    /* Call the dialog */
    AddEditFunctionDialog dlg(this, editInfo);
    if (dlg.exec() == QDialog::Accepted) 
    {
        /* Save the id of the member, because the dialog will return for id -1 back, because there is no widget fot the id */
        int currID = editInfo->id;
        /* First remove old entry from functions */
        delete editInfo;
        editInfo = NULL;
        /* Get the info from the dialog */
        FunctionProperties *info = dlg.getInformation();
        /* Store the id back */
        info->id = currID;
        /* Edit the info to qlist */
        functions[index] = info;
        /* Change the name in the list window */
        QListWidgetItem *oldItem = functionsList->takeItem(index);
        /* A function must be identified with its parameters, therefore we need to modify the name of the function first */
        QListWidgetItem *item;
        int paramCount = info->parameters.count();
         /* Create the function name */
        if (paramCount == 0)
            //item = new QListWidgetItem(QString("%1() : %2").arg(info->name).arg(info->returnType));
            item = new QListWidgetItem(info->string(false));
        else
            item = new QListWidgetItem(info->string(false));
        functionsList->insertItem(index, item);
        /* Remove the old item from the memory */
        delete oldItem;
        oldItem = NULL;
        /* Get the list of the deleted parameters */
        parametersToDelete = dlg.getDeleteInformation();
        /* Tell parent to check for the ok-button - smtg maybe changed */
        emit checkOKState();
    }
    /* Update buttons */
    updateUI();    
}

void FunctionsPage::upPressed()
{
    int index = functionsList->currentRow();
    /* Item up in the list widget */
    QListWidgetItem *item = functionsList->takeItem(index);
    functionsList->insertItem(index-1, item);
    functionsList->setCurrentRow(index-1);
    /* Item up in the qlist */
    FunctionProperties *info = functions.at(index-1);
    functions[index-1] = functions[index];
    functions[index] = info;
    /* Update buttons */
    updateUI();
}

void FunctionsPage::downPressed()
{
    int index = functionsList->currentRow();
    /* Item down in the list widget */
    QListWidgetItem *item = functionsList->takeItem(index);
    functionsList->insertItem(index+1, item);
    functionsList->setCurrentRow(index+1);
    /* Item up in the qlist */
    FunctionProperties *info = functions.at(index+1);
    functions[index+1] = functions[index];
    functions[index] = info;
    /* Update buttons */
    updateUI();
}

QList<FunctionProperties *> FunctionsPage::getInformation() const
{
    return functions;
}

QList<FunctionProperties *> FunctionsPage::getDeleteInformation() const
{
    return functionsToDelete;
}

QList<ParameterProperties *> FunctionsPage::getParametersDeleteInformation() const
{
    return parametersToDelete;
}

/*!
 * \brief This function updates the description of a member, whenever it will be updated. */
void FunctionsPage::updateDescription()
{
    /* If QPlainTextEdit will be cleared, then don't update the list.
       This clear will be sent, if no item is selected in the list. Otherwise
       if the description info will be cleared by the user, no check is necessary. */
    if (clearingDescription)
    {
        clearingDescription = false;
        return;
    }
    int index = functionsList->currentRow();
    if (index > -1)
    {
        functions[index]->description = description->toPlainText();
        /* Tell parent to check for the ok-button - smtg maybe changed */
        emit checkOKState();
    }
}

/*!
 * \brief This function updates the code of a member, whenever it will be updated. */
void FunctionsPage::updateCode()
{
    /* If QPlainTextEdit will be cleared, then don't update the list.
       This clear will be sent, if no item is selected in the list. Otherwise
       if the description info will be cleared by the user, no check is necessary. */
    if (clearingCode)
    {
        clearingCode = false;
        return;
    }
    int index = functionsList->currentRow();
    if (index > -1)
    {
        functions[index]->code = code->toPlainText();
        /* Tell parent to check for the ok-button - smtg maybe changed */
        emit checkOKState();
    }
}


/*!
 * \brief This function will be called, when an item will be inserted into, deleted from the widget list or edited.
 * @return true if the user changed something, false everything are the same. If true, the ok-Button will be enabled. */
bool FunctionsPage::checkTheListNow()
{
    /* If the counts not equal, then enable the ok-button */
    if (functionsAtBeginning.count() != functions.count())
        return true;
        
    /* If both has no items */
    if (functionsAtBeginning.count() == 0 && functions.count() == 0)
        return false;
        
    /* Else compare the elements */
    for (int index = 0; index < functions.count(); ++index)
    {
        bool state = functionsAtBeginning[index]->name != functions[index]->name || 
                functionsAtBeginning[index]->returnType != functions[index]->returnType ||
                functionsAtBeginning[index]->stereotype != functions[index]->stereotype || 
                functionsAtBeginning[index]->isStatic != functions[index]->isStatic ||
                functionsAtBeginning[index]->isVirtual != functions[index]->isVirtual ||
                functionsAtBeginning[index]->visibility != functions[index]->visibility ||
                functionsAtBeginning[index]->description != functions[index]->description ||
                functionsAtBeginning[index]->code != functions[index]->code;
        /* It there is something is different, return true */
        if (state)
            return true;
        
        /* If not, continue with parameters */
        /* If the counts not equal, then return true */
        if (functionsAtBeginning[index]->parameters.count() != functions[index]->parameters.count())
            return true;
        
        /* Falls parameter are different, then true */
        for (int i = 0; i < functions[index]->parameters.count(); ++i)
        {
            bool parState = functionsAtBeginning[index]->parameters[i]->name != functions[index]->parameters[i]->name ||
                    functionsAtBeginning[index]->parameters[i]->type != functions[index]->parameters[i]->type ||
                    functionsAtBeginning[index]->parameters[i]->stereotype != functions[index]->parameters[i]->stereotype ||
                    functionsAtBeginning[index]->parameters[i]->initValue != functions[index]->parameters[i]->initValue ||
                    functionsAtBeginning[index]->parameters[i]->direction != functions[index]->parameters[i]->direction;
            
            if (parState)
                return true;
        }
        
        /* Nothing was different, continue with next item*/
    }
    
    /* At this point all elements are equal, no changes */
    return false;
}

/*!
 * \brief This function checks, whether the given function already exists. */
bool FunctionsPage::functionExists(const QString &name, const QString &retType, const QList<ParameterProperties *> &paramList)
{
    bool ret = false;
    
    foreach (FunctionProperties *function, functions)
    {
        if (function->name == name && function->returnType == retType && function->parameters.count() == paramList.count())
        {
            int matchCount = 0;
            for (int i = 0; i < function->parameters.count(); ++i)
                if (function->parameters[i]->type == paramList[i]->type)
                    matchCount++;
            
            /* Then found */
            if (matchCount == function->parameters.count())
                ret = true;
        }
    }
    
    return ret;
}


///*!
// * \brief Creates the function name. */
//QString FunctionsPage::createFunctionName(FunctionProperties *info)
//{
//    QString funcName = QString("%1(").arg(info->name);
//    foreach (ParameterProperties *parameter, info->parameters)
//    {
//        QString argString;
//        argString.append("%1 : %2, ");
//        argString = argString.arg("%1").arg(parameter->name);
//        argString = argString.arg("%2").arg(parameter->type);
//        funcName.append(argString);
//    }
//    /* Remove the last comma and space */
//    funcName = funcName.remove(funcName.count()-2, 2);
//    funcName.append(") : %1");
//    funcName = funcName.arg(info->returnType);
    
//    return funcName;
//}
