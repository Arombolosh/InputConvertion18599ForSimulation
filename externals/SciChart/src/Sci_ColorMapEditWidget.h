/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ColorMapEditWidgetH
#define Sci_ColorMapEditWidgetH

#include <QFrame>
#include <QTableWidgetItem>

class QToolButton;

#include "Sci_Globals.h"
#include "Sci_ColorMap.h"

namespace SCI {

namespace Ui {
	class ColorMapEditWidget;
}

class AbstractChartModel;

/*! Widget class to create/edit color maps.
	This widget holds a table with the individual stops set in the color map.
	A color map defines colors in certain intervals between a minimum and maximum z value.
	For example, if the minimum value is 10 and the maximum value is 20, a color map
	with 5 linerly spaced steps defines colors for 10, 12, 14, 16, 18, 20.

	The widget communicates exclusively with the associated model. All properties visualized
	in the widget are drawn from the model, all changes are done through the model.

	The widget currently works with models of type AbstractColorGridSeriesModel or
	AbstractVectorFieldSeriesModel.

	\note The color map is shown in the table bottom-to-top, so that minimum value is at bottom
		  and maximum value is at top.
*/
class ColorMapEditWidget : public QFrame {
	Q_OBJECT

public:
	explicit ColorMapEditWidget(QWidget *parent, const QString & colorMapDirectory);
	~ColorMapEditWidget();

	/*! Change the model. Disconnect and connect signals and slots. */
	void setModel(AbstractChartModel* model);

private slots:

	/*! Connected to all changes signaled by the model.
		Updates user interface to model content.
	*/
	void setProperties();

	/*! Connected to AbstractColorGridSeriesModel::seriesViewChanged() signal, listens to
		change events that may impact visualization of color map.
	*/
	void onSeriesViewChanged(int,int,int dataRole);

	// *** Ui slots ***

	void on_buttonCalculate_clicked();
	void on_buttonLoadMap_clicked();
	void on_buttonSaveMap_clicked();

	/*! Generate a new color map by interpolating through existing master color map. */
	void on_spinBoxCount_valueChanged(int arg1);

	void on_stopPointTable_cellChanged(int row, int column);

	void on_pushButtonInvertColorMap_clicked();

	void onToolButtonPresetClicked();

	void on_pushButtonGenerateHSVMap_clicked();

private:
	/*! Read all color map files from the internal resource (from resource directory :/colormaps/) and fills the m_colorMaps map. */
	void readColorMapsFromResource();

	/*! Generates a color map from a given preset. */
	ColorMap colorMapFromPreset(int idx, int stepCount) const;

	Ui::ColorMapEditWidget			*m_ui = nullptr;
	AbstractChartModel				*m_model = nullptr;

	/*! Holds the applications colormap directory. */
	QString							m_colorMapDirectory;

	ChartSeriesInfos::SeriesType	m_chartType;
	/*! The model role for accessing the color map in the currently used model. */
	int								m_colorMapRole;
	int								m_autoScaleRole;
	int								m_zValueRangeRole;

	/*! Here we store the last location from load/save color map. */
	QString							m_lastOpenFileLocation;

	std::map<QString,ColorMap>		m_colorMaps;

	/*! Stores IDs/file paths for color map presets. */
	QList< QPair<QToolButton*, QString> >		m_presets;
};

} //namespace SCI {


#endif // Sci_ColorMapEditWidgetH
