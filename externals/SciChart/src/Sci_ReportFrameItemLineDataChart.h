#ifndef SCI_ReportFrameItemLineDataChartH
#define SCI_ReportFrameItemLineDataChartH

#include <QApplication>

#include <QtExt_ReportFrameItemBase.h>

#include <IBK_UnitVector.h>

namespace SCI {

class Chart;
class LineSeriesContentModel;

/*! Clase for simplifying of rendering a line chart to a report.
	It handles arbitrary line charts.
	It can contain more than one line but all data sets must have the same base unit for both axis
*/
class ReportFrameItemLineDataChart : public QtExt::ReportFrameItemBase {
	Q_DECLARE_TR_FUNCTIONS(ReportFrameItemLineDataChart)
public:

	enum MarkerStyle {
		NoSymbol = -1,
		Ellipse,			///< Ellipse or circle
		Rect,				///< Rectangle
		Diamond,			///< Diamond
		Triangle,			///< Triangle pointing upwards
		DTriangle,			///< Triangle pointing downwards
		UTriangle,			///< Triangle pointing upwards
		LTriangle,			///< Triangle pointing left
		RTriangle,			///< Triangle pointing right
		Cross,				///< Cross (+)
		XCross,				///< Diagonal cross (X)
		HLine,				///< Horizontal line
		VLine,				///< Vertical line
		Star1,				///< X combined with +
		Star2,				///< Six-pointed star
		Hexagon				///< Hexagon
	};

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
	ReportFrameItemLineDataChart(QPaintDevice* paintDevice, double width, unsigned int spaceAfter, unsigned int spaceBefore,
								 const QString& bottomAxisTitle, const QString& leftAxisTitle, bool canPageBreakAfter = false);

	/*! Copy constructor deleted.*/
	ReportFrameItemLineDataChart(const ReportFrameItemLineDataChart&) = delete;

	/*! Destructor.*/
	~ReportFrameItemLineDataChart();

	/*! Add a data set for a line.
		\param x x-axis data. In case of existing lines all data must have the same base unit
		\param y y-axis data. In case of existing lines all data must have the same base unit
		\param name Name of the data set. Is used for legend in case of more than one line
		\param col Color of the line
		\param lineWidth Width of the line (line is always solid).
	*/
	void addLineData(const IBK::UnitVector& x, const IBK::UnitVector& y, const QString& name, const QColor& col, int lineWidth);


	/*! Add a data set for a markers.
		\param x x-axis data. In case of existing lines all data must have the same base unit
		\param y y-axis data. In case of existing lines all data must have the same base unit
		\param name Name of the data set. Is used for legend in case of more than one line
		\param col Color of the marker
		\param size Size of the marker
		\param style Style of the marker
	*/
	void addMarkerData(const IBK::UnitVector& x, const IBK::UnitVector& y, const QString& name, const QColor& col,
													 int size, MarkerStyle style);
	/*! Delete all data sets.*/
	void clearLineData();

	/*! Set properties for the bottom axis (time). Should be used if yearChart flag in constructor is false.
	*/
	void setAxis(bool bottomAxis, bool autoScale, const IBK::Unit& unit, double max = 0, double min = 0);

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

#endif // SCI_ReportFrameItemLineDataChartH
