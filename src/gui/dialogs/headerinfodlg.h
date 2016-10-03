#ifndef HEADERINFODLG_H
#define HEADERINFODLG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QHBoxLayout;
class QVBoxLayout;
class QLabel;
QT_END_NAMESPACE


/*!
 * \class HeaderInfoDlg
 * \brief This class opens a dialog window to get a new class name. It checks the name, whether it is allowed or not.
 * If not, a warning will be shown on dialog in red-color. */
class HeaderInfoDlg : public QDialog
{
    Q_OBJECT
public:
    explicit HeaderInfoDlg(QString &);
    QString className() const;
    void setClassName(const QString &className);

private:
    bool nameIsInList(const QString &);
    
private slots:
    void okPressed();
    void closeDialog();
    void clearWarning();
    
private:
    QString m_strClassName;
    QLabel *m_pLblClassName;
    QLineEdit *m_pEdtClassName;
    QHBoxLayout *m_pHorLayoutAbove;
    QHBoxLayout *m_pHorLayoutBelow;
    QVBoxLayout *m_pMainLayout;
    QPushButton *m_pButOk;
    QPushButton *m_pButCancel;
    QLabel *m_pLblWarning;
};

#endif // HEADERINFODLG_H
