/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ColorGrid3DDataH
#define Sci_ColorGrid3DDataH

#include <qwt_raster_data.h>
#include <qwt_interval.h>

#include <DATAIO_GeoFile.h>

namespace SCI {

/*! \brief The class SCI::ColorGrid3DData contains the data of a ColorGridSeries.

	Holds data for Finite-Volume cell-centered data or for time-coordinate-value plots the time values are
	located at node position, while the values are centered in space.

	The location type is detected by comparing xGridSize and xValueSize. If both sizes are the same, it is a
	time-coordinate-value plot, if xGridSize = xValueSize + 1, it is a standard cell-centered plot.

	\note The SCI::PlotSpectrogramm only recognizes these two data location types and cannot handle generic grids,
		i.e. where grid and value coordinates are somewhat arbitrary. The linear interpolation algorithm depends
		on these locations.
*/
class ColorGrid3DData : public QwtRasterData {
public:
	/*! Constructor.
		Parameter documentation, see member variables.
	*/
	ColorGrid3DData(unsigned int xGridSize, const double* xGrid, unsigned int yGridSize, const double* yGrid,
					unsigned int xValueSize, unsigned int yValueSize, const double* data);

	/*! Dummy implementation, does not compute anything meaningful. When used together with SCI::PlotSpectrogram,
		this function is not used.
	*/
	virtual double value(double, double) const override { return 0.0; }

	/*! For the right coordinate axis (zAxis) it returns the min/max z-value range, for other axes it returns
		an empty interval.
	*/
	QwtInterval interval( Qt::Axis ) const override;

	/*! Sets the z-value range (call from ColorGridSeries::setZValueRange) */
	void setInterval(Qt::Axis axis, const QwtInterval & interval);

	/*! Number of grid lines in x direction. */
	unsigned int		m_xGridSize;
	/*! Coordinates of grid lines in x direction. */
	const double*		m_xGrid;
	/*! Number of grid lines in y direction. */
	unsigned int		m_yGridSize;
	/*! Coordinates of grid lines in y direction. */
	const double*		m_yGrid;
	/*! Number of values in x direction. */
	unsigned int		m_xValueSize;
	/*! Number of values in y direction. */
	unsigned int		m_yValueSize;
	/*! Linear memory array with values in row-major order (quiet_NAN signal gap/missing data).
		Row-index starts at bottom.
	*/
	const double*		m_data;

	/*! Stores the minimum and maximum z-values, set from setInterval(). */
	QwtInterval			m_zValueRange;
};

} // namespace SCI

#endif // Sci_ColorGrid3DDataH
