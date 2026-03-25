/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_AbstractLineSeriesModelH
#define Sci_AbstractLineSeriesModelH

#include <QColor>
#include <QList>
#include <QPen>
#include <QBrush>

#include <IBK_Unit.h>

#include "Sci_AbstractCartesianChartModel.h"

class TiXmlElement;

namespace SCI {

/*! \brief Abstract base class for line series models.
	This class doesn't contain any data. It implements the interface
	of AbstractChartModel and provides data specific to multiple line series.
	The functions xValues() and yValues() provide raw-pointers to memory
	arrays holding the data.
	The meta-data for each line series is stored and managed for convenience. Derivad
	classes only need to implement seriesCount(), xValues() and yValues().
	When implementing a line series model based on AbstractLineSeriesModel you need
	manage the line meta-data (line information), typically by calling beginInsertSeries(), endInsertSeries()
	or beginRemoveSeries(), endRemoveSeries().
*/
class AbstractLineSeriesModel : public AbstractCartesianChartModel {
	Q_OBJECT
	Q_DISABLE_COPY(AbstractLineSeriesModel)
public:

	/*! Additional roles for seriesData() specific to line series.
		Don't change role order. Always add new roles before NUM_SDR.
	*/
	enum SeriesDataRole {

		SeriesSize = 0,			///< Role for size (number of value pairs) in series.
		SeriesTitleText,		///< Series title text (QString, may contain placeholders, used for property widgets).
		SeriesTitle,			///< Series title where placeholders have been replaced (using data from extractor).
		SeriesPen,				///< Line pen (QPen), also sets color and width as part of pen properties.
		SeriesWidth,			///< Line width (int, 0 means cosmetic pen). Mind that setting pen overwrites width.
		SeriesColor,			///< Line color (QColor). Mind that setting pen overwrites color.
		/*! Role for determining axis used by series: 0 for left, 1 for right y axis.
			When moving a series to the right axis, this axis will be automatically enabled (AxisEnabled property).
			When moving the last series from the right axis to the left axis, the right axis will be automatically disabled.
			\warning You must not set this property when the axis unit of the source axis (where the series is moved from)
				is still undefined.
		*/
		SeriesLeftYAxis,
		SeriesLineStyle,		///< Line type \sa QwtPlotCurve::CurveStyle
		SeriesMarkerStyle,		///< Marker type \sa QwtSymbol::Style
		SeriesInverted,			///< Inverted attribute for drawing step curves
		SeriesFitted,			///< Fitted attribute (spline interpolation) for drawing line curves
		SeriesInLegend,			///< If true, series is indicated in legend
		SeriesMarkerSize,		///< Set size for marker (unsigned int).
		SeriesMarkerFilled,		///< Fill marker with solid color (boolean).
		SeriesMarkerColor,		///< Marker color (QColor). Color for line and background will be the same.
		SeriesBrush,			///< Brush used for area fill from line to x-axis.

		/*! Always last value.
			\note All data roles from SeriesTitle up to NUM_SDR are applied during a chart reset.
		*/
		NUM_SDR
	};

	/*! Struct holds informations about line series.
		It is used in seriesData and setSeriesData.
	*/
	struct LineInformation {
		LineInformation() :
			m_pen(QPen(QBrush(Qt::black), 2, Qt::SolidLine)),
			m_leftAxis(true),
			m_lineStyle(0),		// lines
			m_markerStyle(-1),	// no marker
			m_inverted(false),
			m_fitted(false),
			m_inLegend(true),
			m_markerSize(4),
			m_markerFilled(true),
			m_markerColor(Qt::blue),
			m_brush(QBrush(Qt::blue, Qt::NoBrush))
		{
		}

		QString			m_title;					///< TitleText
		QPen			m_pen;						///< Pen style.
		bool			m_leftAxis;					///< Attachment to axis. If true to left axis, otherwise to right axis.
		int				m_lineStyle;				///< Holds line style (0 = no line)
		int				m_markerStyle;				///< Holds marker style (-1 = no line)
		bool			m_inverted;					///< Invered attribute for drawing step curves.
		bool			m_fitted;					///< Fitted attribute for drawing line curves.
		bool			m_inLegend;					///< If true, series has a corresponding legend entry.
		unsigned int	m_markerSize;				///< Size for marker (in pixels).
		bool			m_markerFilled;				///< If true, marker is filled with marker color.
		QColor			m_markerColor;				///< Background and line color for marker.
		QBrush			m_brush;					///< Brush for area fill.
	};

	/*! Standard constructor.*/
	explicit AbstractLineSeriesModel(QObject *parent = nullptr);

	/*! Returns the type of chart/series. */
	virtual ChartSeriesInfos::SeriesType chartType() const override { return ChartSeriesInfos::LineSeries; }

	/*! Returns number of series (number of cached line meta-data must match number of line series). */
	virtual int seriesCount() const override { return m_lineInformation.count(); }

	/*! Returns the data stored under the given role for the item referred to by the index.
		\note If you do not have a value to return, return an invalid QVariant instead of returning 0.
		\param seriesIndex Series index.
		\param seriesDataRole Role selects the value kind, \see AbstractLineSeriesModel::SeriesDataRole.
	*/
	virtual QVariant seriesData(int seriesIndex, int seriesDataRole) const override;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		This function and data() must be reimplemented for editable models.
		\param value Value to be set.
		\param seriesIndex Series index.
		\param seriesDataRole Role selects the value kind, \see AbstractLineSeriesModel::SeriesDataRole.
	*/
	virtual bool setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) override;

	/*! Sets the complete data for a series at once.
		The dataChanged() signal should be emitted if the data was successfully set.
		\param seriesIndex Series index.
		\param value The line series data.
	*/
	virtual void setSeriesData(int seriesIndex, const LineInformation& value);

	/*! Swaps the line information for the given indexes.
		Indexes must be valid (assert is used).
	*/
	virtual void swapSeriesData(int first, int second);


	/*! Returns a pointer to the x data set.
		The array must have the size from function size().
		\param index Series index.
	*/
	virtual const double*	xValues(unsigned int index) const = 0;

	/*! Returns a pointer to the y data set.
		The array must have the size from function size().
		\param index Series index.
	*/
	virtual const double*	yValues(unsigned int index) const = 0;

	/*! Returns number of construction lines. */
	virtual int constructionLinesCount() const { return 0; }

	/*! Returns a color from a circular internal color list.
		If index is bigger than list size, counting begins from 0.
		\param index Color index.
	*/
	QColor colorFromIndex(unsigned int index);

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	virtual void readXML(const TiXmlElement * element) override;

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	virtual void writeXML(TiXmlElement * parent) const override;


protected:

	/*! Begins a series insertion operation.
		Re-implemented from AbstractChartModel::beginInsertSeries()
	*/
	virtual void beginInsertSeries( int first, int last) override;

	/*! Begins a series removal operation.
		Re-implemented from AbstractChartModel::beginRemoveSeries()
	*/
	virtual void beginRemoveSeries ( int first, int last ) override;

	/*! Clears internally stored data.
		Use this function to clear the internal state of the AbstractLineSeriesModel
		whenever derived classes reset their content.
		Calling this function implies that all line series have been removed
		and new line series are inserted again.
		A call to clearCachedData() should be place between emitted signals
		of type beginResetModel() and endResetModel().
	*/
	virtual void clearCachedData() override;

	/*! Returns cached line information structure. */
	const QList<LineInformation> & lineInformation() const { return m_lineInformation; }


private:

	/*! List of line informations.
		List must have the same length like series count.
		Length will be adapted in insertSeries and removeSeries.
	*/
	QList<LineInformation>		m_lineInformation;
};

} // namespace SCI

#endif // Sci_AbstractLineSeriesModelH
