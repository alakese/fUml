#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsItemGroup;
class QPrinter;
class QUndoStack;
QT_END_NAMESPACE
class SqlDatabase;
class ClassItem;
class ParameterProperties;

/*!
 * \class Project
 * \brief This class holds a project information. Through this class the scene infotmation can be saved in database. */
class Project : public QObject
{
    Q_OBJECT
    
    QString m_strProjectName;
    QString m_strProjectPath;
    int m_width;
    int m_height;
    QGraphicsScene *m_pScene;
    SqlDatabase *m_pSQLDatabase;
    bool m_bSetDirty;
    /* These items are deleted from the scene and will be deleted from the
       database by next save-action */
    QList<ClassItem*> m_pOnDeleteItems;
    /* Gridlines */
    QGraphicsItemGroup *m_pGridLines;
    int m_gridDensity;
    bool m_bSnapGrid;
    /* Undo stack */
    QUndoStack *m_pUndoStack;
    
public:
    Project(QString &, QString &, int, int, QGraphicsScene *);
    void saveTheSceneToDatabase();
    bool find(const QString &);
    void renameItem(const QString &, const QString &);
    void deleteItem(const QString &);
    void selectItem(const QString &);
    bool loadItemsToScene(QList<ClassItem *> &/*out*/);
    void createGridlines(QPrinter *);
    void generateCode();
    ClassItem *getClassItem(const QString &);
    void setItemIdAsDbId(const QString &, int);
    void setMemberIdAsDbId(const QString &, const QString &, int mID);
    void setFunctionIdAsDbId(const QString &, const QString &, const QString &, const QList<ParameterProperties*>, int);
    void setParameterIdAsDbId(const QString &, const QString &, int, int);
    void removeFromDeleteList(ClassItem *);
    void addToDeleteList(ClassItem *);
    void updateClassItem(ClassItem *, ClassItem *);

    /* Setters getters */
    QString fileNameFullPath();
    QString projectName();
    QString projectPath();    
    QGraphicsScene *scene() const;
    QList<ClassItem*> *getClassItems();
    void setScene(QGraphicsScene *);
    bool dirty() const;
    void setDirty(bool);
    int width() const;
    void setWidth(int width);
    int height() const;
    void setHeight(int height);
    QString databasePath();
    void toString();
    void showGridlines(bool);
    bool isGridVisible();
    void setSnapGrid(bool);
    bool snapGrid();
    int gridDensity() const;
    int getLastInstanceNo();
    QUndoStack *undoStack() const;
    
signals:
    void setProjectDirty(const QString &);
};

#endif // PROJET_H
