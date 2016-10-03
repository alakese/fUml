#ifndef ADDEDITFUNCTIONDIALOG_H
#define ADDEDITFUNCTIONDIALOG_H

#include <QDialog>
#include "../../globals/globals.h"
#include "../../globals/properties.h"

QT_BEGIN_NAMESPACE
class QLabel;
class QComboBox;
class QLineEdit;
class QGroupBox;
class QCheckBox;
class QRadioButton;
class QDialogButtonBox;
class QListWidget;
class QPushButton;
QT_END_NAMESPACE
class FunctionsPage;

class AddEditFunctionDialog : public QDialog
{
    Q_OBJECT

    FunctionsPage *functionsPage;
    FunctionProperties *functionProperties;
    QList<ParameterProperties *> parameters;
    QList<ParameterProperties *> parametersToDelete;    
    QGroupBox *groupBoxGeneral;
    QLabel *typeLabel;
    QComboBox *retTypeComboBox;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *stereoTypeLabel;
    QLineEdit *stereoTypeEdit;
    QCheckBox *isStatic;
    QCheckBox *isVirtual;
    QGroupBox *groupBoxVisibility;
    QRadioButton *radio1;
    QRadioButton *radio2;
    QRadioButton *radio3;
    QGroupBox *groupBoxParameters;
    QListWidget *parametersList;
    QPushButton *buttonUp;
    QPushButton *buttonDown;
    QPushButton *buttonAdd;
    QPushButton *buttonDel;
    QPushButton *buttonEdit;
    QDialogButtonBox *buttonBox;
    QLabel *labelWarning;
        
public:
    explicit AddEditFunctionDialog(FunctionsPage *, FunctionProperties *info = 0);
    FunctionProperties *getInformation() const;
    QList<ParameterProperties *> getDeleteInformation() const;
    bool parameterExists(const QString &);
    
private slots:
    void okPressed();
    void addParameter();
    void deleteParameter();
    void editParameter();
    void upPressed();
    void downPressed();
    void updateUI();
    void setOKButtonState();
};

#endif // ADDEDITFUNCTIONDIALOG_H
