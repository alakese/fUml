#ifndef ADDEDITITEMDIALOG_H
#define ADDEDITITEMDIALOG_H

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
class MembersPage;


class AddEditMemberDialog : public QDialog
{
    Q_OBJECT
    
    MembersPage *membersPage;
    MemberProperties *memberProperties;
    QGroupBox *configGroup;
    QLabel *typeLabel;
    QComboBox *typeComboBox;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *initialValueLabel;
    QLineEdit *initialValueEdit;
    QLabel *stereoTypeLabel;
    QLineEdit *stereoTypeEdit;
    QCheckBox *isStatic;
    QGroupBox *groupBox;
    QRadioButton *radio1;
    QRadioButton *radio2;
    QRadioButton *radio3;
    QDialogButtonBox *buttonBox;
    QLabel *labelWarning;
    
public:
    explicit AddEditMemberDialog(MembersPage*, MemberProperties *info = 0);
    MemberProperties *getInformation() const;
    
private slots:
    void okPressed();
    void setOKButtonState();
};

#endif // ADDEDITITEMDIALOG_H
