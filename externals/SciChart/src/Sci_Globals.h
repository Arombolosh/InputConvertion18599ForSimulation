/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_GlobalsH
#define Sci_GlobalsH

#include <QPair>

namespace SCI {

/*! Struct that contains the SeriesType enum in order to make it type-safe.*/
struct ChartSeriesInfos {
	/*! Type of the series (shape).*/
	enum  SeriesType { UnknownSeries,		///< Unknown series type (error flag).
					   LineSeries,			///< Line series (can have markers).
					   SurfaceSeries,		///< Surface or wire frame series (not implemented yet).
					   ContourSeries,		///< Contour or isoline series (not implemented yet).
					   ColorGridSeries,		///< Color grid or image series.
					   BarSeries,			///< Bar series (columns).
					   VectorFieldSeries	///< Vector field series
					 };
};


/*! A simple typedef to make things more readable.
	Range is meant to be used for intervals, and is mainly needed to avoid
	exposing internals of the QwtPlot to users of SciChart.
*/
typedef QPair<double,double> Range;

/*! \file Sci_Globals.h
	\brief Contains the declaration of function used in the whole SciChart lib.
*/

} // namespace SCI


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


#endif // Sci_GlobalsH
