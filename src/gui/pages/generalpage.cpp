#include "generalpage.h"
#include "../../gui/dialogs/classitemdialog.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QGroupBox>
#include <QRegExpValidator>


GeneralPage::GeneralPage(GeneralProperties *genProp, ClassItemDialog *parent)
{
    generalProperties = genProp;
    configGroup = new QGroupBox(tr("General settings"));

    /* Create the items */
    nameLabel = new QLabel(tr("Class name:"));
    nameEdit = new QLineEdit();
    nameEdit->setText(genProp->name);
    /* Allow only chars and digits with size max 128, no space */
    nameEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9_]{1,128}"), this));
    stereotypeLabel = new QLabel(tr("Stereotype:"));
    stereotypeEdit = new QLineEdit();
    stereotypeEdit->setText(genProp->stereotype);
    nameSpaceLabel = new QLabel(tr("Namespace:"));
    nameSpaceEdit = new QLineEdit();
    nameSpaceEdit->setText(genProp->nameSpace);
    /* Allow only chars, digits and _, : */
    nameSpaceEdit->setValidator(new QRegExpValidator(QRegExp("[a-zA-Z0-9_:.]{1,128}"), this));
    abstractSelection = new QCheckBox(tr("Abstract class"));
    abstractSelection->setChecked(genProp->isAbstract);

    groupBox = new QGroupBox(tr("Visibility"));
    radio1 = new QRadioButton(tr("Public"));
    radio2 = new QRadioButton(tr("Private"));
    radio3 = new QRadioButton(tr("Protected"));
    if (genProp->visibility == PUBLIC)
        radio1->setChecked(true);
    else if (genProp->visibility == PRIVATE)
        radio2->setChecked(true);
    else if (genProp->visibility == PROTECTED)
        radio3->setChecked(true);
    
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(radio1);
    hbox->addWidget(radio2);
    hbox->addWidget(radio3);
    groupBox->setLayout(hbox);
    
    /* Set the layouts */
    QGridLayout *itemsGLayout = new QGridLayout;
    itemsGLayout->addWidget(nameLabel, 0, 0);
    itemsGLayout->addWidget(nameEdit, 0, 1);
    itemsGLayout->addWidget(stereotypeLabel, 1, 0);
    itemsGLayout->addWidget(stereotypeEdit, 1, 1);
    itemsGLayout->addWidget(nameSpaceLabel, 2, 0);
    itemsGLayout->addWidget(nameSpaceEdit, 2, 1);
    
    QVBoxLayout *itemsVLayout = new QVBoxLayout;
    itemsVLayout->addLayout(itemsGLayout);
    itemsVLayout->addSpacing(15);
    itemsVLayout->addWidget(abstractSelection);
    itemsVLayout->addSpacing(15);
    itemsVLayout->addWidget(groupBox);
    
    configGroup->setLayout(itemsVLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(configGroup);
    mainLayout->addStretch(1);
    
    setLayout(mainLayout);
    
    /* Connect the widgets with dialog to change the state of the ok-button */
    connect(nameEdit, &QLineEdit::textChanged, this, &GeneralPage::anyWidgetInfoChanged);
    connect(stereotypeEdit, &QLineEdit::textChanged, this, &GeneralPage::anyWidgetInfoChanged);
    connect(nameSpaceEdit, &QLineEdit::textChanged, this, &GeneralPage::anyWidgetInfoChanged);
    connect(abstractSelection, &QCheckBox::stateChanged, this, &GeneralPage::anyWidgetInfoChanged);
    connect(radio1, &QRadioButton::released, this, &GeneralPage::anyWidgetInfoChanged);
    connect(radio2, &QRadioButton::released, this, &GeneralPage::anyWidgetInfoChanged);
    connect(radio3, &QRadioButton::released, this, &GeneralPage::anyWidgetInfoChanged);
    connect(this, &GeneralPage::checkOKState, parent, &ClassItemDialog::checkOKState);
}

/*!
 * \brief This function checks, if any property has been changed or not in order to signal parent dialog to 
 * activate/deactivate the ok-button. */
void GeneralPage::anyWidgetInfoChanged()
{
    /* Tell parent to check for the ok-button - smtg maybe changed */
    emit checkOKState();
}

/*!
 * \brief This function checks, if there is anything changed since the dialog has been opened. If yes the 
 * ok button on parent dialog will be enabled, if not then disabled. */
bool GeneralPage::checkTheListNow()
{
    Visibility visibility;
    if (radio1->isChecked())
        visibility = PUBLIC;
    else if (radio2->isChecked())
        visibility = PRIVATE;
    else if (radio3->isChecked())
        visibility = PROTECTED;
    
    bool state = generalProperties->name != nameEdit->text() || generalProperties->stereotype != stereotypeEdit->text() ||
                 generalProperties->nameSpace != nameSpaceEdit->text() || generalProperties->isAbstract != abstractSelection->isChecked() ||
                 generalProperties->visibility != visibility;
    
    return state;
}

GeneralProperties *GeneralPage::getInformation()
{
    Visibility visibility;
    if (radio1->isChecked())
        visibility = PUBLIC;
    else if (radio2->isChecked())
        visibility = PRIVATE;
    else if (radio3->isChecked())
        visibility = PROTECTED;
    
    /* id is here -1, which means, there is no id info-field on dialog */
    return new GeneralProperties(-1, nameEdit->text(), stereotypeEdit->text(), nameSpaceEdit->text().replace(".", "::"), 
                                                       abstractSelection->isChecked(), visibility);
}

QString GeneralPage::getClassName() const
{
    return nameEdit->text();
}
