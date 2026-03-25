/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>

#include <typeinfo>
#include <memory>

#include <qwt_plot_item.h>
#include <qwt_legend.h>
#include <qwt_plot.h>


#include <IBK_UnitList.h>

#include "Sci_ChartSeries.h"
#include "Sci_Chart.h"
#include "Sci_Utils.h"
#include "Sci_Legend.h"
#include "Sci_AbstractChartModel.h"
#include "Sci_AbstractLineSeriesModel.h"

namespace SCI {


ChartSeries::ChartSeries(QwtPlotItem* internalSeries) :
	m_ser(internalSeries),
	m_valid(false)
{
}


ChartSeries::ChartSeries(QwtPlotItem* internalSeries, Chart* chart) :
	m_ser(internalSeries),
	m_valid(false)
{
	addToChart(chart);
}


ChartSeries::~ChartSeries() {
	if( m_chart && m_ser)
		m_ser->detach();
	delete m_ser;
}


void ChartSeries::addToChart(Chart* chart) {
//	qDebug() << "ChartSeries::addToChart";
	if( !chart || !m_ser)
		return;
	m_ser->attach(chart);
	m_chart = chart;
}


void ChartSeries::setTitle(QString title) {
	Q_ASSERT(m_ser);
	Q_ASSERT(m_chart);
	m_ser->setTitle(title);
}


ChartSeriesInfos::SeriesType ChartSeries::type() const {
	return ChartSeriesInfos::UnknownSeries;
}


QString ChartSeries::title() const {
	if( !m_ser)
		return "";
	return m_ser->title().text();
}


bool ChartSeries::isLeftAxisAttached() const {
	return m_ser->yAxis() == QwtPlot::yLeft;
}

void ChartSeries::attachToLeftAxis() {
	if( !isLeftAxisAttached()) {
		m_ser->setYAxis(QwtPlot::yLeft);
		m_chart->enableAxis(QwtPlot::yLeft, true);
	}
}

void ChartSeries::attachToRightAxis() {
	if( isLeftAxisAttached()) {
		m_ser->setYAxis(QwtPlot::yRight);
		m_chart->enableAxis(QwtPlot::yRight, true);
	}
}

void ChartSeries::setShowInLegend(bool visible) {
	m_ser->setItemAttribute(QwtPlotItem::Legend, visible);
}


} // end namespace SCI
