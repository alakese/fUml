#include "monitor.h"
#include "../../project/project.h"
#include "../../project/projectmanagement.h"
#include <QTextEdit>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QStatusBar>
#include <QFile>
#include <QDateTime>
#include <cstdarg>
#include <QDir>
#include <QCloseEvent>

#include <QDebug> // just info for not implemented functions


Monitor::Monitor(QWidget *parent) : QMainWindow(parent), m_bDebugChecked(false)
{
    /* Create central widget for the monitor */
    m_pTextEdit = new QTextEdit(this);
    m_pTextEdit->setReadOnly(true);
    setCentralWidget(m_pTextEdit);
    
    /* Monitor window settings */
    setWindowTitle("Monitor");
    resize(400, 200);
    
    /* Create actions */
    m_pActionOpen = new QAction(QIcon(), tr("Open..."), this);
    m_pActionSave = new QAction(QIcon(), tr("Save"), this);
    m_pActionSaveAs = new QAction(QIcon(), tr("Save as..."), this);
    m_pActionClose = new QAction(QIcon(), tr("Close"), this);
    m_pActionAlwaysTop = new QAction(QIcon(), tr("Always On Top"), this);
    m_pActionAlwaysTop->setCheckable(true);
    m_pActionShowDebug = new QAction(QIcon(), tr("Show Debug Info"), this);
    m_pActionShowDebug->setCheckable(true);
    
    /* Create menu */
    m_pMainMenu = menuBar()->addMenu(tr("&File"));
    m_pMainMenu->addAction(m_pActionOpen);
    m_pMainMenu->addAction(m_pActionSave);
    m_pMainMenu->addAction(m_pActionSaveAs);
    m_pMainMenu->addSeparator();
    m_pMainMenu->addAction(m_pActionClose);
    
    m_pMainMenu = menuBar()->addMenu(tr("&Edit"));
    m_pMainMenu->addAction(m_pActionAlwaysTop);
    m_pMainMenu->addAction(m_pActionShowDebug);
    
    /* Create connections */
    connect(m_pActionOpen, &QAction::triggered, this, &Monitor::openLog);
    connect(m_pActionSave, &QAction::triggered, this, &Monitor::saveLog);
    connect(m_pActionSaveAs, &QAction::triggered, this, &Monitor::saveAsLog);
    connect(m_pActionClose, &QAction::triggered, this, &Monitor::closeWindow);
    connect(m_pActionAlwaysTop, &QAction::triggered, this, &Monitor::alwaysOnTop);
    connect(m_pActionShowDebug, &QAction::triggered, this, &Monitor::showDebugInfo);
    
    /* Show the window */
    show();
    
    /* Status bar */
    statusBar()->showMessage(tr("Ready..."));
    
    /* Logfile name */
    m_strLogFileName = "Monitor.log"; // TODO where to store the log in installation pfad or project pfad
}

void Monitor::openLog()
{
    qDebug() << "Not implemented yet!";
}

void Monitor::saveLog()
{
    qDebug() << "Not implemented yet!";    
}

void Monitor::saveAsLog()
{
    qDebug() << "Not implemented yet!";    
}

void Monitor::closeWindow()
{
    /* Signal MainWindow to hide the monitor */
    emit showWindow(false);
}

void Monitor::alwaysOnTop(bool on)
{
    Q_UNUSED(on);
    
    // TODO This is internet solution - flashes the window when checked - see
    Qt::WindowFlags flags = windowFlags();
    if (on)
    {
        flags ^= Qt::WindowStaysOnBottomHint;
        flags |= Qt::WindowStaysOnTopHint;
    }
    else
    {
        flags ^= Qt::WindowStaysOnTopHint;
        flags |= Qt::WindowStaysOnBottomHint;
    }
    setWindowFlags(flags);
    show();
}

/*!
 * \brief This function sets the property "debug info" to true or false. If true then the debug information will be shown in monitor. */
void Monitor::showDebugInfo(bool on)
{
    m_bDebugChecked = on;
}

/*!
 * \brief This function sends a signal to main window to update the GUI. */
void Monitor::closeEvent(QCloseEvent *event)
{
    /* Tell main window that the monitor is closed, so the menu item can be updated */
    closeWindow();
    /* Close successfully */
    event->accept();
}

/*!
 * \brief This function gets an infinite number of arguments and print it to a logfile and to monitor window. */
void Monitor::logMsg(const char *_string, int numArgs, ...)
{
    /* We need two formats : one plain text and other html */
    QString logFile = _string;
    QString logMonitor = _string;
    /* Get the arguments and merge the string */
    va_list arguments;
    va_start(arguments, numArgs);
    for (int x = 0; x < numArgs; x++)
    {   
        char *msg = va_arg(arguments, char *);
        /* Wildcards are the placeholders in our strings like %0 */
        QString wildcard = QString("\%%1").arg(x);
        /* We need the argument in plain text */
        QString argLog(QString("\"%1\"").arg(msg));
        /* And in html for monitor - here name will be shown in italic format */
        QString argMon(QString("\"<i>%1</i>\"").arg(msg));
        /* Now merge our placeholders */
        logFile.replace(wildcard, argLog);
        logMonitor.replace(wildcard, argMon);
    }
    va_end ( arguments );
    
    /* Add the timestamp to messages */
    QDateTime timestamp(QDateTime::currentDateTime());
    
    /* Write the message to monitor with italic-name */
    QString info(QString("%1 (%2)").arg(logMonitor).arg(timestamp.toString()));
    m_pTextEdit->append(info);    
    
    /* Write the message to the log file : C:/Users/user/AppData/Local/Temp */
    QString strPath = QDir::tempPath();
    QString fullpath(strPath + "\\" + m_strLogFileName);
    
    /* Open file for append */
    QFile file(fullpath);
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    QTextStream log(&file);
    /* Write to file */
    QString info2(QString("%1 at %2").arg(logFile).arg(timestamp.toString()));
    log << info2 << "\n";
}



/* ----------------------------------------------- */
/* All the functions below are setters and getters */
/* ----------------------------------------------- */

bool Monitor::isDebugChecked() const
{
    return m_bDebugChecked;
}

void Monitor::setDebugChecked(bool bDebugChecked)
{
    m_bDebugChecked = bDebugChecked;
}

/* ----------------------------------------------- */
