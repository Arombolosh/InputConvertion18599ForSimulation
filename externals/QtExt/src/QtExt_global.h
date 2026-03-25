/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#ifndef QtExt_globalH
#define QtExt_globalH

#include <QtGlobal>


// Windows DLL handling
// - to use QtExt as DLL set QtExt_DLL define
// - when building the library, also define QtExt_MAKEDLL
#ifdef Q_OS_WIN

#ifdef QtExt_DLL

#if defined(QtExt_MAKEDLL)     // create a QtExt DLL library
#define QtExt_EXPORT Q_DECL_EXPORT
#else                          // use a QtExt DLL library
#define QtExt_EXPORT Q_DECL_IMPORT
#endif

#endif // QtExt_DLL

#endif


// For cases without DLL support and on Mac/Linux define macro to nothing
#ifndef QtExt_EXPORT
#define QtExt_EXPORT
#endif


#ifndef ASCONST_DEFINED
#define ASCONST_DEFINED
/*! Portable replacement for qAsConst() (deprecated Qt 6.6) and std::as_const() (C++17).
	Works with any C++11 compiler and any Qt version. */
template <typename T>
constexpr const T & asConst(T & t) noexcept { return t; }
/*! Deleted overload prevents binding to rvalues (would produce a dangling reference). */
template <typename T>
void asConst(const T &&) = delete;
#endif // ASCONST_DEFINED


#endif // QtExt_globalH
