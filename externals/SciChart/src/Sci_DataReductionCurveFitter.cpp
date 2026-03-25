#include "Sci_DataReductionCurveFitter.h"

#include <QPolygonF>

#include <IBK_messages.h>
#include <IBK_FormatString.h>

namespace SCI {

QPolygonF DataReductionCurveFitter::fitCurve( const QPolygonF & points) const {
	const char * const FUNC_ID = "[DataReductionCurveFitter::fitCurve]";
	QPolygonF fittedPoints;

	// sometimes we have several days with minutely values that are the same or nearly the same
	const_cast<DataReductionCurveFitter*>(this)->setChunkSize(100);
	// apply algorithms until:
	// - tolerance reaches 0.1
	// - number of points is still larger than 500
	double tol = 1e-5;
	do {
		tol *= 10;
		const_cast<DataReductionCurveFitter*>(this)->setTolerance(tol);
		fittedPoints = QwtWeedingCurveFitter::fitCurve(points);
	} while (fittedPoints.size() > 500 && tol < 0.1);

	IBK::IBK_Message(IBK::FormatString("Data reduction (tol=%3): %1   -> %2\n").arg((unsigned int)points.size()).arg((unsigned int)fittedPoints.size()).arg(tol),
					 IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_INFO);
	return fittedPoints;
}

} // namespace SCI
