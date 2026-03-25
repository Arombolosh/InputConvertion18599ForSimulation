/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_VectorPlotSeriesH
#define Sci_VectorPlotSeriesH

#include "Sci_ChartSeries.h"
#include "Sci_Globals.h"
#include "Sci_PlotVectorField.h"

namespace SCI {

	class Chart;

/*! \brief The class SCI::VectorPlotSeries contains all variables and functions for creating
		   and mainpulating of a vector plot series of SCI::Chart.

	A vector plot series draws arrows for vector components onto the plot.

	This a concrete class derived from SCI::ChartSeries. It implements all access functions to
	apply model properties related to a vector field plot item (QwtPlotVectorField).
*/
class VectorFieldSeries : public ChartSeries {
	Q_OBJECT
	Q_DISABLE_COPY(VectorFieldSeries)
public:
	/*! Returns the series type (ChartSeriesInfos::VectorFieldSeries).
	*/
	ChartSeriesInfos::SeriesType type() const {	return ChartSeriesInfos::VectorFieldSeries; }

	/*! Give raw access to internal plot item (needed by plot picker to interpolate for a given coordinate). */
	const SCI::PlotVectorField*	vectorField() const { return static_cast<SCI::PlotVectorField*>(m_ser); }

private:

	/*! Default constructor creates a new series.
	  It contains no data and is not connected with a chart (see ChartSeries::setData and ChartSeries::addToChart).
	*/
	VectorFieldSeries();

	/*! Destructor has to delete data object.*/
	~VectorFieldSeries();

	/*! Constructor that creates an empty series which is connected to the given chart.
	  \param chart Parent chart.
	  \param title Title of the series. Used for identification in legend.
	  \param col Color of the line.
	*/
	VectorFieldSeries(Chart* chart, const QString& title = "");

	/*! Sets data to the series. */
	void setData(unsigned int vectorSampleSize, const double* x, const double* y,
				 const double* vx, const double* vy);

	/*! Sets a new value range, used for color mapping and for recalculation of contour lines.
		\param zValueRange The value range to use for scaling the color map (or normalizing data values).
	*/
	void setZValueRange(QwtInterval zValueRange);

	/*! Internal conversion function.*/
	SCI::PlotVectorField*	vectorField() { return static_cast<SCI::PlotVectorField*>(m_ser); }

	friend class Chart;

};

/*! \file Sci_VectorPlotSeries.h
	\brief Contains the declaration of class VectorPlotSeries.
*/

} // namespace SCI

#endif // Sci_VectorPlotSeriesH
