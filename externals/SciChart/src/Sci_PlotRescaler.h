#ifndef SCI_PLOTRESCALER_H
#define SCI_PLOTRESCALER_H

#include <qwt_plot_rescaler.h>

namespace SCI {

	class Chart;

/*! Specialized class of QwtPlotRescaler. */
class PlotRescaler : public QwtPlotRescaler {
public:
	/*! Constructor. */
	PlotRescaler(SCI::Chart *chart);

	/*! Call this function to switch between adjusting left of bottom axis. */
	void setAdjustingLeftAxis(bool leftAxisAdjusted);

	/*! The min value set for the axis (in axis scale unit). */
	double m_minValueOffset;

protected:
	/*! Re-implemented to apply offset shift. */
	virtual QwtInterval syncScale( int axis, const QwtInterval& reference, const QSize &size ) const override;

private:
	/*! Pointer to the chart we belong to - needed to get currently assigned model. */
	SCI::Chart *m_chart;
};

} // namespace SCI

#endif // SCI_PLOTRESCALER_H
