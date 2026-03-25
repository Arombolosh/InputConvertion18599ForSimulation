#ifndef SciCurveTrackerH
#define SciCurveTrackerH

#include <qwt_plot_tracker.h>

#include <Sci_PlotPickerText.h>

namespace SCI {

/*! QwtPlotTracker implementation for SciChart. */
class CurveTracker  : public QwtPlotTracker, public PlotPickerText {
public:
	CurveTracker( QWidget* canvas, const ChartAxis * bottomAxis );

	void setVisible(bool visible);
	void setNumberFormat(char format);
	void setNumberPrecision(int precision);

	// re-implemented from QwtPlotTracker
	QList<QVariant> trackerDataAt(const QwtPlotItem *plotItem, const QPointF &pos) const override;

protected:
	/*! Re-implemented to provide current x and y coordinates in title. */
	QwtText title() const override;
	virtual void drawTracker( QPainter* ) const override;
	void updateLayoutItems(const QPointF & trackerPosition, QwtDynGridLayout & layout) const override;

private:
	bool	m_visible = false;
	char	m_format = 'f';
	int		m_precision = 2;
};

} // namespace SCI

#endif // SciCurveTrackerH
