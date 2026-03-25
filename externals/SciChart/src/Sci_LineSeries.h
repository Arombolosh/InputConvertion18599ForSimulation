/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_LineSeriesH
#define Sci_LineSeriesH

#include <QSharedPointer>

#include "Sci_ChartSeries.h"
#include "Sci_PlotCurve.h"

namespace SCI {

class Chart;

/*! \brief The class SCI::LineSeries contains all variables and functions for creating and
	manipulating a line series of SCI::Chart.

	This a concrete class derived from SCI::ChartSeries and implements configuration of
	QwtPlotCurve from model properties.

	Note: Line and marker handling is as follows...

	- Line and marker (outline and fill color) are always the same. Use setColor() to change that.
	- The marker properties (marker style, filled, size) are cached in member variables and
	  a new marker symbol is created in updateMarker().
*/
class LineSeries : public ChartSeries {
	Q_OBJECT
	Q_DISABLE_COPY(LineSeries)
public:
	/*! Returns the series type. */
	ChartSeriesInfos::SeriesType type() const { return ChartSeriesInfos::LineSeries; }

	/*! Set the data for the line series.
		\param size Size of the data arrays.
		\param x X-values for the line.
		\param y Y-values for the line.

		The pointer must be valid and the size of the arrays must be the same.
		You can use lockSeries and unlockSeries if the pointers are temporarily unavailable/invalid.
		The function returns false if the series is locked and unable to set new values.
	*/
	bool setData(int size, const double* x, const double* y);

	/*! Returns the actual line pen. */
	QPen pen() const;

	/*! Returns the actual line width. */
	int width() const;

	/*! Returns pen color. */
	QColor color() const;

public slots:
	/*! Lock the data access.
	*/
	void lockSeries();

	/*! Unlock the data access.
	*/
	void unlockSeries();

private:

	/*! Constructor that creates an empty series which is connected to the given chart.
	  \param chart Parent chart.
	  \param title Title of the series. Used for identification in legend.
	*/
	LineSeries(Chart* chart, const QString& title = "", bool nanCheck = false);

	/*! Constructor that creates an empty series which is connected to the given chart.
	  \param chart Parent chart.
	  \param size Number of values
	  \param x X-values.
	  \param y Y-values.
	  \param title Title of the series. Used for identification in legend.
	  \param nanCheck Enable check for non valid values.
	  \param col Color of the line.
	*/
	LineSeries(Chart* chart, int size, const double* x, const double* y, const QString& title = "",
			   bool nanCheck = false, QColor col = Qt::black);

	/*! Internal conversion function in order to have access to functions of SCI::PlotCurve.*/
	PlotCurve * curve() { return static_cast<SCI::PlotCurve*>(m_ser); }

	/*! Internal conversion function in order to have access to functions of SCI::PlotCurve.*/
	const PlotCurve * curve() const { return static_cast<SCI::PlotCurve*>(m_ser); }

	/*! Set a new pen (for lines). The color of the pen is also used for the marker
		(function calls setColor() internally).
	*/
	void setPen(const QPen &);

	/*! Set a pen width for lines. */
	void setLineWidth (int);

	/*! Set a new pen color (only for lines, use setMarkerColor() to change color of marker). */
	void setColor(const QColor& col);

	/*! Sets the series/curve type. */
	void setType(QwtPlotCurve::CurveStyle type);

	/*! Sets the symbol style of the series.
		Outline of markers is always drawn with 1 pixel width. If 'filled' is set, a solid
		brush with same color is set.
	*/
	void setMarkerStyle(int style);

	/*! Modifies color of outline and fill (if enabled) of marker. */
	void setMarkerColor(const QColor& color);

	/*! Turns filling of marker on/off. */
	void setMarkerFilled(bool filled);

	/*! Adjusts the marker size. */
	void setMarkerSize(unsigned int markerSize);

	/*! Sets the inverted flag for step series. */
	void setInverted( bool inverted );

	/*! Sets the fitted flag for line series. */
	void setFitted( bool fitted );

	/*! Sets width and height of a legend item. Sets value only when size is greater 0.
		Mind: when using marker, the icon height shall be > 6, so that the marker can be
		seen clearly. The marker will always be drawn with size iconHeight - 2.
	*/
	void setLegendIconSize( const QSize &size );

	/*! Set the style of the legend items (either a filled rectangle or a line with optionally a marker).
		\sa AbstractChartModel::LegendIconStyleType.
	*/
	void setLegendIconStyle(int style);

	/*! Sets visibility of corresponding legend item. */
	void setLegendIconVisible( bool visible );

	/*! Set brush for possible area fill.
		In case of brush.style() != QBrush::NoBrush
		and style() != QwtPlotCurve::Sticks
		the area between the curve and the baseline will be filled.

		In case !brush.color().isValid() the area will be filled by
		pen.color(). The fill algorithm simply connects the first and the
		last curve point to the baseline. So the curve data has to be sorted
		(ascending or descending).

		\param brush New brush
	*/
	void setBrush(const QBrush& brush);

	const QBrush &brush() const;

	/*! Updates the boundary rect by iterating over samples (updates min/max).*/
	virtual void updateBoundaryRect();

private:
	/*! Constructs a new symbol using the cached properties below and sets the marker as curve symbol. */
	void updateMarker();

	/*! Number of values in data arrays m_xValues and m_yValues. */
	int				m_size;
	const double	*m_xValues;
	const double	*m_yValues;

	int				m_markerStyle;	// corresponds to type QwtSymbol::Style
	QColor			m_markerColor;
	unsigned int	m_markerSize;
	bool			m_markerFilled;

	friend class Chart;

};

/*! \file Sci_LineSeries.h
	\brief Contains the declaration of class LineSeries.
*/

} // namespace SCI

#endif // Sci_LineSeriesH
