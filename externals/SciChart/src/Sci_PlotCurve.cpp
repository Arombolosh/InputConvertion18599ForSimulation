/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_PlotCurve.h"

#include <limits>

#include <qwt_point_data.h>
#include <qwt_text.h>

#include "Sci_SeriesDataEmpty.h"

namespace SCI {

PlotCurve::PlotCurve() :
	m_lockCounter(0),
	m_nanCheck(false),
	m_emptyData(new SeriesDataEmpty)
{
}

PlotCurve::PlotCurve(const QwtText &title) :
	QwtPlotCurve(title),
	m_lockCounter(0),
	m_nanCheck(false),
	m_emptyData(new SeriesDataEmpty)
{
}

PlotCurve::PlotCurve(const QString &title) :
	QwtPlotCurve(title),
	m_lockCounter(0),
	m_nanCheck(false),
	m_emptyData(new SeriesDataEmpty)
{
}

PlotCurve::~PlotCurve() {
	delete m_emptyData;
}

void PlotCurve::lockData() {
	++m_lockCounter;
}

void PlotCurve::unlockData() {
	if( m_lockCounter > 0)
		--m_lockCounter;
}

QString PlotCurve::trackerValueText(int, double value) const {
	const int precision = 2;
	const int fieldWidth = 1 + 2 + 1 + precision;

	QString info( "%L1: %2" );
	info = info.arg( value, fieldWidth, 'f', precision );
	info = info.arg( title().text() );

	return info;
}

QwtText PlotCurve::trackerInfoAt(int attributes, const QPointF & pos) const {
	QwtText text = QwtPlotCurve::trackerInfoAt(attributes, pos);
	text.setRenderFlags( Qt::AlignLeft | Qt::AlignVCenter ); // left aligned
	return text;
}

QwtSeriesData<QPointF>* PlotCurve::data() {
	if( m_lockCounter > 0)
		return m_emptyData;

	return QwtPlotCurve::data();
}

const QwtSeriesData<QPointF>* PlotCurve::data() const {
	if( m_lockCounter > 0)
		return m_emptyData;

	return QwtPlotCurve::data();
}

size_t PlotCurve::dataSize() const {
	if( m_lockCounter > 0)
		return 0;

	return QwtPlotCurve::dataSize();
}

double PlotCurve::x(int i) const
{
	if( m_lockCounter > 0)
		return 0.0;

	const QPointF& sample = QwtPlotCurve::sample(i);
	return sample.x();
}

double PlotCurve::y(int i) const
{
	if( m_lockCounter > 0)
		return 0.0;

	const QPointF& sample = QwtPlotCurve::sample(i);
	return sample.y();
}

bool PlotCurve::setRawSamples(const double *xData, const double *yData, int size, std::string& errstr)
{
	if( m_lockCounter > 0) {
		errstr = "curve is locked";
		return false;
	}

	QwtPlotCurve::setRawSamples(xData, yData, size);
	return true;
}

void PlotCurve::setRawSamples(const double *xData, const double *yData, int size)
{
	if( m_lockCounter <= 0) {
		QwtPlotCurve::setRawSamples(xData, yData, size);
	}
}


QRectF PlotCurve::boundingRect() const {
	if ( m_lockCounter > 0 )
		return QRectF(1.0, 1.0, -2.0, -2.0); // invalid

	double minX = std::numeric_limits<double>::max();
	double maxX = -std::numeric_limits<double>::max();
	double minY = std::numeric_limits<double>::max();
	double maxY = -std::numeric_limits<double>::max();

	const QwtSeriesData<QPointF> * d = QwtPlotCurve::data();
	for (size_t i=0; i < dataSize(); ++i ) {
		const QPointF & p = d->sample(i);
		double x = p.x();
		double y = p.y();
		if (!qIsNaN(x)) {
			minX = qMin(minX, x);
			maxX = qMax(maxX, x);
		}
		if (!qIsNaN(y)) {
			minY = qMin(minY, y);
			maxY = qMax(maxY, y);
		}
	}

	// fall back to defaults if max() values are still in the variables
	if (minX == std::numeric_limits<double>::max() || maxX == -std::numeric_limits<double>::max()) {
		minX = 1;
		maxX = -1;
	}
	if (minY == std::numeric_limits<double>::max() || maxY == -std::numeric_limits<double>::max() ) {
		minY = 1;
		maxY = -1;
	}

	return QRectF(QPointF(minX, minY), QPointF(maxX, maxY) );
}


void PlotCurve::drawSeries( QPainter *p,
	const QwtScaleMap &xMap, const QwtScaleMap &yMap,
	const QRectF &canvasRect, int from, int to ) const {

	if( m_lockCounter > 0)
		return;

	const int numSamples = (int)dataSize();

	if ( !p || numSamples <= 0 )
		return;

	if ( to < 0 )
		to = numSamples - 1;

	int newTo = to;
	int newFrom = from;

	if( m_nanCheck) {
		bool nanFound = false;
		for( int i=from; i<=to; ++i) {
			double xVal = x(i);
			double yVal = y(i);
			bool nanAtPosX = xVal != xVal;
			bool nanAtPosY = yVal != yVal;
			if( !nanFound && (nanAtPosX || nanAtPosY)) {
				// NaN found
				newTo = i-1;
				if( newTo > 0) {
					QwtPlotCurve::drawSeries(p, xMap, yMap, canvasRect, newFrom, newTo);
				}
				nanFound = true;
			}
			if( nanFound && (!nanAtPosX && !nanAtPosY)) {
				newFrom = i;
				newTo = to;
				nanFound = false;
			}
		}
	}

	if (newFrom <= newTo)
		QwtPlotCurve::drawSeries(p, xMap, yMap, canvasRect, newFrom, newTo);
}


void PlotCurve::drawSymbols( QPainter *painter, const QwtSymbol &symbol,
	const QwtScaleMap &xMap, const QwtScaleMap &yMap,
	const QRectF &canvasRect, int from, int to ) const
{
	/// \todo implement marker filter
	///		when "marker_distance != 0 apply filter"
	///			only draw marker when "distance to last marker > marker_distance"

	QwtPlotCurve::drawSymbols(painter, symbol, xMap, yMap, canvasRect, from, to);
}

} // namespace SCI


