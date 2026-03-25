/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PlotPickerH
#define Sci_PlotPickerH

#include <qwt_plot_picker.h>

#include "Sci_PlotPickerText.h"

namespace SCI {

/*! A picker (a cross-hair cursor) that can be moved over a
	line series plot to show the x,y1 and y2 (if axis is enabled) values.

	The picker text is composed by the helper class PlotPickerText.

	For line series, the picker can be set into mode "show legend for nearest line points".
	Then, while moving around on the canvas, an additional internal legend is shown (away from the cursor)
	with a list of the 5-10 closest lines near the cursor and the respective y-values. X-value is shown
	in caption of the legend.
*/
class PlotPicker : public QwtPlotPicker, public PlotPickerText {
	Q_OBJECT
public:
	/*! Constructor. */
	PlotPicker (int xAxis, int yAxis, QWidget * canvas, const ChartAxis * bottomAxis);

protected:
	/*! Overloaded function to compose tracker text (makes use of cached variables). */
	virtual QwtText trackerText (const QPoint &) const override;

	/*! Overloaded to disable tracker text while dragging the zoom rectangle. */
	virtual void begin() override;
	/*! Overloaded to re-enable tracker text after zooming. */
	virtual bool end(bool ok = true) override;
};

} // namespace SCI

#endif // Sci_PlotPickerH
