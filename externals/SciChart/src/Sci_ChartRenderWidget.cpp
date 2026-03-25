#include "Sci_ChartRenderWidget.h"

#include <QPainter>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>

#include "Sci_Chart.h"

namespace SCI {

ChartRenderWidget::ChartRenderWidget(QWidget * parent) :
	QWidget(parent),
	m_chart(NULL),
	m_aspectRatio(1)
{

}


void ChartRenderWidget::paintEvent(QPaintEvent *event) {
	QWidget::paintEvent(event); // draw widget background

	if (m_chart == NULL)
		return;

	// compute drawing rectangle while maintaining aspect ratio
	int w = width();
	int h = height();
	int hAspect = w/m_aspectRatio;
	if (hAspect < h) {
		h = hAspect;
	}
	else {
		w = h*m_aspectRatio;
	}

	// create plot renderer
	QwtPlotRenderer renderer; // the renderer
	renderer.setDiscardFlags(m_discardFlags);
	// TODO : make this option style-dependent
	renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);

	QRect frameRect( 0, 0, w, h); // target size in pixels
	QRect imageRect( 1, 1, w-2, h-2); // target size in pixels

	QPainter painter(this);
	painter.setPen(QPen());
	painter.drawRect(frameRect);
	painter.fillRect(imageRect, Qt::white);

	m_chart->prepareForVectorExport(!(m_discardFlags & QwtPlotRenderer::DiscardLegend), m_useDataReduction);
	// imageRect has now the size of the chart in the current screen resolution (96 dpi for full HD on 24'' screen)

	QScreen *screen = QGuiApplication::primaryScreen();
	int dpi = screen->logicalDotsPerInch(); // should be taken from current screen
//	qDebug() << "Using screen #" << screenNr << " with DPI " << dpi;
	const double mmToInch = 1.0 / 25.4;
	const QSizeF size = m_sizeMM * mmToInch * dpi;

	// compose chart coordinates
	const QRect chartRect( 0.0, 0.0, size.width(), size.height() );
	// set window-view-transformations
	painter.setWindow(chartRect);
	painter.setViewport(imageRect);
	painter.setRenderHint(QPainter::Antialiasing, true);
	renderer.render( m_chart, &painter, chartRect );
	painter.end();

#if 0
	// draw to temporary image and then resample down to achieve good antialiasing
	const QRect chartRect( 0.0, 0.0, size.width()*2, size.height()*2 );
	QImage img(imageRect.width()*2, imageRect.height()*2, QImage::Format_ARGB32 );
	img.fill( Qt::white ); // fill with uniform background color, usually white
	QPainter p( &img );
	p.setWindow(chartRect);
	QRect imageRect2Times = QRect(0,0,imageRect.width()*2, imageRect.height()*2);
	p.setViewport(imageRect2Times);
	renderer.render( m_chart, &p, imageRect2Times );

	QPixmap pix = QPixmap::fromImage(img).scaled(QSize(imageRect.width(), imageRect.height()), Qt::KeepAspectRatio, Qt::SmoothTransformation);

	painter.drawPixmap(0,0,imageRect.width(), imageRect.height(), pix);
#endif

	m_chart->resetAfterVectorExport();
}

} // namespace SCI
