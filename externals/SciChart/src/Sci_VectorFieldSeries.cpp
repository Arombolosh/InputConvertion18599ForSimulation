/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QXmlStreamWriter>
#include <QXmlStreamReader>

#include <IBK_assert.h>

#include <qwt_color_map.h>

#include "Sci_VectorFieldSeries.h"
#include "Sci_Chart.h"

namespace SCI {


VectorFieldSeries::VectorFieldSeries() :
		ChartSeries(new PlotVectorField)
{
}


VectorFieldSeries::VectorFieldSeries(Chart* chart, const QString& title) :
		ChartSeries(new PlotVectorField(title), chart)
{
}


VectorFieldSeries::~VectorFieldSeries() {
}


void VectorFieldSeries::setData(unsigned int vectorSampleSize, const double* x, const double* y,
			 const double* vx, const double* vy)
{
	QVector< QwtVectorFieldSample > samples;

	samples.reserve(vectorSampleSize);
	for (unsigned int i=0; i<vectorSampleSize; ++i) {
		QwtVectorFieldSample s(x[i], y[i], vx[i], vy[i]);
		samples.append(s);
	}
	// create a vector with data samples
	vectorField()->setSamples( samples );
}


void VectorFieldSeries::setZValueRange(QwtInterval zValueRange) {
	vectorField()->setMagnitudeRange(zValueRange);
}


} // namespace SCI
