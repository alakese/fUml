#ifndef MEMBERSPAGE_H
#define MEMBERSPAGE_H

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


class MembersPage : public QWidget
{
    Q_OBJECT
    
    QList<MemberProperties *> members;
    /* If the user deletes a member, which is in DB, it must be removed */
    QList<MemberProperties *> membersToDelete;
    /* Compare the first and last state for ok-button */
    QList<MemberProperties *> membersAtBeginning;
    QGroupBox *configGroup;
    QListWidget *membersList;
    QPushButton *buttonUp;
    QPushButton *buttonDown;
    QPushButton *buttonAdd;
    QPushButton *buttonDel;
    QPushButton *buttonEdit;
    QTabWidget *tabWidget;
    QPlainTextEdit *description;
    /* True, when description.clear() called. Neccessary for updateDescription() */
    bool clearingDescription;
    
public:
    MembersPage(const QList<MemberProperties *> &, ClassItemDialog *parent = 0);
    ~MembersPage();
    QList<MemberProperties *> getInformation() const;
    QList<MemberProperties*> getDeleteInformation() const;
    bool checkTheListNow();
    bool memberExists(const QString &);
    
private slots:
    void addMember();
    void deleteMember();
    void editMember();
    void upPressed();
    void downPressed();
    void updateUI();
    void updateDescription();

signals:
    void checkOKState();
};

#endif //MEMBERSPAGE_H
