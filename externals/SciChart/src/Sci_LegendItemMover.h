/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_LegendItemMoverH
#define Sci_LegendItemMoverH

#include <QObject>
#include <QRegion>
#include <QPointer>
#include <qwt_widget_overlay.h>

class QwtPlot;
class QwtPlotLegendItem;
class QPainter;
class QPoint;

namespace SCI {

class PlotZoomer;

/*! \brief This class handles the drag-drop movement of legend items within
	the plot.

	Within the event filter, the check is done whether the user had clicked
	on the legend item and then the overlay is turned on while dragging. At
	the same time the legend item is hidden. Once the legend move has finished,
	the new legend position is stored in the chart model.

	This class and its helper classes/functions are adapted from the qwt playground
	example LegendEditor.
*/
class LegendItemMover : public QObject {
	Q_OBJECT

public:
	/*! Constructor, registers newly created LegendItemMover in QwtPlot as event filter.
		When using LegendItemMover together with a picker/zoomer, the LegendItemMover should
		be constructed last (and added last to the event filter list), so that it will be
		activated first.
	*/
	LegendItemMover( QwtPlot *, SCI::PlotZoomer * picker = NULL );
	virtual ~LegendItemMover();

	void drawOverlay( QPainter * ) const;
	QRegion maskHint() const;

	virtual bool eventFilter( QObject *, QEvent *);

private:
	const QwtPlot *plot() const;
	QwtPlot *plot();

	/*! Retrieves mouse click position and stores it in d_currentPos, but only
		if click position is on a legend item.
		\return Returns true when click was on legend item, otherwise false.
	*/
	bool pressed( const QPoint & );
	bool moved( const QPoint & );
	void released( const QPoint & );

	QRectF canvasRect() const;
	QwtPlotLegendItem* itemAt( const QPoint& ) const;

	void setItemVisible( bool on );

	/*! Overlay is created once dragging starts. */
	QPointer<QwtWidgetOverlay> d_overlay;

	// Mouse positions
	QPointF					d_currentPos;
	QwtPlotLegendItem		*d_legendItem;

	SCI::PlotZoomer			*d_pickerZoomer;
};

} // namespace SCI

#endif // Sci_LegendItemMoverH
