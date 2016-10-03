#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>

/*!
 * \brief The GraphicsView class */
class GraphicsView : public QGraphicsView
{
    Q_OBJECT
    
    // TODO zoom and wheel
public:
    explicit GraphicsView(QWidget *parent=0);
    void setAddingItem(bool value);

signals:
    void mouseViewPressed(const QString &, const QPoint &);
    void classItemDeleted(const QString &);
    void escapePressed();
    
protected:
    void mousePressEvent(QMouseEvent *);
    void keyPressEvent(QKeyEvent *);
    
public:
    bool m_bAddingClassItem;
};

#endif // GRAPHICSVIEW_H
