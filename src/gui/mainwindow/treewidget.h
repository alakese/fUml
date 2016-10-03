#ifndef TREEWIDGET_H
#define TREEWIDGET_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QKeyEvent>


/*!
 * \class TreeWidget
 * \brief This class is a GUI class which holds the projects and its items in a tree-list widget.
 * This class sends some signals in order to communicate with other classes when some actions are
 * performed like renaming an item. */
class TreeWidget : public QTreeWidget
{
    Q_OBJECT
    
public:
    explicit TreeWidget(QWidget *parent = 0);
    void setActiveProject(const QString &);
    void changeItemName(const QString &, const QString &);
    
public slots:
    void setProjectDirty(const QString &);
    //void changeItemName(const QString &, const QString &);
    void removeItem(const QString &);
    
signals:
    /* Param : new project name*/
    void activeProjectChanged(QString);
    void itemRenamed(QString, QString);
    void itemDeleted(QString);

private:
    void contextMenuEvent(QContextMenuEvent *);
    void keyPressEvent(QKeyEvent *);
    
private slots:
    void deleteItem();
    void renameItem();
    void showOccurence();    
    void checkForProjectChange();
    
private:
    /* There is only one column, but we must give it as argument to some functions */
    enum { COLUMN_ZERO = 0 };
    QAction *actionDeleteItem;
    QAction *actionRenameItem;
    QAction *actionShowOccurrence;
    QString m_strActiveProjectName;
};

#endif // TREEWIDGET_H
