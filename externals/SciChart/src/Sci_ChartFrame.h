/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ChartFrameH
#define Sci_ChartFrameH

#include <QObject>

#include "Sci_Chart.h"

namespace SCI {

/*! \todo Fixme, currently not used */
class ChartFrame : public QObject
{
	Q_OBJECT
public:
	explicit ChartFrame(QObject *parent = 0);

	~ChartFrame();

	void setChartModel(AbstractChartModel* model);

	/*! Returns the chart height for the given width.
		\param Current painter.
		\param width Preferred width of the chart. If 0 width of paintDevice will be used.
	*/
	qreal frameHeight(QPaintDevice* paintDevice, qreal width = 0);

	/*! Draw the whole frame with the given painter.
		\param painter Painter used for drawing.
		\param pos Left/Top corner.
	*/
	void drawFrame(QPainter* painter, const QPointF& pos, qreal width);

signals:

public slots:

private:
	Chart*	m_chart;
};

} // namespace SCI

#endif // Sci_ChartFrameH
