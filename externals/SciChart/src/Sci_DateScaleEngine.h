/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_DateScaleEngineH
#define Sci_DateScaleEngineH


#include <qwt_date_scale_engine.h>

namespace SCI {

/*! \brief Wrapper class for QwtDateScaleEngine.
*/
class DateScaleEngine : public QwtDateScaleEngine {
public:

	/*! Standard constructor.*/
	DateScaleEngine();
};

} // namespace SCI

#endif // Sci_DateScaleEngineH
