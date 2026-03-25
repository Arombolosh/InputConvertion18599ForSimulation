/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_DateScaleDrawH
#define Sci_DateScaleDrawH


#include <qwt_date_scale_draw.h>

namespace SCI {

/*! \brief Wrapper class for QwtDateScaleDraw.
*/
class DateScaleDraw : public QwtDateScaleDraw {
public:

	/*! Standard constructor.*/
	DateScaleDraw();

	/*!
	  \brief Convert a value into its representing label

	  The value is converted to a datetime value using toDateTime()
	  and converted to a plain text using QwtDate::toString().

	  \param value Value
	  \param formatBack Number of levels lower than normal
	  \return Label string.
	*/
	QwtText dateLabel( double value, int formatBack) const;

	/*! Set all date formats from the given stringlist.
		\param formats Formats as strings. Must have 8 items. Index represents QwtDate::IntervalType.
	*/
	void setDateFormats(const QStringList& formats);

	/*! Provides a list with default format strings for DateTime scaling.*/
	static QStringList defaultFormats();
};

} // namespace SCI

#endif // Sci_DateScaleDrawH
