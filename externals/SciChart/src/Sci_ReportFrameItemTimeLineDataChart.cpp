#include "Sci_ReportFrameItemTimeLineDataChart.h"

#include <IBK_math.h>

#include "Sci_Chart.h"
#include "Sci_LineSeriesContentModel.h"

namespace SCI {

ReportFrameItemTimeLineDataChart::ReportFrameItemTimeLineDataChart(QPaintDevice* paintDevice, double width, bool yearChart, unsigned int spaceAfter, unsigned int spaceBefore,
														   const QString& bottomAxisTitle, const QString& leftAxisTitle, bool canPageBreakAfter, int pointSize, double chartHeight) :
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
	f.setPointSize(pointSize);
	f.setFamily("Roboto");
	QColor backgroundColor(255,255,255);
#endif

	m_chart->setAutoReplot(false);
	m_chart->setContentsMargins(5, 5, 5, 5);
	m_chart->setCanvasBackground(backgroundColor);
	if (chartHeight < 10)
		chartHeight =  width / 2;
	m_chart->setGeometry(0, 0, width, chartHeight);
	m_currentRect = m_chart->geometry();

	m_chartModel->setData(QString(), SCI::AbstractChartModel::TitleText);
	m_chartModel->setAxisData(bottomAxisTitle, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisTitleText);
	m_chartModel->setAxisData(leftAxisTitle, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisTitleText);

	m_chartModel->setData(false, SCI::AbstractChartModel::LegendVisible);
	m_chartModel->setData(f, SCI::AbstractChartModel::LegendFont);
	m_chartModel->setData(f.pointSize() * 0.5, SCI::AbstractChartModel::LegendSpacing);

	if(yearChart) {
		const double  ONE_YEAR_MS = 365.0*24*3600*1000;
		m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTime);
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		QDateTime dateTimeZero(QDate(2000, 1, 1),QTime(0,0), QTimeZone("UTC"));
#else
		QDateTime dateTimeZero(QDate(2000, 1, 1),QTime(0,0), Qt::UTC);
#endif
		m_chartModel->setAxisData(dateTimeZero, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeZero);
		QVariant vformats = m_chartModel->axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeFormats);
		QStringList dateTimeFormats = vformats.toStringList();
		Q_ASSERT(dateTimeFormats.size() == 8);
		dateTimeFormats[6] = "MMM";
		dateTimeFormats[7] = "MMM";
		double offset = dateTimeZero.toMSecsSinceEpoch();
		double timeEps = 86400000.0; // one day time epsilon for correct view of x axis
		m_chartModel->setAxisData(dateTimeFormats, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeFormats);
		m_chartModel->setAxisData(false, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
		m_chartModel->setAxisData(offset - timeEps, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
		m_chartModel->setAxisData(ONE_YEAR_MS + offset + timeEps, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
		m_chartModel->setAxisData(IBK::Unit("ms").id(), SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit);
	}
	else {
		m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
	}

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
}

void ReportFrameItemTimeLineDataChart::setTimeAxis(bool autoScale, bool dateTime, int zeroYear, int zeroMonth, int zeroDay, double max, double min, const IBK::Unit& axitUnit, const QString& format) {

	m_chartModel->setAxisData(dateTime, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTime);

#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
	QDateTime dateTimeZero(QDate(zeroYear, zeroMonth, zeroDay),QTime(0,0), QTimeZone("UTC"));
#else
	QDateTime dateTimeZero(QDate(zeroYear, zeroMonth, zeroDay),QTime(0,0), Qt::UTC);
#endif

	if (dateTime)
		m_chartModel->setAxisData(dateTimeZero, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeZero);

	m_chartModel->setAxisData(axitUnit.id(), SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit);

	m_chartModel->setAxisData(autoScale, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);

	if (format != QString()) {

		QVariant vformats = m_chartModel->axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeFormats);
		QStringList dateTimeFormats = vformats.toStringList();

		for (int i=0; i < dateTimeFormats.size(); ++i)
			dateTimeFormats[i] = format;

		m_chartModel->setAxisData(dateTimeFormats, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeFormats);
	}

	if (!autoScale && !dateTime && IBK::near_equal(max,min))
		autoScale = true;

	if (!autoScale) {
		//replaced by near min/max
		//if(dateTime) {
		if (IBK::near_equal(max, min)) {
			double offset = dateTimeZero.toMSecsSinceEpoch();
			double timeEps = 86400000.0; // one day time epsilon for correct view of x axis
			m_chartModel->setAxisData(offset - timeEps, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
		}
		else {
			m_chartModel->setAxisData(min, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
			m_chartModel->setAxisData(max, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
		}
	}
}

void ReportFrameItemTimeLineDataChart::setValueAxis(bool autoScale, double max, double min, const IBK::Unit &unit) {
	if(m_chartModel->seriesCount() == 0)
		return;

	if(!autoScale && IBK::near_equal(max,min))
		autoScale = true;
	if(autoScale) {
		m_chartModel->setAxisData(true, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
		return;
	}

	m_chartModel->setAxisData(false, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale);
	m_chartModel->setAxisData(min, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
	m_chartModel->setAxisData(max, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMaximum);

	unsigned int unitId = m_chartModel->axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit).toUInt();
	if(unit != IBK::Unit() && unit.base_id() == unitId) {
		m_chartModel->setAxisData(unit.id(), SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit);
	}
}

void ReportFrameItemTimeLineDataChart::setChartTitle(const QString &title, QFont* font) {
	m_chartModel->setData(title, SCI::AbstractChartModel::TitleText);
	if(font)
		m_chartModel->setData(*font, SCI::AbstractChartModel::TitleFont);
}

ReportFrameItemTimeLineDataChart::~ReportFrameItemTimeLineDataChart() {
	delete m_chart;
	delete m_chartModel;
}

void ReportFrameItemTimeLineDataChart::addLineData(const IBK::UnitVector& x, const IBK::UnitVector& y, const QString& name, const QColor& col, int lineWidth) {
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

void ReportFrameItemTimeLineDataChart::clearLineData() {
	m_chartModel->clear();
}

void ReportFrameItemTimeLineDataChart::setAxisScaleLimits(int min, int max) {

	m_chartModel->setAxisData(min, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaxMinorScale);
	m_chartModel->setAxisData(max, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaxMajorScale);
}

void ReportFrameItemTimeLineDataChart::setCurrentRect() {
	m_currentRect = m_chart->geometry();
}

void ReportFrameItemTimeLineDataChart::drawItem(QPainter* painter, QPointF& pos) {

	m_chart->setModel(m_chartModel);

	QPalette pal;
	pal.setColor(QPalette::Text, Qt::black);
	m_chart->setPalette(pal);
	m_chart->render(painter, pos, true);
}


} // namespace Sci
