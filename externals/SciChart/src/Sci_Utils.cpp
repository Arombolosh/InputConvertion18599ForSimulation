/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_Utils.h"

#include <QLineEdit>
#include <QLocale>
#include <QMessageBox>
#include <QToolButton>
#include <QString>
#include <QIcon>
#include <QDebug>

#include <qwt_color_map.h>

#include <IBK_Exception.h>

#include <QtExt_Locale.h>

namespace SCI {

/*! Helper function for converting a string into a pair of doubles.
*/
bool readValuePair(const QString& str, double& x, double& y) {
	int lpos = str.indexOf(" ");
	bool res;
	x = str.left(lpos).toDouble(&res);
	if( !res )
		return false;
	y = str.right(str.length() - lpos).toDouble(&res);
	return res;
}

bool checkPositiveInput(QLineEdit * edit, double & val, bool allowZero) {
	bool ok;
	QString valStr = edit->text();
	val = QtExt::Locale().toDouble(valStr, &ok);
	if (!ok || (allowZero && val<0) || (!allowZero && val<=0)) {
		QMessageBox::critical(edit,
			QT_TRANSLATE_NOOP("Utilities", "Invalid input"),
			QT_TRANSLATE_NOOP("Utilities", "This is not a valid input."));
		edit->setFocus();
		edit->selectAll();
		return false;
	}
	return true;
}

void setupToolButton(QToolButton * btn, const QString & iconFile, const QString & hint) {
	btn->setIcon(QIcon(iconFile));
	btn->setIconSize(QSize(32,32));
	btn->setAutoRaise(true);
	btn->setToolTip(hint);
}

const unsigned int dayAtMonthStart[12] = {
	31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };

unsigned int month(unsigned int  day) {
	unsigned int k=0;
	while (k < 12 && day > dayAtMonthStart[k]) ++k;
	return k;
}


} // namespace SCI

