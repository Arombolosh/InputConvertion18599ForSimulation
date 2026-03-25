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
#include <QStylePainter>

#include "Sci_LineSeriesSelectionComboBox.h"
#include "Sci_SeriesListDelegate.h"
#include "Sci_SeriesListModel.h"
#include "Sci_AbstractChartModel.h"
#include "Sci_AbstractLineSeriesModel.h"

namespace SCI {

LineSeriesSelectionComboBox::LineSeriesSelectionComboBox(QWidget *parent) :
	QComboBox(parent),
	m_chartModel(0),
	m_seriesListModel(0)
{
	clear();
	setEditable(false);
	setEnabled(false);
}

LineSeriesSelectionComboBox::~LineSeriesSelectionComboBox(){
	delete m_seriesListModel;
}

void LineSeriesSelectionComboBox::setModel(AbstractChartModel* model) {

	if (m_chartModel) {
		disconnect( m_chartModel, SIGNAL(seriesViewChanged(int,int,int)), this, SLOT(repaint()));
	}

	m_chartModel = dynamic_cast<AbstractLineSeriesModel*>(model);

	// remove previous series list model
	delete m_seriesListModel;
	m_seriesListModel = NULL;

	// if we have a new model, create new SeriesListModel and populate combo box
	if ( m_chartModel) {

		m_seriesListModel = new SeriesListModel(m_chartModel, this);
		setItemDelegate(new SeriesListDelegate(m_seriesListModel, this));

		// reset combo box model (cleans out old items and sets new items)
		QComboBox::setModel(m_seriesListModel);
		setEnabled(true);
		/// \todo Need to listen to Chart::modelReset(), since only then the first line information (series) is added
		/// then select the index
		setCurrentIndex(0);
		connect( m_chartModel, SIGNAL(seriesViewChanged(int,int,int)), this, SLOT(repaint()));

	}
	else {
		// clear
		clear();
		setEnabled(false);
		setCurrentIndex(-1);
	}
}


void LineSeriesSelectionComboBox::paintEvent(QPaintEvent * /*event*/) {
	QStylePainter painter(this);
	painter.setPen(palette().color(QPalette::Text));

	// draw the combobox frame, focusrect and selected etc.
	QStyleOptionComboBox opt;
	initStyleOption(&opt);
	painter.drawComplexControl(QStyle::CC_ComboBox, opt);

	if( m_chartModel) {
		// draw the icon and text
		QRect rect = painter.style()->subControlRect(QStyle::CC_ComboBox, &opt, QStyle::SC_ComboBoxEditField, this);

		SeriesListDelegate::paintLineSymbolText(&painter, rect, m_chartModel, currentIndex());
	}
}

} // namespace SCI
