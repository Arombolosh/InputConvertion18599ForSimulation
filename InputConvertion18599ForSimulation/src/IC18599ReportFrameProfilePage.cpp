#include "IC18599ReportFrameProfilePage.h"

#include "IC18599Report.h"
#include "IC18599ReportSettings.h"
#include "IC18599ReportChartItem.h"

#include <QtExt_ReportFrameItemTextFrame.h>
#include <QtExt_ReportFrameItemTable.h>

#include <IBK_Unit.h>
#include <IBK_UnitVector.h>

#include <algorithm>

static const char* DAY_NAMES[] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};



IC18599ReportFrameProfilePage::IC18599ReportFrameProfilePage(QtExt::Report *report, QTextDocument *textDocument,
															 const ReportProfileData &data, PageType pageType) :
	QtExt::ReportFrameBase(report, textDocument),
	m_data(data),
	m_pageType(pageType),
	m_heading(textDocument)
{
	m_onNewPage = true;
}


QString IC18599ReportFrameProfilePage::dayGroupLabel(const std::set<int> &days) {
	if (days.empty())
		return QString();

	std::vector<int> sorted(days.begin(), days.end());

	bool consecutive = true;
	for (size_t i = 1; i < sorted.size(); ++i) {
		if (sorted[i] != sorted[i-1] + 1) {
			consecutive = false;
			break;
		}
	}

	if (consecutive && sorted.size() > 2)
		return QString("%1 - %2").arg(DAY_NAMES[sorted.front()]).arg(DAY_NAMES[sorted.back()]);

	QStringList names;
	for (int d : sorted)
		names.append(DAY_NAMES[d]);
	return names.join(", ");
}


static bool allZero(const std::vector<double> &v) {
	return std::all_of(v.begin(), v.end(), [](double x) { return x == 0.0; });
}


/*! Helper: build staircase X/Y vectors for 168 weekly values.
	Adds Y-axis margin so data doesn't sit on axis borders. */
static void buildWeekStaircase(const std::vector<double> &weekValues,
							   IBK::UnitVector &xVec, IBK::UnitVector &yVec,
							   const std::string &yUnit) {
	xVec.m_unit = IBK::Unit("h");
	yVec.m_unit = IBK::Unit(yUnit);
	xVec.m_data.resize(2 * 168);
	yVec.m_data.resize(2 * 168);
	for (int i = 0; i < 168; ++i) {
		xVec.m_data[2*i]     = (double)i;
		xVec.m_data[2*i + 1] = (double)(i + 1);
		yVec.m_data[2*i]     = weekValues[i];
		yVec.m_data[2*i + 1] = weekValues[i];
	}
}


/*! Compute Y-axis range with margin so data doesn't touch the borders.
	Adds 10% padding above the max and below the min.
	If all values are identical, creates a +-1 range around that value. */
static void computeYRange(const std::vector<double> &weekValues, double &yMin, double &yMax) {
	if (weekValues.empty()) {
		yMin = -1;
		yMax = 1;
		return;
	}
	double lo = *std::min_element(weekValues.begin(), weekValues.end());
	double hi = *std::max_element(weekValues.begin(), weekValues.end());
	double range = hi - lo;
	if (range < 1e-6) {
		// All values identical
		yMin = lo - 1.0;
		yMax = hi + 1.0;
	}
	else {
		double margin = range * 0.10;
		yMin = lo - margin;
		yMax = hi + margin;
	}
}


void IC18599ReportFrameProfilePage::update(QPaintDevice *paintDevice, double width) {
	const double CHART_ASPECT = 3.5;
	const int LINE_WIDTH = 2;

	if (m_pageType == InternalLoads) {
		// --- Page A: Internal Loads ---

		// Heading with profile number
		m_heading.setText(TITLE_H2(QString("%1. %2").arg(m_data.profileIndex).arg(m_data.name)));
		addItem(new QtExt::ReportFrameItemTextFrame(&m_heading, paintDevice, width,
													BOTTOM_DIST_H3, 0));

		// Person chart
		{
			IBK::UnitVector xVec, yVec;
			buildWeekStaircase(m_data.personResultWeek, xVec, yVec, "W/m2");
			double yMin, yMax;
			computeYRange(m_data.personResultWeek, yMin, yMax);

			auto *chart = new IC18599ReportChartItem(paintDevice, width, CHART_ASPECT,
				BOTTOM_DIST_H3, 0,
				QString(), QString::fromUtf8("Person [W/m²]"));
			chart->addLineData(xVec, yVec, tr("Person"), QColor(70, 130, 180), LINE_WIDTH);
			chart->setWeekdayXAxis();
			chart->setAxisRange(false, yMin, yMax);
			addItem(chart);
		}

		// Equipment chart
		{
			IBK::UnitVector xVec, yVec;
			buildWeekStaircase(m_data.equipResultWeek, xVec, yVec, "W/m2");
			double yMin, yMax;
			computeYRange(m_data.equipResultWeek, yMin, yMax);

			auto *chart = new IC18599ReportChartItem(paintDevice, width, CHART_ASPECT,
				BOTTOM_DIST_H3, 0,
				QString(), QString::fromUtf8("Equipment [W/m²]"));
			chart->addLineData(xVec, yVec, tr("Equipment"), QColor(180, 120, 60), LINE_WIDTH);
			chart->setWeekdayXAxis();
			chart->setAxisRange(false, yMin, yMax);
			addItem(chart);
		}

		// Lighting chart
		{
			IBK::UnitVector xVec, yVec;
			buildWeekStaircase(m_data.lightResultWeek, xVec, yVec, "W/m2");
			double yMin, yMax;
			computeYRange(m_data.lightResultWeek, yMin, yMax);

			auto *chart = new IC18599ReportChartItem(paintDevice, width, CHART_ASPECT,
				BOTTOM_DIST_H3, 0,
				QString(), QString::fromUtf8("Lighting [W/m²]"));
			chart->addLineData(xVec, yVec, tr("Lighting"), QColor(200, 180, 50), LINE_WIDTH);
			chart->setWeekdayXAxis();
			chart->setAxisRange(false, yMin, yMax);
			addItem(chart);
		}

		// Data table per day group
		for (size_t g = 0; g < m_data.personGroups.size(); ++g) {
			const DailyCycleGroup &personGrp = m_data.personGroups[g];
			QString groupLabel = dayGroupLabel(personGrp.m_days);

			int repDay = *personGrp.m_days.begin();
			std::vector<double> personDay(24), equipDay(24), lightDay(24);
			for (int h = 0; h < 24; ++h) {
				int idx = repDay * 24 + h;
				personDay[h] = m_data.personResultWeek[idx];
				equipDay[h]  = m_data.equipResultWeek[idx];
				lightDay[h]  = m_data.lightResultWeek[idx];
			}

			// Side-by-side 12h layout: 8 columns
			// Col 0: Time | Cols 1-3: values | Col 4: Time | Cols 5-7: values
			int numRows = 12 + 2;
			int numCols = 8;

			QtExt::Table *table = new QtExt::Table(m_textDocument, true);
			table->set(numRows, numCols, width, 1, 0.5, QColor(255, 255, 255));

			double colW = width / numCols;
			for (int c = 0; c < numCols; ++c)
				table->setColumnSizeFormat(c, QtExt::CellSizeFormater::Fixed, colW);

			// Header row 0: Day(s) label in col 0, unit spanning value cols 1-3, same for right half
			table->setCellText(0, 0, QString("Day(s): %1").arg(groupLabel), Qt::AlignHCenter);
			table->setCellText(1, 0, QString::fromUtf8("[W/m²]"), Qt::AlignHCenter);
			table->mergeCells(1, 0, 3, 1);
			table->setCellText(4, 0, QString(), Qt::AlignHCenter);
			table->setCellText(5, 0, QString::fromUtf8("[W/m²]"), Qt::AlignHCenter);
			table->mergeCells(5, 0, 3, 1);

			// Header row 1: column titles
			table->setCellText(0, 1, tr("Time"), Qt::AlignHCenter);
			table->setCellText(1, 1, tr("Person"), Qt::AlignHCenter);
			table->setCellText(2, 1, tr("Equip."), Qt::AlignHCenter);
			table->setCellText(3, 1, tr("Light"), Qt::AlignHCenter);
			table->setCellText(4, 1, tr("Time"), Qt::AlignHCenter);
			table->setCellText(5, 1, tr("Person"), Qt::AlignHCenter);
			table->setCellText(6, 1, tr("Equip."), Qt::AlignHCenter);
			table->setCellText(7, 1, tr("Light"), Qt::AlignHCenter);

			QColor altColor(240, 240, 240);
			for (int r = 0; r < 12; ++r) {
				int row = r + 2;
				int hour1 = r;
				int hour2 = r + 12;

				table->setCellText(0, row, QString("%1:00-%2:00").arg(hour1, 2, 10, QChar('0')).arg(hour1 + 1, 2, 10, QChar('0')), Qt::AlignHCenter);
				table->setCellText(1, row, QString::number(personDay[hour1], 'f', 2), Qt::AlignHCenter);
				table->setCellText(2, row, QString::number(equipDay[hour1], 'f', 2), Qt::AlignHCenter);
				table->setCellText(3, row, QString::number(lightDay[hour1], 'f', 2), Qt::AlignHCenter);

				table->setCellText(4, row, QString("%1:00-%2:00").arg(hour2, 2, 10, QChar('0')).arg(hour2 + 1, 2, 10, QChar('0')), Qt::AlignHCenter);
				table->setCellText(5, row, QString::number(personDay[hour2], 'f', 2), Qt::AlignHCenter);
				table->setCellText(6, row, QString::number(equipDay[hour2], 'f', 2), Qt::AlignHCenter);
				table->setCellText(7, row, QString::number(lightDay[hour2], 'f', 2), Qt::AlignHCenter);

				if (r % 2 == 1) {
					for (int c = 0; c < numCols; ++c)
						table->cell(c, row).setBackgroundColor(altColor);
				}
			}

			addItem(new QtExt::ReportFrameItemTable(table, paintDevice, width, 0, BOTTOM_DIST_H3, true));
		}
	}
	else {
		// --- Page B: Thermostat Setpoints ---

		m_heading.setText(TITLE_H2(QString("%1. %2").arg(m_data.profileIndex).arg(m_data.name)
									+ tr(" (continued)")));
		addItem(new QtExt::ReportFrameItemTextFrame(&m_heading, paintDevice, width,
													BOTTOM_DIST_H3, 0));

		// Y-axis bounds: 0-35 if data exists, -1 to 1 if all zero
		bool hasData = !allZero(m_data.heatingSetpointWeek) || !allZero(m_data.coolingSetpointWeek);
		double yMin = hasData ? 0.0  : -1.0;
		double yMax = hasData ? 35.0 :  1.0;

		// Heating chart
		{
			IBK::UnitVector xVec, yVec;
			buildWeekStaircase(m_data.heatingSetpointWeek, xVec, yVec, "C");

			auto *chart = new IC18599ReportChartItem(paintDevice, width, 2.5,
				BOTTOM_DIST_H3, 0,
				QString(), QString::fromUtf8("Heating [°C]"));
			chart->addLineData(xVec, yVec, tr("Heating"), QColor(200, 80, 80), LINE_WIDTH);
			chart->setWeekdayXAxis();
			chart->setAxisRange(false, yMin, yMax);
			addItem(chart);
		}

		// Cooling chart
		{
			IBK::UnitVector xVec, yVec;
			buildWeekStaircase(m_data.coolingSetpointWeek, xVec, yVec, "C");

			auto *chart = new IC18599ReportChartItem(paintDevice, width, 2.5,
				BOTTOM_DIST_H3, 0,
				QString(), QString::fromUtf8("Cooling [°C]"));
			chart->addLineData(xVec, yVec, tr("Cooling"), QColor(70, 130, 200), LINE_WIDTH);
			chart->setWeekdayXAxis();
			chart->setAxisRange(false, yMin, yMax);
			addItem(chart);
		}

		// Data table per day group
		for (size_t g = 0; g < m_data.heatingGroups.size(); ++g) {
			const DailyCycleGroup &heatingGrp = m_data.heatingGroups[g];
			QString groupLabel = dayGroupLabel(heatingGrp.m_days);

			int repDay = *heatingGrp.m_days.begin();
			std::vector<double> heatingDay(24), coolingDay(24);
			for (int h = 0; h < 24; ++h) {
				int idx = repDay * 24 + h;
				heatingDay[h] = m_data.heatingSetpointWeek[idx];
				coolingDay[h] = m_data.coolingSetpointWeek[idx];
			}

			// Side-by-side 12h layout: 6 columns
			int numRows = 12 + 2;
			int numCols = 6;

			QtExt::Table *table = new QtExt::Table(m_textDocument, true);
			table->set(numRows, numCols, width, 1, 0.5, QColor(255, 255, 255));

			double colW = width / numCols;
			for (int c = 0; c < numCols; ++c)
				table->setColumnSizeFormat(c, QtExt::CellSizeFormater::Fixed, colW);

			// Header row 0: Day(s) label in col 0, unit spanning value cols 1-2, same for right half
			table->setCellText(0, 0, QString("Day(s): %1").arg(groupLabel), Qt::AlignHCenter);
			table->setCellText(1, 0, QString::fromUtf8("[°C]"), Qt::AlignHCenter);
			table->mergeCells(1, 0, 2, 1);
			table->setCellText(3, 0, QString(), Qt::AlignHCenter);
			table->setCellText(4, 0, QString::fromUtf8("[°C]"), Qt::AlignHCenter);
			table->mergeCells(4, 0, 2, 1);

			// Header row 1
			table->setCellText(0, 1, tr("Time"), Qt::AlignHCenter);
			table->setCellText(1, 1, tr("Heating"), Qt::AlignHCenter);
			table->setCellText(2, 1, tr("Cooling"), Qt::AlignHCenter);
			table->setCellText(3, 1, tr("Time"), Qt::AlignHCenter);
			table->setCellText(4, 1, tr("Heating"), Qt::AlignHCenter);
			table->setCellText(5, 1, tr("Cooling"), Qt::AlignHCenter);

			QColor altColor(240, 240, 240);
			for (int r = 0; r < 12; ++r) {
				int row = r + 2;
				int hour1 = r;
				int hour2 = r + 12;

				table->setCellText(0, row, QString("%1:00-%2:00").arg(hour1, 2, 10, QChar('0')).arg(hour1 + 1, 2, 10, QChar('0')), Qt::AlignHCenter);
				table->setCellText(1, row, QString::number(heatingDay[hour1], 'f', 1), Qt::AlignHCenter);
				table->setCellText(2, row, QString::number(coolingDay[hour1], 'f', 1), Qt::AlignHCenter);

				table->setCellText(3, row, QString("%1:00-%2:00").arg(hour2, 2, 10, QChar('0')).arg(hour2 + 1, 2, 10, QChar('0')), Qt::AlignHCenter);
				table->setCellText(4, row, QString::number(heatingDay[hour2], 'f', 1), Qt::AlignHCenter);
				table->setCellText(5, row, QString::number(coolingDay[hour2], 'f', 1), Qt::AlignHCenter);

				if (r % 2 == 1) {
					for (int c = 0; c < numCols; ++c)
						table->cell(c, row).setBackgroundColor(altColor);
				}
			}

			addItem(new QtExt::ReportFrameItemTable(table, paintDevice, width, 0, BOTTOM_DIST_H3, true));
		}
	}

	QtExt::ReportFrameBase::update(paintDevice, width);
}
