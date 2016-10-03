#include "projectinputdialog.h"
#include "../../project/projectmanagement.h"
#include <QFileDialog>
#include <QSettings>

#include <QDebug>


namespace {
    /* Registry entry for last used project path */
    const QString lastSavedPath = "newProject/lastSavedPath";
}

/*!
 * \brief ProjectInputDialog::ProjectInputDialog : If parent is not 0, the dialog will be shown centered over the parent widget.*/
ProjectInputDialog::ProjectInputDialog(QDialog *parent) : QDialog(parent) {
    m_strProjectName = "";
    m_width = 800;
    m_height = 640;
    /* Create widgets for project name */
    m_editProjectName = new QLineEdit(this);
    m_labelProjectName = new QLabel(this);
    m_labelProjectName->setText("Project name");
    m_labelProjectName->setBuddy(m_editProjectName);
    /* Create widgets for project path */
    m_labelProjectPath = new QLabel(this);
    m_labelProjectPath->setText("Project path");
    m_editProjectPath = new QLineEdit(this);
    // TODO test path for db- delete this line
    m_editProjectPath->setText("D:\\Temp\\test");
    //
    m_labelProjectPath->setBuddy(m_editProjectPath);
    m_pButtonSelectPath = new QPushButton("Select...", this);
    /* Create widgets for Ok - Cancel */
    m_pButtonOk = new QPushButton("OK", this);
    m_pButtonCancel = new QPushButton("Cancel", this);
    /* Create widgets for spinboxes */
    m_labelWidth = new QLabel("Width :");
    m_spinWidth = new QSpinBox();
    m_spinWidth->setRange(0, 65535);
    m_labelHeight = new QLabel("Height :");
    m_spinHeight = new QSpinBox();
    m_spinHeight->setRange(0, 65535);
    m_spinWidth->setValue(m_width);
    m_spinHeight->setValue(m_height);
    /* Warning-Label */
    m_pLblWarning = new QLabel();
    m_pLblWarning->setText("");
    /* Change the text color of the warning to red */
    m_pLblWarning->setStyleSheet("QLabel { color : red; }");
    
    /* Main layout*/
    m_mainLayout = new QVBoxLayout(this);
    /* Add widgets to grid layout, unless ok-cancel */
    m_sizeLayout = new QGridLayout();
    m_sizeLayout->addWidget(m_labelProjectName, 0, 0);
    m_sizeLayout->addWidget(m_editProjectName, 0, 1);
    m_sizeLayout->addWidget(m_labelProjectPath, 1, 0);
    m_sizeLayout->addWidget(m_editProjectPath, 1, 1);
    m_sizeLayout->addWidget(m_pButtonSelectPath, 1, 2);
    m_sizeLayout->addWidget(m_labelWidth, 2, 0);
    m_sizeLayout->addWidget(m_spinWidth, 2, 1);
    m_sizeLayout->addWidget(m_labelHeight, 3, 0);
    m_sizeLayout->addWidget(m_spinHeight, 3, 1);
    m_sizeLayout->addWidget(m_pLblWarning, 4, 1);
    
    /* Layout for ok-cancel */
    m_buttonLayout = new QHBoxLayout();
    m_buttonLayout->insertSpacing(0, this->width()/2);
    m_buttonLayout->addWidget(m_pButtonOk);
    m_buttonLayout->addWidget(m_pButtonCancel);
    /* Add layouts to main layout */
    m_mainLayout->addLayout(m_sizeLayout);
    m_mainLayout->addLayout(m_buttonLayout);
    /* Create connections */
    connect(m_pButtonSelectPath, &QPushButton::pressed, this, &ProjectInputDialog::buttonSelectPathPressed);
    connect(m_pButtonOk, &QPushButton::pressed, this, &ProjectInputDialog::okPressed);
    connect(m_pButtonCancel, &QPushButton::pressed, this, &ProjectInputDialog::closeDialog);
    /* Window settings */
    setWindowTitle("Create a new project");
    setFixedSize(QSize(500, 200));
}

/*!
 * \brief This function checks the inputs and stores them if there is not a similar project in the specified path. */
void ProjectInputDialog::okPressed() {
    QString name = m_editProjectName->text();
    QString path = m_editProjectPath->text();
    
    /* Check name size */
    if (name.isEmpty() || path.isEmpty())
    {
        m_pLblWarning->setText("Project name or path is empty!");
        return;
    }
    
    /* Has the new name allowed - characters and syntax*/
    QRegularExpression re("^[a-zA-Z0-9_]+$");
    QRegularExpressionMatch match = re.match(name);
    if (!match.hasMatch())
    {
        m_pLblWarning->setText("Project name is not allowed!");
        return;
    }
    
    /* Check if the project already exists : in application e.g is there a project, which is in the list but not saved yet */
    if (ProjectManager::getInstance()->isThisAProjectName(name))
    {
        m_pLblWarning->setText("Project already exists in application!");
        return;
    }
    
    /* Check if the project already exists : in file system */
    QString strFile(QString("%1/%2.pj").arg(path).arg(name));
    QFile file(strFile);
    if(file.exists())
    {
        m_pLblWarning->setText("Project already exists in this folder!");
        return;
    }

    // TODO check if there is any other proj in this path - but later
    
    /* Get the file name */
    setProjectName(name);
    /* Get the path */
    m_strProjectPath = path;
    /* Get the width and height */
    setWidth(m_spinWidth->value());
    setHeight(m_spinHeight->value());
    
    /* Finish */
    accept();
}

void ProjectInputDialog::closeDialog() {
    setResult(QDialog::Rejected);
    reject();
}

/*!
 * \brief This function opens a dialog window to get the wish-path of the project. */
void ProjectInputDialog::buttonSelectPathPressed()
{
    /* Get the last used path from the registry (WINDOWS) */
    QString lastPath = loadIniSettings();

    /* If there is no path defined yet, then start with "C" */
    if (lastPath.isEmpty())
        lastPath = "C:\\";

    /* Open the dialog to get the path */
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select the project path"),
                            lastPath, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty())
        return;
    
    m_strProjectPath = dir;
    m_editProjectPath->setText(m_strProjectPath);
    /* Save the path in registry */
    saveIniSettings(m_strProjectPath);
}

/*!
 * \brief This function loads the last defined project path (as the user created a new project). */
QString ProjectInputDialog::loadIniSettings()
{
    /* Parameter : organization, application */
    QSettings settings(QLatin1String("F-Soft"), QLatin1String("fUML"));
    return settings.value(lastSavedPath, "").toString();
}

/*!
 * \brief This function saves the last saved project file path as temp-path to open the 
 * getExistingDirectory-Window. This window will show the last defined project path as
 * target path, when the user will try to create a new project. */
void ProjectInputDialog::saveIniSettings(const QString &setting)
{
    QSettings settings(QLatin1String("F-Soft"), QLatin1String("fUML"));
    settings.setValue(lastSavedPath, setting);
}

/* ----------------------------------------------- */
/* All the functions below are setters and getters */
/* ----------------------------------------------- */
QString ProjectInputDialog::projectPath() const
{
    return m_strProjectPath;
}

void ProjectInputDialog::setProjectPath(const QString &strProjectPath)
{
    m_strProjectPath = strProjectPath;
}


QString ProjectInputDialog::projectName() const
{
    return m_strProjectName;
}

void ProjectInputDialog::setProjectName(const QString &projectName)
{
    m_strProjectName = projectName;
}

int ProjectInputDialog::height() const
{
    return m_height;
}

void ProjectInputDialog::setHeight(int height)
{
    m_height = height;
}

int ProjectInputDialog::width() const
{
    return m_width;
}

void ProjectInputDialog::setWidth(int width)
{
    m_width = width;
}
/* ----------------------------------------------- */
