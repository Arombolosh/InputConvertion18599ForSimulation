#include "Sci_ReportFrameItemChart.h"

namespace SCI {

ReportFrameItemChart::ReportFrameItemChart(Chart* chart, QPaintDevice* paintDevice, double width, unsigned int spaceAfter,
										   unsigned int spaceBefore, bool canBreakAfter) :
	QtExt::ReportFrameItemBase(paintDevice, width, spaceAfter, spaceBefore, canBreakAfter),
	m_chart(chart)
{
}

void ReportFrameItemChart::setCurrentRect() {
	m_currentRect = m_chart->geometry();
}

void ReportFrameItemChart::drawItem(QPainter* painter, QPointF& pos) {
	m_chart->render(painter, pos, true);
}

} // namespace Sci
