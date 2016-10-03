#ifndef FONTDIALOG_H
#define FONTDIALOG_H

#include <QFontDialog>

class FontPage;

class FontDialog : public QFontDialog
{
    Q_OBJECT

    FontPage *page;
    QFont oldFont;
    
public:
    FontDialog(FontPage *, const QFont &, QWidget *parent = 0);
    
protected:
    void keyPressEvent(QKeyEvent *);
    
private slots:
    void fontChanged(const QFont &);
    
signals:
    void signalFontChanged(const QFont &, const QFont &);
};

#endif // FONTDIALOG_H
