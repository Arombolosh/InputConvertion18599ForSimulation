#include "Sci_ReportFrameItemBarDataChart.h"

#include <IBK_math.h>

#include "Sci_Chart.h"
#include "Sci_BarSeriesContentModel.h"

namespace SCI {

ReportFrameItemBarDataChart::ReportFrameItemBarDataChart(QPaintDevice* paintDevice, double width, unsigned int spaceAfter, unsigned int spaceBefore,
														   const QString& bottomAxisTitle, const QString& leftAxisTitle, bool canPageBreakAfter) :
	QtExt::ReportFrameItemBase(paintDevice, width, spaceAfter, spaceBefore, canPageBreakAfter),
	m_chartModel(new BarSeriesContentModel),
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

//	m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);

	m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);

	// axis fonts
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisTitleFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisLabelFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisTitleFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisLabelFont);

	// grid
//	m_chartModel->setAxisData(QPen(Qt::gray, 0, Qt::DotLine), SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisGridPen);
	m_chartModel->setAxisData(QPen(Qt::gray, 0, Qt::DotLine), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisGridPen);
//	m_chartModel->setAxisData(QPen(Qt::lightGray, 0 , Qt::DotLine), SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinorGridPen);
	m_chartModel->setAxisData(QPen(Qt::lightGray, 0 , Qt::DotLine), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinorGridPen);

	m_chart->setModel(m_chartModel);
}

ReportFrameItemBarDataChart::~ReportFrameItemBarDataChart() {
	delete m_chart;
	delete m_chartModel;
}

void ReportFrameItemBarDataChart::addBarData(const QStringList& x, const IBK::UnitVector& y, const QString& name, const QColor& col) {
	std::vector<IBK::UnitVector> data(y.size());
	for(size_t i=0; i<y.m_data.size(); ++i) {
		data[i].m_unit = y.m_unit;
		data[i].m_data.push_back(y.m_data[i]);
	}
	m_chartModel->setAxisData(y.m_unit.id(), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit);
	m_chartModel->setValues(x, data, name);
	m_chartModel->setValueColor(0, col);
	m_chartModel->setAxisData(false, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
	m_chartModel->setAxisData(-0.5, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
	m_chartModel->setAxisData(x.size() - 0.5, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
}

void ReportFrameItemBarDataChart::clearLineData() {
	m_chartModel->clear();
}


void ReportFrameItemBarDataChart::setCurrentRect() {
	m_currentRect = m_chart->geometry();
}

void ReportFrameItemBarDataChart::drawItem(QPainter* painter, QPointF& pos) {
	m_chart->render(painter, pos, true);
}

//QtExt::ReportFrameItemBase* ReportFrameItemBarDataChart::clone() const {
//	ReportFrameItemBarDataChart* res = new ReportFrameItemBarDataChart(*this);
//	return res;
//}

} // namespace Sci
