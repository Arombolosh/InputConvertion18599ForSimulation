#ifndef SCI_ReportFrameItemBarDataChartH
#define SCI_ReportFrameItemBarDataChartH

#include <QApplication>

#include <QtExt_ReportFrameItemBase.h>

#include <IBK_UnitVector.h>

namespace SCI {

class Chart;
class BarSeriesContentModel;

/*! Clase for simplifying of rendering a line chart to a report.
	It handles only line charts where the bottom axis is time.
	It can contain more than one line but all data sets must have the same base unit (only one y axis on the left)
*/
class ReportFrameItemBarDataChart : public QtExt::ReportFrameItemBase {
	Q_DECLARE_TR_FUNCTIONS(ReportFrameItemBarDataChart)
public:
	/*! Standard constructor
		\param paintDevice Paint device used for rendering
		\param width Width of the complete chart
		\param spaceAfter Space after the chart
		\param spaceBefore Space before the chart
		\param bottomAxisTitle Title of bottom axis (time)
		\param leftAxisTitle Title of the left axis
		\param canPageBreakAfter If true a page break is possible after the chart
	*/
	ReportFrameItemBarDataChart(QPaintDevice* paintDevice, double width, unsigned int spaceAfter, unsigned int spaceBefore,
								 const QString& bottomAxisTitle, const QString& leftAxisTitle, bool canPageBreakAfter = false);

	/*! Copy constructor deleted.*/
	ReportFrameItemBarDataChart(const ReportFrameItemBarDataChart&) = delete;

	/*! Destructor.*/
	~ReportFrameItemBarDataChart();

	/*! Add a data set for a bar set.
		\param x Time data
		\param y Value data. In case of existing lines all data must have the same base unit
		\param name Name of the data set. Is used for legend in case of more than one line
		\param col Color of the line
	*/
	void addBarData(const QStringList& x, const IBK::UnitVector& y, const QString& name, const QColor& col);

	/*! Delete all data sets.*/
	void clearLineData();

	/*! Create a surrounding rect based on the current settings and the given paintDevice and width.
		This base version create a rect with the whole width and the height based on space before and after.
	*/
	virtual void setCurrentRect() override;

	/*! Draw the item with the given painter at the given position and set the position for the next item.*/
	virtual void drawItem(QPainter* painter, QPointF& pos) override;

private:
	SCI::BarSeriesContentModel*		m_chartModel;		///< Model for chart data.
	SCI::Chart*						m_chart;			///< Chart for temperature plot
};

} // namespace Sci

#endif // SCI_ReportFrameItemBarDataChartH
