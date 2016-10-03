#-------------------------------------------------
#
# Project created by QtCreator 2014-08-16T17:29:54
#
#-------------------------------------------------

QT       -= gui

TARGET = SyntaxChecker
TEMPLATE = lib

DEFINES += SYNTAXCHECKER_LIBRARY

SOURCES += syntaxchecker.cpp

HEADERS += syntaxchecker.h\
        syntaxchecker_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
