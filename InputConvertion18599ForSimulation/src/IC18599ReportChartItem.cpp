#include "IC18599ReportChartItem.h"

#include "Sci_Chart.h"
#include "Sci_LineSeriesContentModel.h"

#include <qwt_scale_draw.h>

#include <QCoreApplication>

/*! Custom scale draw that shows weekday names for hour values 0-168.
	Matches the VICUS WeekdayScaleDraw implementation. */
class WeekdayScaleDraw : public QwtScaleDraw {
	Q_DECLARE_TR_FUNCTIONS(WeekdayScaleDraw)
public:
	WeekdayScaleDraw() {}

	QwtText label(double value) const override {
		static const QStringList days = {tr("Monday"), tr("Tuesday"), tr("Wednesday"),
										 tr("Thursday"), tr("Friday"), tr("Saturday"), tr("Sunday")};
		int dayIndex = static_cast<int>(value) / 24;
		if (dayIndex >= 0 && dayIndex < 7)
			return days[dayIndex];
		return QwtText();
	}
};

IC18599ReportChartItem::IC18599ReportChartItem(QPaintDevice *paintDevice, double width, double aspectDivisor,
											   unsigned int spaceAfter, unsigned int spaceBefore,
											   const QString &bottomAxisTitle, const QString &leftAxisTitle) :
	QtExt::ReportFrameItemBase(paintDevice, width, spaceAfter, spaceBefore, false),
	m_chartModel(new SCI::LineSeriesContentModel),
	m_chart(new SCI::Chart)
{
	QFont f(m_chart->font());
	f.setPointSize(10);

	m_chart->setAutoReplot(false);
	m_chart->setContentsMargins(5, 5, 5, 5);
	m_chart->setCanvasBackground(QColor(255, 255, 255));

	double chartHeight = width / aspectDivisor;
	m_chart->setGeometry(0, 0, width, chartHeight);
	m_currentRect = m_chart->geometry();

	m_chartModel->setData(QString(), SCI::AbstractChartModel::TitleText);
	m_chartModel->setAxisData(bottomAxisTitle, SCI::AbstractCartesianChartModel::BottomAxis,
							  SCI::AbstractCartesianChartModel::AxisTitleText);
	m_chartModel->setAxisData(leftAxisTitle, SCI::AbstractCartesianChartModel::LeftAxis,
							  SCI::AbstractCartesianChartModel::AxisTitleText);

	m_chartModel->setData(false, SCI::AbstractChartModel::LegendVisible);

	m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::BottomAxis,
							  SCI::AbstractCartesianChartModel::AxisAutoScale);
	m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::LeftAxis,
							  SCI::AbstractCartesianChartModel::AxisAutoScale);

	// Axis fonts
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::BottomAxis,
							  SCI::AbstractCartesianChartModel::AxisTitleFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::BottomAxis,
							  SCI::AbstractCartesianChartModel::AxisLabelFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::LeftAxis,
							  SCI::AbstractCartesianChartModel::AxisTitleFont);
	m_chartModel->setAxisData(f, SCI::AbstractCartesianChartModel::LeftAxis,
							  SCI::AbstractCartesianChartModel::AxisLabelFont);

	// Grid
	m_chartModel->setAxisData(QPen(Qt::gray, 0, Qt::DotLine),
							  SCI::AbstractCartesianChartModel::BottomAxis,
							  SCI::AbstractCartesianChartModel::AxisGridPen);
	m_chartModel->setAxisData(QPen(Qt::gray, 0, Qt::DotLine),
							  SCI::AbstractCartesianChartModel::LeftAxis,
							  SCI::AbstractCartesianChartModel::AxisGridPen);
	m_chartModel->setAxisData(QPen(Qt::lightGray, 0, Qt::DotLine),
							  SCI::AbstractCartesianChartModel::BottomAxis,
							  SCI::AbstractCartesianChartModel::AxisMinorGridPen);
	m_chartModel->setAxisData(QPen(Qt::lightGray, 0, Qt::DotLine),
							  SCI::AbstractCartesianChartModel::LeftAxis,
							  SCI::AbstractCartesianChartModel::AxisMinorGridPen);

	m_chart->setModel(m_chartModel);
}


IC18599ReportChartItem::~IC18599ReportChartItem() {
	delete m_chart;
	delete m_chartModel;
}


void IC18599ReportChartItem::addLineData(const IBK::UnitVector &x, const IBK::UnitVector &y,
										 const QString &name, const QColor &col, int lineWidth) {
	if (m_chartModel->seriesCount() == 0) {
		m_chartModel->setAxisData(y.m_unit.id(), SCI::AbstractCartesianChartModel::LeftAxis,
								  SCI::AbstractCartesianChartModel::AxisUnit);
		m_chartModel->addDataSet(x, y, name);
		QPen seriesPen(QBrush(col), lineWidth);
		m_chartModel->setSeriesData(seriesPen, 0, SCI::AbstractLineSeriesModel::SeriesPen);
	}
	else {
		m_chartModel->addDataSet(x, y, name);
		QPen seriesPen(QBrush(col), lineWidth);
		m_chartModel->setSeriesData(seriesPen, m_chartModel->seriesCount() - 1,
									SCI::AbstractLineSeriesModel::SeriesPen);
		m_chartModel->setData(true, SCI::AbstractChartModel::LegendVisible);
	}
}


void IC18599ReportChartItem::setAxisRange(bool bottomAxis, double minVal, double maxVal) {
	auto axisPos = bottomAxis ? SCI::AbstractCartesianChartModel::BottomAxis
							  : SCI::AbstractCartesianChartModel::LeftAxis;
	m_chartModel->setAxisData(false, axisPos, SCI::AbstractCartesianChartModel::AxisAutoScale);
	m_chartModel->setAxisData(minVal, axisPos, SCI::AbstractCartesianChartModel::AxisMinimum);
	m_chartModel->setAxisData(maxVal, axisPos, SCI::AbstractCartesianChartModel::AxisMaximum);
}


void IC18599ReportChartItem::setAxisMajorTicks(bool bottomAxis, int maxTicks) {
	auto axisPos = bottomAxis ? SCI::AbstractCartesianChartModel::BottomAxis
							  : SCI::AbstractCartesianChartModel::LeftAxis;
	m_chartModel->setAxisData(maxTicks, axisPos, SCI::AbstractCartesianChartModel::AxisMaxMajorScale);
}


void IC18599ReportChartItem::setWeekdayXAxis() {
	// Fixed range 0-168h with step size 24 (one tick per day), weekday labels.
	// Uses direct QwtPlot::setAxisScale instead of the SCI model's AxisMaxMajorScale
	// which only sets the *maximum* tick count and lets qwt choose fewer.
	m_chartModel->setAxisData(false, SCI::AbstractCartesianChartModel::BottomAxis,
							  SCI::AbstractCartesianChartModel::AxisAutoScale);
	m_chart->setAxisScale(QwtPlot::xBottom, 0, 168, 24);
	m_chart->setAxisScaleDraw(QwtPlot::xBottom, new WeekdayScaleDraw);
}


void IC18599ReportChartItem::setCurrentRect() {
	m_currentRect = m_chart->geometry();
}


void IC18599ReportChartItem::drawItem(QPainter *painter, QPointF &pos) {
	m_chart->render(painter, pos, true);
}
