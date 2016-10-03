#ifndef PROJECTINPUTDIALOG_H
#define PROJECTINPUTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>

/*!
 * \class ProjectInputDialog
 * \brief This class opens a dialog window to let the user enter a new project name. */
class ProjectInputDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ProjectInputDialog(QDialog *parent = 0);
    QString projectName() const;
    void setProjectName(const QString &);
    QString projectPath() const;
    void setProjectPath(const QString &);
    int width() const;
    void setWidth(int);
    int height() const;
    void setHeight(int);
    
private slots:
    void okPressed();
    void closeDialog();
    void buttonSelectPathPressed();

private:
    QString loadIniSettings();
    void saveIniSettings(const QString &);
    
private:
    QString m_strProjectName;
    QString m_strProjectPath;
    int m_width;
    int m_height;
    QLabel *m_labelProjectName;
    QLabel *m_labelProjectPath;
    QLabel *m_labelWidth;
    QLabel *m_labelHeight;
    QLineEdit *m_editProjectName;
    QLineEdit *m_editProjectPath;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_buttonLayout;
    QGridLayout *m_sizeLayout;
    QSpinBox *m_spinWidth;
    QSpinBox *m_spinHeight;
    QPushButton *m_pButtonSelectPath;
    QPushButton *m_pButtonOk;
    QPushButton *m_pButtonCancel;
    QLabel *m_pLblWarning;    
};

#endif // PROJECTINPUTDIALOG_H
