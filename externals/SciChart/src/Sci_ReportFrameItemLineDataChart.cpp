#include "Sci_ReportFrameItemLineDataChart.h"

#include <IBK_math.h>

#include "Sci_Chart.h"
#include "Sci_LineSeriesContentModel.h"

namespace SCI {

ReportFrameItemLineDataChart::ReportFrameItemLineDataChart(QPaintDevice* paintDevice, double width, unsigned int spaceAfter, unsigned int spaceBefore,
														   const QString& bottomAxisTitle, const QString& leftAxisTitle, bool canPageBreakAfter) :
	QtExt::ReportFrameItemBase(paintDevice, width, spaceAfter, spaceBefore, canPageBreakAfter),
	m_chartModel(new LineSeriesContentModel),
	m_chart(new Chart)
{

	QFont f(m_chart->font());

#if defined(Q_OS_MAC)
	// tick font
	f.setPointSize(12);
	QColor backgroundColor(255,255,255);
#else
	// tick font
	f.setPointSize(10);
	QColor backgroundColor(255,255,255);
#endif

	m_chart->setAutoReplot(false);
	m_chart->setContentsMargins(5, 5, 5, 5);
	m_chart->setCanvasBackground(backgroundColor);
	qreal chartHeight =  width / 2;
	m_chart->setGeometry(0, 0, width, chartHeight);
	m_currentRect = m_chart->geometry();

	m_chartModel->setData(QString(), SCI::AbstractChartModel::TitleText);
	m_chartModel->setAxisData(bottomAxisTitle, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisTitleText);
	m_chartModel->setAxisData(leftAxisTitle, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisTitleText);

	m_chartModel->setData(false, SCI::AbstractChartModel::LegendVisible);

	m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);

	m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);

	// axis fonts
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisTitleFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisLabelFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisTitleFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisLabelFont);

	// grid
	m_chartModel->setAxisData(QPen(Qt::gray, 0, Qt::DotLine), SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisGridPen);
	m_chartModel->setAxisData(QPen(Qt::gray, 0, Qt::DotLine), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisGridPen);
	m_chartModel->setAxisData(QPen(Qt::lightGray, 0 , Qt::DotLine), SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinorGridPen);
	m_chartModel->setAxisData(QPen(Qt::lightGray, 0 , Qt::DotLine), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinorGridPen);

	m_chart->setModel(m_chartModel);
}

void ReportFrameItemLineDataChart::setAxis(bool bottomAxis, bool autoScale, const IBK::Unit& unit, double max, double min) {
	SCI::AbstractCartesianChartModel::AxisPosition axisPos = bottomAxis ? SCI::AbstractCartesianChartModel::BottomAxis : SCI::AbstractCartesianChartModel::LeftAxis;
	m_chartModel->setAxisData(unit.id(), axisPos, SCI::AbstractCartesianChartModel::AxisUnit);
	m_chartModel->setAxisData(autoScale, axisPos, SCI::AbstractCartesianChartModel::AxisAutoScale);
	if(!autoScale && IBK::near_equal(max,min))
		autoScale = true;
	if(!autoScale) {
		m_chartModel->setAxisData(min, axisPos, SCI::AbstractCartesianChartModel::AxisMinimum);
		m_chartModel->setAxisData(max, axisPos, SCI::AbstractCartesianChartModel::AxisMinimum);
	}
}

void ReportFrameItemLineDataChart::setChartTitle(const QString &title, QFont* font) {
	m_chartModel->setData(title, SCI::AbstractChartModel::TitleText);
	if(font)
		m_chartModel->setData(*font, SCI::AbstractChartModel::TitleFont);
}

ReportFrameItemLineDataChart::~ReportFrameItemLineDataChart() {
	delete m_chart;
	delete m_chartModel;
}

void ReportFrameItemLineDataChart::addLineData(const IBK::UnitVector& x, const IBK::UnitVector& y, const QString& name, const QColor& col, int lineWidth) {
	if(m_chartModel->seriesCount() == 0) {
		m_chartModel->setAxisData(y.m_unit.id(), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit);
		m_chartModel->addDataSet(x, y, name);
		QPen seriesPen(QBrush(col), lineWidth);
		m_chartModel->setSeriesData(seriesPen, 0, SCI::AbstractLineSeriesModel::SeriesPen);
	}
	else {
		m_chartModel->addDataSet(x, y, name);
		QPen seriesPen(QBrush(col), lineWidth);
		m_chartModel->setSeriesData(seriesPen, m_chartModel->seriesCount()-1, SCI::AbstractLineSeriesModel::SeriesPen);
		m_chartModel->setData(true, SCI::AbstractChartModel::LegendVisible);
	}
}

void ReportFrameItemLineDataChart::addMarkerData(const IBK::UnitVector& x, const IBK::UnitVector& y, const QString& name, const QColor& col,
												 int size, MarkerStyle style) {
	int seriesIndex = 0;
	if(m_chartModel->seriesCount() == 0) {
		m_chartModel->setAxisData(y.m_unit.id(), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit);
		m_chartModel->addDataSet(x, y, name);
	}
	else {
		m_chartModel->addDataSet(x, y, name);
		m_chartModel->setData(true, SCI::AbstractChartModel::LegendVisible);
		seriesIndex = m_chartModel->seriesCount() - 1;
	}
	m_chartModel->setSeriesData(-1, seriesIndex, SCI::AbstractLineSeriesModel::SeriesLineStyle);
	m_chartModel->setSeriesData(col, seriesIndex, SCI::AbstractLineSeriesModel::SeriesMarkerColor);
	m_chartModel->setSeriesData(col, seriesIndex, SCI::AbstractLineSeriesModel::SeriesColor);
	m_chartModel->setSeriesData(size, seriesIndex, SCI::AbstractLineSeriesModel::SeriesMarkerSize);
	m_chartModel->setSeriesData(int(style), seriesIndex, SCI::AbstractLineSeriesModel::SeriesMarkerStyle);
}

void ReportFrameItemLineDataChart::clearLineData() {
	m_chartModel->clear();
}


void ReportFrameItemLineDataChart::setCurrentRect() {
	m_currentRect = m_chart->geometry();
}

void ReportFrameItemLineDataChart::drawItem(QPainter* painter, QPointF& pos) {
	m_chart->render(painter, pos, true);
}


} // namespace Sci
