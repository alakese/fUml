#ifndef GENERALPAGE_H
#define GENERALPAGE_H

#include <QWidget>
#include "../../globals/globals.h"
#include "../../globals/properties.h"

QT_BEGIN_NAMESPACE
class QLineEdit;
class QLabel;
class QCheckBox;
class QGroupBox;
class QRadioButton;
QT_END_NAMESPACE
class ClassItemDialog;


class GeneralPage : public QWidget
{
    Q_OBJECT
    
    GeneralProperties *generalProperties;
    QGroupBox *configGroup;
    QLabel *nameLabel;
    QLineEdit *nameEdit;
    QLabel *stereotypeLabel;
    QLineEdit *stereotypeEdit;
    QLabel *nameSpaceLabel;
    QLineEdit *nameSpaceEdit;
    QCheckBox *abstractSelection;
    QGroupBox *groupBox;
    QRadioButton *radio1;
    QRadioButton *radio2;
    QRadioButton *radio3;

public:
    GeneralPage(GeneralProperties *, ClassItemDialog *parent = 0);
    GeneralProperties *getInformation();
    bool checkTheListNow();
    QString getClassName() const;
    
signals:
    void checkOKState();

private slots:
    void anyWidgetInfoChanged();
};

#endif // GENERALPAGE_H
