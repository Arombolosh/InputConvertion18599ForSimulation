/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PlotZoomerH
#define Sci_PlotZoomerH

#include <qwt_plot_zoomer.h>
#include "Sci_PlotPickerText.h"

namespace SCI {

/*! Provides a zoomer for the Qwt widgets.

	The picker text (shown during zoom) is composed by the helper class PlotPickerText.
*/
class PlotZoomer: public QwtPlotZoomer, public PlotPickerText {
	Q_OBJECT
public:
	/*! Constructor. */
	PlotZoomer(int xAxis, int yAxis, QWidget * canvas, const ChartAxis * bottomAxis);

	/*! When called the picker transforms to a cross-picker and on click emits the selected signal. */
	void enablePickMarkerPositionMode();

protected:
	/*! Overloaded function to compose tracker text (makes use of cached variables). */
	virtual QwtText trackerText (const QPoint &) const override;

	/*! Overloaded to disable tracker text while dragging the zoom rectangle. */
	virtual void begin() override;
	/*! Overloaded to re-enable tracker text after zooming. */
	virtual bool end(bool ok = true) override;

	/*! Overloaded to set new zoom rectangle in the chart model. */
	void rescale() override;

	/*! Overloaded to set a smaller minimum zoom size. */
	virtual QSizeF minZoomSize() const override;

	/*! Overloaded to enforce a minimum plot rectangle. */
	bool accept(QPolygon & pa) const override;

private:
	bool		m_pickMarkerPositionMode = false;
};

} //namespace SCI


#endif // Sci_PlotZoomerH
