#include "mainwindow.h"
#include "../../graphics/graphicsview.h"
#include "../../graphics/graphicsscene.h"
#include "../../graphics/items/classitem.h"
#include "../../graphics/items/associationitem.h"
#include "../../project/project.h"
#include "../../project/projectmanagement.h"
#include "../../gui/monitor/monitor.h"
#include "../../gui/mainwindow/treewidget.h"
#include "../../codeentry/codewindow.h"
#include "../../undo/classitem/nsClassItem_addcommand.h"
#include "../../undo/classitem/nsClassItem_movecommand.h"
#include "../../undo/classitem/nsClassItem_resizecommand.h"
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QHeaderView>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QFileDialog>
#include <QPrinter>
#include <QUndoGroup>
#include <QUndoView>

#include <QDebug>


MainWindow::~MainWindow()
{
    delete m_pPrinter;
    m_pPrinter = NULL;
}

/*!
 * \brief C'tor calls various helper functions to create the inital state of the program. */
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    createObjects();
    setWindowProperties();
    createActions();
    createMenus();
    createToolBars();
    createDockWidgets();
    createStatusBar();
    createConnections();
    updateActions();
}

/*!
 * \brief This function adds an association item between two class items. */
void MainWindow::addAssociationItemActionTriggered()
{
    /* Change the toolbar icon to busy */
    addAssociationItemAction->setChecked(true);
    /* Tell the scene that an association item may be drawn now */
    m_pProjectManagement->getActiveProject()->scene()->addItemTriggered();
    
}

/*!
 * \brief This function informs the view for a new item. Wenn the user presses the view, GraphicsView sends a signal with
 * the view coordinates. Scene and view coordinates are same. */
void MainWindow::addClassItemActionTriggered()
{
    /* Tell view that there will be an item inserted so it will send us the pos in scene-coordinates */
    m_pGraphicsView->setAddingItem(true);
    /* Change the toolbar icon to busy */
    addClassItemAction->setChecked(true);
}

/*!
 * \brief This function adds a new classitem to scene and treewidget. */
void MainWindow::addClassItem(const QString &name, const QPoint &insertPos)
{
    /* Define default properties : -1 means the item has not been stored to db yet */
    /* Initial values : id:-1, name:ClassItem, stereotype:"", nameSpace:"", isabstract:false, visibility:public */
    GeneralProperties *genProp = new GeneralProperties(-1, name, "", "", false, PUBLIC);
    /* insertPos is the position of the item where the user inserts the item */
    GUIProperties *guiProp = new GUIProperties(insertPos, QRect(0, 0, 100, 100), 28, 28, 28, QFont("Arial", 10));
    
    /* Create a new class item : two parametered c'tor */
    ClassItem *classItem = new ClassItem(genProp, guiProp);
    
    /* Test, whether this item is in the active project list */
    if (m_pProjectManagement->getActiveProject()->find(name))
    {
        /* Get the highest index */
        int index = m_pProjectManagement->getActiveProject()->getLastInstanceNo();
        /* Index will be the next highest e.g index+1 - Second parameter is false : dont update scene */
        classItem->setClassName(QString("%1_%2").arg(classItem->className()).arg(index+1), false);
    }

    /* Create for this item the connections */
    createClassItemConnections(classItem);
    
    /* Adding the new item via undo-framework - see redo() for classitem adding */
    nsClassItem::AddCommand *addCommand = new nsClassItem::AddCommand(m_pProjectManagement->getActiveProject(), 
                                                                      insertPos, classItem, m_pTopLevelComponent, m_pProjectItems, this);
    m_pProjectManagement->getActiveProject()->undoStack()->push(addCommand);
    
    /* Tell view that it can reset the cursor */
    m_pGraphicsView->setAddingItem(false);
    /* Change the toolbar icon to free */
    addClassItemAction->setChecked(false);
    /* Update the actions */
    updateActions();
    /* Send the info to monitor - convert QString to char * */
    MonitorManager::getInstance()->logMsg("INFO : New item inserted successfully", 0);
    statusBar()->showMessage(tr("New class added..."));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /* Send the info to monitor - convert QString to char * */
    MonitorManager::getInstance()->logMsg("INFO : Closing the application", 0);
    /* Close the monitor window */
    m_pMonitor->close();
    /* Close successfully */
    event->accept();
    
    // TODO if (maybeSave()) ???
    // else
    //  event->ignore();
}

/*!
 * \brief This function creates various non-gui objects. */
void MainWindow::createObjects()
{
    /* Create a singleton ProjectManager in order to use it from every class */
    m_pProjectManagement = ProjectManager::getInstance();
    /* Create printer */
    m_pPrinter = new QPrinter();
    /* Set it as landscape : user may change this */
    m_pPrinter->setOrientation(QPrinter::Landscape);
    /* Create monitor window */
    m_pMonitor = MonitorManager::getInstance();
    /* Create the undo-group for each project-undostack */    
    m_pUndoGroup = new QUndoGroup(this);
}

/*!
 * \brief This function creates two actions : create a new project and load a project. */
void MainWindow::createActions()
{
    /* Menu - Project */
    projectNewAction = new QAction(QIcon(":/images/new.png"), tr("&New..."), this);
    projectNewAction->setShortcuts(QKeySequence::New);
    projectNewAction->setStatusTip(tr("Create a new project"));
    projectLoadAction = new QAction(QIcon(":/images/load.png"), tr("&Load..."), this);
    projectLoadAction->setShortcuts(QKeySequence::Open);
    projectLoadAction->setStatusTip(tr("Load a project"));
    projectSaveAction = new QAction(QIcon(":/images/save.png"), tr("&Save..."), this);
    projectSaveAction->setShortcuts(QKeySequence::Save);
    projectSaveAction->setStatusTip(tr("Save the project"));
    projectExitAction = new QAction(QIcon(), tr("&Exit"), this);
    projectExitAction->setShortcuts(QKeySequence::Quit);
    projectExitAction->setStatusTip(tr("Quit the application"));
    /* Menu - Items */
    addClassItemAction = new QAction(QIcon(":/images/classitem.png"), tr("&Class"), this);
    addClassItemAction->setStatusTip(tr("Add a new class"));
    addClassItemAction->setCheckable(true);
    addAssociationItemAction = new QAction(QIcon(":/images/association.png"), tr("&Association"), this);
    addAssociationItemAction->setStatusTip(tr("Add an associaotion between two class items"));
    addAssociationItemAction->setCheckable(true);
    /* Menu - Generate */
    generateCodeActvPrjAction = new QAction(QIcon(), tr("Active Project"), this);
    generateCodeActvPrjAction->setStatusTip(tr("Generate code for only active project"));
    generateCodeAllPrjsAction = new QAction(QIcon(), tr("All Projects"), this);
    generateCodeAllPrjsAction->setStatusTip(tr("Generate code for all projects"));
    /* Menu - Diagram */
    diagramShowGridAction = new QAction(QIcon(), tr("Show grid"), this);
    diagramShowGridAction->setCheckable(true);
    diagramShowGridAction->setChecked(true);
    diagramSnapGridAction = new QAction(QIcon(), tr("Snap grid"), this);
    diagramSnapGridAction->setCheckable(true);
    diagramSnapGridAction->setChecked(true);
    /* Menu - Window-Show/Hide */
    showHideMonitor = new QAction(QIcon(), tr("Monitor"), this);
    showHideMonitor->setCheckable(true);
    showHideMonitor->setChecked(true);
    /* Undo - Redo */
    undoAction = m_pUndoGroup->createUndoAction(this);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/images/undo.png"));    
    redoAction = m_pUndoGroup->createRedoAction(this);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/images/redo.png"));
}

/*!
 * \brief This function creates two menü items : create a new project and load a project. */
void MainWindow::createMenus()
{
    mainMenu = menuBar()->addMenu(tr("&Project"));
    mainMenu->addAction(projectNewAction);
    mainMenu->addAction(projectLoadAction);
    mainMenu->addAction(projectSaveAction);
    mainMenu->addSeparator();
    mainMenu->addAction(projectExitAction);

    mainMenu = menuBar()->addMenu(tr("&Edit"));
    mainMenu->addAction(undoAction);
    mainMenu->addAction(redoAction);
        
    mainMenu = menuBar()->addMenu(tr("&Item"));
    QMenu *addMenu = mainMenu->addMenu(tr("Add"));
    addMenu->addAction(addClassItemAction);
    addMenu->addAction(addAssociationItemAction);
    
    mainMenu = menuBar()->addMenu(tr("&Generate Code"));
    mainMenu->addAction(generateCodeActvPrjAction);
    mainMenu->addAction(generateCodeAllPrjsAction);
    
    mainMenu = menuBar()->addMenu(tr("&Diagram"));
    mainMenu->addAction(diagramShowGridAction);
    mainMenu->addAction(diagramSnapGridAction);
    
    mainMenu = menuBar()->addMenu(tr("&Window"));
    showHideMenu = mainMenu->addMenu(tr("Show/Hide"));
    showHideMenu->addAction(showHideMonitor);
}

/*!
 * \brief This function creates two toolbar items : create a new project and load a project. */
void MainWindow::createToolBars()
{
    projectToolBar = addToolBar(tr("Project"));
    projectToolBar->addAction(projectNewAction);
    projectToolBar->addAction(projectLoadAction);
    projectToolBar->addAction(projectSaveAction);
    projectToolBar->addAction(undoAction);
    projectToolBar->addAction(redoAction);
    itemsToolBar = addToolBar(tr("Items"));
    itemsToolBar->addAction(addClassItemAction);
    itemsToolBar->addAction(addAssociationItemAction);
}

/*!
 * \brief This function creates the dock windows:
 * * Project view
 * * Graphics view */
void MainWindow::createDockWidgets()
{
    /* Add a seperator for "Show Monitor" */
    showHideMenu->addSeparator();
    
    /* I - Dock window for project view */
    m_pDockProjectList = new QDockWidget(tr("Projects"), this);
    m_pDockProjectList->setAllowedAreas(Qt::LeftDockWidgetArea);
    
    m_pProjectItems = new TreeWidget(m_pDockProjectList);
    m_pProjectItems->header()->close();
    m_pProjectItems->setColumnCount(1);
    m_pProjectItems->resizeColumnToContents(1);
    
    m_pDockProjectList->setMinimumSize(m_pProjectItems->size().width(), 0);
    m_pDockProjectList->setMaximumSize(m_pProjectItems->size().width()*2, this->size().height()*2);
    m_pDockProjectList->setWidget(m_pProjectItems);
    m_pDockProjectList->setFeatures(QDockWidget::DockWidgetClosable);
    
    addDockWidget(Qt::LeftDockWidgetArea, m_pDockProjectList);
    /* Possiblity to open-close the dock window */
    showHideMenu->addAction(m_pDockProjectList->toggleViewAction());
    
    /* II - Dock window for graphics view */
    m_pDiagramTabs = new QTabWidget(this);
    m_pDiagramTabs->setObjectName(QStringLiteral("diagramTabs"));
    /* Graphics view settings */
    m_pGraphicsView = new GraphicsView(m_pDiagramTabs);
    /* Indicates that the engine should antialias edges of primitives if possible. */
    m_pGraphicsView->setRenderHint(QPainter::Antialiasing);
    /* Point (0, 0) will stay fixed, when window will be resized */
    m_pGraphicsView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    /* When any visible part of the scene changes or is reexposed, QGraphicsView will update the entire viewport */
//    m_pGraphicsView->setViewportUpdateMode(QGraphicsView::FullViewportUpdate); //--> this makes all slow (after adding grid)
    m_pDiagramTabs->addTab(m_pGraphicsView, QString("Diagram"));
    setCentralWidget(m_pDiagramTabs);
    
    /* III - Dock window for code window */
    m_pDockCodeWindow = new QDockWidget(tr("Command window"), this);
    m_pDockCodeWindow->setAllowedAreas(Qt::BottomDockWidgetArea);
    m_pDockCodeWindow->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_pCodeWindow = new CodeWindow(this);
    m_pDockCodeWindow->setWidget(m_pCodeWindow);
    addDockWidget(Qt::BottomDockWidgetArea, m_pDockCodeWindow);
    showHideMenu->addAction(m_pDockCodeWindow->toggleViewAction());
    
    /* IV - Dock window for undo view */
    /* Create undo-view-window */
    m_pDockUndoView = new QDockWidget(tr("Undo view"), this);
    m_pDockUndoView->setAllowedAreas(Qt::RightDockWidgetArea);
    m_pDockUndoView->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    m_pUndoView = new QUndoView();
    m_pUndoView->setGroup(m_pUndoGroup);
    m_pUndoView->setCleanIcon(QIcon(":/images/ok.png"));
    m_pUndoView->setAttribute(Qt::WA_QuitOnClose, false);
    m_pDockUndoView->setWidget(m_pUndoView);
    addDockWidget(Qt::RightDockWidgetArea, m_pDockUndoView);
    showHideMenu->addAction(m_pDockUndoView->toggleViewAction());
    
//    /* Show both docks together */
//    tabifyDockWidget(m_pDockCodeWindow, m_pDockUndoView);
}


/*!
 * \brief This funtion creates the status bar. */
void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready..."));
}

/*!
 * \brief This function creates two connections : create a new project and load a project. */
void MainWindow::createConnections()
{
    /* Actions */
    connect(projectNewAction, &QAction::triggered, this, &MainWindow::projectNewActionTriggered);
    connect(projectLoadAction, &QAction::triggered, this, &MainWindow::projectLoadActionTriggered);
    connect(projectSaveAction, &QAction::triggered, this, &MainWindow::projectSaveActionTriggered);
    connect(projectExitAction, &QAction::triggered, this, &MainWindow::projectExitActionTriggered);
    connect(addClassItemAction, &QAction::triggered, this, &MainWindow::addClassItemActionTriggered);
    connect(addAssociationItemAction, &QAction::triggered, this, &MainWindow::addAssociationItemActionTriggered);
    connect(generateCodeActvPrjAction, &QAction::triggered, this, &MainWindow::genCodeActvPrjActionTriggered);
    connect(generateCodeAllPrjsAction, &QAction::triggered, this, &MainWindow::genCodeAllPrjsActionTriggered);
    connect(diagramShowGridAction, &QAction::triggered, this, &MainWindow::showGridActionTriggered);
    connect(diagramSnapGridAction, &QAction::triggered, this, &MainWindow::snapGridActionTriggered);
    connect(m_pGraphicsView, &GraphicsView::mouseViewPressed, this, &MainWindow::addClassItem);
    connect(m_pGraphicsView, &GraphicsView::classItemDeleted, m_pProjectItems, &TreeWidget::removeItem);
    connect(m_pGraphicsView, &GraphicsView::escapePressed, this, &MainWindow::drawItemCanceled);
    
    /* These two connections are to monitor the active project using tree-widget */
    connect(m_pProjectItems, &TreeWidget::activeProjectChanged, m_pProjectManagement, &ProjectManagement::setActiveProject);
    connect(m_pProjectManagement, &ProjectManagement::activeProjectChanged, this, &MainWindow::updateGUIForActiveProject);
    connect(showHideMonitor, &QAction::triggered, this, &MainWindow::toggleMonitorWindowActionTriggered);
    connect(MonitorManager::getInstance(), &Monitor::showWindow, this, &MainWindow::toggleMonitorWindowActionTriggered);
    /* For renaming the item */
    connect(m_pProjectItems, &TreeWidget::itemRenamed, this, &MainWindow::itemRenamed);
}

/*!
 * \brief This function creates the connections between new created class item and tree widget. */
void MainWindow::createClassItemConnections(ClassItem *classItem)
{
    /* Set connections between the new class item and other objects */
    /* Create a connection to tell to treewidget that an item is renamed or deleted, so it changes the name too */
    connect(classItem, &ClassItem::classItemDeleted, m_pProjectItems, &TreeWidget::removeItem);
    /* Create a connection for MoveCommand */
    connect(classItem, &ClassItem::itemMoved, this, &MainWindow::itemMoved);
    connect(classItem, &ClassItem::itemScaled, this, &MainWindow::itemScaled);
    /* For renaming the item */
    connect(classItem, &ClassItem::itemRenamed, this, &MainWindow::itemRenamed);
    /* To activate the save button, if the property will be changed */
    connect(classItem, &ClassItem::itemPropertyChanged, this, &MainWindow::setGUIDirty);
}

/*!
 * \brief This function sets the toolbar states to default state; which is none element is pressed.
 * If the user presses an element in toolbar, and then escape, cancel the adding the item. */
void MainWindow::drawItemCanceled()
{
    /* Change the toolbar icon to free */
    addClassItemAction->setChecked(false);
    addAssociationItemAction->setChecked(false);
    /* Tell the scene that adding an association is canceled */
    m_pProjectManagement->getActiveProject()->scene()->addItemCanceled();
}

/*!
 * \brief This function will be called, when an association item will be drawn. */
void MainWindow::drawItemFinished()
{
    addAssociationItemAction->setChecked(false);
}

/*!
 * \brief This function creates a new MoveCommand. */
void MainWindow::itemMoved(ClassItem *classItem)
{
    /* Adding the new item via undo-framework - see redo() for classitem adding */
    nsClassItem::MoveCommand *command = new nsClassItem::MoveCommand(classItem);
    m_pProjectManagement->getActiveProject()->undoStack()->push(command);
}

/*!
 * \brief This function creates a new ResizeCommand. */
void MainWindow::itemScaled(ClassItem *classItem)
{
    /* Adding the new item via undo-framework - see redo() for classitem adding */
    nsClassItem::ResizeCommand *command = new nsClassItem::ResizeCommand(classItem);
    m_pProjectManagement->getActiveProject()->undoStack()->push(command);
}

/*!
 * \brief This function creates a new ResizeCommand. */
void MainWindow::itemRenamed(const QString &oldName, const QString &newName)
{
    m_pProjectManagement->getActiveProject()->renameItem(oldName, newName);
    m_pProjectItems->changeItemName(oldName, newName);
}

/*!
 * \brief This function generates code for each class item for the active project. */
void MainWindow::genCodeActvPrjActionTriggered()
{
    /* Send the info to monitor - convert QString to char * */
    MonitorManager::getInstance()->logMsg("INFO : Generating code...", 0);
    // TODO : menü item generate all for generating all project at once
    /* Generate the code */
    m_pProjectManagement->getActiveProject()->generateCode();
}

/*!
 * \brief This function generates code for each class item for all projects. */
void MainWindow::genCodeAllPrjsActionTriggered()
{
    /* TODO genCodeAllPrjsActionTriggered() */
    qDebug() << "Not implemented yet";
}


/*!
 * \brief This function will create a new project and change the view of the main window. 
 * Before the main window will be totally created, there are only two options: create a new project
 * or load a project from the file system. After a project will be created or loaded, the window will 
 * jump to its normal state, which has all the menü items and toolbars. */
void MainWindow::projectNewActionTriggered()
{
    /* Create a new scene for each project */
    GraphicsScene *scene = new GraphicsScene(m_pGraphicsView);
    
    /* Always set the scene rect */
    QSize size = m_pPrinter->paperSize(QPrinter::Point).toSize();
    scene->setSceneRect(0, 0, size.width(), size.height());
    /* Create the project with its scene */
    if (m_pProjectManagement->createNewProject(scene))
    {
        /* Add the project to tree widget */
        QString prjName = m_pProjectManagement->getActiveProjectName();
        m_pTopLevelComponent = new QTreeWidgetItem(QStringList() << prjName);
        m_pProjectItems->addTopLevelItem(m_pTopLevelComponent);
        /* We must tell treewidget that a new project has been inserted, to set it as active project */
        m_pProjectItems->setActiveProject(prjName);
        /* Add these connections for the communication between tree widget and the project */
        Project *proj = m_pProjectManagement->getActiveProject();
        /* Add gridlines to the project */
        proj->createGridlines(m_pPrinter);
        /* Show them as default */
        proj->showGridlines(true);
        /* Connect the signal of the project with tree widget for dirty-star */
        connect(proj, &Project::setProjectDirty, m_pProjectItems, &TreeWidget::setProjectDirty);
        /* Save the project and database files */
        proj->saveTheSceneToDatabase();
        saveProjectFile(proj);
        /* Add the scene to the view */
        m_pGraphicsView->setScene(scene);
        /* Create undostack connections for the new created project */
        connect(proj->undoStack(), &QUndoStack::indexChanged, this, &MainWindow::updateActions);
        connect(proj->undoStack(), &QUndoStack::cleanChanged, this, &MainWindow::updateActions);
        /* Add to the undogroup the new undostack */
        m_pUndoGroup->addStack(proj->undoStack());
        /* Set the active stack for undo-redo */
        m_pUndoGroup->setActiveStack(proj->undoStack());
        /* Create scene-mainwindow connection */
        connect(scene, &GraphicsScene::resetAssociationToolbar, this, &MainWindow::drawItemFinished);
        /* Status bar */
        statusBar()->showMessage(tr("New project created..."));
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("INFO : Project %0 created", 1, prjName.toLocal8Bit().data());
        /* Update the actions */
        updateActions();
    }
    else
    {
        /* Project could not be created successfully - delete scene */
        delete scene;
        scene = NULL;
    }
}

/*!
 * \brief This function will load a project from a given file. 
 * Before the main window will be totally created, there are only two options: create a new project
 * or load a project from the file system. After a project will be created or loaded, the window will 
 * jump to its normal state, which has all the menü items and toolbars. */
void MainWindow::projectLoadActionTriggered()
{
    // TODO change directory
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load a project"), QString("D:\\Temp\\test"), tr("pj file (*.pj)"));

    /* Read project info from the project file .pj */
    if (fileName.isEmpty())
        return;

    /* Send the info to monitor - convert QString to char * */
    MonitorManager::getInstance()->logMsg("INFO : Loading project %0", 1, fileName.toLocal8Bit().data());

    QString name, path;
    int width = 0;
    int height = 0;
    int showGrid = 0;
    int snapGrid = 0;
    
    /* Shall we show the debug info? */
    bool bDebugChecked = MonitorManager::getInstance()->isDebugChecked();
            
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QXmlStreamReader xmlReader(&file);
    bool isProject = false;
    bool canReadElements = false;
    do
    {
        xmlReader.readNext();
        if (xmlReader.isStartElement())
        {
            if (!isProject)
            {
                /* Check if the project file contains <PROJECT> in first line */
                if (xmlReader.name() == "PROJECT")
                {
                    isProject = true;
                    continue;
                }
                else
                {
                    /* Send the info to monitor */
                    MonitorManager::getInstance()->logMsg("ERROR : Not a project file", 0);
                    return;
                }
            }
            
            if (isProject && !canReadElements)
            {
                /* Check for properties entry in xml <PROPERTIES>*/
                if (xmlReader.name() == "PROPERTIES")
                {
                    canReadElements = true;
                    continue;
                }
                else
                {
                    /* Send the info to monitor */
                    MonitorManager::getInstance()->logMsg("ERROR : Properties not defined", 0);
                    return;
                }
            }
            
            /* The text items are under PROPERTIES */
            if (canReadElements)
            {
                if (bDebugChecked)
                    /* Send the info to monitor - convert QString to char * */
                    MonitorManager::getInstance()->logMsg("DEBUG : Item %0 found in file", 1, xmlReader.name().toString().toLocal8Bit().data());

                if(xmlReader.name() == "Name")
                {
                    name = xmlReader.readElementText();
                }
                else if(xmlReader.name() == "Path")
                {
                    path = xmlReader.readElementText();
                }
                else if(xmlReader.name() == "Width")
                {
                    width = xmlReader.readElementText().toInt();
                }
                else if(xmlReader.name() == "Height")
                {
                    height = xmlReader.readElementText().toInt();
                }
                else if(xmlReader.name() == "ShowGrid")
                {
                    showGrid = xmlReader.readElementText().toInt();
                }
                else if(xmlReader.name() == "SnapGrid")
                {
                    snapGrid = xmlReader.readElementText().toInt();
                }
                else
                {
                    /* Send the info to monitor */
                    MonitorManager::getInstance()->logMsg("ERROR : Wrong information in project property", 0);
                }
            }
        }
    } while (!xmlReader.atEnd());
    
    file.close();
    if (xmlReader.hasError())
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Failed to parse file %0 : %1", 2, fileName.toLocal8Bit().data(), xmlReader.errorString().toLocal8Bit().data());
    else if (file.error() != QFile::NoError)
        /* Send the info to monitor */
        MonitorManager::getInstance()->logMsg("ERROR : Can not read file %0 : %1", 2, fileName.toLocal8Bit().data(), file.errorString().toLocal8Bit().data());
    
    /* Test if this project already loaded */
    if (m_pProjectManagement->isThisAProjectName(name))
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Project is already loaded. Canceling!", 0);
        return;
    }
   
    /* Send the info to monitor - convert QString to char * */
    MonitorManager::getInstance()->logMsg("INFO : Project read successfully", 0);

    /* Create a new scene for each project */
    GraphicsScene *scene = new GraphicsScene(m_pGraphicsView);
    /* Create scene-mainwindow connection */
    connect(scene, &GraphicsScene::resetAssociationToolbar, this, &MainWindow::drawItemFinished);
    /* Always set the scene rect */
    QSize size = m_pPrinter->paperSize(QPrinter::Point).toSize();
    scene->setSceneRect(0, 0, size.width(), size.height());
    /* Create a project object with scene */
    Project *loadProject = new Project(name, path, width, height, scene);
    /* Register it to project management */    
    m_pProjectManagement->loadNewProject(loadProject);
    /* Add the project to tree widget */
    m_pTopLevelComponent = new QTreeWidgetItem(QStringList() << loadProject->projectName());
    m_pProjectItems->addTopLevelItem(m_pTopLevelComponent);
    /* We must tell treewidget that a new project has been inserted, to set it as active project */
    m_pProjectItems->setActiveProject(loadProject->projectName());
    /* Add these connections for the communication between tree widget and the project */
    connect(loadProject, &Project::setProjectDirty, m_pProjectItems, &TreeWidget::setProjectDirty);
    /* Create the grid first, otherwise the lines or over the items */
    loadProject->createGridlines(m_pPrinter);
    /* Show the grid if it was true as the user saved the project last time */
    loadProject->showGridlines(showGrid == 1);
    /* Update GUI - Menu item show grid */
    diagramShowGridAction->setChecked(showGrid == 1);
    /* Snap to grid if it was true as the user saved the project */
    loadProject->setSnapGrid(snapGrid == 1);
    /* Update GUI - Menu item snap grid */
    diagramSnapGridAction->setChecked(snapGrid == 1);
    
    /* Load all the items (first classes then relations) to scene */
    QList<QGraphicsObject *> items;
    if (!loadProject->loadItemsToScene(items))
    {
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("ERROR : Can not load the project %0", 1,
                                              loadProject->projectName().toLocal8Bit().data());
        return;
    }
    
    /* Load class items and relation items */
    foreach(QGraphicsObject *cItem, items)
    {
        /* If this is a classitem */
        if (ClassItem *classItem = dynamic_cast<ClassItem*>(cItem))
        {
            /* Create for this item the connections */    
            createClassItemConnections(classItem);
            /* Create new treeitem for tree widget */
            QTreeWidgetItem *treeItem = new QTreeWidgetItem(QStringList() << classItem->className());
            /* Add the item */
            m_pTopLevelComponent->addChild(treeItem);
        
            if (bDebugChecked)
                /* Send the info to monitor - convert QString to char * */
                MonitorManager::getInstance()->logMsg("DEBUG : Item %0 loaded succesfully", 1,
                                                      classItem->getGeneralProperties()->name.toLocal8Bit().data());
        }
        /* The class items will be loaded first, then the association items : therefore the order is ok */
        else if (AssociationItem *assoItem = dynamic_cast<AssociationItem*>(cItem))
        {
            GraphicsScene *scene = loadProject->scene();
            
            /* Create connection to classitems for move and resize signals */
            connect(assoItem->getItemBegin(), &ClassItem::itemMoving, scene, &GraphicsScene::redrawAssociation);
            connect(assoItem->getItemEnd(), &ClassItem::itemMoving, scene, &GraphicsScene::redrawAssociation);
            connect(assoItem->getItemBegin(), &ClassItem::itemResizing, scene, &GraphicsScene::redrawAssociation);
            connect(assoItem->getItemEnd(), &ClassItem::itemResizing, scene, &GraphicsScene::redrawAssociation);
            
            /* Create connection for scene item relation */
            QObject::connect(scene, &GraphicsScene::mousePressed, assoItem, &AssociationItem::mousePressed);
            QObject::connect(scene, &GraphicsScene::mouseReleased, assoItem, &AssociationItem::mouseReleased);
            QObject::connect(scene, &GraphicsScene::mouseMoved, assoItem, &AssociationItem::mouseMoved);
            QObject::connect(assoItem, &AssociationItem::deleteMe, scene, &GraphicsScene::deleteAssoItem);
        }
        
        /* Add the item to the scene */
        loadProject->scene()->addItem(cItem);
    }
    
    /* Update GUI */
    updateGUIForActiveProject(loadProject);
    /* Create undostack connections for the new created project */
    connect(loadProject->undoStack(), SIGNAL(indexChanged(int)), this, SLOT(updateActions()));
    connect(loadProject->undoStack(), SIGNAL(cleanChanged(bool)), this, SLOT(updateActions()));
    /* Update the actions */
    updateActions();
    /* Send the info to monitor - convert QString to char * */
    MonitorManager::getInstance()->logMsg("INFO : Project %0 loaded successfully", 1, fileName.toLocal8Bit().data());
    statusBar()->showMessage(tr("Project loaded"));
}

/*!
 * \brief This function saves the database and project file. */
void MainWindow::projectSaveActionTriggered()
{
    /* Send the info to monitor */
    MonitorManager::getInstance()->logMsg("INFO : Saving project!", 0);    

    /* Now the items must be added to database */
    Project *project = m_pProjectManagement->getActiveProject();
    project->saveTheSceneToDatabase();

    /* Save the project file .pj in xml format */
    saveProjectFile(project);
    
    /* Send the info to monitor - convert QString to char * */
    MonitorManager::getInstance()->logMsg("INFO : Project %0 saved successfully", 1, project->projectName().toLocal8Bit().data());

    statusBar()->showMessage(tr("Project saved successfully..."));
}

/*!
 * \brief This function closes the application. */
void MainWindow::projectExitActionTriggered()
{
    // TODO : check if saved data exists before close
    
    /* Send the info to monitor */
    MonitorManager::getInstance()->logMsg("INFO : Closing application...", 0);  
    
    statusBar()->showMessage(tr("Closing..."));
    close();
}

/*!
 * \brief This function saves the project informations in xml Format. */
void MainWindow::saveProjectFile(Project *project)
{
    QString fullPath = project->fileNameFullPath(); 

    if (MonitorManager::getInstance()->isDebugChecked())
        /* Send the info to monitor - convert QString to char * */
        MonitorManager::getInstance()->logMsg("DEBUG : Full path of the project is %0", 1, fullPath.toLocal8Bit().data());    
    
    QFile file(fullPath);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QXmlStreamWriter xmlWriter(&file);
    xmlWriter.setAutoFormatting(true);
    xmlWriter.writeStartDocument();
    xmlWriter.writeStartElement("PROJECT");
    xmlWriter.writeStartElement("PROPERTIES");
    xmlWriter.writeTextElement("Name", project->projectName());
    xmlWriter.writeTextElement("Path", project->projectPath());
    xmlWriter.writeTextElement("Width", QString::number(project->width()));
    xmlWriter.writeTextElement("Height", QString::number(project->height()));
    xmlWriter.writeTextElement("ShowGrid", QString::number(project->isGridVisible()));
    xmlWriter.writeTextElement("SnapGrid", QString::number(project->snapGrid()));
    xmlWriter.writeEndElement(); // </PROPERTIES>
    xmlWriter.writeEndElement(); // </PROJECT>
    file.close();
    
    /* Set the last saved point in the stack */
    project->undoStack()->setClean();
    
    /* Update the actions */
    updateActions();
}

/*!
 * \brief This function set the project item and tree item to dirty, if in any property of the item
 * will be changed. */
void MainWindow::setGUIDirty()
{
    /* Tree widget will be informed via a connection in projectNewActionTriggered() */
    /* Show [*] in project name in the tree list */
    m_pProjectManagement->getActiveProject()->setDirty(true);
    /* Activate the save button */
    projectSaveAction->setEnabled(true);
}

/*!
 * \brief This function sets some window properties like size and title. */
void MainWindow::setWindowProperties()
{
    /* TODO resize window */
    resize(1200, 800);
    /* [*] will not be shown in the title - near the project names depending on project */
    setWindowTitle("fUML");
}

/*!
 * \brief This function shows the gridlines. */
void MainWindow::showGridActionTriggered(bool on)
{
    m_pProjectManagement->getActiveProject()->showGridlines(on);
}

/*!
 * \brief This function positions the items to gridlines. */
void MainWindow::snapGridActionTriggered(bool on)
{
    m_pProjectManagement->getActiveProject()->setSnapGrid(on);
}

/*!
 * \brief This function shows or hides the monitor window. */
void MainWindow::toggleMonitorWindowActionTriggered(bool on)
{
    if (on)
    {
        MonitorManager::getInstance()->show();
        showHideMonitor->setChecked(true);
    }
    else
    {
        MonitorManager::getInstance()->hide();
        showHideMonitor->setChecked(false);
    }
}

/*!
 * \brief This function updates the menu and toolbar items. */
void MainWindow::updateActions()
{
    Project *project = m_pProjectManagement->getActiveProject();
    /* If there is no project created yet */
    if (project == NULL)
    {
        /* Activate-Deactivate item : save */
        projectSaveAction->setEnabled(false);
        /* Activate-Deactivate item : add class item */
        addClassItemAction->setEnabled(false);
        /* Activate-Deactivate item : add association item */
        addAssociationItemAction->setChecked(false);
    }
    /* If there is min one project created */
    else
    {
        /* Activate-Deactivate item : save */
        projectSaveAction->setEnabled(!project->undoStack()->isClean());
        /* Activate-Deactivate item : add class item */
        addClassItemAction->setEnabled(true);
        /* Activate-Deactivate item : add association item */
        addAssociationItemAction->setEnabled(true);
        /* Show a [*] near the project name, if there is a change after last save-point */
        m_pProjectManagement->getActiveProject()->setDirty(!project->undoStack()->isClean());
    }
}

/*!
 * \brief This function changes the scene for the active project. */
void MainWindow::updateGUIForActiveProject(Project *project)
{
    /* Update the scene */
    m_pGraphicsView->setScene(project->scene());
    m_pGraphicsView->update();
    
    /* Update the tree widget */
    QList<QTreeWidgetItem*> items = m_pProjectItems->findItems(project->projectName().remove("[*]"), Qt::MatchContains);
    m_pTopLevelComponent = items.at(0);
    
    /* Update the undogroup - set active undostack */
    m_pUndoGroup->setActiveStack(project->undoStack());
    
    /* Update the actions for active project */
    updateActions();
}

TreeWidget *MainWindow::projectItems() const
{
    return m_pProjectItems;
}


