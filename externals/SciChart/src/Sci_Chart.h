/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ChartH
#define Sci_ChartH

#include <QColor>
#include <QPrinter>

#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_renderer.h>

#include <IBK_Time.h>

#include "Sci_ChartSeries.h"
#include "Sci_ChartAxis.h"

class QwtPlotGrid;
class QwtPlotCurve;

namespace IBK {
	class LinearSpline;
	class Unit;
}

/*! \brief Namespace for SciChart library.
	The namespace SCI includes all classes and functions for handling SciChart.
	This includes widgets for chart configuration.
*/
namespace SCI {

	class SeriesCreatorBase;
	class ChartState;
	class LineSeries;
	class Legend;
	class LegendItem;
	class ColorLegend;
	class AbstractChartModel;

	class PlotZoomer;
	class PlotPanner;
	class PlotPicker;
	class PlotRescaler;
	class PlotScaleItem;
	class PlotMarker;
	class CurveTracker;

	class ChartFormatSelectionDialog;

/*! \brief The class SCI::Chart is the view for the SciChart model-view-framework.

	This version (2.x of SCI::Chart) uses series models in order to use the model/view concept.

	Usage is very simple:
	- use SCI::Chart as a widget to show your chart
	- create a SCI::LineSeriesContentModel, add series and customize their appearance
	- use the various model roles to specify properties of the chart

	General chart properties are set with SCI::AbstractChartModel::setData().
	Properties of an axis are set with SCI::AbstractCartesianChartModel::setAxisData().
	Properties of an axis are set with SCI::AbstractLineSeriesModel::setAxisData().

	The convenience class SCI::LineSeriesContentModel inherits these classes and provides all three
	model functions. See the respective documentation for possible data roles and required data types.

	\note While you can call QwtPlot member functions directly despite using a model for the chart, you should
		rather set chart properties via model->setData() functionality.

	\todo Discussion item: encapsulate QwtPlot entirely so that users of SciChart do not even need
		to know about Qwt and its classes and do not need to include the headers of Qwt. With the item models,
		this shouldn't be necessary at all.
		Pro: Qwt can be wrapped entirely, future API changes in Qwt do not impact SciChart
		Con: lots of work and SciChart API is sort of aligned with Qwt API in many places - encapsulation would
			 require lots of duplicate types/enums/wrappers which would be hard to maintain
		Suggestion: leave this as is and use Qwt and live with the compile-time dependency.
*/
class Chart : public QwtPlot {
	Q_OBJECT
	Q_DISABLE_COPY(Chart)

public:

	/*! Default constructor for class Chart.
	  \param parent Parent widget of the chart. Responsible for memory management.
	*/
	Chart(QWidget * parent = nullptr);

	/*! Destructor of Chart. Deletes all contents (series) of the chart. */
	~Chart();

	/*! Sets a new series model.
		The model must exist as long as the chart exists. The chart can be deleted
		any time.
		There is no ownership transfer. Model must e deleted separately.
	*/
	void setModel(AbstractChartModel* model);

	/*! Returns the series model. Returns 0 if no model is set.*/
	AbstractChartModel* model();

	/*! Returns the series model. Returns 0 if no model is set (const-version).*/
	const AbstractChartModel* model() const;

	/*! Wrapper of global clear function.*/
	void clear();

	/*! Sets the width of the y axis. */
	void setYAxisWidth( int width );

	/*! Sets the height of the x axis. */
	void setXAxisHeight( int height );

	/*! Returns the actual axis width from plot layout. */
	double yAxisWidth() const;

	/*! Returns the actual axis height from plot layout. */
	double xAxisHeight() const;

	/*! Get the start date of a time dependend chart. */
	const IBK::Time startDate() const;

	/*! Get the end date of a time dependend chart. */
	const IBK::Time endDate() const;

	/*! Get the current plot date of a time dependend chart. */
	IBK::Time currentDate() const;

	/*! Returns number of time points available for this chart.
		This is the total number of time points that are available in
		all datasets shown in the chart.
		\return Returns the number of time points in the chart. Returns 1, if
				only one time index is available.
	*/
	unsigned int nTimePoints() const;

	/*! Sets time points and start data vectors.
		This function must be called prior to a call to addLineSeries
	*/
	void setTimeFrame(const std::vector<double> & timePoints, const IBK::Time& startDateTime);

	/*! Returns the current time index that the chart shows.
		\return Returns a time index that is lessn than nTimePoints().
		*/
	unsigned int currentTimeIndex() const;

	/*! Sets the current time index. */
	void setCurrentTimeIndex(unsigned int tIdx);

	/*! Get a pointer to a ChartAxis.
	 \param type Identifys the kind of the desired axis.
	*/
	ChartAxis* axis(ChartAxis::AxisType type);

	/*! Get a pointer to a const ChartAxis.
	 \param type Identifys the kind of the desired axis.
	*/
	ChartAxis const * axis(ChartAxis::AxisType type) const;

#ifdef SCICHART_HAS_EMFENGINE
	/*! Renders the chart into an EMF vector image using a fixed size (for now).
		This function is only available on Windows.
	*/
	void copyToClipboardAsEMF();
#endif // SCICHART_HAS_EMFENGINE

	/*! Copies the chart to a clipboard as raster image, basically does a screenshot of the widget. */
	void copyToClipboardAsPixmap();

	/*! Modifies chart properties directly to improve vector output.
		- grid pen width (0.5)
		- axis scale pen width (0.5)
		- legend frame pen width (0.5)
	*/
	void prepareForVectorExport(bool legendVisible, bool useDataReduction);
	void prepareForBitmapExport(bool legendVisible);

	/*! Restores chart properties directly to our screen defaults (call this function after vector export). */
	void resetAfterVectorExport();
	void resetAfterBitmapExport();

	/*! Returns a pointer to the internal grid.*/
	QwtPlotGrid* grid() const { return m_grid; }

	/*! Returns a pointer to the picker/zoomer object.
		Shows a picker text with current coordinates and value under curser and when active shows a rectangular rubber band
		to drag the selection rect.
	*/
	PlotZoomer* zoomer() const { return m_zoomer; }

	/*! Returns a pointer to the panner object.*/
	PlotPanner* panner() const { return m_panner; }

	/*! Returns a ChartSeries pointer or 0 if no corresponding series can be found.
		\param index Index of the series in the list.
		This function can be used to create a list of series in the chart.
	*/
	const ChartSeries* seriesAt(unsigned int index) const;

	/*! Configures the chart to match the IBK style. */
	void configureChartFormat();

	/*! Writes the current chart style/format to file.
		\return Returns true on success (if file was written successfully) or false in case of any error.
	*/
	bool saveChartStyle(const QString & styleFilePath) const;

	/*! Reads the chart style/format from file and opens a dialog for user to select suitable options
		to apply to chart. Then, the selected properties are transferred to the model currently used
		by the chart.
		The function pops up error messages when file cannot be read.
	*/
	void applyChartStyle(const QString & styleFilePath);

	/*! Render the chart to the given painter with given size to given position.
	   \param p Painter used for rendering.
	   \param pos Position of the upper left corner of the chart.
	   \param horizontal If true chart is renderd horizontal at the viewport otherwise it will be rotated by 90Deg ccw.
	   \param layoutFlag Flag for controlling layout \sa (QwtPlotRenderer::LayoutFlag)
	   \param discardFlags Flags for skipping layout elements \sa (QwtPlotRenderer::DiscardFlag)
	   The size of the chart must be set by using chart geometry before use of this function.
	*/
	void render(QPainter * p, const QPointF& pos, bool horizontal = true,
				QwtPlotRenderer::LayoutFlag layoutFlag = QwtPlotRenderer::FrameWithScales,
				const QFlags<QwtPlotRenderer::DiscardFlag>& discardFlags = QwtPlotRenderer::DiscardBackground | QwtPlotRenderer::DiscardCanvasBackground);

public slots:
	/*! Redraws the whole chart.*/
	void replot();

	/*! Increase m_noUpdateCounter variable and prohibits internal updates.*/
	void prohibitUpdate();

	/*! Decrease m_noUpdateCounter variable.
		Once m_noUpdateCounter == 0 a call to replot() will again re-draw the chart.
		This function does not call replot automatically.
	*/
	void allowUpdate();

	/*! Calls allowUpdate() and afterwards replot(). */
	void allowUpdateAndReplot();

private slots:
	/*! Reacts on chartChanged from model. This is emitted after changes of chart properties in model (ChartDataRole).
		Essentially applies properties to underlying QwtPlot widget.
		\param role Role of the changed item \see AbstractChartModel::ChartDataRole
	*/
	void onChartChanged( int role );

	/*! Reacts on axisChanged from model. This is emitted after changes of axis properties in model (AxisDataRole).
		Essentially applies properties to underlying QwtPlot widget.
		\param axisPosition Position of the axis or axis type (bottom, left or right for cartesian axis).
		\param role Role of the changed item \see AxisDataRole.
	*/
	void onAxisChanged( int axisPosition, int role );

	/*! Reacts on signal seriesViewChanged from model and has to make the corresponding changes.
		Essentially applies properties to underlying QwtPlot widget.
		\param start Start index
		\param end End index
		\param role Role of the chnaged item \see AbstractChartModel::Role
	*/
	void onSeriesViewChanged(int start, int end, int role);

	/*! Reacts on signal seriesInserted from model ( is emitted just after series are inserted into the model).
		The new items will be positioned between start and end inclusive.
	*/
	void onSeriesInserted(int start, int end);

	/*! Reacts on signal seriesAboutToBeRemoved from model ( is emitted just before series are removed from the model).
		The items that will be removed are those between start and end inclusive.
	*/
	void onSeriesAboutToBeRemoved(int start, int end );

	/*! Reacts on signal seriesAboutToBeChanged from model ( is emitted just before series data are changed in the model).
		The items that will be changed in the range between start and end inclusive.
	*/
	void onSeriesAboutToBeChanged(int start, int end );

	/*! Reacts on signal seriesChanged from model (is emitted just after series are changed in the model).
		The items that will be changed are those between start and end inclusive.
	*/
	void onSeriesChanged(int start, int end);


	// *** same as for series, here are the modification functions for marker ***

	void onMarkerDataChanged(unsigned int start, unsigned int end, int role);
	void onMarkerInserted(unsigned int start, unsigned int end);
	void onMarkerAboutToBeRemoved(unsigned int start, unsigned int end );
	void onMarkerChanged(unsigned int start, unsigned int end);


	/*! Reacts on modelAboutToBeDeleted from destructor AbstractChartModel.
		Removes the current model.
	*/
	void onModelAboutToBeDeleted( );

	/*! */
	void onModelAboutToBeReset();

	/*! Reacts on signal modelReset() from model (emitted just after a model reset has completed).
		Removes all existing series (without replot) and calls resetView() to rebuild the cached
		chart data from scratch.
	´*/
	void onSeriesModelReset();

private:
	/*! Insert one series from the internal model at the given position.
		\param index Series data index in internal model SCI::AbstractChartModel).
		\return true if successful; otherwise false.
	*/
	bool insertLineSeriesFromModel(int index);

	/*! Insert a series from the internal model.
		Model must be from base type AbstractGolorGridModel.
		\return true if successful; otherwise false.
	*/
	bool insertColorGridSeriesFromModel();

	/*! Insert a series from the internal model.
		Model must be from base type AbstractBarSeriesModel.
		\return true if successful; otherwise false.
	*/
	bool insertBarSeriesFromModel();

	/*! Insert a series from the internal model.
		Model must be from base type AbstractVectorFieldSeriesModel.
		\return true if successful; otherwise false.
	*/
	bool insertVectorFieldSeriesFromModel();

	/*! Create a new construction line (marker) at given position (coordinate in m) and set the pen.
		The line object will be added to internal list (m_verticalConstructionLines).
	*/
	void addVerticalConstructionLine(double position, const QPen& pen);

	/*! Remove all vertical construction lines from the chart.*/
	void removeVerticalConstructionLines();

	/*! Updates construction lines in chart for all type of series.
		It will be done by removing and recreate of all construction lines.
	*/
	void updateConstructionLines();

	/*! Remove series with given index from internal list and deletes it.
		\param index Series index.
	*/
	bool removeSeries(unsigned int index);

	/*! Inserts marker using the model parameters at the given index.
		\param index The insert index. If index = markerCount() than marker is appended.
	*/
	bool insertMarkerFromModel(unsigned int index);

	/*! Remove marker with given index from internal list and deletes it.
		\param index Marker index.
	*/
	bool removeMarker(unsigned int index);

	/*! This function updates all axis configuration and is called whenever the "Internal Scales" property
		has changed.
	*/
	void updateInternalAxisConfiguration();

	/*! Resets whole view after a new model was set or a model reset occurred.
		This will remove all chart series and re-add them again.
		It will also replot the chart.
	*/
	void resetView();

	/*! Series model, not owned by chart.*/
	AbstractChartModel				*m_model;

	bool							m_dataLock;				///< Flag for data locks. Access to data should be locked before changes.

	/*! If not 0 no update of chart view is called.
		Modified in prohibitUpdate() and allowUpdate()/allowUpdateAndReplot().
	*/
	int								m_noUpdateCounter;

	/*! Vector of all attached series.
		For each series in QwtPlot we have a copy with series meta data.
	*/
	QVector<ChartSeries*>			m_series;

	/*! Plot markers. */
	QVector<PlotMarker*>			m_markers;

	QString							m_name;					///< ID-Name of the chart.

	ChartAxis						m_leftAxis;				///< Left axis
	ChartAxis						m_rightAxis;			///< Right axis
	ChartAxis						m_topAxis;				///< Top axis
	ChartAxis						m_bottomAxis;			///< Bottom axis

	PlotScaleItem					*m_internalBottomAxis;	///< Item that draws bottom axis inside canvas.
	PlotScaleItem					*m_internalTopAxis;		///< Item that draws top axis inside canvas.
	PlotScaleItem					*m_internalLeftAxis;	///< Item that draws left axis inside canvas.
	PlotScaleItem					*m_internalRightAxis;	///< Item that draws right axis inside canvas.

	/*! Pointer to the internal grid (not owned). */
	QwtPlotGrid						*m_grid;
	/*! Pointer to internal chart legend (not owned). */
	Legend							*m_legend;
	/*! Pointer to chart legend item (drawn inside the chart) (not owned). */
	LegendItem						*m_legendItem;

	/*! Color legend for 3D and 4D Plot (always present, not owned). */
	QwtScaleWidget					*m_colorLegend;

	IBK::Time						m_startDate;			///< Start date for time dependend 3D Plot and 4D Plot.
	unsigned int					m_currentTimeIdx;		///< Current date to plot for time dependend 3D Plot and 4D Plot.
	std::vector<double>				m_timePoints;			///< Vector with union of time points (in seconds since startDate) in all datasets used in this chart (updated in updateTimeFrame()).

	/*! Plot zoomer (passed to QwtPlot::canvas() parent, not owned). */
	PlotZoomer						*m_zoomer;
	/*! Plot panner (passed to QwtPlot::canvas() parent, not owned). */
	PlotPanner						*m_panner;
	/*! Curve tracker (passed to QwtPlot::canvas() parent, not owned). */
	CurveTracker					*m_curveTracker = nullptr;

	/*! Marker, that indicates via vertical line the current time point.
		This marker is also used when the plot picker is used to mark the marker position.
	*/
	QwtPlotMarker					*m_markerCurrentTimePosition;

	/*! The rescaler, attached whenever the plot is set to proportional, otherwise
		detached. Reference axis is either x-axis or y-axis.
	*/
	PlotRescaler					*m_rescaler;

	/*! Pointer to transfer settings dialog (created on first use). */
	ChartFormatSelectionDialog		*m_formatSelectionDialog;

	QVector<QwtPlotMarker*>			m_verticalConstructionLines;

	friend class ChartState;								///< ChartState class should have direct access to member of SCI::Chart.
};


} // namespace SCI

/*! \file Sci_Chart.h
	\brief Contains the declaration of class SCI::Chart.
*/

#endif // Sci_ChartH
