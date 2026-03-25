# ---------------------------------------------------
# Project for SciChart library
# ---------------------------------------------------
TARGET = SciChart
TEMPLATE = lib

# this pri must be sourced from all our libraries,
# it contains all functions defined for casual libraries
include( ../IBK/IBK.pri )

QT += core gui svg printsupport widgets printsupport

unix|mac {
	VER_MAJ = 2
	VER_MIN = 1
	VER_PAT = 0
	VERSION = $${VER_MAJ}.$${VER_MIN}.$${VER_PAT}
}

win32 {
	# on Windows, when building as Dll, also link EmfEngine
	# comment next lines out, if building without EmfEngine
	DEFINES += SCICHART_HAS_EMFENGINE
	LIBS += -lQtExt \
			-lIBK \
			-lTiCPP \
			-lDataIO \
			-lqwt6 \
			-lEmfEngine

	# all the DLLs we link to
	#DEFINES += QtExt_DLL
	#DEFINES += EMFENGINE_DLL
	#DEFINES += QWT_DLL
}

INCLUDEPATH += ui \
	src \
	../EmfEngine/src \
	../QtExt/src \
	../DataIO/src \
	../TiCPP/src \
	../qwt/src \
	../IBK/src

DEPENDPATH = $${INCLUDEPATH}

HEADERS += \
	src/Sci_AbstractBarSeriesModel.h \
	src/Sci_AbstractCartesianChartModel.h \
	src/Sci_AbstractChartModel.h \
	src/Sci_AbstractColorGridSeriesModel.h \
	src/Sci_AbstractLineSeriesModel.h \
	src/Sci_AbstractVectorFieldSeriesModel.h \
	src/Sci_BarSeries.h \
	src/Sci_BarSeriesContentModel.h \
	src/Sci_Chart.h \
	src/Sci_ChartAxis.h \
	src/Sci_ChartFormatSelectionDialog.h \
	src/Sci_ChartFrame.h \
	src/Sci_ChartRenderWidget.h \
	src/Sci_ChartSeries.h \
	src/Sci_ColorEditWidget.h \
	src/Sci_ColorGrid3DData.h \
	src/Sci_ColorGridSeries.h \
	src/Sci_ColorMap.h \
	src/Sci_ColorMapCreator.h \
	src/Sci_ColorMapEditWidget.h \
	src/Sci_CurveTracker.h \
	src/Sci_DataReductionCurveFitter.h \
	src/Sci_DateScaleDraw.h \
	src/Sci_DateScaleEngine.h \
	src/Sci_Export2BitmapWidget.h \
	src/Sci_Export2VectorWidget.h \
	src/Sci_ExportDialog.h \
	src/Sci_Globals.h \
	src/Sci_Legend.h \
	src/Sci_LegendItem.h \
	src/Sci_LegendItemMover.h \
	src/Sci_LineSeries.h \
	src/Sci_LineSeriesContentModel.h \
	src/Sci_LineSeriesSelectionComboBox.h \
	src/Sci_Marker.h \
	src/Sci_PenStyleComboBox.h \
	src/Sci_PenStyleDelegate.h \
	src/Sci_PlotCurve.h \
	src/Sci_PlotLayout.h \
	src/Sci_PlotMarker.h \
	src/Sci_PlotPanner.h \
	src/Sci_PlotPicker.h \
	src/Sci_PlotPickerText.h \
	src/Sci_PlotRescaler.h \
	src/Sci_PlotScaleItem.h \
	src/Sci_PlotSpectrogram.h \
	src/Sci_PlotVectorField.h \
	src/Sci_PlotZoomer.h \
	src/Sci_ReportFrameItemBarDataChart.h \
	src/Sci_ReportFrameItemChart.h \
	src/Sci_ReportFrameItemLineDataChart.h \
	src/Sci_ReportFrameItemTimeLineDataChart.h \
	src/Sci_SeriesDataEmpty.h \
	src/Sci_SeriesListDelegate.h \
	src/Sci_SeriesListModel.h \
	src/Sci_StopPointDelegate.h \
	src/Sci_Utils.h \
	src/Sci_VectorFieldSeries.h

SOURCES += \
	src/Sci_AbstractBarSeriesModel.cpp \
	src/Sci_AbstractCartesianChartModel.cpp \
	src/Sci_AbstractChartModel.cpp \
	src/Sci_AbstractColorGridSeriesModel.cpp \
	src/Sci_AbstractLineSeriesModel.cpp \
	src/Sci_AbstractVectorFieldSeriesModel.cpp \
	src/Sci_BarSeries.cpp \
	src/Sci_BarSeriesContentModel.cpp \
	src/Sci_Chart.cpp \
	src/Sci_ChartAxis.cpp \
	src/Sci_ChartFormatSelectionDialog.cpp \
	src/Sci_ChartFrame.cpp \
	src/Sci_ChartRenderWidget.cpp \
	src/Sci_ChartSeries.cpp \
	src/Sci_ColorEditWidget.cpp \
	src/Sci_ColorGrid3DData.cpp \
	src/Sci_ColorGridSeries.cpp \
	src/Sci_ColorMap.cpp \
	src/Sci_ColorMapCreator.cpp \
	src/Sci_ColorMapEditWidget.cpp \
	src/Sci_CurveTracker.cpp \
	src/Sci_DataReductionCurveFitter.cpp \
	src/Sci_DateScaleDraw.cpp \
	src/Sci_DateScaleEngine.cpp \
	src/Sci_Export2BitmapWidget.cpp \
	src/Sci_Export2VectorWidget.cpp \
	src/Sci_ExportDialog.cpp \
	src/Sci_Legend.cpp \
	src/Sci_LegendItem.cpp \
	src/Sci_LegendItemMover.cpp \
	src/Sci_LineSeries.cpp \
	src/Sci_LineSeriesContentModel.cpp \
	src/Sci_LineSeriesSelectionComboBox.cpp \
	src/Sci_Marker.cpp \
	src/Sci_PenStyleComboBox.cpp \
	src/Sci_PenStyleDelegate.cpp \
	src/Sci_PlotCurve.cpp \
	src/Sci_PlotLayout.cpp \
	src/Sci_PlotMarker.cpp \
	src/Sci_PlotPanner.cpp \
	src/Sci_PlotPicker.cpp \
	src/Sci_PlotPickerText.cpp \
	src/Sci_PlotRescaler.cpp \
	src/Sci_PlotScaleItem.cpp \
	src/Sci_PlotSpectrogram.cpp \
	src/Sci_PlotVectorField.cpp \
	src/Sci_PlotZoomer.cpp \
	src/Sci_ReportFrameItemBarDataChart.cpp \
	src/Sci_ReportFrameItemChart.cpp \
	src/Sci_ReportFrameItemLineDataChart.cpp \
	src/Sci_ReportFrameItemTimeLineDataChart.cpp \
	src/Sci_SeriesDataEmpty.cpp \
	src/Sci_SeriesListDelegate.cpp \
	src/Sci_SeriesListModel.cpp \
	src/Sci_StopPointDelegate.cpp \
	src/Sci_Utils.cpp \
	src/Sci_VectorFieldSeries.cpp

FORMS +=  \
	src/Sci_ChartFormatSelectionDialog.ui \
	src/Sci_ColorMapEditWidget.ui \
	src/Sci_Export2BitmapWidget.ui \
	src/Sci_Export2VectorWidget.ui \
	src/Sci_ExportDialog.ui

RESOURCES += resources/gfx/SciChart.qrc

TRANSLATIONS += resources/translations/SciChart_de.ts \
	resources/translations/SciChart_fr.ts \
	resources/translations/SciChart_it.ts

