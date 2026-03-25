/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PlotSpectrogramH
#define Sci_PlotSpectrogramH

#include <qwt_plot_spectrogram.h>

#include <QPoint>
#include <QPen>
#include <QFont>

#include <IBK_matrix.h>

#include <DATAIO_ConstructionLines2D.h>
#include <DATAIO_GeoFile.h>

namespace SCI {

class ColorGrid3DData;

/*! Wrapper class for QwtPlotSpectogram to provide optimized image rendering.

	The PlotSpectrogram support currently input and geometry data in two formats:

	- Finite-Volume cell-centered data, where values are stored as representative/mean values per element.
	- The x-coordinates for the elements are aligned to the grid coordinates (time-coordinate-value plots)

	See \ref ColorGrid3DData for details.

	Nodal value calculation algorithm
	---------------------------------

	For the provided rectilinear grid, first all nodal values are calculated. For cell-centered data, first the
	values x-gridlines are computed. Now the data represents the same as for the time-coordinate-value plots.
	Afterwards, the values at the y-gridlines are calculated, giving nodal values.

	This is done using the following algorithm:

	- process all elements, check if there is a next neighbor - if that is the case, interpolate the value at the grid
	- for elements without neighbor in one direction, extrapolate (constant/linear?) the value at the node

	Value interpolation algorithm
	-----------------------------

	When rendering the image, it is done using element-by-element, while drawing scanline by scanline. The result
	depends on the interpolation type and the color map.

	When interpolation is enabled:

	For each scanline first the values at the left and right edges of the element are computed by interpolating the
	nodal values. Then, for each pixel in the scanline the values is interpolated between left and right edge value.


	Without interpolation:

	For cell-centered data, the value at the center is used for the entire element.
	For time-coordinate-value data, the cell is painted using the left grid-value for the left part of the element,
	and the right grid-value for the right part of the element.
*/
class PlotSpectrogram : public QwtPlotSpectrogram {
public:

	enum LabelPlacementFlags {
		NO_LABELS   = 0,
		N_PER_LEVEL = (1 << 0),
		AT_MIN_X    = (1 << 1),
		AT_MAX_X    = (1 << 2),
		AT_MIN_Y    = (1 << 3),
		AT_MAX_Y    = (1 << 4)
	};

	/*! Standard constructor.
		\param title Title of the series (noramlly not used):
	*/
	PlotSpectrogram(const QString & title = QString());

	/*! Set the geometry data and result values needed for drawing.
		\param data Data structure with all values to be drawn.
		\param interpolate If true the data will be interpolated between coordinates.
		\warning May throw an exception. In this case, the spectrogram is invalid and must not be used.
	*/
	void setData(ColorGrid3DData* data, bool interpolate);

	/*! Provides read/write access to internal ColorGrid3DData that holds the actual data of the chart. */
	ColorGrid3DData* data();

	/*! Provides read access to internal ColorGrid3DData that holds the actual data of the chart. */
	const ColorGrid3DData* data() const;

	/*! Should the values within elements be interpolated or not (false sets 'raw' mode). */
	void setInterpolation(bool val) { m_interpolate = val; }

	/*! Returns the current interpolation setting.*/
	bool interpolation() const { return m_interpolate; }

	/*! Returns the interpolated value for the given coordinates.
		\param x Absolute x-coordinate (physical coordinates) of the point where the value should be calculated.
		\param y Absolute y-coordinate (physical coordinates) of the point where the value should be calculated.
		\return Returns the interpolated/constant value in the element, or std::numeric_limits<double>::quiet_NaN()
			if point is outside geometry.

		This function is rather slow, so it is not used for the generation of the spectrogram plot, but only
		for the picker text operation.
	*/
	double value(double x, double y) const;

	/*! Returns the current bounding rect.
		It will be calculated from the cuurent grid.
	*/
	virtual QRectF boundingRect() const;

	/*! Returns the pen for boundary lines.*/
	QPen boundaryPen() const { return m_constructionLinePen; }

	/*! Sets the construction/boundary line pen.*/
	void setConstructionLinePen(const QPen& pen);

	/*! Set construction lines.*/
	void setConstructionLines(const DATAIO::ConstructionLines2D& clines);

	/*! Sets the contour pen.*/
	void setContourPen(const QPen& pen);

	/*! Returns the contour pen with the color matching the given value. */
	virtual QPen contourPen(double value) const;

	/*! Set the font for contour labels.
		The default font is Helvetica, 6 point, bold.
	*/
	void setLabelFont( const QFont &labelFont);

	/*! Set the font background colour.
		The default font background colour is plot()->canvasBackground().
	*/
	void setLabelBackground( const QBrush &labelBackground);

	/*! Set number of labels per contour level.
		The default value is 2 contour labels per level.
	*/
	void setLabelsPerContourLevel( int labelsPerContourLevel);

	/*! Set the margin between the edge of the plot window and the label.
		The default margin is 10 pixels.
	*/
	void setLabelMargin( int labelMargin);

	/*! Sets the label step value.
	  A value of 1 means label every contour. A value of 2 means label
	  every 2nd contour. The default label step is 1.
	*/
	void setLabelStep( int labelStep);

	/*! Set the label placement flags.
		The current placement flags are NO_LABELS, N_PER_LEVEL, AT_MIN_X,
		AT_MIN_Y and AT_MAXY. Any combination of flags may be used. If you
		want to set both AT_MIN_Y and AT_MAX_Y then call this functions as follows:
		\code setLabelPlacement( AT_MIN_Y | AT_MAX_Y); \endcode
		The default flag is AT_MAX_Y.
	*/
	void setLabelPlacement( unsigned int labelPlacement);

	/*! Get the label placement flags. */
	unsigned int labelPlacement() const;

protected:

	/*! Re-implemented to provide optimized version of render function.
		This function draws each element at a time, hereby interpolating the elements' rectangle
		using the nodal data.
	*/
	virtual QImage renderImage(
		const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		const QRectF &area, const QSize &imageSize) const;

	/*! Reimplement to paint label on contour levels. */
	virtual void drawContourLines (QPainter *p, const QwtScaleMap &xMap,
								   const QwtScaleMap &yMap, const QwtRasterData::ContourLines &lines) const;

	/*! Draws boundary lines. */
	void drawBoundaryLines(QPainter * painter, const QRectF & rect,
						   const QwtScaleMap & xxMap, const QwtScaleMap & yyMap) const;


private:
	/*! Returns the interpolated value for the element with number elemNo for the given coordinates.
		\param valueIndex Index of the value/element/node in the row-major numbered value array.
		\param x Absolute x-coordinate (physical coordinates) of the point where the value should be calculated.
		\param y Absolute y-coordinate (physical coordinates) of the point where the value should be calculated.

		\code
		// convert from valueIndex to i,j coordinates
		unsigned int i = e % m_xValueSize;
		unsigned int j = e / m_xValueSize;
		\endcode
	*/
	double value(int valueIndex, double x, double y) const;

	/*! Re-implemented from QwtPlotSpectrogram::renderContourLines() with our own contour line algorithm.
		Just a wrapper function, calls contourLines() internally.
	*/
	virtual QwtRasterData::ContourLines renderContourLines(
		const QRectF &rect, const QSize &raster) const;

	/*! Re-implemented from QwtPlotSpectrogram::contourLines() with our own contour line algorithm. */
	QwtRasterData::ContourLines contourLines(
		const QRectF &rect, const QSize &raster,
		const QList<double> &levels) const;

	/*! Helper function to do triangular interpolation. */
	double triangleInterpolate(const DATAIO::GeoFile::Element & elem1,const DATAIO::GeoFile::Element & elem2,const DATAIO::GeoFile::Element & elem3,
							   double w1, double w2, double w3,
							   double x, double y);

	/*! Size of the x-grid array.*/
	unsigned int									m_xGridSize;
	/*! X-grid array.*/
	const double*									m_xGrid;
	/*! Size of the y-grid array.*/
	unsigned int									m_yGridSize;
	/*! Y-grid array.*/
	const double*									m_yGrid;
	/*! Number of x values in value vector.*/
	unsigned int									m_xValueSize;
	/*! Number of y values in value vector.*/
	unsigned int									m_yValueSize;
	/*! Internal data size. Will be calculated by m_xValueSize * m_yValueSize.*/
	unsigned int									m_dataSize;
	/*! Data array with m_dataSize elements (row-major ordering).
		Access to single value via m_data[jIdx * m_xValueSize + iIdx].
	*/
	const double*									m_data;

	/*! Holds nodal values on x-grid lines in the middle of cells, size m_xGridSize x m_yValueSize.
		This coincides with the values from a time-coordinate-value data set.
		Note: empty node values (i.e. gaps in the construction) are indicated by quiet_NAN values.
	*/
	IBK::matrix< double >							m_xGridNodeValues;
	/*! Holds nodal values (intersections of grid lines), size m_xGridSize x m_yGridSize.
		Note: empty node values (i.e. gaps in the construction) are indicated by quiet_NAN values.
	*/
	IBK::matrix< double >							m_nodeValues;
	/*! Should the colors be interpolated inside the element rects.
		For cell-centered data, disabling interplation will return the nodal value in the entire geometry.
		For time-coordinate-value data (aligned to x-grid), left and right halfs of elements will be
		associated with data from grid.
	*/
	bool											m_interpolate;


	/*! Classe that handles construction and boundary lines.*/
	DATAIO::ConstructionLines2D						m_constructionLines;

	/*! Pen for construction/boundary lines.*/
	QPen											m_constructionLinePen;

	/*! Pen for contour lines.*/
	QPen											m_contourPen;

	QFont											m_labelFont;
	QBrush											m_labelBackground;
	/*! Number of labels per contour line, if N_PER_LEVEL flag is set. */
	int												m_labelsPerContourLevel;
	int												m_labelMargin;
	int												m_labelStep;
	unsigned int									m_labelPlacement;

	class Contour3DPoint;
	class ContourPlane;

};


} // namespace SCI

#endif // Sci_PlotSpectrogramH
