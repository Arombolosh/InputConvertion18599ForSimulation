# Qt Session file for shadow builds

TEMPLATE = subdirs

SUBDIRS = \
	IBK \
	TiCPP \
	InputConvertion18599ForSimulation

# where to find the sub projects
IBK.file = externals/IBK/IBK.pro
TiCPP.file = externals/TiCPP/TiCPP.pro
InputConvertion18599ForSimulation.file = InputConvertion18599ForSimulation/InputConvertion18599ForSimulation.pro

# dependencies
TiCPP.depends = IBK
InputConvertion18599ForSimulation.depends = IBK TiCPP
