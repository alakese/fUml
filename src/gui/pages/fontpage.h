#ifndef FONTPAGE_H
#define FONTPAGE_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QMdiArea;
class QFont;
QT_END_NAMESPACE
class ClassItemDialog;

/*!
 * \class The FontPage class 
 * \brief Font-properties
 */
class FontPage : public QWidget
{
    Q_OBJECT

    QMdiArea *mdiArea;
    QFont oldFont;
    QFont newFont;
    
public:
    FontPage(const QFont &, ClassItemDialog *);
    bool checkTheFontState();
    QFont getFont() const;
    
public slots:
    void currentFontChanged(const QFont &, const QFont &);
    
signals:
    void checkOKState();
};

#endif // FONTPAGE_H
