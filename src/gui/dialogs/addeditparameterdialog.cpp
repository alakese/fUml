#include "addeditparameterdialog.h"
#include "addeditfunctiondialog.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>

AddEditParameterDialog::AddEditParameterDialog(AddEditFunctionDialog *fDlg, ParameterProperties *info)
{
    /* We need AddEditFunctionDialog to check if the given parameter name already exists */
    functionDlg = fDlg;
    parameterProperties = info;
    
    configGroup = new QGroupBox(tr("Parameter properties"));
    
    /* Create the items */
    typeLabel = new QLabel(tr("Type:"));
    typeComboBox = new QComboBox();
    /* These are all native c++ types */
    typeComboBox->addItems(QStringList() << "" << "char" << "char16_t" << "char32_t" << "wchar_t" << "signed short int"  << "signed int"  <<
                          "signed long int" << "signed long long int" << "unsigned char" << "unsigned short int" <<
                          "unsigned int" << "unsigned long int" << "unsigned long long int" << "float" << "double" <<
                          "long double" << "bool" << "void");
    typeComboBox->setEditable(true);
    /* QComboBox validator must be set after setEditable, or else it wont work */
    typeComboBox->setValidator(new QRegExpValidator(QRegExp("[ a-zA-Z0-9_]{1,128}"), this));   
    nameLabel = new QLabel(tr("Name:"));
    nameEdit = new QLineEdit();
    nameEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9_]{1,128}"), this));   
    initialValueLabel = new QLabel(tr("Initial value:"));
    initialValueEdit = new QLineEdit();
    stereoTypeLabel = new QLabel(tr("Stereotype name:"));
    stereoTypeEdit = new QLineEdit();
    
    groupBoxDirection = new QGroupBox(tr("Direction"));
    radio1 = new QRadioButton(tr("in"));
    radio2 = new QRadioButton(tr("out"));
    radio3 = new QRadioButton(tr("inout"));
    radio1->setChecked(true);
    
    /* Set layouts */
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(typeLabel, 0, 0);
    layout->addWidget(typeComboBox, 0, 1);
    layout->addWidget(nameLabel, 1, 0);
    layout->addWidget(nameEdit, 1, 1);
    layout->addWidget(initialValueLabel, 2, 0);
    layout->addWidget(initialValueEdit, 2, 1);
    layout->addWidget(stereoTypeLabel, 3, 0);
    layout->addWidget(stereoTypeEdit, 3, 1);
    configGroup->setLayout(layout);
    
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(radio1);
    hbox->addWidget(radio2);
    hbox->addWidget(radio3);
    groupBoxDirection->setLayout(hbox);
    
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
    mainLayout->addWidget(configGroup);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(groupBoxDirection);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(labelWarning);
    mainLayout->addSpacing(15);    
    mainLayout->addWidget(buttonBox);
    mainLayout->addSpacing(15);
    mainLayout->addStretch(1);
    
    setLayout(mainLayout);
    setWindowTitle(tr("Add parameter"));
    
    /* If there is a infotmation in info, then this suppose to be edit. 
       Otherwise it is add. */
    if (parameterProperties)
    {
        /* Fill the blanks */
        typeComboBox->setCurrentText(info->type);
        nameEdit->setText(info->name);
        initialValueEdit->setText(info->initValue);
        stereoTypeEdit->setText(info->stereotype);

        if (info->direction == IN)
            radio1->setChecked(true);
        else if (info->direction == OUT)
            radio2->setChecked(true);
        else if (info->direction == INOUT)
            radio3->setChecked(true);
        
        setWindowTitle(tr("Edit parameter"));
    }
    
    /* At least type and must be given, to enable the ok-button */
    connect(typeComboBox, &QComboBox::currentTextChanged, this, &AddEditParameterDialog::setOKButtonState);    
    connect(nameEdit, &QLineEdit::textChanged, this, &AddEditParameterDialog::setOKButtonState);
    
    /* Check the state each time the dialog will be opened */
    setOKButtonState();
}

void AddEditParameterDialog::okPressed()
{
    /* Accept, if the parameter not exists yet */
    if(parameterProperties || !functionDlg->parameterExists(nameEdit->text()))
        accept();
    else
        labelWarning->setText("Parameter already exists!");
}

ParameterProperties *AddEditParameterDialog::getInformation() const
{
    Direction dir;
    if (radio1->isChecked())
        dir = IN;
    else if (radio2->isChecked())
        dir = OUT;
    else if (radio3->isChecked())
        dir = INOUT;
    
    ParameterProperties *info = new ParameterProperties(-1, typeComboBox->currentText(), nameEdit->text(),
                                                        initialValueEdit->text(), stereoTypeEdit->text(), dir);

    return info;
}

/*!
 * \brief This function enables the ok-button, if the min state reached : type and name may not be empty. */
void AddEditParameterDialog::setOKButtonState()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!nameEdit->text().isEmpty() && !typeComboBox->currentText().isEmpty());
}
