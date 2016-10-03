#ifndef DISPLAYSPAGE_H
#define DISPLAYSPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QGroupBox;
class QCheckBox;
QT_END_NAMESPACE
class ClassItemDialog;


class DisplaysPage : public QWidget
{
    Q_OBJECT
    /* States of the checks in a 8 bit member
     *  Bit 1 : show members 
     *  Bit 2 : show membernames only
     *  Bit 3 : show methods
     *  Bit 4 : show methodnames only
     *  Bit 5 : show public only */
    qint8 checksStatesInitial;
    QGroupBox *groupBox;
    QCheckBox *showMembers;
    QCheckBox *showOnlyMemberNames;
    QCheckBox *showMethods;
    QCheckBox *showOnlyMethodNames;
    QCheckBox *showOnlyPublic;

public:
    DisplaysPage(qint8 states, ClassItemDialog *parent);
    bool checkTheStates();
    qint8 getCurrentStates();
    
private slots:
    void stateShowMembersChanged(int);
    void stateShowMethodsChanged(int);
    void anyStateChanged(int);
    
signals:
    void checkOKState();
};

#endif // DISPLAYSPAGE_H
