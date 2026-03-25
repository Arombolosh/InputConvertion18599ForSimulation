/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <limits>
#include <algorithm>
#include <memory>
#include <typeinfo>

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include <qwt_color_map.h>

#include <IBK_assert.h>

#include "Sci_ColorGridSeries.h"
#include "Sci_Chart.h"
#include "Sci_ColorGrid3DData.h"

namespace SCI {


ColorGridSeries::ColorGridSeries() :
		ChartSeries(new PlotSpectrogram),
		m_contourInterval(std::numeric_limits<double>::quiet_NaN()),
		m_contourIntervalColor(true)
{
}


ColorGridSeries::ColorGridSeries(Chart* chart, const QString& title) :
		ChartSeries(new PlotSpectrogram(title), chart),
		m_contourInterval(std::numeric_limits<double>::quiet_NaN()),
		m_contourIntervalColor(true)
{
}


ColorGridSeries::~ColorGridSeries() {
}


void ColorGridSeries::setData(unsigned int xGridSize, const double* xGrid, unsigned int yGridSize, const double* yGrid,
							  unsigned int xValueSize, unsigned int yValueSize, const double* data)
{
	const char * const FUNC_ID = "[ColorGridSeries::setData]";

	// check if the right grid and value sizes are given
	if (xGridSize != xValueSize && xGridSize != xValueSize + 1)
		throw IBK::Exception( "Invalid x-grid and x-value sizes.", FUNC_ID);

	if (yGridSize != yValueSize + 1)
		throw IBK::Exception("Invalid y-grid and y-value sizes.", FUNC_ID);

	ColorGrid3DData* dataTmp = new ColorGrid3DData(xGridSize, xGrid, yGridSize, yGrid, xValueSize, yValueSize, data);

	try {
		spectrogram()->setData(dataTmp, false); // ownership is transfered, no exception can occur up to this point
		spectrogram()->setDisplayMode(QwtPlotSpectrogram::ImageMode);
		setContourInterval( m_contourInterval );
		m_valid = true;
	}
	catch (IBK::Exception & ex) {
		m_valid = false;
		throw ex;
	}
}


void ColorGridSeries::setInterpolation(bool val) {
	spectrogram()->setInterpolation(val);
	m_chart->replot();
}


bool ColorGridSeries::isInterpolated() const {
	return spectrogram()->interpolation();
}


void ColorGridSeries::setDisplayMode(int val) {
	QwtPlotSpectrogram::DisplayMode mode = static_cast<QwtPlotSpectrogram::DisplayMode>(val);
	// both
	if ( mode > QwtPlotSpectrogram::ContourMode ) {

		spectrogram()->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
		spectrogram()->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);

		setContourInterval( m_contourInterval );
		setContourIntervalColor( m_contourIntervalColor );
	}
	// contour lines only
	else if ( mode == QwtPlotSpectrogram::ContourMode ) {

		spectrogram()->setDisplayMode(QwtPlotSpectrogram::ContourMode, true);
		spectrogram()->setDisplayMode(QwtPlotSpectrogram::ImageMode, false);

		setContourIntervalColor( m_contourIntervalColor );
		setContourInterval( m_contourInterval );
	}
	// color map only
	else if ( mode == QwtPlotSpectrogram::ImageMode ) {

		spectrogram()->setDisplayMode(QwtPlotSpectrogram::ContourMode, false);
		spectrogram()->setDisplayMode(QwtPlotSpectrogram::ImageMode, true);
		setContourIntervalColor( m_contourIntervalColor );
	}
}


void ColorGridSeries::setZValueRange(QwtInterval zValueRange) {
	spectrogram()->data()->setInterval(Qt::ZAxis, zValueRange);

	// update contour lines
	setContourInterval( m_contourInterval );
}


void ColorGridSeries::setContourInterval( double value ) {

	if ( value != value )
		return;

	m_contourInterval = value;

	QList<double> contourLevels;
	QwtInterval valueRange = spectrogram()->data()->interval(Qt::ZAxis);

	double min = valueRange.minValue();
	double max = valueRange.maxValue();
	Q_ASSERT(min <= max);
	Q_ASSERT(value > 0);
	for ( double level = min; level < max; level += m_contourInterval )
		 contourLevels += level;
	spectrogram()->setContourLevels(contourLevels);
}


void ColorGridSeries::setContourPen( QPen pen ) {

	m_contourPen = pen;
	if ( m_contourIntervalColor ) {
		spectrogram()->setDefaultContourPen( QPen( Qt::NoPen ) );
		spectrogram()->setContourPen( m_contourPen );
	}
	else {
		spectrogram()->setDefaultContourPen( m_contourPen );
		spectrogram()->setContourPen( m_contourPen );
	}
}


void ColorGridSeries::setContourIntervalColor( bool value ) {

	if ( m_contourIntervalColor != value ) {

		m_contourIntervalColor = value;
		setContourPen( m_contourPen );

	}
}


void ColorGridSeries::setLabelPlacement(bool on, unsigned int labelPlacementFlag) {

	unsigned int labelPlacement = spectrogram()->labelPlacement();
	if ( on) {
		labelPlacement |= labelPlacementFlag;
	}
	else {
		labelPlacement &= ~labelPlacementFlag;
	}
	spectrogram()->setLabelPlacement( labelPlacement );
}


void ColorGridSeries::setLabelPlacement(unsigned int labelPlacementFlags) {
	spectrogram()->setLabelPlacement( labelPlacementFlags );
}


void ColorGridSeries::setNPerLevel( int count) {
	spectrogram()->setLabelsPerContourLevel( count );
}


void ColorGridSeries::setLabelStep( int step ) {
	spectrogram()->setLabelStep( step);
}



} // namespace SCI
