#include "Sci_CurveTracker.h"

#include <cmath>

#include <QPen>

#include <qwt_dyngrid_layout.h>
#include <qwt_graphic.h>
#include <qwt_plot.h>
#include <qwt_scale_map.h>
#include <qwt_text.h>

#include "Sci_PlotCurve.h"

namespace SCI {

CurveTracker::CurveTracker(QWidget * canvas, const ChartAxis * bottomAxis) :
	QwtPlotTracker( canvas ),
	PlotPickerText(bottomAxis)
{
	const QPen pen( "Navy" );

	setRubberBand( VLineRubberBand );
	setRubberBandPen( pen );

	setBorderPen( pen );
	setBorderRadius( 10 );

	QColor bg( Qt::white );
	bg.setAlpha( 180 );

	setBackgroundBrush( bg );
	setMaxColumns( 1 );

	// a monospace font to have the numbers aligned
#ifdef Q_OS_WIN
	setTitleFont( QFont( "Consolas", 10, QFont::Bold ) );
	setTrackerFont( QFont( "Consolas", 9 ) );
#elif defined(Q_OS_MAC)
	setTitleFont( QFont( "Menlo", 10, QFont::Bold ) );
	setTrackerFont( QFont( "Menlo", 9 ) );
#else
	// all other systems like Linux
	setTitleFont( QFont( "Monospace", 10, QFont::Bold ) );
	setTrackerFont( QFont( "Monospace", 9 ) );
#endif
}


void CurveTracker::setVisible(bool visible) {
	m_visible = visible;
	// turn off rubber band
	if (!visible)
		setRubberBand( NoRubberBand );
	else
		setRubberBand( VLineRubberBand );
}


void CurveTracker::setNumberFormat(char format) {
	m_format = format;
}


void CurveTracker::setNumberPrecision(int precision) {
	m_precision = precision;
}


QList<QVariant> CurveTracker::trackerDataAt(const QwtPlotItem * plotItem, const QPointF & pos) const {
	QList< QVariant > data;

	const QwtPlotCurve * plotCurve = dynamic_cast<const QwtPlotCurve*>(plotItem);
	if (plotCurve == nullptr)
		return data;

	// interpolate value
	const double x = plotCurve->interpolatedValueAt( Qt::Horizontal, pos.x() );
	if ( qIsNaN( x ) )
		return data;

	const QRectF br = plotCurve->boundingRect();
	if ( br.width() <= 0.0 )
		return data;

	// depending on format, use different layouting
	const int fieldWidth = 1 + 2 + 1 + m_precision;

	QString info( "%L1: %2" );
	info = info.arg( x, fieldWidth, m_format, m_precision );
	info = info.arg( plotCurve->title().text() );

	QwtText label(info);
	label.setRenderFlags( Qt::AlignLeft | Qt::AlignVCenter ); // left aligned

	if ( !label.isEmpty() )
	{
		const QwtGraphic graphic = plotItem->legendIcon( 0, plotItem->legendIconSize() );
		if ( !graphic.isNull() )
			data += QVariant::fromValue( graphic );

		data += QVariant::fromValue( label );
	}

	return data;
}


QwtText CurveTracker::title() const {
	QString title;

	const QwtPlot* plot = this->plot();
	if ( plot == nullptr )
		return QString();

	QwtText text(composeTrackerText(plot, QwtPicker::CrossRubberBand, trackerPosition()));
	text.setRenderFlags( Qt::AlignRight | Qt::AlignVCenter );
	return text;
}


void CurveTracker::drawTracker(QPainter * p) const {
	if (m_visible)
		QwtPlotTracker::drawTracker(p);
}


bool lessThanByDistance(const std::pair<double, const SCI::PlotCurve*> & lhs, const std::pair<double, const SCI::PlotCurve*> & rhs) {
	return lhs.first < rhs.first;
}

void CurveTracker::updateLayoutItems(const QPointF & trackerPosition, QwtDynGridLayout & layout) const {
	// complete replacement of original code
	const QwtPlot* plt = plot();
	if ( plt == nullptr )
	{
		// just a precaution: this is normally not necessary, as without plot, this function won't be called at all
		for ( int i = layout.count() - 1; i >= 0; i-- )
			delete layout.takeAt( i );
		return;
	}

	int index = 0;

	double pos[ QwtPlot::axisCnt ];
	const double x = trackerPosition.x();
	const double y = trackerPosition.y();

	pos[ QwtPlot::yLeft ] = plt->canvasMap( QwtPlot::yLeft ).invTransform( y );
	pos[ QwtPlot::yRight ] = plt->canvasMap( QwtPlot::yRight ).invTransform( y );
	pos[ QwtPlot::xBottom ] = plt->canvasMap( QwtPlot::xBottom ).invTransform( x );
	pos[ QwtPlot::xTop ] = plt->canvasMap( QwtPlot::xTop ).invTransform( x );

	QwtInterval yAxisInterval[ QwtPlot::axisCnt ];
	yAxisInterval[QwtPlot::yLeft] = plt->axisInterval( QwtPlot::yLeft );
	yAxisInterval[QwtPlot::yRight] = plt->axisInterval( QwtPlot::yRight );

	// first collect a list of items to be inserted in legend at all, with interpolated y value as key for sorting and filtering
	std::vector<std::pair<double, const SCI::PlotCurve*> > plotItemList;
	const QwtPlotItemList& items = plt->itemList();
	for ( QwtPlotItemIterator it = items.begin(); it != items.end(); ++it )
	{
		const QwtPlotItem* plotItem = *it;
		if ( plotItem->testItemAttribute( QwtPlotItem::Tracker ) )  {
			const SCI::PlotCurve* curve = dynamic_cast<const SCI::PlotCurve*>(plotItem);
			if (curve != nullptr) {
				double y = curve->interpolatedValueAt(Qt::Horizontal, pos[ plotItem->xAxis() ]);
				if (y != y)
					continue; // ignore NANs
				// only include if in visible range, i.e. if the intersection point is in view
				if (y >= yAxisInterval[ plotItem->yAxis() ].minValue() && y <= yAxisInterval[ plotItem->yAxis()].maxValue() )
					plotItemList.push_back(std::make_pair(y, curve));
			}
		}
	}

	// now apply filtering and sorting
	// Note: currently plotItemList contains elements in order of plot items
	std::stable_sort(plotItemList.begin(), plotItemList.end(), lessThanByDistance);

	const SCI::PlotCurve* closestLine = nullptr; // indicates, which line is closest on cursor
	std::vector< std::pair<double, const SCI::PlotCurve*> > curves; // we have a list that holds all sorted lines in order

	const unsigned int MAX_CURVES = 20;

	double dist = std::numeric_limits<double>::max();
	// transfer curves in sorted order (from high to low) and find curve closest to cursor
	for (std::vector<std::pair<double, const SCI::PlotCurve*> >::const_iterator it = plotItemList.begin(); it != plotItemList.end(); ++it) {
		const SCI::PlotCurve* c = it->second;
		double curveY = it->first;

		double d = std::fabs(pos[ c->yAxis() ] - curveY);
		if (d < dist) {
			closestLine = c;
			dist = d;
		}

		if (curves.size() >= MAX_CURVES) {
			// too many curves, remove first curve that is further away than the current
			unsigned int i=0;
			for (; i<curves.size(); ++i) {
				if (curves[i].first > d) {
					curves.erase(curves.begin() + i);
					break;
				}
			}
			// removed one curve? otherwise all curves in vector are closer than current and we drop the current curve
			if (i < curves.size())
				curves.push_back( std::make_pair(d, c) );
		}
		else
			curves.push_back( std::make_pair(d, c) );
	}


	// finally, update/insert all
	for (std::vector< std::pair<double, const SCI::PlotCurve*> >::reverse_iterator rit = curves.rbegin(); rit != curves.rend(); ++rit) {
		const SCI::PlotCurve* plotItem = (*rit).second;
		const QPointF trackerPos(
			pos[ plotItem->xAxis() ], pos[ plotItem->yAxis() ] );

		QList< QVariant > data
			= trackerDataAt( plotItem, trackerPos );

		if ( !data.isEmpty() )
		{
			LayoutItem* layoutItem = nullptr;
			if ( index < layout.count() )
			{
				layoutItem = static_cast< LayoutItem* >( layout.itemAt( index ) );
			}
			else
			{
				layoutItem = new LayoutItem( this );
				layout.addItem( layoutItem );
			}

			layoutItem->setPlotItem( plotItem );
			if (plotItem == closestLine) {
				// mark closest line as bold
				QwtText label = data[1].value<QwtText>();
				QFont f = label.font();
				f.setBold(true);
				label.setFont(f);
				data[1] = QVariant::fromValue(label);
			}
			layoutItem->setTrackerData( data );

			index++;
		}
	}

	for ( int i = layout.count() - 1; i >= index; i-- )
		delete layout.takeAt( i );

	layout.invalidate();
	layout.activate();
}


} // namespace SCI






