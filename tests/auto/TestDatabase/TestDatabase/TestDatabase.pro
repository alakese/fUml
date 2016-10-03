#-------------------------------------------------
#
# Project created by QtCreator 2014-06-22T16:55:32
#
#-------------------------------------------------

QT       += gui widgets sql testlib printsupport

TARGET = tst_testdatabasetest

TEMPLATE = app

HEADERS     +=  project/project.h \
                project/projectmanagement.h \
                db/sqldatabase.h \
                globals/properties.h \
                globals/globals.h \
                graphics/items/classitem.h

SOURCES     +=  project/project.cpp \
                project/projectmanagement.cpp \
                graphics/items/classitem.cpp \
                db/sqldatabase.cpp \
                tst_testdatabasetest.cpp

LIBS += -L$$PWD/Libs/ -lSyntaxChecker

DEFINES += SRCDIR=\\\"$$PWD/\\\"
