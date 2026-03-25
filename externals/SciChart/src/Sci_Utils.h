/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_UtilsH
#define Sci_UtilsH

#include <limits>

#include <QString>

class QLineEdit;
class QToolButton;
class QwtColorMap;

namespace SCI {

/*! Tries to interprete a string as a pair of doubles.
	\param str Original string
	\param x 1st double value.
	\param y 2nd double value.
*/
bool readValuePair(const QString& str, double& x, double& y);

/*! Function calculates maximum and minimum value of an array of T.
	Additionally to the standard algorithms it has a correct handling of NaN valus.
	\param array Pointer to const values with type T
	\param size Size of this array.
*/
template< typename T>
std::pair<T, T> maxMin(const T* array, unsigned int size) {
	std::pair<T, T> res;
	res.first = std::numeric_limits<T>::max() * -1;
	res.second = std::numeric_limits<T>::max();
	bool allNaN = true;
	double val;
	for( unsigned int i=0; i<size; ++i) {
		val = array[i];
		// test if the data array contains only NaNs
		if( allNaN && val == val)
			allNaN = false;
		res.first = std::max(res.first, val);
		res.second = std::min(res.second, val);
	}
	if(allNaN)
		return std::make_pair(std::numeric_limits<T>::quiet_NaN(),std::numeric_limits<T>::quiet_NaN());
	return res;
}

/*! Convert the value in the line edit into a number and reports success.

	This function tries to convert the value into a double number and pops up
	a message box if this fails. In this case, it also sets the focus back to
	the line edit, marks all text and returns false.
	If the number was converted correctly, but is less or equal zero, another
	error message dialog pops up and again, the line edit gets focus and text
	becomes marked.
	\param edit		Pointer to line edit to read the value from.
	\param val		The converted value is stored in this variable.
	\param allowZero If false (the default) only numbers > 0 are accepted.
	\return			Returns if the conversion was successful (implies that
					val is larger than zero).
*/
bool checkPositiveInput(QLineEdit * edit, double & val, bool allowZero = false);

/*! Sets the properties of a tool button. */
void setupToolButton(QToolButton * btn, const QString & iconFile, const QString & hint);

/// \todo move to IBK lib
unsigned int month(unsigned int  day);

/// \todo move to IBK lib
extern const unsigned int dayAtMonthStart[12];

} // namespace SCI

/*! \file Sci_Utils.h
	\brief Contains utility functions.
*/

#endif // Sci_UtilsH
