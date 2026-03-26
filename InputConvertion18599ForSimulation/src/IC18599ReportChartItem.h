#ifndef IC18599ReportChartItemH
#define IC18599ReportChartItemH

#include <QtExt_ReportFrameItemBase.h>

#include <QColor>

#include <IBK_UnitVector.h>

namespace SCI {
	class Chart;
	class LineSeriesContentModel;
}

/*! Thin wrapper around SCI::Chart for report line charts with configurable aspect ratio.
	Unlike SCI::ReportFrameItemLineDataChart (which uses fixed width/2 height),
	this class allows specifying the chart height via an aspect divisor.
*/
class IC18599ReportChartItem : public QtExt::ReportFrameItemBase {
public:
	/*! Constructor.
		\param paintDevice Paint device for rendering.
		\param width Full chart width.
		\param aspectDivisor Height = width / aspectDivisor (e.g. 3.5 for compact charts).
		\param spaceAfter Space after the chart.
		\param spaceBefore Space before the chart.
		\param bottomAxisTitle Title of X axis.
		\param leftAxisTitle Title of Y axis.
	*/
	IC18599ReportChartItem(QPaintDevice *paintDevice, double width, double aspectDivisor,
						   unsigned int spaceAfter, unsigned int spaceBefore,
						   const QString &bottomAxisTitle, const QString &leftAxisTitle);

	~IC18599ReportChartItem();

	/*! Add a line data series. */
	void addLineData(const IBK::UnitVector &x, const IBK::UnitVector &y,
					 const QString &name, const QColor &col, int lineWidth);

	/*! Set fixed axis range. If bottomAxis is false, sets the left (Y) axis. */
	void setAxisRange(bool bottomAxis, double minVal, double maxVal);

	/*! Set maximum number of major tick intervals for an axis. */
	void setAxisMajorTicks(bool bottomAxis, int maxTicks);

	/*! Install weekday labels on the bottom X-axis (expects X data in hours 0-168). */
	void setWeekdayXAxis();

	void setCurrentRect() override;
	void drawItem(QPainter *painter, QPointF &pos) override;

private:
	SCI::LineSeriesContentModel	*m_chartModel;
	SCI::Chart					*m_chart;
};

#endif // IC18599ReportChartItemH
