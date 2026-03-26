#ifndef IC18599DayProfileWidgetH
#define IC18599DayProfileWidgetH

#include <QWidget>
#include <vector>

/*! Custom widget that draws a weekly bar chart (7 days x 24 hours).
	Displays percentage values (0-100) as vertical bars.
	The currently selected day is highlighted.
*/
class IC18599DayProfileWidget : public QWidget {
	Q_OBJECT
public:
	explicit IC18599DayProfileWidget(QWidget *parent = nullptr);

	/*! Sets all 168 hourly values (7 days x 24 hours, 0..100 percent). */
	void setWeekValues(const std::vector<double> &vals);

	/*! Returns current 168 weekly values. */
	const std::vector<double> & weekValues() const { return m_weekValues; }

	/*! Sets which day (0=Mo..6=So) is highlighted. -1 clears selection. */
	void setSelectedDay(int day) { m_selectedDays.clear(); if (day >= 0) m_selectedDays.push_back(day); update(); }

	/*! Sets multiple highlighted days. */
	void setSelectedDays(const std::vector<int> &days) { m_selectedDays = days; update(); }

	/*! Sets the Y-axis label. */
	void setYAxisLabel(const QString &label) { m_yAxisLabel = label; update(); }

	/*! Sets the chart title drawn above the plot area. */
	void setTitle(const QString &t) { m_title = t; update(); }

	/*! Sets the color of the bars. */
	void setBarColor(const QColor &c) { m_barColor = c; update(); }

	/*! Sets read-only mode (disables mouse interaction). */
	void setReadOnly(bool ro) { m_readOnly = ro; }

	/*! Sets the maximum value for Y-axis (default 100). */
	void setMaxValue(double mv) { m_maxValue = mv; update(); }

	/*! Sets the minimum value for Y-axis (default 0).
		Note: bars are always drawn from the minimum value upward. */
	void setMinValue(double mv) { m_minValue = mv; update(); }

	QSize minimumSizeHint() const override { return QSize(600, 100); }
	QSize sizeHint() const override { return QSize(900, 160); }

signals:
	/*! Emitted when user clicks/drags on a bar. */
	void barClicked(int weekHour, double valuePct);

protected:
	void paintEvent(QPaintEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

private:
	void paintChart(QPainter *p, const QRect &rect, bool showSelection) const;
	QRectF plotArea() const;
	int hourFromPos(const QPointF &pos) const;
	double pctFromPos(const QPointF &pos) const;

	/*! 168 hourly values (7 days x 24 hours). */
	std::vector<double>		m_weekValues;

	/*! Title drawn above the plot area. */
	QString					m_title;

	/*! Label shown on the Y-axis. */
	QString					m_yAxisLabel;

	/*! Color used for the bars. */
	QColor					m_barColor;

	/*! Currently highlighted days (0=Mo..6=So). */
	std::vector<int>		m_selectedDays;

	/*! If true, mouse interaction is disabled. */
	bool					m_readOnly = false;

	/*! Maximum value for Y-axis scaling. */
	double					m_maxValue = 100.0;

	/*! Minimum value for Y-axis scaling. */
	double					m_minValue = 0.0;

	int marginTop() const { return m_title.isEmpty() ? 10 : 22; }

	static constexpr int	MARGIN_LEFT = 50;
	static constexpr int	MARGIN_RIGHT = 10;
	static constexpr int	MARGIN_BOTTOM = 30;
};

#endif // IC18599DayProfileWidgetH
