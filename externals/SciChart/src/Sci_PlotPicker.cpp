/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_PlotPicker.h"

#include <QDateTime>

#include <qwt_plot.h>
#include <qwt_scale_map.h>
#include <qwt_text.h>

namespace SCI {

PlotPicker::PlotPicker(int xAxis, int yAxis, QWidget * canvas, const ChartAxis * bottomAxis):
	QwtPlotPicker(xAxis, yAxis, canvas),
	PlotPickerText(bottomAxis)
{
}


QwtText PlotPicker::trackerText (const QPoint & canvasPos) const {
	return composeTrackerText(plot(), rubberBand(), canvasPos);
}


void PlotPicker::begin() {
	setTrackerMode(QwtPicker::AlwaysOff);
	QwtPlotPicker::begin();
}


bool PlotPicker::end(bool ok) {
	setTrackerMode(QwtPicker::AlwaysOn);
	return QwtPicker::end(ok);
}


} //namespace SCI
