#ifndef SCI_PLOTSCALEITEM_H
#define SCI_PLOTSCALEITEM_H

#include <qwt_plot_scaleitem.h>

namespace SCI {

/*! Re-implemented scale item that positions the scale inside the chart at the associated axis.
*/
class PlotScaleItem : public QwtPlotScaleItem {
public:
	/*! Set the alignment using the alignment of the scales and mind that QwtScaleDraw::BottomScale means "scales are drawn below"
		and actually mean this is a top axis.
	*/
	PlotScaleItem(QwtScaleDraw::Alignment align = QwtScaleDraw::BottomScale);

	/*! Overloaded draw function, that computes position of scale such, that it aligns
		to the assigned border.
	*/
	virtual void draw( QPainter *p,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		const QRectF &rect ) const;
};

} // namespace SCI

#endif // SCI_PLOTSCALEITEM_H
