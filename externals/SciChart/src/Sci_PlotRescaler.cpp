#include "Sci_PlotRescaler.h"

#include <IBK_UnitList.h>

#include <qwt_interval.h>

#include "Sci_Chart.h"
#include "Sci_AbstractCartesianChartModel.h"

namespace SCI {

PlotRescaler::PlotRescaler(SCI::Chart * chart) :
	QwtPlotRescaler(chart->canvas()),
	m_minValueOffset(0),
	m_chart(chart)
{
	setRescalePolicy(QwtPlotRescaler::Fitting);
	setExpandingDirection(QwtPlotRescaler::ExpandUp); // adjust max value, since we specify min value
	setAspectRatio(0.0); // no axis is scaled
}


void PlotRescaler::setAdjustingLeftAxis(bool leftAxisAdjusted) {
	setAspectRatio(0.0);
	if (leftAxisAdjusted) {
		setReferenceAxis( QwtPlot::xBottom);
		setAspectRatio(QwtPlot::yLeft, 1.0);
	}
	else {
		setReferenceAxis( QwtPlot::yLeft);
		setAspectRatio(QwtPlot::xBottom, 1.0);
	}

	// update aspect ratio based on x and yLeft axis units
	try {
		IBK::Unit xAxis(m_chart->model()->axisData(SCI::AbstractChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());
		IBK::Unit yAxis(m_chart->model()->axisData(SCI::AbstractChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());

		if (leftAxisAdjusted) {
			double fact;
			unsigned int op;
			yAxis.relate_to(xAxis, fact, op);
			if (op == IBK::UnitList::OP_MUL)
				const_cast<PlotRescaler*>(this)->setAspectRatio(QwtPlot::yLeft, fact);
			else
				const_cast<PlotRescaler*>(this)->setAspectRatio(QwtPlot::yLeft, 1.0); // fall back to 1
		}
		else {
			double fact;
			unsigned int op;
			xAxis.relate_to(yAxis, fact, op);
			if (op == IBK::UnitList::OP_MUL)
				const_cast<PlotRescaler*>(this)->setAspectRatio(QwtPlot::xBottom, fact);
			else
				const_cast<PlotRescaler*>(this)->setAspectRatio(QwtPlot::xBottom, 1.0); // fall back to 1
		}
	}
	catch (...) {
		// in case of error, fall back to aspect ratio of 1
		if (leftAxisAdjusted)
			const_cast<PlotRescaler*>(this)->setAspectRatio(QwtPlot::yLeft, 1.0);
		else
			const_cast<PlotRescaler*>(this)->setAspectRatio(QwtPlot::xBottom, 1.0);
	}
}


QwtInterval PlotRescaler::syncScale( int axis,
	const QwtInterval& reference, const QSize &size ) const
{
	QwtInterval intv = QwtPlotRescaler::syncScale(axis, reference, size);
	intv.setMaxValue(intv.maxValue() + m_minValueOffset);
	intv.setMinValue(intv.minValue() + m_minValueOffset);

	return intv;
}


} // namespace SCI
