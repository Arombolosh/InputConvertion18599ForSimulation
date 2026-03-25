/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PlotVectorFieldH
#define Sci_PlotVectorFieldH

#include <QPen>

#include <qwt_plot_vectorfield.h>

#include <DATAIO_ConstructionLines2D.h>
#include <DATAIO_GeoFile.h>

class QwtColorMap;

namespace SCI {

/*! Wrapper class for QwtPlotVectorField to customize certain drawing properties.

	A vector field can be customized in the following way:

	Data handling
	-------------

	- show raw data, for each vector sample a vector is shown
	- show rasterized data (\sa see setRasterEnabled()), samples are collated (summarized) in raster
	  cells of given (quadratic) size (\sa setRasterSize)


	Vector magnitude/arrow length
	-----------------------------
	- built-in scaling via scale factor (see setMagnitudeScaleFactor() ) with
		tailLength = magnitude * d_data->magnitudeScaleFactor * arrow.tailWidth();

	- custom function as overloaded virtual function double tailLength(magnitude)


	Arrow display
	-------------

	- Position of the arrow head: tail, head, middle (see setIndicatorOrigin())
	- uniform color or via color map (see setMagnitudeMode(), setColorMap())
	- arrow shape: line width, head width and tail width
	  -> either absolute in pixels/points, or relative to vector magnitude


	Notes:
	- rename QwtPlotVectorField::FilterVector  -> CollateVectors
*/
class PlotVectorField : public QwtPlotVectorField {
public:

	/*! Standard constructor.
		\param title Title of the series (normally not used):
	*/
	PlotVectorField(const QString & title = QString());

	void setRasterEnabled(bool on);
	bool rasterEnabled() const;

	void setArrowColor(const QColor & color);
	QColor arrowColor() const;

	/*! Sets arrow attributes. */
	void setArrowAttributes(bool thinArrow, double maxLength, double minLength, double scaleFactor);

	double arrowLength(double magnitude) const override;

	/*! Returns the pen for boundary lines.*/
	QPen boundaryPen() const { return m_constructionLinePen; }

	/*! Sets the construction/boundary line pen.*/
	void setConstructionLinePen(const QPen& pen);

	/*! Set construction lines.*/
	void setConstructionLines(const DATAIO::ConstructionLines2D& clines);

	/*! Re-implemented to draw construction lines below the vector field. */
	virtual void drawSeries( QPainter *,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		const QRectF &canvasRect, int from, int to ) const override;

	/*! Draws boundary lines. */
	void drawBoundaryLines(QPainter * painter, const QRectF & rect,
						   const QwtScaleMap & xxMap, const QwtScaleMap & yyMap) const;

private:

	/*! Classe that handles construction and boundary lines.*/
	DATAIO::ConstructionLines2D						m_constructionLines;

	/*! Pen for construction/boundary lines.*/
	QPen											m_constructionLinePen;

	double											m_maxLength;
	double											m_minLength;
};


} // namespace SCI

#endif // Sci_PlotVectorFieldH
