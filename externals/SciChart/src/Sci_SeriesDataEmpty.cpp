/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_SeriesDataEmpty.h"

namespace SCI {

SeriesDataEmpty::SeriesDataEmpty()
{}

size_t SeriesDataEmpty::size() const {
	return 1;
}

QPointF SeriesDataEmpty::sample( size_t  ) const {
	return QPointF(0, 0);
}

QRectF SeriesDataEmpty::boundingRect() const {
	return QRectF(0,0,0,0);
}

} // namespace SCI
