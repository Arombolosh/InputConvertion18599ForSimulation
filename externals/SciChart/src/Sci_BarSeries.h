/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_BarSeriesH
#define Sci_BarSeriesH

#include <QObject>

#include "Sci_ChartSeries.h"

#include "qwt_plot_multi_barchart.h"
#include "qwt_column_symbol.h"

namespace SCI {

class Chart;
class AbstractBarSeriesModel;

class BarSeries : public ChartSeries {
	Q_OBJECT
public:
	/*! Returns the series type. */
	ChartSeriesInfos::SeriesType type() const {	return ChartSeriesInfos::BarSeries; }

	/*! Set the data for the bar series.
		\param size Size of the data arrays.
		\param x \param y X and y-values for the line.
		The pointer must be valid and the size of the arrays must be the same.
		You can use lockSeries and unlockSeries if the pointers temporary unvalid.
		The function returns false if the series is locked and unable to set new values.
		The function sets data for a single bar chart system. One bar per one value.
	*/
//	bool setData(int size, const double* x, const double* y);

	/*! Set the data for the bar series.
		\param values Contains a whole data set.
		It can contain multiple x values and for each x values multiple y-values.
		The number of y values for each bar must be the same.
		The function returns false if the series is locked and unable to set new values.
		The function sets data for a multi bar chart system. Many bars per one value.
	*/
//	bool setData(const std::vector<std::pair<double,std::vector<double> > > & values);

	/*! Set the data for the bar series from the given model.
		\param values Contains a whole data set.
		The function returns false if the series is locked and unable to set new values.
		The function sets data for a multi bar chart system. Many bars per one value.
	*/
	bool setData(AbstractBarSeriesModel* model);

	/*! The combination of layoutPolicy() and layoutHint() define how the width of the bars is calculated.
		Possible values are:
	 \li AutoAdjustSamples	The sample width is calculated by dividing the bounding rectangle
		  by the number of samples. The layoutHint() is used as a minimum width
		  in paint device coordinates.
	 \li ScaleSamplesToAxes	layoutHint() defines an interval in axis coordinates
	 \li ScaleSampleToCanvas	The bar width is calculated by multiplying layoutHint()
		  with the height or width of the canvas.
	 \li FixedSampleSize	layoutHint() defines a fixed width in paint device coordinates.
	*/
	void setLayoutPolicy(QwtPlotAbstractBarChart::LayoutPolicy layoutPolicy);

	/*! set the value for calculating bar width. Methos is described in layout policy (\sa setLayoutPolicy()).*/
	void setLayoutHint( double val);

	/*!
	  \brief Set the spacing
	  The spacing is the distance between 2 samples ( bars for QwtPlotBarChart or
	  a group of bars for QwtPlotMultiBarChart ) in paint device coordinates.
	  \param spacing Value to set.
	 */
	void setSpacing( int spacing);

	/*!
	  \brief Set the margin
	  The margin is the distance between the outmost bars and the contentsRect()
	  of the canvas. The default setting is 5 pixels.
	  \param margin Margin
	 */
	void setMargin( int margin);

	/*!
	   \brief Set the baseline
	   The baseline is the origin for the chart. Each bar is
	   painted from the baseline in the direction of the sample
	   value. In case of a horizontal orientation() the baseline
	   is interpreted as x - otherwise as y - value.
	   The default value for the baseline is 0.
	   \param value Value for the baseline
	*/
	void setBaseline( double value);

	/*! The combination of layoutPolicy() and layoutHint() define how the width of the bars is calculated.
		Possible values are:
		\li Grouped	The bars of a set are displayed side by side.
		\li Stacked	The bars are displayed on top of each other accumulating to a single bar. All values of a set need to have the same sign.
	*/
	void setBarChartStyle(QwtPlotMultiBarChart::ChartStyle chartStyle);

	/*! Set the titles for each bar.
		Size must be equal to size of x values.
	*/
	void setBarTitles(const QStringList& titles);

	/*! Frame style used for draw bars.
		\param index Index for y value for the bars.
		\param width Frame style
		\li NoFrame	Bar has no frame.
		\li Plain	Bar has a plain frame.
		\li Raised	Bar has a raised frame.
		The value is set for the y-value section in all bars.
	*/
	void setBarFrameStyle(unsigned int index, QwtColumnSymbol::FrameStyle frameStyle);

	/*! Line width of bar frames.
		\param index Index for y value for the bars.
		\param width Frame line width
		The value is set for the y-value section in all bars.
	*/
	void setBarLineWidth(unsigned int index, int width);

	/*! Color palette for bar rectangle.
		\param index Index for y value for the bars.
		\param palette Color palette.
		The value is set for the y-value section in all bars.
	*/
	void setBarPalette(unsigned int index, const QPalette& palette);

	/*! Color for bar rectangle.
		\param index Index for y value for the bars.
		\param color Color for bar rectangle.
		The value is set for the y-value section in all bars.
	*/
	void setBarColor(unsigned int index, const QColor& color);

	/*! Set the orientation for the chart. That means should the bars grow horizontal or vertical.*/
	void setOrientation(Qt::Orientation orientation);


private:
	Q_DISABLE_COPY(BarSeries)

	/*! Internal conversion function in order to have access to functions of QwtPlotMultiBarChart.*/
	QwtPlotMultiBarChart * curve() {	return static_cast<QwtPlotMultiBarChart*>(m_ser); }

	/*! Internal conversion function in order to have access to functions of QwtPlotMultiBarChart.*/
	const QwtPlotMultiBarChart * curve() const {	return static_cast<QwtPlotMultiBarChart*>(m_ser); }

	/*! Constructor that creates an empty series which is connected to the given chart.
	  \param chart Parent chart.
	  \param title Title of the series. Used for identification in legend.
	*/
	BarSeries(Chart* chart, const QString& title = "");

	friend class Chart;

	unsigned int	m_barCount;
	unsigned int	m_barValueCount;
};

} // namespace SCI

#endif // Sci_BarSeriesH
