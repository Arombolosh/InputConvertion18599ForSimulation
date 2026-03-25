#include "Sci_PlotScaleItem.h"

#include <QRectF>

#include <qwt_scale_map.h>

namespace SCI {


PlotScaleItem::PlotScaleItem(QwtScaleDraw::Alignment align) :
	QwtPlotScaleItem(align, 0)
{
	// Mind: position is updated just before drawing

	// we disable labels and backbone
	scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, false);
	scaleDraw()->enableComponent(QwtAbstractScaleDraw::Labels, false);
	scaleDraw()->setPenWidthF(1);
}

void PlotScaleItem::draw( QPainter *p,
	const QwtScaleMap &xMap, const QwtScaleMap &yMap,
	const QRectF &scaleRect ) const
{
	// use inverse maps - we need to compute positions of scales such that they are
	// aligned to the respective side in canvas
	double pos = 0; // dummy initialization to make compiler happy
	switch (scaleDraw()->alignment()) {
		//! The scale is below, i.e. xTop axis
		case QwtScaleDraw::BottomScale :
			pos = yMap.invTransform(scaleRect.top()+0.01);
			break;

		//! The scale is above
		case QwtScaleDraw::TopScale :
			pos = yMap.invTransform(scaleRect.bottom()-1);
			break;

		//! The scale is left
		case QwtScaleDraw::LeftScale :
			pos = xMap.invTransform(scaleRect.right()-1);
			break;

		//! The scale is right
		case QwtScaleDraw::RightScale :
			pos = xMap.invTransform(scaleRect.left()+0.01);
			break;

	}
	const_cast<PlotScaleItem*>(this)->setPosition(pos);


	QwtPlotScaleItem::draw(p, xMap, yMap, scaleRect);
}

} // namespace SCI
