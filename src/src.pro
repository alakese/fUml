QT          += gui widgets sql printsupport
HEADERS     +=  \
                # GLOBALS
                globals/globals.h \
                # PROJECT
                project/project.h \
                project/projectmanagement.h \
                project/generatecode.h \
                # CODE ENTRY
                codeentry/codewindow.h \
                codeentry/wordhighlighter.h \
                # GRAPHICS
                graphics/graphicsview.h \
                graphics/graphicsscene.h \
                graphics/items/classitem.h \
                graphics/items/associationitem.h \
                graphics/items/literalitem.h \
                # DB
                db/sqldatabase.h \
                # GUI / DIALOG
                gui/dialogs/headerinfodlg.h \
                gui/dialogs/projectinputdialog.h \
                gui/dialogs/classitemdialog.h \
                gui/dialogs/addeditmemberdialog.h \
                gui/dialogs/addeditfunctiondialog.h \
                gui/dialogs/addeditparameterdialog.h \
                # GUI / PAGES
                gui/pages/generalpage.h \
                gui/pages/memberspage.h \
                gui/pages/functionspage.h \
                # GUI / MONITOR
                gui/monitor/monitor.h \
                # GUI / MAINWINDOW
                gui/mainwindow/mainwindow.h \
                gui/mainwindow/treewidget.h \
                globals/properties.h \
                gui/pages/fontpage.h \
                gui/pages/displayspage.h \
                gui/pages/propertiespage.h \
                gui/dialogs/fontdialog.h \
                # UNDO
                undo/classitem/nsClassItem_addcommand.h \
                undo/assoitem/nsAssociationItem_addcommand.h \
                undo/classitem/nsClassItem_resizecommand.h \
                undo/classitem/nsClassItem_movecommand.h \
                undo/classitem/nsClassItem_deletecommand.h \
                undo/classitem/nsClassItem_changefontcommand.h \
    undo/assoitem/nsAssociationItem_deletecommand.h \
    undo/assoitem/nsAssociationItem_movecommand.h \
    undo/assoitem/nsAssociationItem_addbreakpointcommand.h \
    undo/assoitem/nsAssociationItem_deletebreakpointcommand.h \
    undo/assoitem/nsAssociationItem_addliteralcommand.h \
    undo/assoitem/nsAssociationItem_deleteliteralcommand.h

SOURCES     +=  main.cpp \
                # PROJECT
                project/project.cpp \
                project/projectmanagement.cpp \
                project/generatecode.cpp \
                # CODE ENTRY
                codeentry/codewindow.cpp \
                codeentry/wordhighlighter.cpp \
                # GRAPHICS
                graphics/graphicsview.cpp \
                graphics/graphicsscene.cpp \
                graphics/items/classitem.cpp \
                graphics/items/associationitem.cpp \
                graphics/items/literalitem.cpp \
                # DB
                db/sqldatabase.cpp \
                # GUI / DIALOG
                gui/dialogs/headerinfodlg.cpp \
                gui/dialogs/projectinputdialog.cpp \
                gui/dialogs/classitemdialog.cpp \
                gui/dialogs/addeditmemberdialog.cpp \
                gui/dialogs/addeditfunctiondialog.cpp \
                gui/dialogs/addeditparameterdialog.cpp \
                # GUI / PAGES
                gui/pages/generalpage.cpp \
                gui/pages/memberspage.cpp \
                gui/pages/functionspage.cpp \
                # GUI / MONITOR
                gui/monitor/monitor.cpp \
                # GUI / MAINWINDOW
                gui/mainwindow/mainwindow.cpp \
                gui/mainwindow/treewidget.cpp \
                gui/pages/fontpage.cpp \
                gui/pages/displayspage.cpp \
                gui/pages/propertiespage.cpp \
                gui/dialogs/fontdialog.cpp \
                # UNDO
                undo/classitem/nsClassItem_addcommand.cpp \
                undo/assoitem/nsAssociationItem_addcommand.cpp \
                undo/classitem/nsClassItem_changefontcommand.cpp \
                undo/classitem/nsClassItem_movecommand.cpp \
                undo/classitem/nsClassItem_deletecommand.cpp \
                undo/classitem/nsClassItem_resizecommand.cpp \
    undo/assoitem/nsAssociationItem_deletecommand.cpp \
    undo/assoitem/nsAssociationItem_movecommand.cpp \
    undo/assoitem/nsAssociationItem_addbreakpointcommand.cpp \
    undo/assoitem/nsAssociationItem_deletebreakpointcommand.cpp \
    undo/assoitem/nsAssociationItem_addliteralcommand.cpp \
    undo/assoitem/nsAssociationItem_deleteliteralcommand.cpp

RESOURCES   += resources/dockwidgets.qrc

LIBS += -L$$PWD/Libs/ -lSyntaxChecker

OTHER_FILES += \
    ../../Lotfusspunkt.pdf
