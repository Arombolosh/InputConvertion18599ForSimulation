/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_PlotPanner.h"

#include <qwt_interval.h>

#include "Sci_Chart.h"
#include "Sci_AbstractCartesianChartModel.h"

namespace SCI {

PlotPanner::PlotPanner(QWidget * canvas) : QwtPlotPanner(canvas){
}


void SCI::PlotPanner::moveCanvas(int dx, int dy) {
	QwtPlotPanner::moveCanvas(dx, dy); // this updates axis limits and sets auto-scale to false for all axis

	QwtPlot *plt = plot();
	if ( !plt )
		return;

	SCI::Chart * chart = qobject_cast<SCI::Chart *>(plt);
	Q_ASSERT(chart != NULL);

	SCI::AbstractCartesianChartModel * cartesianChartModel = qobject_cast<SCI::AbstractCartesianChartModel *>(chart->model());

	QwtInterval xInterval = plt->axisInterval(QwtPlot::xBottom);
	cartesianChartModel->setAxisData(false, SCI::AbstractChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
	cartesianChartModel->setAxisData(xInterval.minValue(), SCI::AbstractChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
	cartesianChartModel->setAxisData(xInterval.maxValue(), SCI::AbstractChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum);

	if (cartesianChartModel->axisData(SCI::AbstractChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisEnabled).toBool()) {
		QwtInterval yInterval = plt->axisInterval(QwtPlot::yLeft);
		cartesianChartModel->setAxisData(false, SCI::AbstractChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
		cartesianChartModel->setAxisData(yInterval.minValue(), SCI::AbstractChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
		cartesianChartModel->setAxisData(yInterval.maxValue(), SCI::AbstractChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
	}
	if (cartesianChartModel->axisData(SCI::AbstractChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisEnabled).toBool()) {
		QwtInterval yInterval = plt->axisInterval(QwtPlot::yRight);
		cartesianChartModel->setAxisData(false, SCI::AbstractChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
		cartesianChartModel->setAxisData(yInterval.minValue(), SCI::AbstractChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
		cartesianChartModel->setAxisData(yInterval.maxValue(), SCI::AbstractChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
	}
}



} //namespace SCI



