#ifndef SCI_ReportFrameItemTimeLineDataChartH
#define SCI_ReportFrameItemTimeLineDataChartH

#include <QApplication>

#include <QtExt_ReportFrameItemBase.h>

#include <IBK_UnitVector.h>

namespace SCI {

class Chart;
class LineSeriesContentModel;

/*! Clase for simplifying of rendering a line chart to a report.
	It handles only line charts where the bottom axis is time.
	It can contain more than one line but all data sets must have the same base unit (only one y axis on the left)
*/
class ReportFrameItemTimeLineDataChart : public QtExt::ReportFrameItemBase {
	Q_DECLARE_TR_FUNCTIONS(ReportFrameItemTimeLineDataChart)
public:
	/*! Standard constructor
		\param paintDevice Paint device used for rendering
		\param width Width of the complete chart
		\param yearChart If true only one year will be shown and the bottom axis uses a date-time format with month view
		\param spaceAfter Space after the chart
		\param spaceBefore Space before the chart
		\param bottomAxisTitle Title of bottom axis (time)
		\param leftAxisTitle Title of the left axis
		\param canPageBreakAfter If true a page break is possible after the chart
	*/
	ReportFrameItemTimeLineDataChart(QPaintDevice* paintDevice, double width, bool yearChart, unsigned int spaceAfter, unsigned int spaceBefore,
									 const QString& bottomAxisTitle, const QString& leftAxisTitle, bool canPageBreakAfter = false, int pointSize = 10, double chartHeight = 0);

	/*! Copy constructor deleted.*/
	ReportFrameItemTimeLineDataChart(const ReportFrameItemTimeLineDataChart&) = delete;

	/*! Destructor.*/
	~ReportFrameItemTimeLineDataChart();

	/*! Add a data set for a line.
		\param x Time data
		\param y Value data. In case of existing lines all data must have the same base unit
		\param name Name of the data set. Is used for legend in case of more than one line
		\param col Color of the line
		\param lineWidth Width of the line (line is always solid).
	*/
	void addLineData(const IBK::UnitVector& x, const IBK::UnitVector& y, const QString& name, const QColor& col, int lineWidth);

	/*! Delete all data sets.*/
	void clearLineData();

	/*! Set axis scales by minimum and maximum
	*/
	void setAxisScaleLimits(int min, int max);

	/*! Set properties for the bottom axis (time). Should be used if yearChart flag in constructor is false.
	*/
	void setTimeAxis(bool autoScale, bool dateTime, int zeroYear = 2000, int zeroMonth = 1, int zeroDay = 1, double max = 0, double min = 0, const IBK::Unit& axitUnit = IBK::Unit("d"), const QString& format = QString());

	/*! Set properties for the left axis (value).
	*/
	void setValueAxis(bool autoScale, double max = 0, double min = 0, const IBK::Unit& unit = IBK::Unit());

	void setChartTitle(const QString& title, QFont* font = nullptr);

	/*! Create a surrounding rect based on the current settings and the given paintDevice and width.
		This base version create a rect with the whole width and the height based on space before and after.
	*/
	virtual void setCurrentRect() override;

	/*! Draw the item with the given painter at the given position and set the position for the next item.*/
	virtual void drawItem(QPainter* painter, QPointF& pos) override;

//	/*! Clone the current frame item.*/
//	virtual QtExt::ReportFrameItemBase* clone() const override;

private:
	SCI::LineSeriesContentModel*	m_chartModel;		///< Model for chart data.
	SCI::Chart*						m_chart;			///< Chart for temperature plot
};

} // namespace Sci

#endif // SCI_ReportFrameItemTimeLineDataChartH
