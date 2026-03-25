/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ColorGridSeriesH
#define Sci_ColorGridSeriesH

#include "Sci_ChartSeries.h"
#include "Sci_Globals.h"
#include "Sci_PlotSpectrogram.h"

namespace SCI {

	class Chart;
	class ColorGrid3DData;

/*! \brief The class SCI::ColorGridSeries contains all variables and functions for creating
		   and mainpulating of a color grid series of SCI::Chart.

	A color grid is a 2D representation of a 3D data set. It is normally used for presenting a
	distribution of a parameter on 2D location grid.

	This a concrete class derived from SCI::ChartSeries. It implements all access functions to
	apply model properties related to a spectrogram plot item (QwtPlotSpectrogram).
*/
class ColorGridSeries : public ChartSeries {
	Q_OBJECT
	Q_DISABLE_COPY(ColorGridSeries)
public:
	/*! Returns the series type. */
	ChartSeriesInfos::SeriesType type() const {	return ChartSeriesInfos::ColorGridSeries; }

	/*! Give raw access to internal plot item (needed by plot picker to interpolate for a given coordinate). */
	const SCI::PlotSpectrogram*	spectrogram() const { return static_cast<SCI::PlotSpectrogram*>(m_ser); }

private:

	/*! Default constructor creates a new series.
	  It contains no data and is not connected with a chart (see ChartSeries::setData and ChartSeries::addToChart).
	*/
	ColorGridSeries();

	/*! Destructor has to delete data object.*/
	~ColorGridSeries();

	/*! Constructor that creates an empty series which is connected to the given chart.
	  \param chart Parent chart.
	  \param title Title of the series. Used for identification in legend.
	  \param col Color of the line.
	*/
	ColorGridSeries(Chart* chart, const QString& title = "");

	/*! Set the data for the color grid series.
	  \param xGridSize Size of x grid line array
	  \param xGrid x values for the grid (left to right).
	  \param yGridSize Size of y grid line array
	  \param yGrid y values for the grid (bottom to top).
	  \param xValueSize Number of elements in x direction.
	  \param yValueSize Number of elements in y direction.
	  \param data 2D data array with dimension xValueSize * yValueSize. The data is stored in row-major order, where
				row-indexes j start go from bottom to top (as y-coordinate axis).
	*/
	void setData(unsigned int xGridSize, const double* xGrid, unsigned int yGridSize, const double* yGrid,
				 unsigned int xValueSize, unsigned int yValueSize, const double* data);

	/*! Sets a new value range, used for color mapping and for recalculation of contour lines.
		\param zValueRange The value range to use for scaling the color map (or normalizing data values).
	*/
	void setZValueRange(QwtInterval zValueRange);

	/*! Sets the contour interval. */
	void setContourInterval( double value );

	/*! Sets the contour pen. */
	void setContourPen( QPen pen );

	/*! Sets the contour lines to interval or to pen color. */
	void setContourIntervalColor( bool value );

	/*! Turn a label placement flag on/off.
		\param labelPlacementFlag Can be any bit combination of SCI::PlotSpectrogram::LabelPlacementFlags
	*/
	void setLabelPlacement( bool on, unsigned int labelPlacementFlag);
	/*! Function to set label placement alltogether. */
	void setLabelPlacement(unsigned int labelPlacementFlags);
	void setNPerLevel( int count);
	void setLabelStep( int count);

	/*! Internal conversion function.*/
	SCI::PlotSpectrogram*	spectrogram() {	return static_cast<SCI::PlotSpectrogram*>(m_ser); }

	/*! Sets internal value interpolation.*/
	void setInterpolation(bool val);

	/*! Returns internal value interpolation.*/
	bool isInterpolated() const;

	/*! Sets the display mode to image and/or contour mode. */
	void setDisplayMode(int val);

	double						m_contourInterval;
	bool						m_contourIntervalColor;
	QPen						m_contourPen;

	friend class Chart;

};

/*! \file Sci_ColorGridSeries.h
	\brief Contains the declaration of class ColorGridSeries.
*/

} // namespace SCI

#endif // Sci_ColorGridSeriesH
