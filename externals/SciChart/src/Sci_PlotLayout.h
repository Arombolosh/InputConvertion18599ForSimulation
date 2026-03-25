#ifndef SciPlotLayoutH
#define SciPlotLayoutH

#include <qwt_plot_layout.h>

namespace SCI {

/*! Re-implemented QwtPlotLayout, only for fixing the pixel-offset when drawing with a black thin
	frame without margins.
*/
class PlotLayout : public QwtPlotLayout {
public:
	virtual void activate( const QwtPlot *,
		const QRectF &rect, Options options = Options() );
};

} // namespace SCI

#endif // SciPlotLayoutH
