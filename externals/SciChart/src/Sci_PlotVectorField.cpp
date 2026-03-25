/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_PlotVectorField.h"

#include <algorithm>

#include <QPainter>

#include <IBK_Constants.h>
#include <IBK_messages.h>

#include "qwt_painter.h"
#include "qwt_plot.h"
#include "qwt_scale_map.h"
#include "qwt_vectorfield_symbol.h"

namespace SCI {



PlotVectorField::PlotVectorField(const QString & title) :
	QwtPlotVectorField(title),
	m_constructionLinePen(Qt::black),
	m_maxLength(50),
	m_minLength(0)
{
	setRenderHint(RenderAntialiased, true);
}


void PlotVectorField::setRasterEnabled(bool on) {
	setPaintAttribute(FilterVectors, on);
}

bool PlotVectorField::rasterEnabled() const {
	return testPaintAttribute(FilterVectors);
}

void PlotVectorField::setArrowColor(const QColor & color) {
	QPen p = pen();
	QBrush b = brush();
	p.setColor(color);
	b.setColor(color);
	setPen(p);
	setBrush(b);
}

QColor PlotVectorField::arrowColor() const {
	return brush().color();
}

double PlotVectorField::arrowLength(double magnitude) const {
	double l = QwtPlotVectorField::arrowLength(magnitude);
	if (l == 0)
		return 0;
	if (l < m_minLength)
		return m_minLength;
	if (l > m_maxLength)
		return m_maxLength;
	return l;
}

void PlotVectorField::setArrowAttributes(bool thinArrow, double maxLength, double minLength, double scaleFactor) {
	if (thinArrow)
		setSymbol( new QwtVectorFieldThinArrow );
	else
		setSymbol( new QwtVectorFieldArrow);
	m_maxLength = maxLength;
	m_minLength = minLength;

	setMagnitudeScaleFactor(scaleFactor);
}


void PlotVectorField::drawSeries( QPainter * p,
	const QwtScaleMap &xMap, const QwtScaleMap &yMap,
	const QRectF &canvasRect, int from, int to ) const
{
	p->setRenderHint( QPainter::Antialiasing, false);

	// first draw boundary and construction lines
	drawBoundaryLines(p, canvasRect, xMap, yMap);

	p->setRenderHint( QPainter::Antialiasing,
		testRenderHint( QwtPlotItem::RenderAntialiased ) );

	// then draw vector arrows
	QwtPlotVectorField::drawSeries(p, xMap, yMap, canvasRect, from, to);
}


static void drawHorizontalLine(QPainter * painter, const QRectF & rect, const QwtScaleMap & xxMap, const QwtScaleMap & yyMap,
						const DATAIO::ConstructionLine2D& line)
{
	int yp = yyMap.transform(line.m_pos);
	if (yp < rect.top()+1 || yp > rect.bottom()+1)
		return;

	// yp is now relative to plot area

	// get length
	int xp_left = xxMap.transform(line.m_begin);
	int xp_right = xxMap.transform(line.m_end);
	if (xp_left > xp_right)
		std::swap(xp_left, xp_right);
	// clip line to plotted area
	xp_right = std::min(xp_right, (int)rect.right());
	xp_left = std::max(xp_left, (int)rect.left());
	painter->drawLine(QPoint(xp_left, yp), QPoint(xp_right, yp));
}


static void drawVerticalLine(QPainter * painter, const QRectF & rect, const QwtScaleMap & xxMap, const QwtScaleMap & yyMap,
						const DATAIO::ConstructionLine2D& line)
{
	int xp = xxMap.transform(line.m_pos);
	if (xp < rect.left()-painter->pen().widthF() || xp > rect.right()+painter->pen().widthF() )
		return;

	int yp_top = yyMap.transform(line.m_begin);
	int yp_bottom = yyMap.transform(line.m_end);
	if (yp_top > yp_bottom)
		std::swap(yp_top, yp_bottom);
	yp_bottom = std::min(yp_bottom, (int)rect.bottom());
	yp_top = std::max(yp_top, (int)rect.top());
	painter->drawLine(QPoint(xp, yp_top), QPoint(xp, yp_bottom));
}

void PlotVectorField::drawBoundaryLines(QPainter * painter, const QRectF & rect,
											const QwtScaleMap & xxMap, const QwtScaleMap & yyMap) const
{
	// draw construction/boundary lines
	if ( m_constructionLinePen.style() == Qt::NoPen )
		return;

	QPen boundaryPen = m_constructionLinePen;
	double width = boundaryPen.widthF();
	width *= 2;
	boundaryPen.setWidthF(width);

	painter->setPen(m_constructionLinePen);
	const std::vector<DATAIO::ConstructionLine2D>& hclines = m_constructionLines.m_horizontalConstructionLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = hclines.begin(); it!=hclines.end(); ++it) {
		drawHorizontalLine(painter, rect, xxMap, yyMap, *it);
	}

	const std::vector<DATAIO::ConstructionLine2D>& vclines = m_constructionLines.m_verticalConstructionLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = vclines.begin(); it!=vclines.end(); ++it) {
		drawVerticalLine(painter, rect, xxMap, yyMap, *it);
	}

	painter->setPen(boundaryPen);
	const std::vector<DATAIO::ConstructionLine2D>& hblines = m_constructionLines.m_horizontalBoundaryLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = hblines.begin(); it!=hblines.end(); ++it) {
		drawHorizontalLine(painter, rect, xxMap, yyMap, *it);
	}

	const std::vector<DATAIO::ConstructionLine2D>& vblines = m_constructionLines.m_verticalBoundaryLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = vblines.begin(); it!=vblines.end(); ++it) {
		drawVerticalLine(painter, rect, xxMap, yyMap, *it);
	}
}


void PlotVectorField::setConstructionLinePen(const QPen& pen) {
	m_constructionLinePen = pen;
}


void PlotVectorField::setConstructionLines(const DATAIO::ConstructionLines2D& clines) {
	m_constructionLines = clines;
}


} // namespace SCI
