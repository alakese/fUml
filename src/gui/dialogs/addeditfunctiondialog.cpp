#include "addeditfunctiondialog.h"
#include "addeditparameterdialog.h"
#include "../pages/functionspage.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QListWidget>
#include <QBitmap>
#include <QPixmap>
#include <QPushButton>


AddEditFunctionDialog::AddEditFunctionDialog(FunctionsPage *fPage, FunctionProperties *info)
{
    /* We need functionspage to check if the given function already exists */
    functionsPage = fPage;
    functionProperties = info;
    
    groupBoxGeneral = new QGroupBox(tr("Function properties"));
    
    /* Create the items */
    typeLabel = new QLabel(tr("Return type:"));
    retTypeComboBox = new QComboBox();
    /* These are all native c++ types */
    retTypeComboBox->addItems(QStringList() << "" << "char" << "char16_t" << "char32_t" << "wchar_t" << "signed short int"  << "signed int"  <<
                           "signed long int" << "signed long long int" << "unsigned char" << "unsigned short int" <<
                           "unsigned int" << "unsigned long int" << "unsigned long long int" << "float" << "double" <<
                           "long double" << "bool" << "void");
    retTypeComboBox->setEditable(true);
    /* QComboBox validator must be set after setEditable, or else it wont work */
    retTypeComboBox->setValidator(new QRegExpValidator(QRegExp("[ a-zA-Z0-9_]{1,128}"), this));   
    nameLabel = new QLabel(tr("Name:"));
    nameEdit = new QLineEdit();
    nameEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9_]{1,128}"), this));   
    stereoTypeLabel = new QLabel(tr("Stereotype name:"));
    stereoTypeEdit = new QLineEdit();
    isStatic = new QCheckBox(tr("Static"));
    isStatic->setChecked(false);
    isVirtual = new QCheckBox(tr("Virtual"));
    isVirtual->setChecked(false);
    
    groupBoxVisibility = new QGroupBox(tr("Visibility"));
    radio1 = new QRadioButton(tr("Public"));
    radio2 = new QRadioButton(tr("Private"));
    radio3 = new QRadioButton(tr("Protected"));
    radio1->setChecked(true);

    groupBoxParameters = new QGroupBox(tr("Parameters"));
    parametersList = new QListWidget;
    parametersList->setSelectionMode(QAbstractItemView::SingleSelection);
    
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
    
    /* Set the layouts */
    
    /* Function properties */
    QGridLayout *gridlayout = new QGridLayout;
    gridlayout->addWidget(typeLabel, 0, 0);
    gridlayout->addWidget(retTypeComboBox, 0, 1);
    gridlayout->addWidget(nameLabel, 1, 0);
    gridlayout->addWidget(nameEdit, 1, 1);
    gridlayout->addWidget(stereoTypeLabel, 2, 0);
    gridlayout->addWidget(stereoTypeEdit, 2, 1);
    gridlayout->addWidget(isStatic, 3, 0);
    gridlayout->addWidget(isVirtual, 3, 1);
    groupBoxGeneral->setLayout(gridlayout);
    
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(radio1);
    hbox->addWidget(radio2);
    hbox->addWidget(radio3);
    groupBoxVisibility->setLayout(hbox);
    
    /* Parameters list */
    QVBoxLayout *arrowsLayout = new QVBoxLayout;
    arrowsLayout->addWidget(buttonUp);
    arrowsLayout->addWidget(buttonDown);
    arrowsLayout->addStretch(1);
    
    QHBoxLayout *listLayout = new QHBoxLayout;
    listLayout->addWidget(parametersList);
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
        
    groupBoxParameters->setLayout(allLayout);
    
    /* Warning-Label */
    labelWarning = new QLabel();
    labelWarning->setText("");
    /* Change the text color of the warning to red */
    labelWarning->setStyleSheet("QLabel { color : red; }");
    
    /* Create the buttons and connections */
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(okPressed()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBoxGeneral);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(groupBoxVisibility);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(groupBoxParameters);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(labelWarning);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(buttonBox);
    mainLayout->addSpacing(15);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
    setWindowTitle(tr("Add function"));
    
    /* Create the connections for the parameters list */
    connect(parametersList, &QListWidget::itemPressed, this, &AddEditFunctionDialog::updateUI);
    connect(parametersList, &QListWidget::itemDoubleClicked, this, &AddEditFunctionDialog::editParameter);
    connect(buttonAdd, &QPushButton::pressed, this, &AddEditFunctionDialog::addParameter);
    connect(buttonDel, &QPushButton::pressed, this, &AddEditFunctionDialog::deleteParameter);
    connect(buttonEdit, &QPushButton::pressed, this, &AddEditFunctionDialog::editParameter);
    connect(buttonUp, &QPushButton::pressed, this, &AddEditFunctionDialog::upPressed);
    connect(buttonDown, &QPushButton::pressed, this, &AddEditFunctionDialog::downPressed);
    /* At least type and must be given, to enable the ok-button */
    connect(retTypeComboBox, &QComboBox::currentTextChanged, this, &AddEditFunctionDialog::setOKButtonState);    
    connect(nameEdit, &QLineEdit::textChanged, this, &AddEditFunctionDialog::setOKButtonState);

    /* Disable the buttons */
    updateUI();
    
    /* If there is a infotmation in info, then this suppose to be edit. 
       Otherwise it is add. */
    if (functionProperties)
    {
        /* Fill the blanks */
        retTypeComboBox->setCurrentText(info->returnType);
        nameEdit->setText(info->name);
        stereoTypeEdit->setText(info->stereotype);
                
        foreach(ParameterProperties *param, info->parameters)
        {
            /* Add the item to the list window */
            //QListWidgetItem *item = new QListWidgetItem(QString("%1 : %2").arg(param->name).arg(param->type));
            QListWidgetItem *item = new QListWidgetItem(param->string());
            parametersList->addItem(item);
            /* Add the parameters to the qlist */
            parameters.append(param);
        }
        
        isStatic->setChecked(info->isStatic);
        isVirtual->setChecked(info->isVirtual);
        if (info->visibility == PUBLIC)
            radio1->setChecked(true);
        else if (info->visibility == PRIVATE)
            radio2->setChecked(true);
        else if (info->visibility == PROTECTED)
            radio3->setChecked(true);
        setWindowTitle(tr("Edit function"));
    }
      
    /* Check the state each time the dialog will be opened */
    setOKButtonState();
}

void AddEditFunctionDialog::updateUI()
{
    bool itemSelected = parametersList->selectedItems().count() > 0;
    /* Delete and edit buttons are enabled, if any item is selected */
    buttonDel->setEnabled(itemSelected);
    buttonEdit->setEnabled(itemSelected);
    /* Up enabled if not the first item is selected */
    buttonUp->setEnabled(itemSelected && parametersList->currentRow() != 0);
    /* Down enabled if not the last item is selected */
    buttonDown->setEnabled(itemSelected && parametersList->currentRow() != parametersList->count() - 1);
}

void AddEditFunctionDialog::okPressed()
{
    /* Accept, if edit function called (not add function) or if the name not exists */
    if(functionProperties || !functionsPage->functionExists(nameEdit->text(), retTypeComboBox->currentText(), parameters))
        accept();
    else
        labelWarning->setText("Function already exists!");
}

void AddEditFunctionDialog::addParameter()
{
    /* Call the dialog for adding a new item : nullptr */
    AddEditParameterDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) 
    {
        /* Get the info from the dialog */
        ParameterProperties *info = dlg.getInformation();
        /* Add the name to list widget */
        //parametersList->addItem(QString("%1 : %2").arg(info->name).arg(info->type));
        parametersList->addItem(info->string());
        /* Add the info to qlist */
        parameters.append(info);
        /* Update buttons */
        updateUI();
    }
}

void AddEditFunctionDialog::deleteParameter()
{
    /* 0 : can delete only one item */
    int index = parametersList->currentRow();
    
    /* Check, if there is a function without this parameter exists */
    QList<ParameterProperties *> parameterTmp(parameters);
    parameterTmp.removeAt(index);
    if(functionsPage->functionExists(nameEdit->text(), retTypeComboBox->currentText(), parameterTmp))
    {
        labelWarning->setText("This parameter can not be removed, because a same function exists then!");
        return;
    }

    /* Clear the warning, if shown */
    labelWarning->setText("");
    
    /* Ok, remove it */
    QListWidgetItem *item = parametersList->takeItem(index);
    delete item;
    item = NULL;
    /* Get the element before removing it */
    ParameterProperties *parameter = parameters[index];
    /* Remove the element from the list */
    parameters.removeAt(index);
    /* Update buttons */
    updateUI();
    /* If -1, means not in db yet */
    if (parameter->id == -1)
    {
        /* Just remove the member from the memory */
        delete parameter;
        parameter = NULL;
    }
    else
    {
        /* This member is in db, add to "todelete-list" to remove it from the db */
        parametersToDelete.append(parameter);
    }
}

void AddEditFunctionDialog::editParameter()
{
    /* 0 : can edit only one item */
    int index = parametersList->currentRow();
    ParameterProperties *editInfo = parameters.at(index);
    /* Call the dialog */
    AddEditParameterDialog dlg(this, editInfo);
    if (dlg.exec() == QDialog::Accepted) 
    {
        /* Save the id of the member, because the dialog will return for id -1 back, because there is no widget fot the id */
        int currID = editInfo->id;
        /* First remove old entry from members */
        delete editInfo;
        editInfo = NULL;
        /* Get the info from the dialog */
        ParameterProperties *info = dlg.getInformation();
        /* Store the id back */
        info->id = currID;
        /* Edit the info to qlist */
        parameters[parametersList->currentRow()] = info;
        /* Change the name in the list window */
        QListWidgetItem *oldItem = parametersList->takeItem(index);
        //QListWidgetItem *item = new QListWidgetItem(QString("%1 : %2").arg(info->name).arg(info->type));
        QListWidgetItem *item = new QListWidgetItem(info->string());
        parametersList->insertItem(index, item);
        /* Remove the old item from the memory */
        delete oldItem;
        oldItem = NULL;
    }
    /* Update buttons */
    updateUI();
}

void AddEditFunctionDialog::upPressed()
{
    int index = parametersList->currentRow();
    
    /* Item up in the list widget */
    QListWidgetItem *item = parametersList->takeItem(index);
    parametersList->insertItem(index-1, item);
    parametersList->setCurrentRow(index-1);
    
    /* Item up in the qlist */
    ParameterProperties *info = parameters.at(index-1);
    parameters[index-1] = parameters[index];
    parameters[index] = info;
    
    /* Update buttons */
    updateUI();
}

void AddEditFunctionDialog::downPressed()
{
    int index = parametersList->currentRow();
    
    /* Item down in the list widget */
    QListWidgetItem *item = parametersList->takeItem(index);
    parametersList->insertItem(index+1, item);
    parametersList->setCurrentRow(index+1);
    
    /* Item up in the qlist */
    ParameterProperties *info = parameters.at(index+1);
    parameters[index+1] = parameters[index];
    parameters[index] = info;
    
    /* Update buttons */
    updateUI();
}

FunctionProperties *AddEditFunctionDialog::getInformation() const
{
    Visibility visi = PUBLIC;
    if (radio1->isChecked())
        visi = PUBLIC;
    else if (radio2->isChecked())
        visi = PRIVATE;
    else if (radio3->isChecked())
        visi = PROTECTED;
    
    FunctionProperties *info = new FunctionProperties(-1, retTypeComboBox->currentText(), nameEdit->text(), stereoTypeEdit->text(), 
                                                      isStatic->isChecked(), isVirtual->isChecked(), visi, parameters, QString(), QString());

    return info;
}

QList<ParameterProperties*> AddEditFunctionDialog::getDeleteInformation() const
{
    return parametersToDelete;
}

/*!
 * \brief This function checks, whether the given parameter alredy exists. */
bool AddEditFunctionDialog::parameterExists(const QString &paramName)
{
    bool ret = false;
    foreach (ParameterProperties *parameter, parameters)
        if (parameter->name == paramName)
            ret = true;
            
    return ret;
}

/*!
 * \brief This function enables the ok-button, if the min state reached : type and name may not be empty. */
void AddEditFunctionDialog::setOKButtonState()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!nameEdit->text().isEmpty() && !retTypeComboBox->currentText().isEmpty());
}
