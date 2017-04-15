#-------------------------------------------------
#
# Project created by QtCreator 2017-04-14T17:06:10
#
#-------------------------------------------------

QT += core gui
QT += xmlpatterns
QT += sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = googleScholarParser
TEMPLATE = app


SOURCES += main.cpp
SOURCES += gsp.cpp
SOURCES += publication.cpp

HEADERS  += gsp.h
HEADERS  += publication.h

FORMS    += gsp.ui
