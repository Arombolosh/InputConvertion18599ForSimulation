#include "IC18599ScheduleEditWidget.h"
#include "IC18599DayProfileWidget.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QHeaderView>
#include <QLabel>
#include <QTableWidget>
#include <QToolButton>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

static const char* DAY_SHORT[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

IC18599ScheduleEditWidget::IC18599ScheduleEditWidget(const QString &title,
													   const QColor &barColor,
													   QWidget *parent) :
	QWidget(parent),
	m_currentGroupIdx(0),
	m_blockSignals(false),
	m_barColor(barColor)
{
	// Create initial group with all weekdays
	DailyCycleGroup weekdays;
	weekdays.m_days = {0, 1, 2, 3, 4};
	m_groups.push_back(weekdays);

	// Create second group with weekend
	DailyCycleGroup weekend;
	weekend.m_days = {5, 6};
	m_groups.push_back(weekend);

	setupUI(title, barColor);
	updateUI();
}


void IC18599ScheduleEditWidget::setupUI(const QString &title, const QColor &barColor) {
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setContentsMargins(4, 4, 4, 4);
	m_mainLayout->setSpacing(4);

	// Title
	m_infoLabel = new QLabel(title, this);
	QFont f = m_infoLabel->font();
	f.setBold(true);
	f.setPointSize(11);
	m_infoLabel->setFont(f);
	m_mainLayout->addWidget(m_infoLabel);

	// Week chart (top, full width)
	m_chartWidget = new IC18599DayProfileWidget(this);
	m_chartWidget->setBarColor(barColor);
	m_chartWidget->setYAxisLabel(tr("[%]"));
	m_chartWidget->setMinimumHeight(160);
	m_mainLayout->addWidget(m_chartWidget, 3);

	// Bottom: table (left) + day group panel (right)
	QHBoxLayout *bottomLayout = new QHBoxLayout;
	bottomLayout->setSpacing(8);

	// --- Compact table (left) ---
	m_tableWidget = new QTableWidget(24, 2, this);
	m_tableWidget->setHorizontalHeaderLabels({tr("Time"), tr("[%]")});
	m_tableWidget->horizontalHeader()->setStretchLastSection(true);
	m_tableWidget->horizontalHeader()->setDefaultSectionSize(50);
	m_tableWidget->setColumnWidth(0, 80);
	m_tableWidget->setAlternatingRowColors(true);
	m_tableWidget->verticalHeader()->setVisible(false);
	m_tableWidget->verticalHeader()->setDefaultSectionSize(20);
	m_tableWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_tableWidget->setMaximumWidth(240);
	m_tableWidget->setMinimumWidth(180);

	QFont tableFont = font();
	tableFont.setPointSize(8);
	m_tableWidget->setFont(tableFont);

	for (int h = 0; h < 24; ++h) {
		QTableWidgetItem *timeItem = new QTableWidgetItem(
			QString("%1:00-%2:00").arg(h, 2, 10, QChar('0')).arg(h + 1, 2, 10, QChar('0')));
		timeItem->setFlags(timeItem->flags() & ~Qt::ItemIsEditable);
		timeItem->setBackground(QColor(240, 240, 240));
		m_tableWidget->setItem(h, 0, timeItem);

		QTableWidgetItem *valItem = new QTableWidgetItem("0");
		valItem->setTextAlignment(Qt::AlignCenter);
		m_tableWidget->setItem(h, 1, valItem);
	}
	bottomLayout->addWidget(m_tableWidget);

	// --- Day group panel (right) ---
	QVBoxLayout *panelLayout = new QVBoxLayout;
	panelLayout->setSpacing(4);

	// Navigation: backward, group label, forward
	QHBoxLayout *navLayout = new QHBoxLayout;
	navLayout->setSpacing(2);

	m_btnBackward = new QToolButton(this);
	m_btnBackward->setArrowType(Qt::LeftArrow);
	m_btnBackward->setToolTip(tr("Previous Group"));
	navLayout->addWidget(m_btnBackward);

	m_groupLabel = new QLabel(this);
	m_groupLabel->setAlignment(Qt::AlignCenter);
	m_groupLabel->setMinimumWidth(80);
	QFont glf = m_groupLabel->font();
	glf.setBold(true);
	m_groupLabel->setFont(glf);
	navLayout->addWidget(m_groupLabel);

	m_btnForward = new QToolButton(this);
	m_btnForward->setArrowType(Qt::RightArrow);
	m_btnForward->setToolTip(tr("Next Group"));
	navLayout->addWidget(m_btnForward);

	panelLayout->addLayout(navLayout);

	// Add / Delete buttons
	QHBoxLayout *addDelLayout = new QHBoxLayout;
	addDelLayout->setSpacing(2);

	m_btnAdd = new QToolButton(this);
	m_btnAdd->setText("+");
	m_btnAdd->setToolTip(tr("Create New Group"));
	m_btnAdd->setFixedSize(28, 28);
	addDelLayout->addWidget(m_btnAdd);

	m_btnDelete = new QToolButton(this);
	m_btnDelete->setText("-");  // Unicode minus
	m_btnDelete->setToolTip(tr("Delete Current Group"));
	m_btnDelete->setFixedSize(28, 28);
	addDelLayout->addWidget(m_btnDelete);

	addDelLayout->addStretch();
	panelLayout->addLayout(addDelLayout);

	// Validity label
	m_validLabel = new QLabel(this);
	m_validLabel->setAlignment(Qt::AlignCenter);
	m_validLabel->setFixedHeight(22);
	m_validLabel->setFont(QFont(font().family(), 8, QFont::Bold));
	panelLayout->addWidget(m_validLabel);

	// Day checkboxes (vertical)
	QGroupBox *dayGroup = new QGroupBox(tr("Day Types"), this);
	QVBoxLayout *dayGroupLayout = new QVBoxLayout(dayGroup);
	dayGroupLayout->setSpacing(2);
	dayGroupLayout->setContentsMargins(6, 10, 6, 6);

	for (int d = 0; d < 7; ++d) {
		m_dayChecks[d] = new QCheckBox(DAY_SHORT[d], dayGroup);
		dayGroupLayout->addWidget(m_dayChecks[d]);
		connect(m_dayChecks[d], &QCheckBox::toggled, this,
				[this, d]() { onDayCheckChanged(d); });
		if (d == 4) {
			QFrame *line = new QFrame(dayGroup);
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			dayGroupLayout->addWidget(line);
		}
	}

	dayGroupLayout->addStretch();
	panelLayout->addWidget(dayGroup);

	// Hint label
	QLabel *hint = new QLabel(tr("Select multiple cells,\nenter a value + Enter\n= apply to all."), this);
	hint->setStyleSheet("color: gray; font-size: 8pt;");
	hint->setWordWrap(true);
	panelLayout->addWidget(hint);

	panelLayout->addStretch();
	bottomLayout->addLayout(panelLayout);
	bottomLayout->addStretch();

	m_mainLayout->addLayout(bottomLayout, 5);

	// Connections
	connect(m_tableWidget, &QTableWidget::cellChanged,
			this, &IC18599ScheduleEditWidget::onTableCellChanged);
	connect(m_chartWidget, &IC18599DayProfileWidget::barClicked,
			this, &IC18599ScheduleEditWidget::onBarClicked);
	connect(m_btnBackward, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onBackward);
	connect(m_btnForward, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onForward);
	connect(m_btnAdd, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onAddGroup);
	connect(m_btnDelete, &QToolButton::clicked, this, &IC18599ScheduleEditWidget::onDeleteGroup);
}


// --- Data access ---

std::vector<double> IC18599ScheduleEditWidget::weekValues() const {
	std::vector<double> week(168, 0.0);
	for (const DailyCycleGroup &g : m_groups) {
		for (int d : g.m_days) {
			for (int h = 0; h < 24; ++h)
				week[(size_t)(d * 24 + h)] = g.m_values[(size_t)h];
		}
	}
	return week;
}


std::vector<double> IC18599ScheduleEditWidget::annualValues() const {
	std::vector<double> week = weekValues();
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
		m_groups.push_back(weekdays);
		DailyCycleGroup weekend;
		weekend.m_days = {5, 6};
		m_groups.push_back(weekend);
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
	if (m_blockSignals || col != 1 || row < 0 || row >= 24)
		return;
	if (m_currentGroupIdx < 0 || m_currentGroupIdx >= (int)m_groups.size())
		return;

	QTableWidgetItem *item = m_tableWidget->item(row, 1);
	if (!item) return;

	bool ok;
	double val = item->text().toDouble(&ok);
	if (!ok) val = 0.0;
	val = std::max(0.0, std::min(100.0, val));

	DailyCycleGroup &grp = m_groups[(size_t)m_currentGroupIdx];

	// Apply to all selected ranges (multi-cell editing)
	QList<QTableWidgetSelectionRange> ranges = m_tableWidget->selectedRanges();
	m_blockSignals = true;
	for (const QTableWidgetSelectionRange &range : ranges) {
		for (int r = range.topRow(); r <= range.bottomRow(); ++r) {
			QTableWidgetItem *cellItem = m_tableWidget->item(r, 1);
			if (cellItem)
				cellItem->setText(QString::number(val, 'f', (val == (int)val) ? 0 : 1));
			grp.m_values[(size_t)r] = val;
		}
	}
	// Also the edited cell itself
	grp.m_values[(size_t)row] = val;
	item->setText(QString::number(val, 'f', (val == (int)val) ? 0 : 1));
	m_blockSignals = false;

	updateChartFromValues();
	emit valuesChanged();
}


void IC18599ScheduleEditWidget::onBarClicked(int weekHour, double valuePct) {
	if (weekHour < 0 || weekHour >= 168)
		return;

	int clickedDay = weekHour / 24;
	int clickedHour = weekHour % 24;

	// Find which group owns this day
	for (int g = 0; g < (int)m_groups.size(); ++g) {
		if (m_groups[(size_t)g].m_days.count(clickedDay)) {
			m_currentGroupIdx = g;
			m_groups[(size_t)g].m_values[(size_t)clickedHour] = valuePct;
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
	m_groups.push_back(newGrp);
	m_currentGroupIdx = (int)m_groups.size() - 1;
	updateUI();
}


void IC18599ScheduleEditWidget::onDeleteGroup() {
	if (m_groups.size() <= 1)
		return;  // keep at least one group
	// Days become unassigned (or reassign to first remaining group?)
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
	m_groupLabel->setText(tr("Group %1/%2")
		.arg(m_currentGroupIdx + 1)
		.arg(m_groups.size()));

	m_btnBackward->setEnabled(m_currentGroupIdx > 0);
	m_btnForward->setEnabled(m_currentGroupIdx < (int)m_groups.size() - 1);
	m_btnDelete->setEnabled(m_groups.size() > 1);
	m_btnAdd->setEnabled(!unassignedDays().empty() || m_groups.size() < 7);
}


void IC18599ScheduleEditWidget::updateTableFromGroup() {
	m_blockSignals = true;
	if (m_currentGroupIdx >= 0 && m_currentGroupIdx < (int)m_groups.size()) {
		const DailyCycleGroup &grp = m_groups[(size_t)m_currentGroupIdx];
		for (int h = 0; h < 24; ++h) {
			QTableWidgetItem *item = m_tableWidget->item(h, 1);
			if (item) {
				double val = grp.m_values[(size_t)h];
				item->setText(QString::number(val, 'f', (val == (int)val) ? 0 : 1));
			}
		}
	}
	m_blockSignals = false;
}


void IC18599ScheduleEditWidget::updateChartFromValues() {
	std::vector<double> week = weekValues();
	m_chartWidget->setWeekValues(week);

	// Highlight days of current group
	if (m_currentGroupIdx >= 0 && m_currentGroupIdx < (int)m_groups.size()) {
		const DailyCycleGroup &grp = m_groups[(size_t)m_currentGroupIdx];
		std::vector<int> days(grp.m_days.begin(), grp.m_days.end());
		m_chartWidget->setSelectedDays(days);
	}
}


void IC18599ScheduleEditWidget::setResultChartCount(int count) {
	// Remove excess charts
	while ((int)m_resultCharts.size() > count) {
		IC18599DayProfileWidget *w = m_resultCharts.back();
		m_mainLayout->removeWidget(w);
		delete w;
		m_resultCharts.pop_back();
	}
	// Add missing charts
	while ((int)m_resultCharts.size() < count) {
		IC18599DayProfileWidget *w = new IC18599DayProfileWidget(this);
		w->setReadOnly(true);
		w->setMaxValue(1.0);
		w->setMinimumHeight(100);
		m_mainLayout->addWidget(w, 2);
		m_resultCharts.push_back(w);
	}
}


void IC18599ScheduleEditWidget::setResultData(int idx, const std::vector<double> &weekValues,
											   const QString &yLabel, const QColor &color) {
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

	chart->setMaxValue(maxVal);
	chart->setYAxisLabel(yLabel);
	chart->setBarColor(color);
	chart->setWeekValues(weekValues);
}
