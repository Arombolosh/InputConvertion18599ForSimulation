/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_LegendItemH
#define Sci_LegendItemH

#include <qwt_plot_legenditem.h>

namespace SCI {

/*! \brief Plot item for internal/embedded legend widgets in a SCI::Chart.
	It is derived from QwtPlotLegendItem.
	Key purpose of this class is the common handling of legend item properties.
*/
class LegendItem : public QwtPlotLegendItem {
public:
	LegendItem();
};

} // namespace SCI

#endif // Sci_LegendItemH
