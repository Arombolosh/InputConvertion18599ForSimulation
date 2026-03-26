# ------------------------------------------
# Project for InputConvertion18599ForSimulation
# ------------------------------------------

TARGET = InputConvertion18599ForSimulation
TEMPLATE = app

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../externals/IBK/IBK.pri )

QT += core gui widgets printsupport svg

INCLUDEPATH = \
	src \
	../externals/IBK/src \
	../externals/TiCPP/src \
	../externals/DataIO/src \
	../externals/qwt/src \
	../externals/QtExt/src \
	../externals/SciChart/src

DEPENDPATH = $${INCLUDEPATH}

LIBS += \
	-lSciChart \
	-lQtExt \
	-lqwt6 \
	-lDataIO \
	-lTiCPP \
	-lIBK

SOURCES += \
	src/main.cpp \
	src/IC18599MainWindow.cpp \
	src/IC18599NormDataWidget.cpp \
	src/IC18599DayProfileWidget.cpp \
	src/IC18599ScheduleEditWidget.cpp \
	src/IC18599Project.cpp \
	src/IC18599Report.cpp \
	src/IC18599ReportFrameTitlePage.cpp \
	src/IC18599ReportFrameProfilePage.cpp \
	src/IC18599ReportChartItem.cpp \
	src/IC18599ExportDialog.cpp

HEADERS += \
	src/IC18599MainWindow.h \
	src/IC18599NormDataWidget.h \
	src/IC18599DayProfileWidget.h \
	src/IC18599ScheduleEditWidget.h \
	src/IC18599Project.h \
	src/IC18599ReportSettings.h \
	src/IC18599Report.h \
	src/IC18599ReportFrameTitlePage.h \
	src/IC18599ReportFrameProfilePage.h \
	src/IC18599ReportChartItem.h \
	src/IC18599ExportDialog.h

FORMS += \
	src/IC18599MainWindow.ui \
	src/IC18599ScheduleEditWidget.ui
