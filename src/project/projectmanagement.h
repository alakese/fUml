#ifndef PROJECTMANAGEMENT_H
#define PROJECTMANAGEMENT_H

#include <QObject>
#include <QList>

class GraphicsScene;
class Project;


/*!
 * \class ProjectManagement
 * \brief This class manages the loaded or created classes in application. This class is designed as singleton.
 * Therefore it is possible to reach project information from any class. Specially the communication between tree widget 
 * and project is needed for example when a item will be renamed. */
class ProjectManagement : public QObject
{
    Q_OBJECT
    
public:
    ProjectManagement();
    ~ProjectManagement();
    bool createNewProject(GraphicsScene *scene);
    QString getActiveProjectName();
    Project *getActiveProject();
    bool isThisAProjectName(const QString &);
    void loadNewProject(Project *);
    
signals:
    void activeProjectChanged(Project *);
    
public slots:    
    void setActiveProject(const QString &);
    
private:
    /* There can be more then one project at once */
    QList<Project*> m_listProjects;
    Project *m_pActiveProject;
};


/*!
 * \brief The ProjectManager class */
class ProjectManager
{
    ProjectManager() { }
    ~ProjectManager() { }
    
public:
    static ProjectManagement *getInstance()
    {
        static ProjectManagement pMan;
        return &pMan;
    }
};


#endif // PROJECTMANAGEMENT_H
