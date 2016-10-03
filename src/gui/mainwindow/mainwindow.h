#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QDockWidget;
class QMenu;
class QToolBar;
class QAction;
class QPrinter;
class Monitor;
class QUndoGroup;
class QUndoView;
class QTreeWidgetItem;
QT_END_NAMESPACE
class ProjectManagement;
class GraphicsView;
class Project;
class ClassItem;
class TreeWidget;
class CodeWindow;

/*!
 * \class MainWindow 
 * \brief The MainWindow class */
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    TreeWidget *projectItems() const;
    
public slots:
    void updateGUIForActiveProject(Project *);
    void setGUIDirty();
    void addClassItem(const QString &, const QPoint &);
    void drawItemCanceled();
    void drawItemFinished();
    void updateActions();
    /* From ClassItem */
    void itemMoved(ClassItem *);
    /* From ClassItem */
    void itemScaled(ClassItem *);
    /* From ClassItem & TreeWidget */
    void itemRenamed(const QString &, const QString &);
    
private slots:
    void projectNewActionTriggered();
    void projectLoadActionTriggered();
    void projectSaveActionTriggered();
    void projectExitActionTriggered();
    void addClassItemActionTriggered();
    void addAssociationItemActionTriggered();
    void showGridActionTriggered(bool);
    void snapGridActionTriggered(bool);
    void toggleMonitorWindowActionTriggered(bool);
    void genCodeActvPrjActionTriggered();
    void genCodeAllPrjsActionTriggered();
    
private:
    void createObjects();
    void setWindowProperties();
    void createActions();
    void createMenus();
    void createToolBars();
    void createDockWidgets();
    void createStatusBar();
    void createConnections();
    void saveProjectFile(Project *);
    void createClassItemConnections(ClassItem *);
    
protected:
    void closeEvent(QCloseEvent *);
    
private:
    /* left of the window */
    TreeWidget *m_pProjectItems;
    QTreeWidgetItem *m_pTopLevelComponent;
    QDockWidget *m_pDockProjectList;

    /* right of the window : diagram */
    GraphicsView *m_pGraphicsView;
    //QDockWidget *m_pDockGraphicsView;
    QTabWidget *m_pDiagramTabs;
    
    /* bottom of the diagram : code entry */
    CodeWindow *m_pCodeWindow;
    QDockWidget *m_pDockCodeWindow;
    
    /* right of the diagram  */
    QUndoView *m_pUndoView; // TODO For monitoring purposes, change in iteration 3
    QDockWidget *m_pDockUndoView;
    
    /* GUI stuff */
    QMenu *mainMenu;
    QToolBar *projectToolBar;
    QAction *projectNewAction;
    QAction *projectLoadAction;
    QAction *projectSaveAction;
    QAction *projectExportAction;
    QAction *projectExitAction;
    QToolBar *itemsToolBar;
    QAction *addClassItemAction;
    QAction *addAssociationItemAction;
    QAction *diagramShowGridAction;
    QAction *diagramSnapGridAction;
    QAction *generateCodeActvPrjAction;
    QAction *generateCodeAllPrjsAction;
    QMenu *showHideMenu;
    QAction *showHideMonitor;
    QAction *undoAction;
    QAction *redoAction;
    
    /* Class to manage more then one projects */
    ProjectManagement *m_pProjectManagement;
    /* Printer settings */
    QPrinter *m_pPrinter;
    /* Monitor window */
    Monitor *m_pMonitor;
    /* Undo/Redo group - undo-redo stack for each project */
    QUndoGroup *m_pUndoGroup;
};




#endif // MAINWINDOW_H
