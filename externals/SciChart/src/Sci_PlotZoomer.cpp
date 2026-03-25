/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QDebug>
#include <QDateTime>
#include "Sci_PlotZoomer.h"
#include "Sci_Chart.h"
#include "Sci_AbstractLineSeriesModel.h"

#include <qwt_plot.h>
#include <qwt_scale_div.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

namespace SCI {

PlotZoomer::PlotZoomer(int xAxis, int yAxis, QWidget * canvas, const ChartAxis * bottomAxis) :
	QwtPlotZoomer(xAxis, yAxis, canvas, false),
	PlotPickerText(bottomAxis)
{
	setTrackerMode(QwtPicker::AlwaysOn);
	setTrackerPen(QColor(Qt::black));
	setRubberBand(QwtPicker::RectRubberBand);
	QColor brightBlue(181,207,255);
	setRubberBandPen(brightBlue);

	// RightButton: zoom out by 1
	setMousePattern(QwtEventPattern::MouseSelect3, Qt::RightButton, Qt::NoModifier);
	// Ctrl+LeftButton: reset zoom
	setMousePattern(QwtEventPattern::MouseSelect2, Qt::LeftButton, Qt::ControlModifier);
}


void PlotZoomer::enablePickMarkerPositionMode() {
	m_pickMarkerPositionMode = true;
	setTrackerMode(QwtPicker::AlwaysOn);
	setRubberBand(QwtPicker::CrossRubberBand);
	setStateMachine(new QwtPickerClickPointMachine );
}


QwtText PlotZoomer::trackerText (const QPoint & canvasPos) const {
	return composeTrackerText(plot(), rubberBand(), canvasPos);
}


void PlotZoomer::begin() {
	QwtPlotZoomer::begin();
}


bool PlotZoomer::end(bool ok) {
	bool res = QwtPlotZoomer::end(ok); // emits a selected() signal if in click-point mode
	if (m_pickMarkerPositionMode) {
		// reset back to zoom mode
		setRubberBand(QwtPicker::RectRubberBand);
		setStateMachine(new QwtPickerDragRectMachine );
		m_pickMarkerPositionMode = false;
		// also disconnect out selected signal again
		disconnect(SIGNAL(selected( const QPointF &)));
	}
	return res;
}


void PlotZoomer::rescale() {
	QwtPlot *plt = plot();
	if ( !plt )
		return;

	SCI::Chart * chart = qobject_cast<SCI::Chart *>(plt);
	Q_ASSERT(chart != NULL);


	const QRectF &rect = zoomRect(); // in plot coordinates
	if ( rect != scaleRect() ) {
		chart->prohibitUpdate();

		const bool doReplot = plt->autoReplot();
		plt->setAutoReplot(false);

		SCI::AbstractCartesianChartModel * cartesianChartModel = qobject_cast<SCI::AbstractCartesianChartModel *>(chart->model());
		// set the autoscale property, which will trigger SCI::Chart::onAxisChanged() which in turn updates the axis scaling properties
		// Note: this will also update the axis property widget, which will, however, access the current (old) axis limits and show them
		//       in the property widget. Therefore, we manually inform the property widget afterwards via change signal.
		bool autoScale = (zoomRectIndex() == 0);

		if (!autoScale) {
			double x1 = rect.left();
			double x2 = rect.right();
			if ( plt->axisScaleDiv(QwtPlot::xBottom).lowerBound() > plt->axisScaleDiv(QwtPlot::xBottom).upperBound() ) {
				qSwap(x1, x2);
			}
			if ( x1 > x2 )
				qSwap(x1, x2);

			cartesianChartModel->setAxisData(x1, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
			cartesianChartModel->setAxisData(x2, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum);

			double y1 = rect.top();
			double y2 = rect.bottom();
			if ( plt->axisScaleDiv(QwtPlot::yLeft).lowerBound() > plt->axisScaleDiv(QwtPlot::yLeft).upperBound() ) {
				qSwap(y1, y2);
			}
			if ( y1 > y2 )
				qSwap(y1, y2);

			double newY1 = y1;
			double newY2 = y2;

			// zoom only into right axis if we have a line series model/chart
			if (qobject_cast<SCI::AbstractLineSeriesModel*>(cartesianChartModel) != NULL) {

				// for the y2 axis, we first need to convert plot coordinates back to canvasCoordinates
				QwtScaleMap yMap = plot()->canvasMap(QwtPlot::yLeft);
				QwtScaleMap y2Map = plot()->canvasMap(QwtPlot::yRight);

				y1 = rect.top();
				y2 = rect.bottom();
				int canvasY1 = yMap.transform(y1);
				int canvasY2 = yMap.transform(y2);
				// now we convert them back to y2-axis coordinates
				y1 = y2Map.invTransform(canvasY1);
				y2 = y2Map.invTransform(canvasY2);
				if (plt->axisScaleDiv(QwtPlot::yRight).lowerBound() > plt->axisScaleDiv(QwtPlot::yRight).upperBound()) {
					qSwap(y1, y2);
				}
				if (y1 > y2)
					qSwap(y1, y2);

				cartesianChartModel->setAxisData(y1, SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
				cartesianChartModel->setAxisData(y2, SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
			}

			cartesianChartModel->setAxisData(newY1, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
			cartesianChartModel->setAxisData(newY2, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMaximum);

			// Now set the autoscale properties
			// Mind: When we turn off autoScale, the model will automatically transfer the cached min/max values to
			// the axis scale engine. This is why we have to wait until all access to the previous transformation maps (canvasMap())
			// is done.
			cartesianChartModel->setAxisData(autoScale, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
			cartesianChartModel->setAxisData(autoScale, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
			if (qobject_cast<SCI::AbstractLineSeriesModel*>(cartesianChartModel) != NULL)
				cartesianChartModel->setAxisData(autoScale, SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
		}
		plt->setAutoReplot(doReplot);

		chart->allowUpdateAndReplot();

		// in case of autoscale, the last call (chart update) has modified the axis min/max values (due to scale engine)
		// we now need to inform the axis property widget to show the updated values. Since we cannot send the axisChanged() signal directly,
		// we simply switch off autoscale and on again, whereby the axis minimum/maximum will not change and the property widget will
		// get the currect scales
		if (autoScale) {
			chart->prohibitUpdate();

			const bool doReplot = plt->autoReplot();
			plt->setAutoReplot(false);

			cartesianChartModel->setAxisData(false, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
			cartesianChartModel->setAxisData(false, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
			if (qobject_cast<SCI::AbstractLineSeriesModel*>(cartesianChartModel) != NULL)
				cartesianChartModel->setAxisData(false, SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);

			cartesianChartModel->setAxisData(true, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
			cartesianChartModel->setAxisData(true, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
			if (qobject_cast<SCI::AbstractLineSeriesModel*>(cartesianChartModel) != NULL)
				cartesianChartModel->setAxisData(true, SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
			plt->setAutoReplot(doReplot);

			chart->allowUpdate();
		}
	}
}


QSizeF PlotZoomer::minZoomSize() const {
	QSizeF minS = QwtPlotZoomer::minZoomSize();
	const QwtPlot *plt = plot();
	if ( !plt )
		return minS;

	const SCI::Chart * chart = qobject_cast<const SCI::Chart *>(plt);
	Q_ASSERT(chart != NULL);

	const SCI::AbstractCartesianChartModel * cartesianChartModel = qobject_cast<const SCI::AbstractCartesianChartModel *>(chart->model());
	// in case of logatithmic y-axis, reduce zoom limit
	if (cartesianChartModel->axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisLogarithmic).toBool()) {
		minS.setHeight( 1e-50);
	}
	return minS;
}

bool SCI::PlotZoomer::accept(QPolygon &pa) const {
	// if we are in picking-mode, we accept a single point
	if (rubberBand() == QwtPicker::CrossRubberBand)
		return true;

	if ( pa.count() < 2 )
		return false;

	QRect rect = QRect( pa[0], pa[int( pa.count() ) - 1] );
	rect = rect.normalized();

	// minimum zoom rectangle to prevent accidental clicks
	const int minSize = 10;
	if ( rect.width() < minSize && rect.height() < minSize )
		return false;

	// minimum zoom distance to use, 50 pixels should give enough fine-tuning space
	const int minZoomSize = 51;

	const QPoint center = rect.center();
	rect.setSize( rect.size().expandedTo( QSize( minZoomSize, minZoomSize ) ) );
	rect.moveCenter( center );

	pa.resize( 2 );
	pa[0] = rect.topLeft();
	pa[1] = rect.bottomRight();

	return true;
}


} //namespace SCI



