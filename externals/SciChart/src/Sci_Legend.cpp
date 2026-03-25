/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QScrollBar>
#include <QEvent>
#include <QDebug>

#include <qwt_dyngrid_layout.h>
#include <qwt_legend_label.h>

#include "Sci_Legend.h"
#include "Sci_Chart.h"
#include "Sci_AbstractChartModel.h"

namespace SCI {

Legend::Legend(QwtPlot * parent, QwtPlot::LegendPosition pos) :
	QwtLegend(parent),
	m_spacing(5)
{
	Q_ASSERT(parent != nullptr);
	horizontalScrollBar()->setVisible(false);
	verticalScrollBar()->setVisible(false);

	/// \todo set spacing
	//	contentsWidget()->layout()->setSpacing(50);

	// add to chart, plot takes ownership
	parent->insertLegend(this, pos);
	// register
	connect(parent, &QwtPlot::legendDataChanged, this, &Legend::sortLegend);
	// the above is short for:
	// connect(
	//     parent,  SIGNAL( legendDataChanged( const QVariant &, const QList<QwtLegendData> & ) ),
	//     this, SLOT( sortLegend() )
	// );

}


void Legend::setSpacing(int space) {
	QLayout *contentsLayout = contentsWidget()->layout();
	if ( contentsLayout ) {
		for ( int i = 0; i < contentsLayout->count(); i++ ) {
			QLayoutItem *item = contentsLayout->itemAt( i );
			QWidget * w = item->widget();
			QwtLegendLabel * label = qobject_cast<QwtLegendLabel *>(w);
			if (label != nullptr)
				label->setSpacing(space);
		}
	}
	m_spacing = space;
}


void Legend::setMaxColumns(unsigned int maxColumns) {
	QwtLegend::setMaxColumns(maxColumns);
	updateGeometry(); // signal layout system to update/recalculate geometry
}


QWidget * Legend::createWidget( const QwtLegendData & d) const {
	QWidget * w = QwtLegend::createWidget(d);
	QwtLegendLabel * label = qobject_cast<QwtLegendLabel *>(w);
	if (label != nullptr)
		label->setSpacing(m_spacing);
	return w;
}


void Legend::sortLegend() {
	// get layout from legend contentswidget
	QwtDynGridLayout *legendLayout =
		qobject_cast<QwtDynGridLayout *>( contentsWidget()->layout() );

	if ( !legendLayout ) return;
}


} // namespace SCI
