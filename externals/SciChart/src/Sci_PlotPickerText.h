/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PlotPickerTextH
#define Sci_PlotPickerTextH

#include <QString>

#include <qwt_plot_picker.h>

namespace SCI {

class ChartAxis;

/*! This is a helper class derived from SCI::PlotPicker and SCI::PlotZoomer to
	handle the composition of the tracker text.

	The picker text is generated using cached values, namely the unit
	strings and the bottom axis, the latter being needed only for DateTime axis.

	The pointer to the bottom axis is persistant during the lifetime of the plot
	widget, since we only have one x-axis and never replace it with something
	different.

*/
class PlotPickerText {
public:
	/*! Constructor. */
	PlotPickerText (const ChartAxis * bottomAxis);

	/*! Cached x unit, used to compose the tracker text. */
	QString	m_xUnit;
	/*! Cached y1 unit, used to compose the tracker text. */
	QString	m_y1Unit;
	/*! Cached y2 unit, used to compose the tracker text (if empty, y2 value
		will not be requested and shown).
	*/
	QString	m_y2Unit;

	/*! z-unit or value unit, needed for color map plots where value under cursor is displayed. */
	QString m_valueUnit;


	/*! Function to compose tracker text (makes use of cached variables). */
	QwtText composeTrackerText (const QwtPlot * plot, QwtPicker::RubberBand rubberBand,
								const QPoint & canvasPos) const;

	/*! Can be used to turn tracker text composition on/off without enabling/disabling the tracker, which
		would cause the installEventFilter() function to be alled.
		Currently, this function is only useful for LegendItemMover.
	*/
	bool m_trackerTextVisible;

private:
	/*! Cached pointer to the bottom axis.
		Needed to check for date time axis and to request a DateTime label
		string for the current x coordinate.

		\note Bottom axis pointer is valid as long as picker lives.
	*/
	const ChartAxis * m_bottomAxis;
};

} // namespace SCI

#endif // Sci_PlotPickerTextH
