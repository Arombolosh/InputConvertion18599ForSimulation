/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QItemDelegate>
#include <QPen>
#include <QStylePainter>

#include "Sci_PenStyleComboBox.h"
#include "Sci_PenStyleDelegate.h"

namespace SCI {

PenStyleComboBox::PenStyleComboBox(QWidget *parent) : QComboBox(parent)
{
	clear();
	setEditable(false);
	for( int i=0; i<7; ++i)
		addItem("", i);
	setItemDelegate(new PenStyleDelegate(this));
	connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(translateCurrentIndex(int)));
}

Qt::PenStyle PenStyleComboBox::currentStyle() const {
	return static_cast<Qt::PenStyle>(currentIndex());
}

void PenStyleComboBox::setCurrentStyle(Qt::PenStyle style) {
	setCurrentIndex(static_cast<int>(style));
}

void PenStyleComboBox::translateCurrentIndex(int index) {
	emit currentIndexChanged(static_cast<Qt::PenStyle>(index));
}

void PenStyleComboBox::paintEvent(QPaintEvent * /*event*/)
{
	QStylePainter painter(this);
	painter.setPen(palette().color(QPalette::Text));

	// draw the combobox frame, focusrect and selected etc.
	QStyleOptionComboBox opt;
	initStyleOption(&opt);
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);

	// draw the icon and text
	QRect rect = painter.style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);
	QPen pen(Qt::black);
	pen.setWidth(2);
	pen.setStyle(currentStyle());
	painter.setPen(pen);

	// draw pen in rect
	int ypos = rect.height() / 2 + rect.top();
	painter.drawLine(rect.left() + 2, ypos, rect.right() - 2, ypos);
//	painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

} // namespace SCI
