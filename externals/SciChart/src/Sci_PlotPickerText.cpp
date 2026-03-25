/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_PlotPickerText.h"

#include <QDateTime>

#include <qwt_plot.h>
#include <qwt_scale_map.h>

#include "Sci_ChartAxis.h"
#include "Sci_Chart.h"
#include "Sci_ColorGridSeries.h"

namespace SCI {

PlotPickerText::PlotPickerText(const ChartAxis * bottomAxis) :
	m_trackerTextVisible(true),
	m_bottomAxis(bottomAxis)
{
}


QwtText PlotPickerText::composeTrackerText (const QwtPlot * plot, QwtPicker::RubberBand rubberBand,
											const QPoint & canvasPos) const
{
	if (!m_trackerTextVisible)
		return QwtText();

	// transform into plot coordinate
	double x = plot->canvasMap(QwtPlot::xBottom).invTransform(canvasPos.x());
	double y1 = plot->canvasMap(QwtPlot::yLeft).invTransform(canvasPos.y());
	double y2 = plot->canvasMap(QwtPlot::yRight).invTransform(canvasPos.y());

	QString xText = QString("%L1 %2").arg(x).arg(m_xUnit);
	if (m_bottomAxis->isDateTime()) {
		const int FormatBack = 1;
		xText = m_bottomAxis->label(x, FormatBack).text();
	}
	QString yText = QString("%L1 %2").arg(y1).arg(m_y1Unit);
	QString y2Text = QString("%L1 %2").arg(y2).arg(m_y2Unit);

	QString text;
	switch (rubberBand) {
		case QwtPicker::HLineRubberBand:
			text = xText;
		break;
		case QwtPicker::VLineRubberBand:
			text = yText;
			if ( !m_y2Unit.isEmpty() && m_y2Unit != "undefined")
				text = QString("%1, %2").arg(yText).arg(y2Text);
		break;
		default:
			text = QString("%1, %2").arg(xText).arg(yText);
			if ( !m_y2Unit.isEmpty() && m_y2Unit != "undefined" ) {
				text = QString("%1, %2, %3").arg(xText).arg(yText).arg(y2Text);
			}
	}

	// special handling for color map plots
	if (!m_valueUnit.isEmpty() && m_valueUnit != "undefined") {
		// look up current plot spectrogram
		const SCI::Chart * chart = dynamic_cast<const SCI::Chart*>(plot);
		const ChartSeries* ser = chart->seriesAt(0);
		const ColorGridSeries * colorSeries = dynamic_cast<const ColorGridSeries *>(ser);
		if (colorSeries != nullptr) {
			const SCI::PlotSpectrogram*	spectro = colorSeries->spectrogram();
			// find element number
			double val = spectro->value(x,y1);
			if (val != val) {
				text = QString("%1, %2").arg(xText, yText);
			}
			else {
				QString y2Text = QString("%L1 %2").arg(val).arg(m_valueUnit);
				text = QString("%1, %2, %3").arg(xText, yText, y2Text);
			}
		}
		/// \todo : implement for vector diagram
	}

	QwtText theText(text, QwtText::RichText);

	return theText;
}


} //namespace SCI
