#include "addeditmemberdialog.h"
#include "../pages/memberspage.h"
#include <QDialogButtonBox>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QPushButton>


AddEditMemberDialog::AddEditMemberDialog(MembersPage *mPage, MemberProperties *info)
{
    /* We need memberspage to check if the given member name already exists */
    membersPage = mPage;
    memberProperties = info;
    
    configGroup = new QGroupBox(tr("Member properties"));
    
    /* Create the items */
    typeLabel = new QLabel(tr("Type:"));
    typeComboBox = new QComboBox();
    /* These are all native c++ types */
    typeComboBox->addItems(QStringList() << "" << "char" << "char16_t" << "char32_t" << "wchar_t" << "signed short int"  << "signed int"  <<
                           "signed long int" << "signed long long int" << "unsigned char" << "unsigned short int" <<
                           "unsigned int" << "unsigned long int" << "unsigned long long int" << "float" << "double" <<
                           "long double" << "bool");
    /* Allow only chars and digits with size max 128, no space */
    typeComboBox->setEditable(true);
    /* QComboBox validator must be set after setEditable, or else it wont work */
    typeComboBox->setValidator(new QRegExpValidator(QRegExp("[ a-zA-Z0-9_]{1,128}"), this));    
    nameLabel = new QLabel(tr("Name:"));
    nameEdit = new QLineEdit();
    /* Allow only chars and digits with size max 128, no space */
    nameEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9_]{1,128}"), this));
    initialValueLabel = new QLabel(tr("Initial value:"));
    initialValueEdit = new QLineEdit();
    stereoTypeLabel = new QLabel(tr("Stereotype name:"));
    stereoTypeEdit = new QLineEdit();
    isStatic = new QCheckBox(tr("Static"));
    isStatic->setChecked(false);
    
    groupBox = new QGroupBox(tr("Visibility"));
    radio1 = new QRadioButton(tr("Public"));
    radio2 = new QRadioButton(tr("Private"));
    radio3 = new QRadioButton(tr("Protected"));
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
    layout->addWidget(isStatic, 4, 0, 4, 1);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(radio1);
    hbox->addWidget(radio2);
    hbox->addWidget(radio3);
    groupBox->setLayout(hbox);

    /* Warning-Label */
    labelWarning = new QLabel();
    labelWarning->setText("");
    /* Change the text color of the warning to red */
    labelWarning->setStyleSheet("QLabel { color : red; }");
    
    /* Create the buttons and connections */
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(okPressed())); // TODO old version of connect
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    configGroup->setLayout(layout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(configGroup);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(groupBox);
    mainLayout->addSpacing(15);
    mainLayout->addWidget(labelWarning);
    mainLayout->addSpacing(15);    
    mainLayout->addWidget(buttonBox);
    mainLayout->addSpacing(15);
    mainLayout->addStretch(1);

    setLayout(mainLayout);
    setWindowTitle(tr("Add item"));
    
    /* If there is a information in info, then this suppose to be edit. 
       Otherwise it is add. */
    if (memberProperties)
    {
        /* Fill the blanks */
        typeComboBox->setCurrentText(info->type);
        nameEdit->setText(info->name);
        initialValueEdit->setText(info->initValue);
        stereoTypeEdit->setText(info->stereotype);
        isStatic->setChecked(info->isStatic);
        if (info->visibility == PUBLIC)
            radio1->setChecked(true);
        else if (info->visibility == PRIVATE)
            radio2->setChecked(true);
        else if (info->visibility == PROTECTED)
            radio3->setChecked(true);
        setWindowTitle(tr("Edit item"));
    }
    
    /* At least type and must be given, to enable the ok-button */
    connect(typeComboBox, &QComboBox::currentTextChanged, this, &AddEditMemberDialog::setOKButtonState);    
    connect(nameEdit, &QLineEdit::textChanged, this, &AddEditMemberDialog::setOKButtonState);
    
    /* Check the state each time the dialog will be opened */
    setOKButtonState();
}

/*!
 * \brief This function enables the ok-button, if the min state reached : type and name may not be empty. */
void AddEditMemberDialog::setOKButtonState()
{
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(!nameEdit->text().isEmpty() && !typeComboBox->currentText().isEmpty());
}

void AddEditMemberDialog::okPressed()
{
    /* Accept, if the name not exists yet */
    if(memberProperties || !membersPage->memberExists(nameEdit->text()))
        accept();
    else
        labelWarning->setText("Member already exists!");
}

MemberProperties *AddEditMemberDialog::getInformation() const
{
    Visibility visi = PUBLIC;
    if (radio1->isChecked())
        visi = PUBLIC;
    else if (radio2->isChecked())
        visi = PRIVATE;
    else if (radio3->isChecked())
        visi = PROTECTED;

    
    /* id will not be shown in gui, it is for internall usage only */
    MemberProperties *info = new MemberProperties(-1, typeComboBox->currentText(), nameEdit->text(), initialValueEdit->text(),
                                                  stereoTypeEdit->text(), isStatic->isChecked(), visi, QString());
    
    return info;
}
