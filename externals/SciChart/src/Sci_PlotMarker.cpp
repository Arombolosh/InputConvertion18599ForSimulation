#include "Sci_PlotMarker.h"

#include <qwt_text.h>

#include "Sci_Marker.h"

namespace SCI {

void PlotMarker::setProperties(const Marker & m) {
	setLineStyle( (QwtPlotMarker::LineStyle)m.m_markerType );
	setXValue(m.m_xPos);
	setYValue(m.m_yPos);
	setXAxis(m.m_xAxisID);
	setYAxis(m.m_yAxisID);
	QwtText labelText(m.m_label);
	labelText.setFont(m.m_labelFont);
	setLabel(labelText);
	setLabelAlignment(m.m_labelAlignment);
	setLabelOrientation(m.m_labelOrientation);
	setSpacing(m.m_spacing);
	setLinePen(m.m_pen);
	setZ(m.m_zOrder);
}


} // namespace SCI
