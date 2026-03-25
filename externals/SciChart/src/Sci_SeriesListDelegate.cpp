/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QLineEdit>
#include <QPainter>
#include <QAbstractItemView>
#include <QDebug>

#include <qwt_symbol.h>

#include "Sci_SeriesListDelegate.h"
#include "Sci_SeriesListModel.h"
#include "Sci_LineSeries.h"
#include "Sci_Chart.h"
#include "Sci_AbstractChartModel.h"
#include "Sci_AbstractLineSeriesModel.h"

namespace SCI {

SeriesListDelegate::SeriesListDelegate(SeriesListModel* parentModel, QObject * parent) :
	QItemDelegate(parent),
	m_model(parentModel)
{
	Q_ASSERT(m_model);
}

void SeriesListDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
	QItemDelegate::paint(painter, option, index);

	if (!m_model->chartModel())
		return;

	paintLineSymbolText(painter, option.rect, m_model->chartModel(), index.row());
}


void SeriesListDelegate::paintLineSymbolText( QPainter * painter, const QRect& rect, AbstractChartModel* serModel, int index) {

	painter->setPen(serModel->seriesData(index, AbstractLineSeriesModel::SeriesPen).value<QPen>());
	int ypos = rect.height() / 2 + rect.top();

	// line should 1/5 of the rect long, but at least 5 pixels
	int endpos = std::max(5, rect.left() + rect.width() / 5);

	// rect is to small for line and text - in this case we do not have space for a marker anyway
	if (endpos >= rect.right()) {
		endpos = rect.right() - 1;
		painter->drawLine(rect.left() + 2, ypos, endpos, ypos);
		return;
	}

	QString text = serModel->seriesData(index, AbstractLineSeriesModel::SeriesTitle).toString();
	// text rect with 4 spaces distance to line
	QRect textRect(endpos + 4, rect.top(), rect.right() - endpos - 4, rect.height());
	// get rect necessary for whole text
	QRectF boundRect = painter->boundingRect(textRect, text, Qt::AlignVCenter | Qt::AlignLeft);
	if( boundRect.width() > textRect.width()) {
		// text is too long and will be shortened
		text = painter->fontMetrics().elidedText(text, Qt::ElideMiddle, textRect.width());
	}
	painter->drawLine(rect.left() + 2, ypos, endpos, ypos);
	painter->setPen(QPalette().color(QPalette::Text));
	painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, text);

	// if we have a marker, draw the marker in the middle of the line
	int markerStyle = serModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerStyle).toInt();
	if (markerStyle != QwtSymbol::NoSymbol) {
		QwtSymbol sym;
		sym.setStyle((QwtSymbol::Style)markerStyle);
		int markerSize = serModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerSize).toInt();
		markerSize = qMax(4, markerSize);
		markerSize = qMin(markerSize, rect.height()-2);
		sym.setSize(markerSize);
		bool filled = serModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerFilled).toBool();
		QColor c = serModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerColor).value<QColor>();
		sym.setPen(c);
		if (filled)
			sym.setBrush(c);
		else
			sym.setBrush(Qt::NoBrush);
		// draw symbol centered in line
		int lineCenterX = (rect.left() + 2 + endpos)/2;
		int lineCenterY = ypos;
		QRectF markerRect(lineCenterX - markerSize/2-2, lineCenterY - markerSize/2-2, sym.boundingRect().width(), sym.boundingRect().width());
		painter->save();
		painter->setRenderHint(QPainter::Antialiasing, true);
		sym.drawSymbol(painter, markerRect);
		painter->restore();
	}
}

} // end namespace SCI
