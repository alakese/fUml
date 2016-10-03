#ifndef CLASSITEMDIALOG_H
#define CLASSITEMDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QListWidget;
class QStackedWidget;
class QListWidgetItem;
class QDialogButtonBox;
QT_END_NAMESPACE
class GeneralProperties;
class MemberProperties;
class FunctionProperties;
class ParameterProperties;


class ClassItemDialog : public QDialog
{
    Q_OBJECT
    
public:
    ClassItemDialog(GeneralProperties *, const QList<MemberProperties *> &, const QList<FunctionProperties *> &, int, const QFont &, qint8);
    GeneralProperties *getGeneralInformation();
    QList<MemberProperties*> getMemberInformation() const;
    QList<MemberProperties*> getDeleteMemberInformation() const;
    QList<FunctionProperties *> getFunctionInformation() const;
    QList<FunctionProperties *> getDeleteFunctionInformation() const;
    QList<ParameterProperties*> getDeleteParameterInformation() const;
    QFont getNewFont() const;
    qint8 getDisplayOptions() const;
    
public slots:
    void changePage(QListWidgetItem *, QListWidgetItem *);
    void checkOKState();
    
private:
    void createContents();

private:
    QListWidget *m_pContentsList;
    QStackedWidget *m_pPagesWidget;
    QDialogButtonBox *m_pButtonBox;
};

#endif // CLASSITEMDIALOG_H
