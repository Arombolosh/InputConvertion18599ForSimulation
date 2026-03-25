#include "Sci_LegendItem.h"

#include <qwt_plot_item.h>

namespace SCI {

LegendItem::LegendItem() {
//	setRenderHint( QwtPlotItem::RenderAntialiased );

	// properties set below are not (yet) managed by chart model
	setMargin(4);
}

} // namespace SCI
