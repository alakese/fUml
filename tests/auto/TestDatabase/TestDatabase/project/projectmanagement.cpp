#include "projectmanagement.h"
#include "project.h"
#include <QGraphicsScene>
#include <QSqlDatabase>


/*!
 * \brief In this constructor the database-driver will be defined as QSQLITE. In an application such kind of definition
 * must be done only once. Because this class is a singleton class and the projects are using database connections, the
 * definition will be done here. */
ProjectManagement::ProjectManagement()
{
    /* Find QSQLITE driver */
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "DBConnectionName");
    
    /* There is no active project yet */
    m_pActiveProject = NULL;
}

ProjectManagement::~ProjectManagement()
{
}

/*!
 * \brief This function creates a new project and adds it to the list of the projects which will be shown e.g. in tree widget. */
bool ProjectManagement::createNewProject(QGraphicsScene *scene)
{
    bool ret = false;
//    /* Get a new name for the new project */
//    ProjectInputDialog *pid = new ProjectInputDialog();
//    /* If accepted */
//    if (pid->exec() == QDialog::Accepted)
//    {
//        /* Get project infos from the dialog */
//        QString projectName = pid->projectName();
//        QString projectPath = pid->projectPath();
//        int width = pid->width();
//        int height = pid->height();
//        /* Create a new project */
//        Project *newProject = new Project(projectName, projectPath, width, height, scene);
//        /* Add the project to projectslist */
//        m_listProjects.append(newProject);
//        /* Last created project is an active project */
//        m_pActiveProject = newProject;
        
//        ret = true;
//    }
//    /* Remove the dialog object */
//    delete pid;
//    pid = NULL;
    
    return ret;
}

/*!
 * \brief This function registers a new loaded project from a file. */
void ProjectManagement::loadNewProject(Project *newProject)
{
    /* Add the project to projectslist */
    m_listProjects.append(newProject);
    /* Last created project is an active project */
    m_pActiveProject = newProject;
}

/*!
 * \brief This function returns the currently active project name from the list of projects. */
QString ProjectManagement::getActiveProjectName()
{
    return m_pActiveProject->projectName();
}

/*!
 * \brief This function returns the currently active project from the list of projects. */
Project *ProjectManagement::getActiveProject()
{
    return m_pActiveProject;
}

/*!
 * \brief This function checks, if a given name is a project name. */
bool ProjectManagement::isThisAProjectName(const QString &testName)
{
    /* Find the project in the list */
    foreach(Project *p, m_listProjects)
    {
        /* If the names has star in them, then remove them first for compare */
        QString projName = p->projectName();
        projName.remove("[*]");
        QString name = testName;
        name.remove("[*]");
        if (projName == name)
            return true;
    }
    
    return false;
}


/*!
 * \brief This function (slot) will be called when the user changes the project. 
 * TreeWidget sends the signal activeProjectChanged(QString), which is connected to this slot. */
void ProjectManagement::setActiveProject(const QString &newActiveProject)
{
    /* Find the project in the list */
    foreach(Project *p, m_listProjects)
    {
        /* If the names has star in them, then remove them first for compare */
        QString projName = p->projectName();
        projName.remove("[*]");
        QString newName = newActiveProject;
        newName.remove("[*]");
        
        if (projName == newName)
        {
            /* And change the active project in this class too */
            m_pActiveProject = p;
            return;
        }
    }
}

