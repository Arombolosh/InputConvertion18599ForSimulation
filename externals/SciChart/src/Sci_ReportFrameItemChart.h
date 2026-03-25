#ifndef SCI_ReportFrameItemChartH
#define SCI_ReportFrameItemChartH

#include <QtExt_ReportFrameItemBase.h>

#include "Sci_Chart.h"

namespace SCI {

class ReportFrameItemChart : public QtExt::ReportFrameItemBase
{
public:
	ReportFrameItemChart(Chart* chart, QPaintDevice* paintDevice, double width, unsigned int spaceAfter, unsigned int spaceBefore, bool canBreakAfter = false);

	/*! Create a surrounding rect based on the current settings and the given paintDevice and width.
		This base version create a rect with the whole width and the height based on space before and after.
	*/
	virtual void setCurrentRect() override;

	/*! Draw the item with the given painter at the given position and set the position for the next item.*/
	virtual void drawItem(QPainter* painter, QPointF& pos) override;

//	/*! Clone the current frame item.*/
//	virtual QtExt::ReportFrameItemBase* clone() const override;

private:
	Chart*	m_chart;
};

} // namespace Sci

#endif // SCI_ReportFrameItemChartH
