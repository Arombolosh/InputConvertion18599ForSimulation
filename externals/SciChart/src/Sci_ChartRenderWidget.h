#ifndef Sci_ChartRenderWidgetH
#define Sci_ChartRenderWidgetH

#include <QWidget>

#include <qwt_plot_renderer.h>

namespace SCI {

class Chart;

/*! A widget that has the only purpose to render the chart (as preview)
	into the area.
	It can be set an aspect ratio which is honored while resizing the widget.

	Inside the paintEvent(), the render
*/
class ChartRenderWidget : public QWidget {
	Q_OBJECT
	Q_DISABLE_COPY(ChartRenderWidget)
public:
	ChartRenderWidget(QWidget * parent);

	/*! The actual chart. */
	Chart							*m_chart;
	/*! The desired aspect ratio of the chart in aspect = width/height */
	double							m_aspectRatio;
	/*! Size of the exported image in mm. */
	QSizeF							m_sizeMM;
	/*! Apply data reduction algorithm. */
	bool							m_useDataReduction;

	/*! Discard flags. */
	QwtPlotRenderer::DiscardFlags	m_discardFlags;

protected:
	virtual void paintEvent(QPaintEvent *event);

};

} // namespace SCI

#endif // Sci_ChartRenderWidgetH
