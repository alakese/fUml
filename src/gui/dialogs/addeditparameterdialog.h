#ifndef ADDEDITPARAMETERDIALOG_H
#define ADDEDITPARAMETERDIALOG_H

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
QT_END_NAMESPACE
class AddEditFunctionDialog;


class AddEditParameterDialog : public QDialog
{
    Q_OBJECT

    AddEditFunctionDialog *functionDlg;
    ParameterProperties *parameterProperties;
    QGroupBox *configGroup;
    QLabel *typeLabel;
    QComboBox *typeComboBox;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *initialValueLabel;
    QLineEdit *initialValueEdit;
    QLabel *stereoTypeLabel;
    QLineEdit *stereoTypeEdit;
    QRadioButton *radio1;
    QRadioButton *radio2;
    QRadioButton *radio3;
    QGroupBox *groupBoxDirection;
    QDialogButtonBox *buttonBox;
    QLabel *labelWarning;

public:
    explicit AddEditParameterDialog(AddEditFunctionDialog *, ParameterProperties *info = 0);
    ParameterProperties *getInformation() const;

private:
    void setOKButtonState();
    
private slots:
    void okPressed();
};

#endif // ADDEDITPARAMETERDIALOG_H
