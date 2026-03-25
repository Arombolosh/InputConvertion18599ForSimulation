# Qt Session file for shadow builds

TEMPLATE = subdirs

SUBDIRS = \
	IBK \
	TiCPP \
	DataIO \
	qwt \
	QtExt \
	SciChart \
	InputConvertion18599ForSimulation

# where to find the sub projects
IBK.file = externals/IBK/IBK.pro
TiCPP.file = externals/TiCPP/TiCPP.pro
DataIO.file = externals/DataIO/DataIO.pro
qwt.file = externals/qwt/qwt.pro
QtExt.file = externals/QtExt/QtExt.pro
SciChart.file = externals/SciChart/SciChart.pro
InputConvertion18599ForSimulation.file = InputConvertion18599ForSimulation/InputConvertion18599ForSimulation.pro

# dependencies
TiCPP.depends = IBK
DataIO.depends = IBK
QtExt.depends = IBK
SciChart.depends = IBK TiCPP DataIO qwt QtExt
InputConvertion18599ForSimulation.depends = IBK TiCPP DataIO qwt QtExt SciChart
