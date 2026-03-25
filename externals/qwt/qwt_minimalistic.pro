# Project file for qwt
# Library is expected to be placed in same directory as
# UitCore

TARGET = qwt
TEMPLATE = lib

# we want the lib as static library on Windows
win32 {
	CONFIG += shared
	DEFINES *= QWT_DLL
	DEFINES += QWT_MAKEDLL
}

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../IBK/IBK.pri )

CONFIG += console

# enable function/file info also in release mode
DEFINES += QT_MESSAGELOGCONTEXT
DEFINES += QWT_NO_SVG
DEFINES += QWT_NO_OPENGL

DEFINES += QWT_MOC_INCLUDE=1


QT += widgets
QT += printsupport
QT += concurrent

INCLUDEPATH = \
	src

HEADERS += \
	src/qwt.h \
	src/qwt_abstract_legend.h \
	src/qwt_axis.h \
	src/qwt_clipper.h \
	src/qwt_color_map.h \
	src/qwt_global.h \
	src/qwt_graphic.h \
	src/qwt_interval.h \
	src/qwt_legend_data.h \
	src/qwt_legend_label.h \
	src/qwt_null_paintdevice.h \
	src/qwt_painter.h \
	src/qwt_painter_command.h \
	src/qwt_scale_map.h \
	src/qwt_text.h \
	src/qwt_text_engine.h \
	src/qwt_text_label.h

SOURCES += \
	src/qwt.cpp \
	src/qwt_abstract_legend.cpp \
	src/qwt_clipper.cpp \
	src/qwt_color_map.cpp \
	src/qwt_graphic.cpp \
	src/qwt_interval.cpp \
	src/qwt_legend_data.cpp \
	src/qwt_legend_label.cpp \
	src/qwt_null_paintdevice.cpp \
	src/qwt_painter.cpp \
	src/qwt_painter_command.cpp \
	src/qwt_scale_map.cpp \
	src/qwt_text.cpp \
	src/qwt_text_engine.cpp \
	src/qwt_text_label.cpp


