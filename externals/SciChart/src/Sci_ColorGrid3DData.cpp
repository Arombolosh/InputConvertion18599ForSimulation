/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_ColorGrid3DData.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace SCI {

ColorGrid3DData::ColorGrid3DData(unsigned int xGridSize, const double* xGrid, unsigned int yGridSize, const double* yGrid,
								 unsigned int xValueSize, unsigned int yValueSize, const double* data) :
	m_xGridSize(xGridSize),
	m_xGrid(xGrid),
	m_yGridSize(yGridSize),
	m_yGrid(yGrid),
	m_xValueSize(xValueSize),
	m_yValueSize(yValueSize),
	m_data(data)
{
	Q_ASSERT(m_xGridSize>0);
	Q_ASSERT(m_yGridSize>0);
}

QwtInterval ColorGrid3DData::interval( Qt::Axis a) const {
	if (a == Qt::ZAxis)
		return m_zValueRange;
	return QwtInterval();
}

void ColorGrid3DData::setInterval(Qt::Axis axis, const QwtInterval & interval) {
	Q_ASSERT(axis == Qt::ZAxis);
	m_zValueRange = interval;
}

} // namespace SCI
