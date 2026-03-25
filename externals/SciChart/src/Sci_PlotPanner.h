/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PlotPannerH
#define Sci_PlotPannerH

#include <qwt_plot_panner.h>

namespace SCI {

/*! Overloaded QwtPlotPanner to update model axis limits after panning. */
class PlotPanner: public QwtPlotPanner {
	Q_OBJECT
public:
	/*! Constructor. */
	PlotPanner(QWidget * canvas);

protected slots:
	/*! Re-implemented from QwtPlotPanner to inform SciChart cartisian chart model about now manual axis limits. */
	void moveCanvas(int dx, int dy) override;
};

} //namespace SCI


#endif // Sci_PlotZoomerH
