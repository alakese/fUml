#include "gui/mainwindow/mainwindow.h"

#include <QApplication>

/*!
 * \mainpage fUML
 * \brief Organization, application : "F-Soft", "fUML"
 * \todo Description here.
 * \image html MainWindow.png 
*/
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("fUML");

    Q_INIT_RESOURCE(dockwidgets);
    
    MainWindow mainWin;
    mainWin.show();
    
    return app.exec();
}
