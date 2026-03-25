#include "Sci_PlotLayout.h"

namespace SCI {

void PlotLayout::activate( const QwtPlot *plot,
						   const QRectF &plotRect, Options options )
{
	options |= IgnoreFrames;
	QwtPlotLayout::activate(plot, plotRect, options);
}

} // namespace SCI
