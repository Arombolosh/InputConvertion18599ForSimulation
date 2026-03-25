/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_SeriesDataEmptyH
#define Sci_SeriesDataEmptyH

#include <qwt_series_data.h>

#include <QPointF>

namespace SCI {

class SeriesDataEmpty : public QwtSeriesData<QPointF> {
public:
	SeriesDataEmpty();

	/*! \return Number of samples.*/
	virtual size_t size() const;

	/*!
	  Return a sample
	  \param i Index
	  \return Sample at position i
	 */
	virtual QPointF sample( size_t i ) const;

	/*!
	   Calculate the bounding rect of all samples

	   The bounding rect is necessary for autoscaling and can be used
	   for a couple of painting optimizations.

	   qwtBoundingRect(...) offers slow implementations iterating
	   over the samples. For large sets it is recommended to implement
	   something faster f.e. by caching the bounding rectangle.

	   \return Bounding rectangle
	 */
	virtual QRectF boundingRect() const;

};

} // namespace SCI

#endif // Sci_SeriesDataEmptyH
