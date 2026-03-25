#ifndef SCI_DATAREDUCTIONCURVEFITTER_H
#define SCI_DATAREDUCTIONCURVEFITTER_H

#include <qwt_weeding_curve_fitter.h>

namespace SCI {

/*! Re-implementation of the weeding curve fitter. */
class DataReductionCurveFitter : public QwtWeedingCurveFitter {
public:
	/*! Overloaded with adapting algorithm.
		Rules are:

		- if 0 points, return empty curve
		- if < 500 points, return all points
		- start with tolerance 1, reduce iteratively until 0.001 is reached
		  or > 200 points are remaining
	*/
	virtual QPolygonF fitCurve( const QPolygonF & ) const;
};

} // namespace SCI

#endif // SCI_DATAREDUCTIONCURVEFITTER_H
