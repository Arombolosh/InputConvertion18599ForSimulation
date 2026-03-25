/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <memory>
#include <stdexcept>
#include <algorithm>

#include <QDebug>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include "Sci_LineSeries.h"
#include "Sci_Utils.h"
#include "Sci_Chart.h"
#include "Sci_DataReductionCurveFitter.h"

namespace SCI {

LineSeries::LineSeries(Chart* chart, const QString& title, bool nanCheck) :
	ChartSeries(new SCI::PlotCurve(title), chart),
	m_size(0),
	m_xValues(0),
	m_yValues(0)
{
	setColor(Qt::black);
	curve()->setBrush(QBrush(Qt::NoBrush));
	curve()->setNaNCheck(nanCheck);
	// Note: we don't want antialiased lines in screen displays - we loose accuracy
//	curve()->setRenderHint(QwtPlotItem::RenderAntialiased);

	curve()->setCurveFitter(new DataReductionCurveFitter);
}

LineSeries::LineSeries(Chart* chart, int size, const double* x, const double* y, const QString& title,
					   bool nanCheck, QColor col) :
	ChartSeries(new SCI::PlotCurve(title), chart),
	m_size(0),
	m_xValues(0),
	m_yValues(0)
{
	setData(size, x, y);
	setColor(col);
	curve()->setBrush(QBrush(Qt::NoBrush));
	curve()->setNaNCheck(nanCheck);
	// Note: we don't want antialiased lines in screen displays - we loose accuracy
//	curve()->setRenderHint(QwtPlotItem::RenderAntialiased);
	curve()->setCurveFitter(new DataReductionCurveFitter);
}


void LineSeries::lockSeries() {
	curve()->lockData();
}

void LineSeries::unlockSeries() {
	curve()->unlockData();
}

bool LineSeries::setData(int size, const double* x, const double* y) {
	if( !m_ser)
		return false;

	static double xDummy = 0;
	static double yDummy = 0;

	m_size = size;
	m_xValues = x;
	m_yValues = y;

	std::string errstr;
	bool res;
	if (size == 0) {
		m_size = 1;
		m_xValues = &xDummy;
		m_yValues = &yDummy;
	}
	res = curve()->setRawSamples(m_xValues, m_yValues, m_size, errstr);

	m_valid = res;
	return res;
}


void LineSeries::updateBoundaryRect() {
	curve()->setRawSamples(m_xValues, m_yValues, m_size);
}


QPen LineSeries::pen() const {
	Q_ASSERT(m_ser != 0);
	return curve()->pen();
}


void LineSeries::setPen (const QPen& pen) {
	Q_ASSERT(m_ser != 0);
	curve()->setPen(pen);
	m_chart->replot();
}


int LineSeries::width() const {
	Q_ASSERT(m_ser != 0);
	return curve()->pen().width();
}


void LineSeries::setLineWidth (int w) {
	Q_ASSERT(m_ser != 0);
	QPen pen = curve()->pen();
	pen.setWidth(w);
	curve()->setPen(pen);

	m_chart->replot();
}


QColor LineSeries::color() const {
	Q_ASSERT(m_ser != 0);
	return curve()->pen().color();
}


void LineSeries::setColor(const QColor& color) {
	Q_ASSERT(m_ser != 0);
	QPen pen = curve()->pen();
	pen.setColor(color);
	curve()->setPen(pen);

	m_chart->replot();
}


void LineSeries::setType(QwtPlotCurve::CurveStyle type) {

	SCI::PlotCurve::CurveStyle style = curve()->style();
	if( type == style )
		return;

	curve()->setBrush(QBrush(Qt::NoBrush));

	curve()->setStyle(type);
	m_chart->replot();
}


void LineSeries::setMarkerStyle(int style) {
	if (style == m_markerStyle)
		return;
	m_markerStyle = style;
	updateMarker();
	m_chart->replot();
}


void LineSeries::setMarkerColor(const QColor& color) {
	if (color == m_markerColor)
		return;
	m_markerColor = color;
	updateMarker();
	m_chart->replot();
}


void LineSeries::setMarkerFilled(bool filled) {
	if (filled == m_markerFilled)
		return;
	m_markerFilled = filled;
	updateMarker();
	m_chart->replot();
}


void LineSeries::setMarkerSize(unsigned int markerSize) {
	if (markerSize == m_markerSize)
		return;
	m_markerSize = markerSize;
	updateMarker();
	m_chart->replot();
}


void LineSeries::setInverted( bool inverted ) {

	curve()->setCurveAttribute(SCI::PlotCurve::Inverted, inverted);
	m_chart->replot();
}


void LineSeries::setFitted( bool /*fitted*/ ) {

//	curve()->setCurveAttribute(SCI::PlotCurve::Fitted, fitted);
//	if (fitted) {
//		QwtWeedingCurveFitter * fitter = new QwtWeedingCurveFitter(1);
//		fitter->setChunkSize(2000);
//		curve()->setCurveFitter(fitter);
//	}
//	m_chart->replot();
}


void LineSeries::setLegendIconSize( const QSize &size ) {

	QSize sizeIcon = curve()->legendIconSize();

	// set value only when size is greater 0
	if ( size.height() > 0 )
		sizeIcon.setHeight( size.height() );

	if ( size.width() > 0 )
		sizeIcon.setWidth( size.width() );

	curve()->setLegendIconSize( sizeIcon );
}


void LineSeries::setLegendIconStyle(int style) {
	if (style == 0) {
		curve()->setLegendAttribute( QwtPlotCurve::LegendShowLine, false);
		curve()->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, false);
		curve()->setLegendAttribute( QwtPlotCurve::LegendNoAttribute, true);
	}
	else {
		curve()->setLegendAttribute( QwtPlotCurve::LegendShowLine, true);
		curve()->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true);
		curve()->setLegendAttribute( QwtPlotCurve::LegendNoAttribute, false);
	}
}


void LineSeries::setLegendIconVisible( bool visible ) {
	curve()->setItemAttribute(QwtPlotItem::Legend, visible);
}

void LineSeries::setBrush(const QBrush& brush) {
	curve()->setBrush(brush);
}

const QBrush& LineSeries::brush() const {
	return curve()->brush();
}


void LineSeries::updateMarker() {
	QwtSymbol::Style symbolStyle = (QwtSymbol::Style)m_markerStyle;
	const QwtSymbol* symbol = curve()->symbol();

	// clear symbol?
	if (symbolStyle == QwtSymbol::NoSymbol) {
		if (symbol == nullptr)
			return; // currently no symbol, and no symbol needed -> nothing to do
		curve()->setSymbol(nullptr);
		return;
	}

	QPen pen(m_markerColor);
	QBrush brush(m_markerColor);
	if (!m_markerFilled)
		brush = Qt::NoBrush;

	QwtSymbol* newsymbol = new QwtSymbol(symbolStyle, brush, pen, QSize(m_markerSize, m_markerSize));
	curve()->setSymbol( newsymbol );
}


} // namespace SCI
