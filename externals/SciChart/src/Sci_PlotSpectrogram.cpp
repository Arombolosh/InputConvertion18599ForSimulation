/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_PlotSpectrogram.h"

#include <limits>
#include <algorithm>
#include <cmath>

#include <QMap>
#include <QPainter>

#include <IBK_Constants.h>
#include <IBK_messages.h>

#include "qwt_painter.h"
#include "qwt_scale_map.h"
#include "qwt_color_map.h"
#include "qwt_plot_spectrogram.h"
#include "qwt_raster_data.h"
#include "qwt_scale_div.h"
#include "qwt_plot.h"

#include "Sci_ColorGridSeries.h"
#include "Sci_ColorGrid3DData.h"

const double d180_pi = 57.295779513082320876798154814105;

enum LabelPlacementIndex {
  AT_MIN_X_IDX = 0,
  AT_MAX_X_IDX = 1,
  AT_MIN_Y_IDX = 2,
  AT_MAX_Y_IDX = 3,
  PLACEMENT_SIZE = 4
};

bool operator<(const QPoint& lhs, const QPoint& rhs) {
	if( lhs.x() > rhs.x())
		return false;
	if( lhs.y() > rhs.y())
		return false;
	return true;
}

namespace SCI {

class PlotSpectrogram::Contour3DPoint
{
public:
	inline void setPos(double x, double y)
	{
		d_x = x;
		d_y = y;
	}

	inline QPointF pos() const
	{
		return QPointF(d_x, d_y);
	}

	inline void setX(double x) { d_x = x; }
	inline void setY(double y) { d_y = y; }
	inline void setZ(double z) { d_z = z; }

	inline double x() const { return d_x; }
	inline double y() const { return d_y; }
	inline double z() const { return d_z; }

private:
	double d_x;
	double d_y;
	double d_z;
};

class PlotSpectrogram::ContourPlane
{
public:
	inline ContourPlane(double z):
		d_z(z)
	{
	}

	inline bool intersect(const Contour3DPoint vertex[3],
		QPointF line[2], bool ignoreOnPlane) const;

	inline double z() const { return d_z; }

private:
	inline int compare(double z) const;
	inline QPointF intersection(
		const Contour3DPoint& p1, const Contour3DPoint &p2) const;

	double d_z;
};

inline bool PlotSpectrogram::ContourPlane::intersect(
	const Contour3DPoint vertex[3], QPointF line[2],
	bool ignoreOnPlane) const
{
	bool found = true;

	// Are the vertices below (-1), on (0) or above (1) the plan ?
	const int eq1 = compare(vertex[0].z());
	const int eq2 = compare(vertex[1].z());
	const int eq3 = compare(vertex[2].z());

	/*
		(a) All the vertices lie below the contour level.
		(b) Two vertices lie below and one on the contour level.
		(c) Two vertices lie below and one above the contour level.
		(d) One vertex lies below and two on the contour level.
		(e) One vertex lies below, one on and one above the contour level.
		(f) One vertex lies below and two above the contour level.
		(g) Three vertices lie on the contour level.
		(h) Two vertices lie on and one above the contour level.
		(i) One vertex lies on and two above the contour level.
		(j) All the vertices lie above the contour level.
	 */

	static const int tab[3][3][3] =
	{
		// jump table to avoid nested case statements
		{ { 0, 0, 8 }, { 0, 2, 5 }, { 7, 6, 9 } },
		{ { 0, 3, 4 }, { 1, 10, 1 }, { 4, 3, 0 } },
		{ { 9, 6, 7 }, { 5, 2, 0 }, { 8, 0, 0 } }
	};

	const int edgeType = tab[eq1+1][eq2+1][eq3+1];
	switch (edgeType)
	{
		case 1:
			// d(0,0,-1), h(0,0,1)
			line[0] = vertex[0].pos();
			line[1] = vertex[1].pos();
			break;
		case 2:
			// d(-1,0,0), h(1,0,0)
			line[0] = vertex[1].pos();
			line[1] = vertex[2].pos();
			break;
		case 3:
			// d(0,-1,0), h(0,1,0)
			line[0] = vertex[2].pos();
			line[1] = vertex[0].pos();
			break;
		case 4:
			// e(0,-1,1), e(0,1,-1)
			line[0] = vertex[0].pos();
			line[1] = intersection(vertex[1], vertex[2]);
			break;
		case 5:
			// e(-1,0,1), e(1,0,-1)
			line[0] = vertex[1].pos();
			line[1] = intersection(vertex[2], vertex[0]);
			break;
		case 6:
			// e(-1,1,0), e(1,0,-1)
			line[0] = vertex[1].pos();
			line[1] = intersection(vertex[0], vertex[1]);
			break;
		case 7:
			// c(-1,1,-1), f(1,1,-1)
			line[0] = intersection(vertex[0], vertex[1]);
			line[1] = intersection(vertex[1], vertex[2]);
			break;
		case 8:
			// c(-1,-1,1), f(1,1,-1)
			line[0] = intersection(vertex[1], vertex[2]);
			line[1] = intersection(vertex[2], vertex[0]);
			break;
		case 9:
			// f(-1,1,1), c(1,-1,-1)
			line[0] = intersection(vertex[2], vertex[0]);
			line[1] = intersection(vertex[0], vertex[1]);
			break;
		case 10:
			// g(0,0,0)
			// The CONREC algorithm has no satisfying solution for
			// what to do, when all vertices are on the plane.

			if ( ignoreOnPlane )
				found = false;
			else
			{
				line[0] = vertex[2].pos();
				line[1] = vertex[0].pos();
			}
			break;
		default:
			found = false;
	}

	return found;
}

inline int PlotSpectrogram::ContourPlane::compare(double z) const
{
	if (z > d_z)
		return 1;

	if (z < d_z)
		return -1;

	return 0;
}

inline QPointF PlotSpectrogram::ContourPlane::intersection(
	const Contour3DPoint& p1, const Contour3DPoint &p2) const
{
	const double h1 = p1.z() - d_z;
	const double h2 = p2.z() - d_z;

	const double x = (h2 * p1.x() - h1 * p2.x()) / (h2 - h1);
	const double y = (h2 * p1.y() - h1 * p2.y()) / (h2 - h1);

	return QPointF(x, y);
}


PlotSpectrogram::PlotSpectrogram(const QString & title) :
	QwtPlotSpectrogram(title),
	m_constructionLinePen(Qt::black),
	m_contourPen( QPen() ),
	m_labelFont( "Helvetica", 8, QFont::Bold),
	m_labelBackground( QColor(0,0,0,0) ), // transparent black
	m_labelsPerContourLevel(2),
	m_labelMargin(15),
	m_labelStep(1),
	m_labelPlacement( AT_MAX_Y)
{
}


ColorGrid3DData* PlotSpectrogram::data() {
	return static_cast<ColorGrid3DData*>(QwtPlotSpectrogram::data());
}


const ColorGrid3DData* PlotSpectrogram::data() const {
	return static_cast<const ColorGrid3DData*>(QwtPlotSpectrogram::data());
}


double PlotSpectrogram::value(double x, double y) const {
	// lookup i index
	unsigned int i=0;
	while (i < m_xGridSize && x > m_xGrid[i])
		++i;
	if(i == m_xGridSize)
		--i;
	unsigned int j=0;
	while (j < m_yGridSize && y > m_yGrid[j]) ++j;
	if(j == m_yGridSize)
		--j;

	// i and j are now indexes of grid lines top/right of the element where the cursor is positioned
	if (i > 0)
		--i;
	if (j > 0)
		--j;

	unsigned int Enum = j*m_xValueSize + i;
	return value(Enum, x, y);
}


void PlotSpectrogram::setData(ColorGrid3DData* data, bool interpolate) {
//	const char * const FUNC_ID = "[SciSpectrogramChart::setData]";

	// qDebug() << "QwtPlotSpectrogram::setData()";
	QwtPlotSpectrogram::setData(data);
	m_interpolate = interpolate;

	m_xGridSize = data->m_xGridSize;
	m_xGrid = data->m_xGrid;
	m_yGridSize = data->m_yGridSize;
	m_yGrid = data->m_yGrid;
	m_xValueSize = data->m_xValueSize;
	m_yValueSize = data->m_yValueSize;
	m_dataSize = m_xValueSize * m_yValueSize;
	m_data = data->m_data;

	// determine source data location type (i.e. cell centered or not)
	bool cellCentered = (m_xGridSize != m_xValueSize);

	// clear and resize the node value matrix
	const double GAP_VALUE = std::numeric_limits<double>::quiet_NaN(); // should be the same as used by the extractors in EM_Helpers.h
	m_xGridNodeValues.resize(m_xGridSize, m_yValueSize);
	m_xGridNodeValues.fill(GAP_VALUE);

	// we first compute the values (*) in the middle of the vertical grid lines
	// for cell-centered data, the values (o) are given
	//
	//   +-----------+-----------+
	//   |           |           |
	//   *     o     *     o     *
	//   |           |           |
	//   +-----------+-----------+
	//   |           |           |
	//   *     o     *     o     *
	//   |           |           |
	//   +-----------+-----------+


	if (cellCentered) {
		// in case of cellCentered values, first interpolate to the grid - we use
		// m_nodeValues[iElement][jElement] to store the interpolated value between elements (i,j) and (i+1,j)

		// We may not have all elements, so we process element by element and check for left and right neighbor.
		// If a left neighbor exists, we interpolate linearly between cell centers (weighted average) and store
		// that in the nodeValues matrix. If no neighbor exists, we store the element's value at the node
		// (constant extrapolation).

		// WARNING: Mind the difference between the m_elements vector and the geometryfile->m_elements vectors!
		//          They are only the same for 4Dfrom4D data or 4Dfrom5D with xy Mapping.

		for (unsigned int e=0; e<m_dataSize; ++e) {
			double value = m_data[e];
			// skip non-existing elements
			if (value != value)
				continue;

			unsigned int i = e % m_xValueSize;
			unsigned int j = e / m_xValueSize;
			// left boundary?
			if (i == 0)
				m_xGridNodeValues(i,j) = value;
			else {
				// check left neighbor
				unsigned int eLeft = e-1;
				if (m_data[eLeft] != m_data[eLeft])
					m_xGridNodeValues(i,j) = value;
				else {
					// let's interpolate value at left grid line linearly between left element and ours
					double valLeft = m_data[eLeft];
					double dxLeft = m_xGrid[i] - m_xGrid[i-1];
					double dx = m_xGrid[i+1] - m_xGrid[i];
					double alpha = dxLeft/(dx+dxLeft);
					double val = valLeft*(1-alpha) + value*alpha;
					m_xGridNodeValues(i,j) = val;
				}
			}
			// right boundary, set the value also at right interface or if there is no element to the right
			if (i==m_xValueSize-1)
				m_xGridNodeValues(i+1,j) = value;
			else {
				// check right neighbor
				unsigned int eRight = e+1;
				if (m_data[eRight] != m_data[eRight])
					m_xGridNodeValues(i+1,j) = value;
			}
		}
	}
	else {
		// simply copy the nodal values over
		for (unsigned int j=0; j<m_yValueSize; ++j) {
			for (unsigned int i=0; i<m_xValueSize; ++i) {
				unsigned int eIdx = j*m_xValueSize + i;
				m_xGridNodeValues(i,j) = m_data[eIdx];
			}
		}
	}

	// we now compute the nodal values (x) m_nodeValues
	//
	//   x-----------x-----------x j = yValueSize = yGridSize - 1
	//   |           |           |
	//   *           *           *  <-- m_xGridNodeValues(i,j)
	//   |           |           |
	//   x-----------x-----------x  j = 1
	//   |           |           |
	//   *           *           *
	//   |           |           |
	//   x-----------x-----------x  j = 0

	m_nodeValues.resize(m_xGridSize, m_yGridSize);
	m_nodeValues.fill(GAP_VALUE);

	// top most row - node value corresponds to mid-cell-value of top cell row
	// bottom most row - node value corresponds to mid-cell-value of bottom cell row
	for (unsigned int i=0; i<m_xGridSize; ++i) {
		m_nodeValues(i, m_yValueSize) = m_xGridNodeValues(i, m_yValueSize-1); // in case of a single row, m_yValueSize = 1
		m_nodeValues(i, 0) = m_xGridNodeValues(i, 0); // in case of a single row, m_yValueSize = 1
	}

	// now we interpolate the nodal values in vertical direction, hereby only using m_xGridNodeValues vector
	for (unsigned int j=m_yValueSize-1; j>0; --j) {
		// generate values for row j
		for (unsigned int i=0; i<m_xGridSize; ++i) {
			// check if there is a value for cell-row j-1
			double value = m_xGridNodeValues(i, j-1);
			if (!(value != value)) { // "value is not a nan"
				double valTop = m_xGridNodeValues(i, j);
				if (valTop != valTop) // "value is a nan"
					m_nodeValues(i,j) = value;
				else {
					// interpolate between values
					double dyTop = m_yGrid[j+1] - m_yGrid[j];
					double dy = m_yGrid[j] - m_yGrid[j-1];
					double alpha = dyTop/(dy+dyTop);
					double val = valTop*(1-alpha) + value*alpha;
					m_nodeValues(i,j) = val;
				}
			}
			else {
				double valTop = m_xGridNodeValues(i, j);
				if (!(valTop != valTop)) // "value is not a nan"
					m_nodeValues(i,j) = valTop;
			}
		}
	}
}


QImage PlotSpectrogram::renderImage(
	const QwtScaleMap &xMap, const QwtScaleMap &yMap,
	const QRectF &area, const QSize & /*imageSize*/) const
{
	const char * const FUNC_ID = "[SciSpectrogramChart::renderImage]";

	// check if we have valid data to avoid access violation afterwards
	if (m_nodeValues.isEmpty())
		return QImage();

	if ( area.isEmpty() )
		return QImage();

	QRectF rect = QwtScaleMap::transform(xMap, yMap, area);

	QwtScaleMap xxMap = xMap;
	QwtScaleMap yyMap = yMap;

#if QT_VERSION < 0x040000
	QImage image(rect.size(), QwtPlotSpectrogram::colorMap().format() == QwtColorMap::RGB ? 32 : 8);
#else
	QImage image(rect.size().toSize(), QwtPlotSpectrogram::colorMap()->format() == QwtColorMap::RGB ? QImage::Format_ARGB32 : QImage::Format_Indexed8 );
#endif
	image.fill(Qt::transparent);

	/// \todo intensityRange should be the range set with the color map, not the range
	///       automatically determined from the color map. This should only be used
	///       in case of autoscale.
	QwtInterval intensityRange = data()->interval(Qt::ZAxis);
	if ( !intensityRange.isValid() ) {
		IBK::IBK_Message("Invalid data/z-value range.", IBK::MSG_ERROR, FUNC_ID);
		return image; // invalid range - empty/transparent image
	}

	bool cellCentered = (m_xGridSize != m_xValueSize);

	if ( QwtPlotSpectrogram::colorMap()->format() == QwtColorMap::RGB ) {
		// determine drawing range in [m]
		//const double ty = yyMap.invTransform(rect.top());
		//const double by = yyMap.invTransform(rect.bottom());
		//const double lx = xxMap.invTransform(rect.left());
		//const double rx = xxMap.invTransform(rect.right());
		const int pty = rect.top();
		const int pby = rect.bottom();
		const int plx = rect.left();
		const int prx = rect.right();

		// For grid-aligned data (time-coordinate-value data), we draw the
		// rectangle between node/element i,j and node i+1,j, which
		// corresponds to the rectangle between x-gridlines i and i+1 and
		// y-gridlines j and j+1.
		// Note: The nodes/elements at the right-most x-gridline would result
		// in a rectangle beyond the right-most x-gridline and are hence
		// skipped.

		// loop over all elements
		for (unsigned int e=0; e<m_dataSize; ++e) { //m_dataSize
			double val = m_data[e];
			// skip non-existing elements
			if (val != val)
				continue;
			unsigned int e_i = e % m_xValueSize;
			unsigned int e_j = e / m_xValueSize;
			if (!cellCentered && e_i+1 == m_xValueSize)
				continue; // skip right-most nodes

			// determine topleft and bottom right
			double x_left = m_xGrid[e_i];
			double x_right = m_xGrid[e_i + 1];
			double y_bottom = m_yGrid[e_j];
			double y_top = m_yGrid[e_j + 1];
			// now get the pixel range to draw into
			int xp_left = xxMap.transform(x_left);
			int xp_right = xxMap.transform(x_right);
			int yp_top = yyMap.transform(y_top);
			int yp_bottom = yyMap.transform(y_bottom);
			// swap ranges if transformation made xp_left > xp_right or yp_top > yp_bottom
			if (xp_left > xp_right)
				std::swap(xp_left, xp_right);
			if (yp_top > yp_bottom)
				std::swap(yp_top, yp_bottom);

			// skip element if outside drawing range
			if (xp_right < plx || xp_left > prx || yp_bottom < pty || yp_top > pby) continue;

			// clip to drawing range
			// determine range to be drawn
			xp_left = std::max(xp_left, plx);
			xp_right = std::min(xp_right, prx);
			yp_top = std::max(yp_top, pty);
			yp_bottom = std::min(yp_bottom, pby);

			for (int j=yp_top; j<yp_bottom; ++j) {
				QRgb *line = (QRgb *)image.scanLine(j);
				const double ty = yyMap.invTransform(j);
				for (int i=xp_left; i<xp_right; ++i) {
					const double tx = xxMap.invTransform(i);
					// determine color
					Q_ASSERT( i-rect.left() < image.width() );
					val = value(e,tx,ty);
					QRgb rgb;
					if (val != val)
						rgb = qRgb(255, 255, 255); // use white color value when value is NAN
					else
						rgb = colorMap()->rgb(intensityRange, val);
					line[i - int(rect.left())] = rgb;
				}
			}
		}
	}

	QPainter painter(&image);
	drawBoundaryLines(&painter, rect, xxMap, yyMap);

	return image;
}

static void drawHorizontalLine(QPainter * painter, const QRectF & rect, const QwtScaleMap & xxMap, const QwtScaleMap & yyMap,
						const DATAIO::ConstructionLine2D& line) {
	int yp = yyMap.transform(line.m_pos);
	if (yp < rect.top() || yp > rect.bottom())
		return;

	// get length
	int xp_left = xxMap.transform(line.m_begin);
	int xp_right = xxMap.transform(line.m_end);
	if (xp_left > xp_right)
		std::swap(xp_left, xp_right);
	// clip line to plotted area
	xp_right = std::min(xp_right, (int)rect.right());
	xp_left = std::max(xp_left, (int)rect.left());
	painter->drawLine(QPoint(xp_left, yp), QPoint(xp_right, yp));
}

static void drawVerticalLine(QPainter * painter, const QRectF & rect, const QwtScaleMap & xxMap, const QwtScaleMap & yyMap,
						const DATAIO::ConstructionLine2D& line) {
	int xp = xxMap.transform(line.m_pos);
	if (xp < rect.left() || xp > rect.right() )
		return;

	int yp_top = yyMap.transform(line.m_begin);
	int yp_bottom = yyMap.transform(line.m_end);
	if (yp_top > yp_bottom)
		std::swap(yp_top, yp_bottom);
	yp_bottom = std::min(yp_bottom, (int)rect.bottom());
	yp_top = std::max(yp_top, (int)rect.top());
	painter->drawLine(QPoint(xp, yp_top), QPoint(xp, yp_bottom));
}

void PlotSpectrogram::drawBoundaryLines(QPainter * painter, const QRectF & rect,
											const QwtScaleMap & xxMap, const QwtScaleMap & yyMap) const
{
	// draw construction/boundary lines
	if ( m_constructionLinePen.style() == Qt::NoPen )
		return;

	QPen boundaryPen = m_constructionLinePen;
	double width = boundaryPen.widthF();
	width *= 2;
	boundaryPen.setWidthF(width);

	painter->setPen(m_constructionLinePen);
	const std::vector<DATAIO::ConstructionLine2D>& hclines = m_constructionLines.m_horizontalConstructionLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = hclines.begin(); it!=hclines.end(); ++it) {
		drawHorizontalLine(painter, rect, xxMap, yyMap, *it);
	}

	const std::vector<DATAIO::ConstructionLine2D>& vclines = m_constructionLines.m_verticalConstructionLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = vclines.begin(); it!=vclines.end(); ++it) {
		drawVerticalLine(painter, rect, xxMap, yyMap, *it);
	}

	painter->setPen(boundaryPen);
	const std::vector<DATAIO::ConstructionLine2D>& hblines = m_constructionLines.m_horizontalBoundaryLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = hblines.begin(); it!=hblines.end(); ++it) {
		drawHorizontalLine(painter, rect, xxMap, yyMap, *it);
	}

	const std::vector<DATAIO::ConstructionLine2D>& vblines = m_constructionLines.m_verticalBoundaryLines;
	for(std::vector<DATAIO::ConstructionLine2D>::const_iterator it = vblines.begin(); it!=vblines.end(); ++it) {
		drawVerticalLine(painter, rect, xxMap, yyMap, *it);
	}
}


double PlotSpectrogram::value(int valueIndex, double x, double y) const {

	if (valueIndex >= static_cast<int>(m_dataSize))
		return std::numeric_limits<double>::quiet_NaN();

	if (m_interpolate) {

		//   x-----------x  e_j+1
		//   |           |
		//   |   o       | y
		//   |           |
		//   x-----------x  e_j
		//   e_i x       e_i+1

		// at the moment use only bilinear interpolation
		unsigned int e_i = valueIndex % m_xValueSize;
		unsigned int e_j = valueIndex / m_xValueSize;

		double y0 = m_nodeValues(e_i,e_j);
		if (m_xGridSize == 1) {
			/// \todo Implement interpolation in y-direction for x-grid line e_i
			return y0;
		}
		else if (m_yGridSize == 1) {
			/// \todo Implement interpolation in x-direction for y-grid line e_j
			return y0;
		}
		else {
			double y1 = m_nodeValues(e_i+1,e_j);
			double y2 = m_nodeValues(e_i+1,e_j+1);
			double y3 = m_nodeValues(e_i,e_j+1);

			double t = ( x - m_xGrid[e_i] ) / ( m_xGrid[e_i+1] - m_xGrid[e_i] );
			double u = ( y - m_yGrid[e_j] ) / ( m_yGrid[e_j+1] - m_yGrid[e_j] );

			double zValue = ( 1-t )*( 1-u )*y0 + t*( 1-u )*y1 + t*u*y2 + ( 1-t )*u*y3;

			return zValue;
		}
	}
	else {
		// time-coordinate-value plot
		if (m_xGridSize == m_xValueSize) {

			//   E - element/node that we are given
			//   E(i,j) value is returned for left half of element,
			//   E(i+1,j) value is returned for right half of element.
			//
			//   x-----------x  e_j+1
			//   |22222333333|
			//   E22222333333|
			//   |22222333333|
			//   x-----------x  e_j
			//   e_i         e_i+1

			unsigned int e_i = valueIndex % m_xValueSize;
			unsigned int e_j = valueIndex / m_xValueSize;
			if (m_xGridSize == 1) {
				return m_data[valueIndex];
			}
			else {
				IBK_ASSERT(e_i < m_xGridSize-1);
				double t = ( x - m_xGrid[e_i] ) / ( m_xGrid[e_i+1] - m_xGrid[e_i] );
				if (t <= 0.5)
					return m_xGridNodeValues(e_i, e_j);
				else {
					// return value of right neighbor cell value
					return m_xGridNodeValues(e_i+1, e_j);
				}
			}
		}
		else
			return m_data[valueIndex];
	}
}

QRectF PlotSpectrogram::boundingRect() const {
	// mind that xGrid or yGrid may be inverted
	double x1 = m_xGrid[0];
	double x2 = m_xGrid[m_xGridSize-1];
	double y1 = m_yGrid[0];
	double y2 = m_yGrid[m_yGridSize-1];
	if (x1 > x2) std::swap(x1,x2);
	if (y1 > y2) std::swap(y1,y2);
	return QRectF(QPointF(x1,y1), QPointF(x2, y2));
}

void PlotSpectrogram::setConstructionLinePen(const QPen& pen) {
	m_constructionLinePen = pen;
}

void PlotSpectrogram::setConstructionLines(const DATAIO::ConstructionLines2D& clines) {
	m_constructionLines = clines;
}

void PlotSpectrogram::setContourPen(const QPen& pen) {
	m_contourPen = pen;
}

QPen PlotSpectrogram::contourPen(double value) const {

	const QwtInterval intensityRange = data()->interval(Qt::ZAxis);
	const QColor c( colorMap()->rgb(intensityRange, value));

	QPen pen( m_contourPen );
	pen.setColor( c );
	return pen;
}


/*!
   Calculate contour lines

   \param rect Rectangle, where to calculate the contour lines
   \param raster Raster, used by the CONREC algorithm

   \sa contourLevels(), setConrecAttribute(),
	   QwtRasterData::contourLines()
*/
QwtRasterData::ContourLines PlotSpectrogram::renderContourLines(
	const QRectF &rect, const QSize &raster) const
{
	return contourLines(rect, raster, contourLevels() );
}

/*!
   Calculate contour lines

   An adaption of CONREC, a simple contouring algorithm.
   https://www.researchgate.net/publication/221943513_A_contouring_subroutine
*/
#if QT_VERSION >= 0x040000
QwtRasterData::ContourLines PlotSpectrogram::contourLines(
	const QRectF &rect, const QSize &raster,
	const QList<double> &levels) const
#else
QwtRasterData::ContourLines QwtRasterData::contourLines(
	const QRectF &rect, const QSize &raster,
	const QValueList<double> &levels) const
#endif
{
	QwtRasterData::ContourLines contourLines;

	if ( levels.size() == 0 || !rect.isValid() || !raster.isValid() )
		return contourLines;

	const bool ignoreOnPlane =
		testConrecFlag( QwtRasterData::IgnoreAllVerticesOnLevel);

	const QwtInterval range = data()->interval(Qt::ZAxis);
	bool ignoreOutOfRange = false;
	if ( range.isValid() )
		ignoreOutOfRange = testConrecFlag( QwtRasterData::IgnoreOutOfRange);

	for ( unsigned int x=0; x+1<m_xGridSize; ++x ) {

		enum Position
		{
			Center,

			TopLeft,
			TopRight,
			BottomRight,
			BottomLeft,

			NumPositions
		};

		std::vector<Contour3DPoint> xy(NumPositions);

		double xp_left = m_xGrid[x];
		double xp_right = m_xGrid[x+1];

		for ( unsigned int y = 0; y+1 < m_yGridSize; ++y ) {

			double yp_top = m_yGrid[y];
			double yp_bottom = m_yGrid[y+1];

			double topRightValue	= m_nodeValues(x+1,y);
			double topLeftValue		= m_nodeValues(x,y);
			double bottomRightValue = m_nodeValues(x+1,y+1);
			double BottomLeftValue	= m_nodeValues(x,y+1);

			// check if any of the nodes in the current rectangle are NANs (i.e. gaps)
			if (qIsNaN(topRightValue) ||
				qIsNaN(topLeftValue) ||
				qIsNaN(bottomRightValue) ||
				qIsNaN(BottomLeftValue))
			{
				continue;
			}

			// calculate border edges
			if ( x == 0 || y == 0 || x == m_xValueSize-1 || y == m_yValueSize ) {

			}
			else {  // calulate inner egdes

			}

			xy[TopRight].setPos(xp_right, yp_top);
			xy[TopRight].setZ( topRightValue );

			xy[BottomRight].setPos(xp_right, yp_bottom);
			xy[BottomRight].setZ( bottomRightValue );

			xy[TopLeft].setPos(xp_left, yp_top);
			xy[TopLeft].setZ( topLeftValue );

			xy[BottomLeft].setPos(xp_left, yp_bottom);
			xy[BottomLeft].setZ( BottomLeftValue );

			double zMin = xy[TopLeft].z();
			double zMax = zMin;
			double zSum = zMin;

			for ( int i = TopRight; i <= BottomLeft; i++ )
			{
				const double z = xy[i].z();

				zSum += z;
				if ( z < zMin )
					zMin = z;
				if ( z > zMax )
					zMax = z;
			}

			if ( ignoreOutOfRange )
			{
				if ( !range.contains(zMin) || !range.contains(zMax) )
					continue;
			}

			if ( zMax < levels[0] ||
				zMin > levels[levels.size() - 1] )
			{
				continue;
			}

			xy[Center].setPos( xp_left + (xp_right-xp_left)/2.0, yp_top + (yp_bottom - yp_top)/2.0 );
			xy[Center].setZ(0.25 * zSum);
			const int numLevels = (int)levels.size();
			for (int l = 0; l < numLevels; l++)
			{
				const double level = levels[l];
				if ( level < zMin || level > zMax )
					continue;
	#if QT_VERSION >= 0x040000
				QPolygonF &lines = contourLines[level];
	#else
				QVector<QPointF> &lines = contourLines[level];
	#endif
				const ContourPlane plane(level);

				QPointF line[2];
				Contour3DPoint vertex[3];

				for (int m = TopLeft; m < NumPositions; m++)
				{
					vertex[0] = xy[m];
					vertex[1] = xy[0];
					vertex[2] = xy[m != BottomLeft ? m + 1 : TopLeft];

					const bool intersects =
						plane.intersect(vertex, line, ignoreOnPlane);
					if ( intersects )
					{
	#if QT_VERSION >= 0x040000
						lines += line[0];
						lines += line[1];
	#else
						const int index = lines.size();
						lines.resize(lines.size() + 2, QGArray::SpeedOptim);

						lines[index] = line[0];
						lines[index+1] = line[1];
	#endif
					}
				}
			}
		}
	}

	return contourLines;
}

void PlotSpectrogram::setLabelFont( const QFont &labelFont) {
	m_labelFont = labelFont;
}

void PlotSpectrogram::setLabelBackground( const QBrush &labelBackground) {
	m_labelBackground = labelBackground;
}

void PlotSpectrogram::setLabelsPerContourLevel( int labelsPerContourLevel) {
	m_labelsPerContourLevel = labelsPerContourLevel;
}

void PlotSpectrogram::setLabelMargin( int labelMargin) {
	m_labelMargin = labelMargin;
}

void PlotSpectrogram::setLabelStep( int labelStep) {
	m_labelStep = labelStep;
}

void PlotSpectrogram::setLabelPlacement( unsigned int labelPlacement) {
	m_labelPlacement = labelPlacement;
}

unsigned int PlotSpectrogram::labelPlacement() const {
	return m_labelPlacement;
}

void PlotSpectrogram::drawContourLines( QPainter *painter, const QwtScaleMap &xMap,
											const QwtScaleMap &yMap, const QwtRasterData::ContourLines &contourLines) const
{
	// Paint the contours.
	QwtPlotSpectrogram::drawContourLines( painter, xMap, yMap, contourLines);

	// draw boundary lines, but only if we do not draw in ImageMode, because then we had drawn the lines
	// already
	if (!(testDisplayMode(ImageMode)) ) {
		QRectF area = QRectF(painter->window());
		drawBoundaryLines(painter, area, xMap, yMap);
	}

	if ( m_labelPlacement == NO_LABELS) return;

	// Set the area that labels are allowed to be within.
	// @TODO This does not work correctly for printed plots.
	QRect labelArea = painter->window();
	labelArea.adjust( m_labelMargin, m_labelMargin, -m_labelMargin, -m_labelMargin);

	// aspectRatio is the ratio between real x/y scales and screen x/y display rect.
	// This is used to scale the text rotation angle, taking into account
	// screen ratio and axis scale ratios. This is required because we can't
	// use the mapped pixel coordinates of line segments to determine line angle.
	// as the line becomes rasterised. The line segments are so small that we loose
	// the real slop of the lines after they have been converted to screen coordinates.
	double aspectRatio = plot()->axisScaleDiv( yAxis()).range()
						/ plot()->axisScaleDiv( xAxis()).range()
						* labelArea.width() / labelArea.height();

	// Cycle through contour levels to add contour labels.
	QList<double> cLevels = contourLevels();
	for ( int i = 0; i < cLevels.size(); i += m_labelStep) {
		double level = cLevels[i];

		// Get the contour pen. Skip level if no pen style.
		QPen pen = defaultContourPen();
		if ( pen.style() == Qt::NoPen ) {
		  pen = contourPen( level);
		}
		if ( pen.style() == Qt::NoPen ) {
			continue;
		}

		// Set the contour label text and font and calculate font metrics.
		QString labelText;
		labelText.setNum( level);
		QFontMetrics fontMetrics( m_labelFont);
		QRect labelRect = fontMetrics.boundingRect( labelText);
		QPoint labelCentre( -labelRect.width() / 2, labelRect.height() / 3);
		painter->setFont( m_labelFont);

		// Set brush for label text.
		painter->setBrush( m_labelBackground );

		// Intermediate variables.
		int k;
		QPolygonF lines;
		const QPolygonF &allLines = contourLines[ level];

		// Locate the contour segments that will be labelled at even intervals.
		if ( m_labelPlacement & N_PER_LEVEL && m_labelsPerContourLevel > 0) {
			int step = allLines.size() / 2 / m_labelsPerContourLevel;
			if ( step % 2 != 0) {
				--step;
			}
//			if ( step < 100) {
//				step = 100;
//			}
			for ( k = step; k < allLines.size() - 1; k += 2 * step) {
				lines += allLines[k];
				lines += allLines[k+1];
			}
		}

		// Locate the contour segments that will be labelled at min/max locations.
		QVector<int> idx( PLACEMENT_SIZE, -1);
		QVector<double> testValue( PLACEMENT_SIZE, 0.0);
		for ( k = 0; k < allLines.size(); k += 2) {
			QPoint p( xMap.transform( allLines[k].x()), yMap.transform( allLines[k].y()));
			if ( labelArea.contains( p)) {
				if ( m_labelPlacement & AT_MIN_X)  {
					if ( ( idx[AT_MIN_X_IDX] == -1) || ( allLines[k].x() < testValue[AT_MIN_X_IDX])) {
						idx[AT_MIN_X_IDX]       = k;
						testValue[AT_MIN_X_IDX] = allLines[k].x();
					}
				}
				if ( m_labelPlacement & AT_MAX_X)  {
					if ( ( idx[AT_MAX_X_IDX] == -1) || ( allLines[k].x() > testValue[AT_MAX_X_IDX])) {
						idx[AT_MAX_X_IDX] = k;
						testValue[AT_MAX_X_IDX] = allLines[k].x();
					}
				}
				if ( m_labelPlacement & AT_MIN_Y)  {
					if ( ( idx[AT_MIN_Y_IDX] == -1) || ( allLines[k].y() < testValue[AT_MIN_Y_IDX])) {
						idx[AT_MIN_Y_IDX]       = k;
						testValue[AT_MIN_Y_IDX] = allLines[k].y();
					}
				}
				if ( m_labelPlacement & AT_MAX_Y)  {
					if ( ( idx[AT_MAX_Y_IDX] == -1) || ( allLines[k].y() > testValue[AT_MAX_Y_IDX])) {
						idx[AT_MAX_Y_IDX] = k;
						testValue[AT_MAX_Y_IDX] = allLines[k].y();
					}
				}
			}
		}
		// Add what we have just found above to the line list.
		for ( k = 0; k < idx.size(); ++k) {
			if ( idx[k] != -1) {
				lines += allLines[idx[k]];
				lines += allLines[idx[k]+1];
			}
		}

		// Draw the labels. Ensure they are rotated inline with the contour segment.
		for ( int j = 0; j < lines.size(); j += 2) {
			QPoint p1( xMap.transform( lines[j].x()),
			   yMap.transform( lines[j].y())); // Beware of rasterised coordinates.
			double dx = lines[j+1].x() - lines[j].x();
			double dy = lines[j+1].y() - lines[j].y();
			qreal angle = -std::atan2( dy, dx * aspectRatio) * d180_pi;
			if ( dx < 0.0) {
				angle -= 180.0;
			}
			painter->save();
			// Translate painter to be located beneath center of text,
			painter->translate( p1);
			// then rotate text
			painter->rotate( angle);
			// move the text slightly above the line
			painter->translate(labelCentre.x(), 0.1*labelRect.y());
			painter->setPen( Qt::NoPen);
			painter->drawRect( labelRect);
			painter->setPen( pen);
			painter->drawText( 0, 0, labelText);
			painter->restore();
		}
	}
}

double PlotSpectrogram::triangleInterpolate(const DATAIO::GeoFile::Element & elem1,const DATAIO::GeoFile::Element & elem2,
											const DATAIO::GeoFile::Element & elem3,
												double w1, double w2, double w3,
												double x, double y ) {

	double d = elem1.x*elem2.y-elem2.x*elem1.y +
			   elem2.x*elem3.y-elem3.x*elem2.y +
			   elem3.x*elem1.y-elem1.x*elem3.y;

	double a = ((elem2.y-elem3.y)*w1 +
			   (elem3.y-elem1.y)*w2 +
			   (elem1.y-elem2.y)*w3)/d;

	double b = ((elem3.x-elem2.x)*w1 +
			   (elem1.x-elem3.x)*w2 +
			   (elem2.x-elem1.x)*w3)/d;

	double c = ((elem2.x*elem3.y-elem3.x*elem2.y)*w1 +
				(elem3.x*elem1.y-elem1.x*elem3.y)*w2 +
				(elem1.x*elem2.y-elem2.x*elem1.y)*w3)/d;

	double value = a*x+b*y+c;
	return value;
}


} // namespace SCI
