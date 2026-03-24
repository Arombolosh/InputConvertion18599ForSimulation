# ------------------------------------------
# Project for InputConvertion18599ForSimulation
# ------------------------------------------

TARGET = InputConvertion18599ForSimulation
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../externals/IBK/IBK.pri )

QT += core gui widgets

INCLUDEPATH = \
	../externals/IBK/src \
	../externals/TiCPP/src

DEPENDPATH = $${INCLUDEPATH}

LIBS += \
	-lIBK \
	-lTiCPP

SOURCES += \
	src/main.cpp \
	src/IC18599MainWindow.cpp \
	src/IC18599NormDataWidget.cpp \
	src/IC18599DayProfileWidget.cpp \
	src/IC18599ScheduleEditWidget.cpp \
	src/IC18599Project.cpp

HEADERS += \
	src/IC18599MainWindow.h \
	src/IC18599NormDataWidget.h \
	src/IC18599DayProfileWidget.h \
	src/IC18599ScheduleEditWidget.h \
	src/IC18599Project.h
