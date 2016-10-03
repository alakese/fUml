#ifndef FUNCTIONSPAGE_H
#define FUNCTIONSPAGE_H

#include <QWidget>
#include "../../globals/globals.h"
#include "../../globals/properties.h"

QT_BEGIN_NAMESPACE
class QGroupBox;
class QListWidget;
class QPushButton;
class QTabWidget;
class QPlainTextEdit;
QT_END_NAMESPACE
class ClassItemDialog;
class ParameterProperties;


class FunctionsPage : public QWidget
{
    Q_OBJECT
    
    QList<FunctionProperties *> functions;
    /* If the user deletes a member, which is in DB, it must be removed */
    QList<FunctionProperties *> functionsToDelete;
    QList<ParameterProperties *> parametersToDelete;
    /* Compare the first and last state for ok-button */
    QList<FunctionProperties *> functionsAtBeginning;
    QGroupBox *configGroup;
    QListWidget *functionsList;
    QPushButton *buttonUp;
    QPushButton *buttonDown;
    QPushButton *buttonAdd;
    QPushButton *buttonDel;
    QPushButton *buttonEdit;
    QTabWidget *tabWidget;
    QPlainTextEdit *description;
    QPlainTextEdit *code;
    bool clearingDescription;
    bool clearingCode;
    
public:
    FunctionsPage(const QList<FunctionProperties *> &, ClassItemDialog *);
    ~FunctionsPage();
    QList<FunctionProperties *> getInformation() const;
    QList<FunctionProperties *> getDeleteInformation() const;
    QList<ParameterProperties *> getParametersDeleteInformation() const;
    bool checkTheListNow();
    bool functionExists(const QString &, const QString &, const QList<ParameterProperties *> &);
    
private slots:
    void addFunction();
    void deleteFunction();
    void editFunction();
    void upPressed();
    void downPressed();
    void updateUI();
    void updateDescription();
    void updateCode();
    
signals:
    void checkOKState();
};

#endif // FUNCTIONSPAGE_H
