#ifndef IC18599ScheduleEditWidgetH
#define IC18599ScheduleEditWidgetH

#include <QWidget>
#include <vector>
#include <set>

class QTableWidget;
class QLabel;
class QCheckBox;
class QToolButton;
class QVBoxLayout;
class IC18599DayProfileWidget;

/*! A daily cycle group: a set of days sharing one 24h profile. */
struct DailyCycleGroup {
	std::set<int>			m_days;			///< assigned days (0=Mo..6=So)
	std::vector<double>		m_values;		///< 24 hourly percent values (0..100)
	DailyCycleGroup() : m_values(24, 0.0) {}
};


/*! Widget for editing a weekly schedule via daily cycle groups.
	Each group has assigned days and a shared 24h profile.
	Navigation: forward/backward, add/delete groups.
*/
class IC18599ScheduleEditWidget : public QWidget {
	Q_OBJECT
public:
	explicit IC18599ScheduleEditWidget(const QString &title,
									   const QColor &barColor,
									   QWidget *parent = nullptr);

	/*! Returns 168 weekly percent values assembled from all groups. */
	std::vector<double> weekValues() const;

	/*! Returns 8760 annual values (week repeated). */
	std::vector<double> annualValues() const;

	/*! Returns the current daily cycle groups. */
	const std::vector<DailyCycleGroup>& groups() const { return m_groups; }

	/*! Replaces the daily cycle groups and updates the UI. */
	void setGroups(const std::vector<DailyCycleGroup> &groups);

	/*! Sets the number of read-only result charts (0, 1 or 2). */
	void setResultChartCount(int count);

	/*! Sets data for a result chart. */
	void setResultData(int idx, const std::vector<double> &weekValues,
					   const QString &yLabel, const QColor &color);

signals:
	void valuesChanged();

private slots:
	void onTableCellChanged(int row, int col);
	void onBarClicked(int weekHour, double valuePct);
	void onDayCheckChanged(int dayIndex);
	void onForward();
	void onBackward();
	void onAddGroup();
	void onDeleteGroup();

private:
	void setupUI(const QString &title, const QColor &barColor);
	void updateUI();
	void updateTableFromGroup();
	void updateChartFromValues();
	void updateDayCheckboxes();
	void updateGroupLabel();
	std::vector<int> unassignedDays() const;

	IC18599DayProfileWidget				*m_chartWidget = nullptr;
	std::vector<IC18599DayProfileWidget*>	m_resultCharts;
	QVBoxLayout							*m_mainLayout = nullptr;
	QTableWidget						*m_tableWidget = nullptr;
	QCheckBox							*m_dayChecks[7] = {};
	QToolButton							*m_btnBackward = nullptr;
	QToolButton							*m_btnForward = nullptr;
	QToolButton							*m_btnAdd = nullptr;
	QToolButton							*m_btnDelete = nullptr;
	QLabel								*m_groupLabel = nullptr;
	QLabel								*m_validLabel = nullptr;
	QLabel								*m_infoLabel = nullptr;

	std::vector<DailyCycleGroup>		m_groups;
	int									m_currentGroupIdx = 0;
	bool								m_blockSignals = false;
	QColor								m_barColor;
};

#endif // IC18599ScheduleEditWidgetH
