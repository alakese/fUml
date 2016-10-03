#include "headerinfodlg.h"
#include "../../project/projectmanagement.h"
#include "../../project/project.h"
#include "../../includes/syntaxchecker.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QLineEdit>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

HeaderInfoDlg::HeaderInfoDlg(QString &name)
{
    m_strClassName = name;
    
    m_pLblClassName = new QLabel();
    m_pLblClassName->setText("Class name");
    m_pEdtClassName = new QLineEdit();
    m_pEdtClassName->setText(m_strClassName);
    m_pButOk = new QPushButton("OK");
    m_pButCancel = new QPushButton("Cancel");

    m_pHorLayoutAbove = new QHBoxLayout();
    m_pHorLayoutAbove->addWidget(m_pLblClassName);
    m_pHorLayoutAbove->addWidget(m_pEdtClassName);
    
    m_pLblWarning = new QLabel();
    m_pLblWarning->setText("");
    /* Change the text color of the warning to red */
    m_pLblWarning->setStyleSheet("QLabel { color : red; }");
    m_pHorLayoutBelow = new QHBoxLayout();
    m_pHorLayoutBelow->insertSpacing(0, width()/2);
    m_pHorLayoutBelow->addWidget(m_pButOk);
    m_pHorLayoutBelow->addWidget(m_pButCancel);
    
    m_pMainLayout = new QVBoxLayout(this);    
    m_pMainLayout->addLayout(m_pHorLayoutAbove);
    m_pMainLayout->addWidget(m_pLblWarning);
    m_pMainLayout->addLayout(m_pHorLayoutBelow);
    
    setLayout(m_pMainLayout);

    connect(m_pButOk, &QPushButton::pressed, this, &HeaderInfoDlg::okPressed);
    connect(m_pButCancel, &QPushButton::pressed, this, &HeaderInfoDlg::closeDialog);
    connect(m_pEdtClassName, &QLineEdit::textChanged, this, &HeaderInfoDlg::clearWarning);
    
    setWindowTitle("Please give a new class name");
    setFixedSize(QSize(300, 100));
}

bool HeaderInfoDlg::nameIsInList(const QString &name)
{
    ProjectManagement *pm = ProjectManager::getInstance();
    return pm->getActiveProject()->find(name);
}

void HeaderInfoDlg::okPressed()
{
    QString lineText = m_pEdtClassName->text();
    /* Check if the new name is equal to old name */
    if (lineText == m_strClassName)
    {
        m_pLblWarning->setText("Class names are equal");
        return;
    }
    
    /* Has the new name allowed - characters and syntax*/
//    QRegularExpression re("^[a-zA-Z_][a-zA-Z0-9_]*$");
//    QRegularExpressionMatch match = re.match(lineText);
//    if (match.hasMatch())
    SyntaxChecker namechecker;
    if (namechecker.isClassNameAllowed(lineText))
    {
        /* Check if the new name is not in the project */
        if (nameIsInList(lineText))
        {
            m_pLblWarning->setAlignment(Qt::AlignHCenter);
            m_pLblWarning->setText("Class name is already in project");
            return;
        }
        /* Name is ok */
        /* Tell project manager that the name must be changed in the scene too */
        ProjectManagement *pm = ProjectManager::getInstance();
        /* Parameters : old name vs new name */
        pm->getActiveProject()->renameItem(m_strClassName, lineText);
        /* Save the name for the caller-class */
        m_pLblWarning->setText("");
        m_strClassName = lineText;

        accept();
    }
    else
    {
        m_pLblWarning->setAlignment(Qt::AlignHCenter);
        m_pLblWarning->setText("Class name not acceptable");
    }
}

void HeaderInfoDlg::closeDialog()
{
    setResult(QDialog::Rejected);
    reject();
}

/* ----------------------------------------------- */
/* All the functions below are setters and getters */
/* ----------------------------------------------- */

QString HeaderInfoDlg::className() const
{
    return m_strClassName;
}

void HeaderInfoDlg::setClassName(const QString &className)
{
    m_strClassName = className;
}

void HeaderInfoDlg::clearWarning()
{
    m_pLblWarning->setText("");
}

/* ----------------------------------------------- */
