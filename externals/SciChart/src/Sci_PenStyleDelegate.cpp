/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QPen>
#include <QPainter>

#include "Sci_PenStyleDelegate.h"

namespace SCI {

PenStyleDelegate::PenStyleDelegate(QObject *parent) : QItemDelegate(parent)
{
}

void PenStyleDelegate::paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const {
//	if (option.state & QStyle::State_Selected) {
//		QPen normalPen(Qt::SolidLine);
//		normalPen.setWidth(2);
//		normalPen.setColor(Qt::blue);
//		painter->setPen(normalPen);
//		painter->drawRect(option.rect);
//	}
	QItemDelegate::paint(painter, option, index);
	// set pen
	QPen pen(Qt::black);
	pen.setWidth(2);
	Qt::PenStyle style = static_cast<Qt::PenStyle>(index.row());
	pen.setStyle(style);
	painter->setPen(pen);

	// draw pen in rect
	const QRect& rect = option.rect;
	int ypos = rect.height() / 2 + rect.top();
	painter->drawLine(rect.left() + 2, ypos, rect.right() - 2, ypos);
}

} // namespace SCI
