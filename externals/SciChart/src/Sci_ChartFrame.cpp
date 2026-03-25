/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_ChartFrame.h"

#include <QPainter>

#include <qwt_plot_renderer.h>
#include <qwt_text.h>

namespace SCI {

ChartFrame::ChartFrame(QObject *parent) :
	QObject(parent),
	m_chart(new Chart(0))
{}

ChartFrame::~ChartFrame() {
	delete m_chart;
}

void ChartFrame::setChartModel(AbstractChartModel* model) {
	m_chart->setModel(model);
}

qreal ChartFrame::frameHeight(QPaintDevice* paintDevice, qreal width) {
	if( !paintDevice)
		return 0;

	if( width <= 0)
		width = paintDevice->width();

	qreal chartHeight =  m_chart->geometry().height() * width / m_chart->geometry().width();
	return chartHeight;
}

void ChartFrame::drawFrame(QPainter* painter, const QPointF& pos, qreal width) {
	QPaintDevice* device = painter->device();
	if( !device)
		return;

	qreal height = frameHeight(device, width);
	QwtPlotRenderer renderer;
	renderer.setDiscardFlags(QwtPlotRenderer::DiscardBackground);
	QSizeF chartSize = QSizeF(width, height);
	QRectF chartrect = QRectF(pos, chartSize);
	renderer.render(m_chart, painter, chartrect);
}


} // namespace SCI
