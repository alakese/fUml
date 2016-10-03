#ifndef MONITOR_H
#define MONITOR_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QTextEdit;
QT_END_NAMESPACE


class Monitor : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Monitor(QWidget *parent = 0);
    void logMsg(const char *, int, ...);
    bool isDebugChecked() const;
    void setDebugChecked(bool);
    
signals:
    void showWindow(bool);
    
private slots:
    void openLog();
    void saveLog();
    void saveAsLog();
    void closeWindow();
    void alwaysOnTop(bool);
    void showDebugInfo(bool);

protected:
    void closeEvent(QCloseEvent *);
    
private:
    QTextEdit *m_pTextEdit;
    QMenu *m_pMainMenu;
    QAction *m_pActionOpen;
    QAction *m_pActionSave;
    QAction *m_pActionSaveAs;
    QAction *m_pActionClose;
    QAction *m_pActionAlwaysTop;
    QAction *m_pActionShowDebug;
    QString m_strLogFileName;
    /* Show debug-info in monitor and in log */
    bool m_bDebugChecked; // TODO orginize debug check
};

/*!
 * \brief The MonitorManager class */
class MonitorManager
{
    MonitorManager() { }
    ~MonitorManager() { }
    
public:
    static Monitor *getInstance()
    {
        static Monitor mon;
        return &mon;
    }
};

#endif // MONITOR_H
