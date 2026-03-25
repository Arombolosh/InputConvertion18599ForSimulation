/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_DateScaleDraw.h"

#include <qwt_text.h>

namespace SCI {

DateScaleDraw::DateScaleDraw()
{
	QStringList defaultFormats = DateScaleDraw::defaultFormats();
	setDateFormat( QwtDate::Millisecond, defaultFormats[0]);
	setDateFormat( QwtDate::Second, defaultFormats[1]);
	setDateFormat( QwtDate::Minute, defaultFormats[2]);
	setDateFormat( QwtDate::Hour, defaultFormats[3]);
	setDateFormat( QwtDate::Day, defaultFormats[4]);
	setDateFormat( QwtDate::Week, defaultFormats[5]);
	setDateFormat( QwtDate::Month, defaultFormats[6]);
	setDateFormat( QwtDate::Year, defaultFormats[7]);

	setTimeSpec(Qt::UTC);
}

QwtText DateScaleDraw::dateLabel( double value, int formatBack) const {
	const QDateTime dt = toDateTime( value );
	int intervaltype = intervalType( scaleDiv() );
	if(formatBack > 0) {
		if((intervaltype - formatBack) >= QwtDate::Millisecond)
			intervaltype -= formatBack;
		else
			intervaltype = QwtDate::Millisecond;
	}
	const QString fmt = dateFormatOfDate( dt, static_cast<QwtDate::IntervalType>(intervaltype) );

	return QwtDate::toString( dt, fmt,  week0Type());
}

QStringList DateScaleDraw::defaultFormats() {
	QStringList res;
	res << "hh:mm:ss:zzz"; // milli seconds
	res << "dd hh:mm:ss"; // seconds
	res << "dd.MM. hh:mm"; // minutes
	res << "hh:mm;;dd.MM."; // hours
	res << "dd.MM.yyyy"; // days
	res << "dd.MM.yyyy"; // week
	res << "MMM yyyy"; // month
	res << "yyyy"; // year
	return res;
}

void DateScaleDraw::setDateFormats(const QStringList& formats) {
	Q_ASSERT(formats.size() == 8);
	for( int i=0; i<8; ++i) {
		// decode time format
		QString format = formats[i];
		format = format.replace(";;","\n");
		setDateFormat( static_cast<QwtDate::IntervalType>(i), format);
	}
	setScaleDiv(scaleDiv());
}

} // namespace SCI
