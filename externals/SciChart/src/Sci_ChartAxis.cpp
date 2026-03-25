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

#include <IBK_assert.h>
#include <IBK_math.h>

#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>

#include "Sci_ChartAxis.h"
#include "Sci_Chart.h"
#include "Sci_LineSeries.h"
#include "Sci_AbstractChartModel.h"
#include "Sci_AbstractLineSeriesModel.h"
#include "Sci_DateScaleDraw.h"
#include "Sci_DateScaleEngine.h"

namespace SCI {

ChartAxis::ChartAxis(Chart* chart, int axisId) :
	m_chart(chart),
	m_globMaxMin(false),
	m_axisId(axisId),
	m_sqrtTime(false),
	m_verticalAxis(axisId == QwtPlot::yLeft || m_axisId == QwtPlot::yRight),
	m_dateTime(false)
{
	Q_ASSERT(m_verticalAxis || m_axisId == QwtPlot::xTop || m_axisId == QwtPlot::xBottom);
	Q_ASSERT(m_chart != 0);
}


bool ChartAxis::isVisible() const {
	return m_chart->axisEnabled(m_axisId);
}


void ChartAxis::setVisible(bool visible) {
	m_chart->enableAxis(m_axisId, visible);
}


QString ChartAxis::title() const {
	return m_chart->axisTitle(m_axisId).text();
}


int axisModelTypeToAxisid(AbstractChartModel::AxisPosition axisPos) {
	switch( axisPos) {
		case AbstractCartesianChartModel::BottomAxis: return QwtPlot::xBottom;
		case AbstractCartesianChartModel::LeftAxis: return QwtPlot::yLeft;
		case AbstractCartesianChartModel::RightAxis: return QwtPlot::yRight;
		default: return -1;
	}
}


AbstractChartModel::AxisPosition axisIdToAxisModelType(int axisId) {
	switch( axisId) {
		case QwtPlot::xBottom: return AbstractChartModel::BottomAxis;
		case QwtPlot::yLeft: return AbstractChartModel::LeftAxis;
		case QwtPlot::yRight: return AbstractChartModel::RightAxis;
		default: throw IBK::Exception("Unknown axis type", "[AbstractChartModel::AxisPosition]");
	}
}


void ChartAxis::setTitle(QString title) {
	QwtText qwtTitle(title, QwtText::RichText);
	qwtTitle.setFont( titleFont() );
	qwtTitle.setPaintAttribute( QwtText::PaintUsingTextFont, true);
	m_chart->setAxisTitle(m_axisId, qwtTitle);
}


bool ChartAxis::isLogarithmic() const {
	QwtScaleEngine* scale = m_chart->axisScaleEngine(m_axisId);
	return dynamic_cast<QwtLogScaleEngine*>(scale) != nullptr;
}


void ChartAxis::setLogarithmic() {
	if( !isLogarithmic()) {
		setDateTime(false);
		m_chart->setAxisScaleEngine(m_axisId, new QwtLogScaleEngine);
		m_chart->autoRefresh();
	}
}


void ChartAxis::setLinear() {
	setDateTime(false);
	m_chart->setAxisScaleEngine(m_axisId, new QwtLinearScaleEngine);
	m_chart->autoRefresh();
}


double ChartAxis::maximum() const {
	const QwtScaleDiv& sdiv = m_chart->axisScaleDiv(m_axisId);
	return sdiv.upperBound();
}


void ChartAxis::setMaximum( double max) {
	double min = minimum();
	if( !IBK::near_equal(max, maximum())) {
		m_chart->setAxisScale(m_axisId, min, max);
		m_chart->autoRefresh();
	}
}


double ChartAxis::minimum() const {
	const QwtScaleDiv& sdiv = m_chart->axisScaleDiv(m_axisId);
	return sdiv.lowerBound();
}


void ChartAxis::setMinimum( double min) {
	double max = maximum();
	if( !IBK::near_equal(std::fabs(min), minimum())) {
		m_chart->setAxisScale(m_axisId, min, max);
		m_chart->autoRefresh();
	}
}


void ChartAxis::setMinMax(double min, double max) {
	bool similarScale;
	// use a different near_equal function when using log scale axis
	if (isLogarithmic()) {
		double lgMin = std::log10(std::max(1e-300, min));
		double lgMax = std::log10(std::max(1e-300, max));
		double oldLgMin = std::log10(minimum());
		double oldLgMax = std::log10(maximum());
		similarScale = (IBK::near_equal(lgMin, oldLgMin) && IBK::near_equal(lgMax, oldLgMax));
	}
	else {
		similarScale = (IBK::near_equal(min, minimum()) && IBK::near_equal(max, maximum()));
	}

	if (!similarScale) {
		bool minNaN = min != min;
		bool maxNaN = max != max;
		if( minNaN && maxNaN) {
			min = 0.0;
			max = 1.0;
		}
		else if(minNaN) {
			min = max * 0.9;
		}
		else if(maxNaN) {
			max = min * 1.1;
		}
		if( min == max) {
			if (min == 0) {
				min = 0.0;
				max = 1.0;
			}
			else {
				min *= 0.9;
				max *= 1.1;
			}
		}
		m_chart->setAxisScale(m_axisId, min, max);
		m_chart->autoRefresh();
	}
}


int ChartAxis::titleSpacing() const {
	Q_ASSERT(m_chart->axisWidget(m_axisId) != NULL);

	return m_chart->axisWidget(m_axisId)->spacing();
}


void ChartAxis::setTitleSpacing(int spacing) {
	Q_ASSERT(m_chart->axisWidget(m_axisId) != NULL);

	// spacing is distance between chart border and backbone scale
	// Note: this should keep the distance the same even if
	//       title is wrapped

	// get scale widget
	QwtScaleWidget * w = m_chart->axisWidget(m_axisId);

	// first set spacing to zero to get minimum possible width/height of scale widget
	w->setSpacing(0);
	m_chart->updateLayout();
	// now, the widget has minimum width/height based on title and scale

	// depending on orientation, subtract width/height from desired spacing
	if (m_axisId == QwtPlot::xBottom)
		spacing -= w->rect().height();
	else
		spacing -= w->rect().width();

	// Mind: clipping is already done in setSpacing()
	w->setSpacing(spacing);
}


void ChartAxis::setTitleInverted(bool inverted) {
	Q_ASSERT(m_chart->axisWidget(m_axisId) != NULL);

	m_chart->axisWidget(m_axisId)->setLayoutFlag(QwtScaleWidget::TitleInverted, inverted);
}


int ChartAxis::labelSpacing() const {
	Q_ASSERT(m_chart->axisScaleDraw(m_axisId) != NULL);

	return m_chart->axisScaleDraw(m_axisId)->spacing();
}


void ChartAxis::setLabelSpacing(int spacing) {
	Q_ASSERT(m_chart->axisScaleDraw(m_axisId) != NULL);

	m_chart->axisScaleDraw(m_axisId)->setSpacing(spacing);
	m_chart->axisWidget(m_axisId)->update();
}


int ChartAxis::axisMaxMajor() const {
	return m_axisMaxMajor;
}


void ChartAxis::setAxisMaxMajor( int maxMajor ) {
	m_axisMaxMajor = maxMajor;
	m_chart->setAxisMaxMajor(m_axisId,maxMajor);
	m_chart->autoRefresh();
}


int ChartAxis::axisMaxMinor() const {
	return m_axisMaxMinor;
}


void ChartAxis::setAxisMaxMinor( int maxMinor ) {
	m_axisMaxMinor = maxMinor;
	m_chart->setAxisMaxMinor(m_axisId,maxMinor);
	m_chart->autoRefresh();
}


bool  ChartAxis::isAutoScaleEnabled() const {
	return m_chart->axisAutoScale(m_axisId);
}


void ChartAxis::setAutoScaleEnabled(bool enabled) {
	if (enabled) {
		m_chart->setAxisAutoScale(m_axisId);
	}
	else {
		m_chart->setAxisScaleDiv(m_axisId, m_chart->axisScaleDiv(m_axisId));
	}
}


bool ChartAxis::hasSeriesAttached() const {
	if( !m_chart->model())
		return false;

	if( !m_verticalAxis)
		return m_chart->model()->seriesCount() > 0;
	bool leftAxis = m_axisId == QwtPlot::yLeft;
	for( unsigned int i=0, sercount = m_chart->model()->seriesCount(); i<sercount; ++i) {
		bool leftAttached = m_chart->model()->seriesData(i, AbstractLineSeriesModel::SeriesLeftYAxis).toBool();
		if( leftAxis == leftAttached)
			return true;
	}
	return false;
}


QFont ChartAxis::titleFont() const {
	return m_chart->axisTitle(m_axisId).font();
}


void ChartAxis::setTitleFont(const QFont& font) {
	QwtText text = m_chart->axisTitle(m_axisId);
	text.setFont(font);
	text.setPaintAttribute( QwtText::PaintUsingTextFont, true);
	m_chart->setAxisTitle(m_axisId, text);
}


QFont ChartAxis::labelFont() const {
	return m_chart->axisFont(m_axisId);
}


void ChartAxis::setLabelFont(const QFont& font) {
	m_chart->setAxisFont(m_axisId, font);
}


void ChartAxis::setAxisLabelAlignment(int Halign, int Valign) {

	Qt::Alignment alignment = Qt::Alignment();
	switch (Halign) {
		case 0: alignment = Qt::AlignLeft; break;
		case 1: alignment = Qt::AlignRight; break;
		case 2: alignment = Qt::AlignHCenter; break;
		default: break;
	}

	switch (Valign) {
		case 0: alignment |= Qt::AlignTop; break;
		case 1: alignment |= Qt::AlignBottom; break;
		case 2: alignment |= Qt::AlignVCenter; break;
		default: break;
	}

	m_chart->axisWidget(m_axisId)->setLabelAlignment(alignment);
}


void ChartAxis::setAxisLabelRotation(double rotation) {
	m_chart->setAxisLabelRotation(m_axisId, rotation);
}


void ChartAxis::setDateTime(bool dateTime) {
	if( dateTime != m_dateTime) {
		QwtScaleDraw* scaleDraw =  dateTime ? new DateScaleDraw() : new QwtScaleDraw();
		m_chart->setAxisScaleDraw(m_axisId, scaleDraw);

		QwtScaleEngine* scaleEngine =  dateTime ? new DateScaleEngine() : new QwtLinearScaleEngine();
		m_chart->setAxisScaleEngine(m_axisId, scaleEngine);

		m_dateTime = dateTime;
	}
}


bool ChartAxis::isDateTime() const {
	return m_dateTime;
}


void ChartAxis::setDateTimeSpec(Qt::TimeSpec timeSpec) {
	if(!m_dateTime)
		return;

	DateScaleDraw* scaleDraw = dynamic_cast<DateScaleDraw*>(m_chart->axisScaleDraw(m_axisId));
	Q_ASSERT(scaleDraw != nullptr);
	scaleDraw->setTimeSpec(timeSpec);

	DateScaleEngine* scaleEngine = dynamic_cast<DateScaleEngine*>(m_chart->axisScaleEngine(m_axisId));
	Q_ASSERT(scaleEngine != nullptr);
	scaleEngine->setTimeSpec(timeSpec);
	m_chart->axisWidget(m_axisId)->update();
}


void ChartAxis::setDateTimeFormats(const QStringList& formats) {
	if (!m_dateTime)
		return;

	DateScaleDraw* scaleDraw = dynamic_cast<DateScaleDraw*>(m_chart->axisScaleDraw(m_axisId));
	Q_ASSERT(scaleDraw != nullptr);

	scaleDraw->setDateFormats(formats);
	m_chart->axisWidget(m_axisId)->update();
}


QwtText ChartAxis::label(double value, int formatBack) const {
	QwtScaleDraw* scaleDraw = m_chart->axisScaleDraw(m_axisId);
	if(scaleDraw == nullptr)
		return QwtText(QString("%L1").arg(value));

	DateScaleDraw* dateScaleDraw = dynamic_cast<DateScaleDraw*>(scaleDraw);
	if (dateScaleDraw == nullptr)
		return scaleDraw->label(value);

	return dateScaleDraw->dateLabel(value, formatBack);
}



} // end namespace SCI
