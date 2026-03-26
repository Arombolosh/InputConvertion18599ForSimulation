#include "IC18599ScheduleEditWidget.h"
#include "ui_IC18599ScheduleEditWidget.h"
#include "IC18599DayProfileWidget.h"

#include <QAbstractItemDelegate>
#include <QCheckBox>
#include <QTableWidget>

IC18599ScheduleEditWidget::IC18599ScheduleEditWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::IC18599ScheduleEditWidget)
{
	m_ui->setupUi(this);
	m_dayChecks[0] = m_ui->m_dayCheck0;
	m_dayChecks[1] = m_ui->m_dayCheck1;
	m_dayChecks[2] = m_ui->m_dayCheck2;
	m_dayChecks[3] = m_ui->m_dayCheck3;
	m_dayChecks[4] = m_ui->m_dayCheck4;
	m_dayChecks[5] = m_ui->m_dayCheck5;
	m_dayChecks[6] = m_ui->m_dayCheck6;
	m_ui->m_resultsGroupBox->setVisible(false);
}


IC18599ScheduleEditWidget::IC18599ScheduleEditWidget(const QString &title,
													   const QColor &barColor,
													   QWidget *parent,
													   const QString &unitLabel,
													   double minVal,
													   double maxVal,
													   double defaultVal) :
	IC18599ScheduleEditWidget(parent)
{
	configure(title, barColor, unitLabel, minVal, maxVal, defaultVal);
}


IC18599ScheduleEditWidget::~IC18599ScheduleEditWidget() {
	delete m_ui;
}


void IC18599ScheduleEditWidget::configure(const QString &title, const QColor &barColor,
										  const QString &unitLabel,
										  double minVal, double maxVal,
										  double defaultVal)
{
	configure(title, { {title, unitLabel, barColor, minVal, maxVal, defaultVal} });
}


void IC18599ScheduleEditWidget::configure(const QString &title,
										  const std::vector<ScheduleChannel> &channels)
{
	if (m_configured)
		return;
	m_configured = true;

	m_channels = channels;
	int nCh = (int)channels.size();

	// Schedule group box title
	m_ui->groupBox->setTitle(title);

	// Table: set column count to 1 + nCh (Time + one column per channel)
	m_ui->m_tableWidget->setColumnCount(1 + nCh);
	QStringList headers = {tr("Time")};
	for (const auto &ch : channels)
		headers << ch.unitLabel;
	m_ui->m_tableWidget->setHorizontalHeaderLabels(headers);
	m_ui->m_tableWidget->setColumnWidth(0, 80);

	// Populate 24 time rows + default values per channel
	for (int h = 0; h < 24; ++h) {
		// Time item in col 0 (read-only, grey background)
		QTableWidgetItem *timeItem = new QTableWidgetItem(
			QString("%1:00-%2:00").arg(h, 2, 10, QChar('0')).arg(h + 1, 2, 10, QChar('0')));
		timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
		timeItem->setBackground(QColor(240, 240, 240));
		m_ui->m_tableWidget->setItem(h, 0, timeItem);

		// Value items in cols 1..nCh (editable, centered)
		for (int c = 0; c < nCh; ++c) {
			double defVal = channels[(size_t)c].defaultVal;
			QTableWidgetItem *valItem = new QTableWidgetItem(
				QString::number(defVal, 'f', (defVal == (int)defVal) ? 0 : 1));
			valItem->setTextAlignment(Qt::AlignCenter);
			m_ui->m_tableWidget->setItem(h, 1 + c, valItem);
		}
	}

	// Create one editable chart per channel, add to m_scheduleLayout
	for (int c = 0; c < nCh; ++c) {
		auto *chart = new IC18599DayProfileWidget(this);
		chart->setTitle(channels[(size_t)c].name);
		chart->setBarColor(channels[(size_t)c].barColor);
		chart->setYAxisLabel(channels[(size_t)c].unitLabel);
		chart->setMaxValue(channels[(size_t)c].maxVal);
		chart->setMinValue(channels[(size_t)c].minVal);
		chart->setMinimumHeight(140);
		m_ui->m_scheduleLayout->addWidget(chart);
		m_editCharts.push_back(chart);
		connect(chart, &IC18599DayProfileWidget::barClicked, this,
			[this, c](int weekHour, double val) { onBarClicked(c, weekHour, val); });
	}

	// Create initial groups with correct channel count
	DailyCycleGroup weekdays;
	weekdays.m_days = {0, 1, 2, 3, 4};
	weekdays.m_values.clear();
	for (const auto &ch : channels)
		weekdays.m_values.push_back(std::vector<double>(24, ch.defaultVal));
	m_groups.push_back(weekdays);

	DailyCycleGroup weekend;
	weekend.m_days = {5, 6};
	weekend.m_values.clear();
	for (const auto &ch : channels)
		weekend.m_values.push_back(std::vector<double>(24, ch.defaultVal));
	m_groups.push_back(weekend);

	// Connections
	connect(m_ui->m_tableWidget, &QTableWidget::cellChanged,
			this, &IC18599ScheduleEditWidget::onTableCellChanged);
	// Fix: cellChanged doesn't fire when the entered value equals the old value.
	// closeEditor fires whenever editing ends, so we force-apply to all selected cells.
	connect(m_ui->m_tableWidget->itemDelegate(), &QAbstractItemDelegate::closeEditor,
			this, [this](QWidget *, QAbstractItemDelegate::EndEditHint hint) {
		if (m_blockSignals)
			return;  // programmatic update in progress
		if (hint == QAbstractItemDelegate::RevertModelCache)
			return;  // Escape pressed — don't apply
		int row = m_ui->m_tableWidget->currentRow();
		int col = m_ui->m_tableWidget->currentColumn();
		if (col >= 1 && row >= 0 && row < 24)
			onTableCellChanged(row, col);
	});
	connect(m_ui->m_btnBackward, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onBackward);
	connect(m_ui->m_btnForward, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onForward);
	connect(m_ui->m_btnAdd, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onAddGroup);
	connect(m_ui->m_btnDelete, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onDeleteGroup);
	for (int d = 0; d < 7; ++d) {
		connect(m_dayChecks[d], &QCheckBox::toggled, this,
				[this, d]() { onDayCheckChanged(d); });
	}

	updateUI();
}


// --- Data access ---

std::vector<double> IC18599ScheduleEditWidget::weekValues(int channel) const {
	std::vector<double> week(168, 0.0);
	for (const DailyCycleGroup &g : m_groups) {
		if (channel >= g.channelCount())
			continue;
		for (int d : g.m_days) {
			for (int h = 0; h < 24; ++h)
				week[(size_t)(d * 24 + h)] = g.m_values[(size_t)channel][(size_t)h];
		}
	}
	return week;
}


std::vector<double> IC18599ScheduleEditWidget::annualValues(int channel) const {
	std::vector<double> week = weekValues(channel);
	std::vector<double> annual(8760, 0.0);
	for (int h = 0; h < 8760; ++h)
		annual[(size_t)h] = week[(size_t)(h % 168)];
	return annual;
}


void IC18599ScheduleEditWidget::setGroups(const std::vector<DailyCycleGroup> &groups) {
	m_groups = groups;
	if (m_groups.empty()) {
		// Ensure at least one group exists
		DailyCycleGroup weekdays;
		weekdays.m_days = {0, 1, 2, 3, 4};
		weekdays.m_values.clear();
		for (const auto &ch : m_channels)
			weekdays.m_values.push_back(std::vector<double>(24, ch.defaultVal));
		m_groups.push_back(weekdays);
		DailyCycleGroup weekend;
		weekend.m_days = {5, 6};
		weekend.m_values.clear();
		for (const auto &ch : m_channels)
			weekend.m_values.push_back(std::vector<double>(24, ch.defaultVal));
		m_groups.push_back(weekend);
	}
	// Pad missing channels with per-channel defaults
	int nCh = (int)m_channels.size();
	for (auto &g : m_groups) {
		for (int c = g.channelCount(); c < nCh; ++c)
			g.m_values.push_back(std::vector<double>(24, m_channels[(size_t)c].defaultVal));
	}
	m_currentGroupIdx = 0;
	updateUI();
}


std::vector<int> IC18599ScheduleEditWidget::unassignedDays() const {
	std::set<int> assigned;
	for (const DailyCycleGroup &g : m_groups)
		for (int d : g.m_days)
			assigned.insert(d);
	std::vector<int> unassigned;
	for (int d = 0; d < 7; ++d)
		if (assigned.find(d) == assigned.end())
			unassigned.push_back(d);
	return unassigned;
}


// --- Slots ---

void IC18599ScheduleEditWidget::onTableCellChanged(int row, int col) {
	if (m_blockSignals || col < 1 || row < 0 || row >= 24)
		return;
	int channel = col - 1;
	if (channel >= (int)m_channels.size())
		return;
	if (m_currentGroupIdx < 0 || m_currentGroupIdx >= (int)m_groups.size())
		return;

	QTableWidgetItem *item = m_ui->m_tableWidget->item(row, col);
	if (!item) return;

	const ScheduleChannel &chDef = m_channels[(size_t)channel];
	bool ok;
	double val = item->text().toDouble(&ok);
	if (!ok) val = chDef.defaultVal;
	val = std::max(chDef.minVal, std::min(chDef.maxVal, val));

	DailyCycleGroup &grp = m_groups[(size_t)m_currentGroupIdx];
	grp.ensureChannelCount((int)m_channels.size());

	// Apply to all selected ranges (multi-cell editing)
	QList<QTableWidgetSelectionRange> ranges = m_ui->m_tableWidget->selectedRanges();
	m_blockSignals = true;
	for (const QTableWidgetSelectionRange &range : ranges) {
		for (int r = range.topRow(); r <= range.bottomRow(); ++r) {
			QTableWidgetItem *cellItem = m_ui->m_tableWidget->item(r, col);
			if (cellItem)
				cellItem->setText(QString::number(val, 'f', (val == (int)val) ? 0 : 1));
			grp.m_values[(size_t)channel][(size_t)r] = val;
		}
	}
	// Also the edited cell itself
	grp.m_values[(size_t)channel][(size_t)row] = val;
	item->setText(QString::number(val, 'f', (val == (int)val) ? 0 : 1));
	m_blockSignals = false;

	updateChartFromValues();
	emit valuesChanged();
}


void IC18599ScheduleEditWidget::onBarClicked(int channel, int weekHour, double valuePct) {
	if (weekHour < 0 || weekHour >= 168)
		return;
	if (channel < 0 || channel >= (int)m_channels.size())
		return;

	int clickedDay = weekHour / 24;
	int clickedHour = weekHour % 24;

	// Find which group owns this day
	for (int g = 0; g < (int)m_groups.size(); ++g) {
		if (m_groups[(size_t)g].m_days.count(clickedDay)) {
			m_currentGroupIdx = g;
			m_groups[(size_t)g].ensureChannelCount((int)m_channels.size());
			m_groups[(size_t)g].m_values[(size_t)channel][(size_t)clickedHour] = valuePct;
			updateUI();
			emit valuesChanged();
			return;
		}
	}
}


void IC18599ScheduleEditWidget::onDayCheckChanged(int dayIndex) {
	if (m_blockSignals)
		return;
	if (m_currentGroupIdx < 0 || m_currentGroupIdx >= (int)m_groups.size())
		return;

	bool checked = m_dayChecks[dayIndex]->isChecked();
	DailyCycleGroup &currentGrp = m_groups[(size_t)m_currentGroupIdx];

	if (checked) {
		// Remove this day from any other group
		for (int g = 0; g < (int)m_groups.size(); ++g) {
			if (g != m_currentGroupIdx)
				m_groups[(size_t)g].m_days.erase(dayIndex);
		}
		// Add to current group
		currentGrp.m_days.insert(dayIndex);
	}
	else {
		// Remove from current group (day becomes unassigned)
		currentGrp.m_days.erase(dayIndex);
	}

	updateUI();
	emit valuesChanged();
}


void IC18599ScheduleEditWidget::onForward() {
	if (m_currentGroupIdx < (int)m_groups.size() - 1) {
		m_currentGroupIdx++;
		updateUI();
	}
}


void IC18599ScheduleEditWidget::onBackward() {
	if (m_currentGroupIdx > 0) {
		m_currentGroupIdx--;
		updateUI();
	}
}


void IC18599ScheduleEditWidget::onAddGroup() {
	DailyCycleGroup newGrp;
	// Assign unassigned days to new group
	std::vector<int> ua = unassignedDays();
	for (int d : ua)
		newGrp.m_days.insert(d);
	// Initialize with correct channel count
	newGrp.m_values.clear();
	for (const auto &ch : m_channels)
		newGrp.m_values.push_back(std::vector<double>(24, ch.defaultVal));
	m_groups.push_back(newGrp);
	m_currentGroupIdx = (int)m_groups.size() - 1;
	updateUI();
}


void IC18599ScheduleEditWidget::onDeleteGroup() {
	if (m_groups.size() <= 1)
		return;  // keep at least one group
	m_groups.erase(m_groups.begin() + m_currentGroupIdx);
	if (m_currentGroupIdx >= (int)m_groups.size())
		m_currentGroupIdx = (int)m_groups.size() - 1;
	updateUI();
	emit valuesChanged();
}


// --- UI updates ---

void IC18599ScheduleEditWidget::updateUI() {
	updateDayCheckboxes();
	updateGroupLabel();
	updateTableFromGroup();
	updateChartFromValues();
}


void IC18599ScheduleEditWidget::updateDayCheckboxes() {
	m_blockSignals = true;
	if (m_currentGroupIdx >= 0 && m_currentGroupIdx < (int)m_groups.size()) {
		const DailyCycleGroup &grp = m_groups[(size_t)m_currentGroupIdx];
		for (int d = 0; d < 7; ++d) {
			m_dayChecks[d]->setChecked(grp.m_days.count(d) > 0);
			// Gray out days assigned to other groups
			bool assignedElsewhere = false;
			for (int g = 0; g < (int)m_groups.size(); ++g) {
				if (g != m_currentGroupIdx && m_groups[(size_t)g].m_days.count(d)) {
					assignedElsewhere = true;
					break;
				}
			}
			// Show which group owns the day via tooltip
			if (assignedElsewhere) {
				for (int g = 0; g < (int)m_groups.size(); ++g) {
					if (m_groups[(size_t)g].m_days.count(d)) {
						m_dayChecks[d]->setToolTip(tr("Assigned to Group %1").arg(g + 1));
						break;
					}
				}
			}
			else if (grp.m_days.count(d)) {
				m_dayChecks[d]->setToolTip(tr("In current group"));
			}
			else {
				m_dayChecks[d]->setToolTip(tr("Not assigned"));
			}
		}
	}
	m_blockSignals = false;
}


void IC18599ScheduleEditWidget::updateGroupLabel() {
	m_ui->m_groupLabel->setText(tr("Group %1/%2")
		.arg(m_currentGroupIdx + 1)
		.arg(m_groups.size()));

	m_ui->m_btnBackward->setEnabled(m_currentGroupIdx > 0);
	m_ui->m_btnForward->setEnabled(m_currentGroupIdx < (int)m_groups.size() - 1);
	m_ui->m_btnDelete->setEnabled(m_groups.size() > 1);
	m_ui->m_btnAdd->setEnabled(!unassignedDays().empty() || m_groups.size() < 7);

	// Validation: check for unassigned days
	static const char * DAY_NAMES[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	std::vector<int> ua = unassignedDays();
	if (!ua.empty()) {
		QStringList dayNames;
		for (int d : ua)
			dayNames << DAY_NAMES[d];
		m_ui->m_validLabel->setText(tr("Warning: %1 not assigned to any group").arg(dayNames.join(", ")));
		m_ui->m_validLabel->setStyleSheet("color: red; font-weight: bold;");
	}
	else {
		m_ui->m_validLabel->setText({});
		m_ui->m_validLabel->setStyleSheet({});
	}
}


void IC18599ScheduleEditWidget::updateTableFromGroup() {
	m_blockSignals = true;
	if (m_currentGroupIdx >= 0 && m_currentGroupIdx < (int)m_groups.size()) {
		const DailyCycleGroup &grp = m_groups[(size_t)m_currentGroupIdx];
		int nCh = (int)m_channels.size();
		for (int h = 0; h < 24; ++h) {
			for (int c = 0; c < nCh; ++c) {
				QTableWidgetItem *item = m_ui->m_tableWidget->item(h, 1 + c);
				if (item) {
					double val = (c < grp.channelCount()) ? grp.m_values[(size_t)c][(size_t)h] : 0.0;
					item->setText(QString::number(val, 'f', (val == (int)val) ? 0 : 1));
				}
			}
		}
	}
	m_blockSignals = false;
}


void IC18599ScheduleEditWidget::updateChartFromValues() {
	// Update each editable chart
	for (int c = 0; c < (int)m_editCharts.size(); ++c) {
		std::vector<double> week = weekValues(c);
		m_editCharts[(size_t)c]->setWeekValues(week);

		// Highlight days of current group
		if (m_currentGroupIdx >= 0 && m_currentGroupIdx < (int)m_groups.size()) {
			const DailyCycleGroup &grp = m_groups[(size_t)m_currentGroupIdx];
			std::vector<int> days(grp.m_days.begin(), grp.m_days.end());
			m_editCharts[(size_t)c]->setSelectedDays(days);
		}
	}
}


void IC18599ScheduleEditWidget::setResultChartCount(int count) {
	// Remove excess charts
	while ((int)m_resultCharts.size() > count) {
		IC18599DayProfileWidget *w = m_resultCharts.back();
		m_ui->m_resultsLayout->removeWidget(w);
		delete w;
		m_resultCharts.pop_back();
	}
	// Add missing charts
	while ((int)m_resultCharts.size() < count) {
		IC18599DayProfileWidget *w = new IC18599DayProfileWidget(this);
		w->setReadOnly(true);
		w->setMaxValue(1.0);
		w->setMinimumHeight(100);
		m_ui->m_resultsLayout->addWidget(w);
		m_resultCharts.push_back(w);
	}
	m_ui->m_resultsGroupBox->setVisible(count > 0);
}


void IC18599ScheduleEditWidget::setResultData(int idx, const std::vector<double> &weekValues,
											   const QString &title, const QString &yLabel,
											   const QColor &color) {
	if (idx < 0 || idx >= (int)m_resultCharts.size())
		return;

	IC18599DayProfileWidget *chart = m_resultCharts[(size_t)idx];

	// Auto-scale: find max value
	double maxVal = 1.0;
	for (double v : weekValues)
		if (v > maxVal) maxVal = v;
	// Round up to nice number
	if (maxVal <= 1.0)
		maxVal = 1.0;
	else if (maxVal <= 10.0)
		maxVal = std::ceil(maxVal);
	else
		maxVal = std::ceil(maxVal / 10.0) * 10.0;

	chart->setTitle(title);
	chart->setMaxValue(maxVal);
	chart->setYAxisLabel(yLabel);
	chart->setBarColor(color);
	chart->setWeekValues(weekValues);
}


IC18599DayProfileWidget* IC18599ScheduleEditWidget::resultChart(int idx) const {
	return (idx >= 0 && idx < (int)m_resultCharts.size()) ? m_resultCharts[(size_t)idx] : nullptr;
}


IC18599DayProfileWidget* IC18599ScheduleEditWidget::editChart(int channel) const {
	return (channel >= 0 && channel < (int)m_editCharts.size()) ? m_editCharts[(size_t)channel] : nullptr;
}
