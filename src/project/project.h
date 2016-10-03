#ifndef PROJECT_H
#define PROJECT_H

#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QGraphicsItemGroup;
class QPrinter;
class QUndoStack;
class QGraphicsObject;
QT_END_NAMESPACE
class GraphicsScene;
class SqlDatabase;
class ClassItem;
class ParameterProperties;

#include "../globals/properties.h"


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
    GraphicsScene *m_pScene;
    SqlDatabase *m_pSQLDatabase;
    bool m_bSetDirty;
    /* These items are deleted from the scene and will be deleted from the
       database by next save-action */
//    QList<QGraphicsObject*> m_pOnDeleteItems;
    /* Gridlines */
    QGraphicsItemGroup *m_pGridLines;
    int m_gridDensity;
    bool m_bSnapGrid;
    /* Undo stack */
    QUndoStack *m_pUndoStack;
    
public:
    Project(QString &, QString &, int, int, GraphicsScene *);
    void saveTheSceneToDatabase();
    bool find(const QString &);
    void renameItem(const QString &, const QString &);
    void removeClassItem(const QString &);
    void selectItem(const QString &);
    bool loadItemsToScene(QList<QGraphicsObject *> &/*out*/);
    void createGridlines(QPrinter *);
    void generateCode();
    ClassItem *getClassItem(const QString &);
    void setItemIdAsDbId(const QString &, int);
    void setMemberIdAsDbId(const QString &, const QString &, int mID);
    void setFunctionIdAsDbId(const QString &, const QString &, const QString &, const QList<ParameterProperties*>, int);
    void setParameterIdAsDbId(const QString &, const QString &, int, int);
//    void removeFromDeleteList(ClassItem *);
//    void addToDeleteList(QGraphicsObject *);
    void updateClassItem(ClassItem *, ClassItem *);
    
    /* Setters getters */
    QString fileNameFullPath();
    QString projectName();
    QString projectPath();    
    GraphicsScene *scene() const;
    QList<ClassItem*> *getClassItems();
    QList<int> getRelatedAssociationItems(int);
    void setScene(GraphicsScene *);
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

//private:
//    QString getMemberDefinitionsAsString(ClassItem *, Visibility);
//    QString getFunctionDefinitionsAsString(ClassItem *, Visibility);
//    QString getFunctionDeclarationsAsString(ClassItem *, Visibility);
//    QString getCtors(ClassItem*);
    
signals:
    void setProjectDirty(const QString &);
};

#endif // PROJET_H
