#ifndef IC18599ScheduleEditWidgetH
#define IC18599ScheduleEditWidgetH

#include <QWidget>
#include <QColor>
#include <vector>
#include <set>

class QCheckBox;
class IC18599DayProfileWidget;

namespace Ui {
	class IC18599ScheduleEditWidget;
}

/*! A daily cycle group: a set of days sharing one 24h profile.
	Supports multiple channels (e.g. occupancy + activity).
	m_values[channel][hour], each inner vector has 24 elements.
*/
struct DailyCycleGroup {
	std::set<int>							m_days;		///< assigned days (0=Mo..6=So)
	std::vector<std::vector<double>>		m_values;	///< m_values[channel][hour]
	DailyCycleGroup() : m_values(1, std::vector<double>(24, 0.0)) {}

	int channelCount() const { return (int)m_values.size(); }
	void ensureChannelCount(int n, double defaultVal = 0.0) {
		while ((int)m_values.size() < n)
			m_values.push_back(std::vector<double>(24, defaultVal));
	}
};


/*! Defines one editable channel in a schedule widget. */
struct ScheduleChannel {
	QString name;			///< Chart title, e.g. "Occupancy"
	QString unitLabel;		///< Table header + Y-axis, e.g. "[%]"
	QColor  barColor;
	double  minVal     = 0.0;
	double  maxVal     = 100.0;
	double  defaultVal = 0.0;
};


/*! Widget for editing a weekly schedule via daily cycle groups.
	Each group has assigned days and a shared 24h profile.
	Navigation: forward/backward, add/delete groups.
*/
class IC18599ScheduleEditWidget : public QWidget {
	Q_OBJECT
public:
	/*! Default constructor for Qt Designer promoted widget usage. */
	explicit IC18599ScheduleEditWidget(QWidget *parent = nullptr);

	/*! Creates a schedule edit widget.
		\param title		Displayed title text.
		\param barColor		Color for bars in the chart.
		\param unitLabel	Column header / Y-axis label, e.g. "[%]" or "[°C]".
		\param minVal		Minimum allowed value (default 0).
		\param maxVal		Maximum allowed value (default 100).
		\param defaultVal	Default value for new groups (default 0).
	*/
	explicit IC18599ScheduleEditWidget(const QString &title,
									   const QColor &barColor,
									   QWidget *parent,
									   const QString &unitLabel = "[%]",
									   double minVal = 0.0,
									   double maxVal = 100.0,
									   double defaultVal = 0.0);

	~IC18599ScheduleEditWidget();

	/*! Configures the widget with multiple channels.
		Must be called exactly once after default construction.
	*/
	void configure(const QString &title, const std::vector<ScheduleChannel> &channels);

	/*! Convenience: single-channel configure (wraps multi-channel version). */
	void configure(const QString &title, const QColor &barColor,
				   const QString &unitLabel = "[%]",
				   double minVal = 0.0, double maxVal = 100.0,
				   double defaultVal = 0.0);

	/*! Returns 168 weekly values assembled from all groups for a given channel. */
	std::vector<double> weekValues(int channel = 0) const;

	/*! Returns 8760 annual values (week repeated) for a given channel. */
	std::vector<double> annualValues(int channel = 0) const;

	/*! Returns number of configured channels. */
	int channelCount() const { return (int)m_channels.size(); }

	/*! Returns the current daily cycle groups. */
	const std::vector<DailyCycleGroup>& groups() const { return m_groups; }

	/*! Replaces the daily cycle groups and updates the UI. */
	void setGroups(const std::vector<DailyCycleGroup> &groups);

	/*! Sets the number of read-only result charts (0, 1 or 2). */
	void setResultChartCount(int count);

	/*! Sets data for a result chart. */
	void setResultData(int idx, const std::vector<double> &weekValues,
					   const QString &title, const QString &yLabel, const QColor &color);

signals:
	void valuesChanged();

private slots:
	void onTableCellChanged(int row, int col);
	void onBarClicked(int channel, int weekHour, double valuePct);
	void onDayCheckChanged(int dayIndex);
	void onForward();
	void onBackward();
	void onAddGroup();
	void onDeleteGroup();

private:
	void updateUI();
	void updateTableFromGroup();
	void updateChartFromValues();
	void updateDayCheckboxes();
	void updateGroupLabel();
	std::vector<int> unassignedDays() const;

	Ui::IC18599ScheduleEditWidget			*m_ui = nullptr;
	QCheckBox								*m_dayChecks[7] = {};
	std::vector<ScheduleChannel>			m_channels;			///< channel definitions
	std::vector<IC18599DayProfileWidget*>	m_editCharts;		///< one editable chart per channel
	std::vector<IC18599DayProfileWidget*>	m_resultCharts;		///< read-only result charts

	std::vector<DailyCycleGroup>			m_groups;
	int										m_currentGroupIdx = 0;
	bool									m_blockSignals = false;
	bool									m_configured = false;
};

#endif // IC18599ScheduleEditWidgetH
