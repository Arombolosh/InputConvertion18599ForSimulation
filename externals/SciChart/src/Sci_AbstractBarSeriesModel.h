/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_AbstractBarSeriesModelH
#define Sci_AbstractBarSeriesModelH

#include <QObject>
#include <QPalette>

#include "qwt_plot_multi_barchart.h"
#include "qwt_column_symbol.h"

#include "Sci_AbstractCartesianChartModel.h"

namespace SCI {

/*! \brief Abstract base class for bar series models.
	This class doesn't contain any data. It implements the interface
	of AbstractChartModel and provides data specific to multiple line series.
	The functions xValues() and yValues(int i) provide raw-pointers to memory
	arrays holding the data.
	Only one bar series can be place in a chart.
	The meta-data for each line series is stored and managed for convenience. Derivad
	classes only need to implement seriesCount(), yValuesCount(int i), xValues() and yValues(int i).
	When implementing a bar series model based on AbstractBarSeriesModel you need
	manage the bar meta-data (bar information), typically by calling beginInsertSeries(), endInsertSeries()
	or beginRemoveSeries(), endRemoveSeries().
*/
class AbstractBarSeriesModel : public AbstractCartesianChartModel {
	Q_OBJECT
	Q_DISABLE_COPY(AbstractBarSeriesModel)
public:
	/*! Additional roles for data() specific to line series.
		Don't change role order. Always add new roles before NUM_ADR.
	*/
	enum SeriesDataRole {

		SeriesSize = 0,			///< Role for size (number of x values) in series.
		SeriesTitleText,		///< Series title text (QString, may contain placeholders, used for property widgets).
		SeriesTitle,			///< Series title where placeholders have been replaced (using data from extractor).
		LayoutPolicy,			///< Define how the width of the bars is calculated.
		LayoutHint,				///< Set the value for calculating bar width.
		Spacing,				///< Distance between two bars.
		Margin,					///< Distance between outmost bar and contents rect (Default 5).
		BaseLine,				///< Origin for chart. Bars will be painted from baseline (Default 0).
		BarChartStyle,			///< Set multi bar typ (grouped or stacked).
		Orientation,			///< Orientation of chart (default is Qt::Vertical).
		Information,			///< A entry in bar chart information was changed

		/*! Always last value. */
		NUM_SDR
	};

	/*! Struct holds informations about each bar.
	*/
	struct BarSymbolInformation {
		BarSymbolInformation() :
			m_frameStyle(QwtColumnSymbol::Plain),
			m_lineWidth(1)
		{}

		QwtColumnSymbol::FrameStyle	m_frameStyle;
		int							m_lineWidth;
		QPalette					m_palette;

	public:
		QwtColumnSymbol::FrameStyle frameStyle() const;
	};

	/*! Struct holds informations about bar series.
		It is used in seriesData and setSeriesData.
	*/
	struct BarChartInformation {
		BarChartInformation() :
			m_layoutPolicy(QwtPlotAbstractBarChart::AutoAdjustSamples),
			m_layoutHint(0.5),
			m_spacing(10),
			m_margin(5),
			m_baseLine(0.0),
			m_barChartStyle(QwtPlotMultiBarChart::Grouped),
			m_orientation(Qt::Vertical)
		{
		}

		QString									m_title;
		QwtPlotAbstractBarChart::LayoutPolicy	m_layoutPolicy;
		double									m_layoutHint;
		int										m_spacing;
		int										m_margin;
		double									m_baseLine;
		QwtPlotMultiBarChart::ChartStyle		m_barChartStyle;
		Qt::Orientation							m_orientation;
		std::vector<BarSymbolInformation>		m_barSymbolInformations;
	};

	AbstractBarSeriesModel(QObject *parent = 0);

	/*! Returns the type of chart/series. */
	virtual ChartSeriesInfos::SeriesType chartType() const { return ChartSeriesInfos::BarSeries; }

	/*! Returns number of bars. */
	virtual int seriesCount() const = 0;

	/*! Returns number of values (sections) for the bars. */
	virtual int yValuesCount() const = 0;

	/*! Returns the data stored under the given role for the item referred to by the index.
		\note If you do not have a value to return, return an invalid QVariant instead of returning 0.
		\param index Series index must be 0.
		\param role Role selects the value kind, \see AbstractBarSeriesModel::SeriesDataRole.
	*/
	virtual QVariant seriesData(int seriesIndex, int seriesDataRole) const;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		This function and data() must be reimplemented for editable models.
		\param index Series index must be 0.
		\param value Value to be set.
		\param role Role selects the value kind, \see AbstractBarSeriesModel::Role.
	*/
	virtual bool setSeriesData( const QVariant& value, int seriesIndex,int seriesDataRole);

	/*! Sets the complete data for a series at once.
		The dataChanged() signal should be emitted if the data was successfully set.
		\param value The line series data.
	*/
	virtual void setSeriesData(const BarChartInformation& value);

	/*! Set the color for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
		\param color Color for section rectange in all bars.
	*/
	void setValueColor(unsigned int index, const QColor& color);

	/*! Returns the color for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
	*/
	QColor valueColor(unsigned int index);

	/*! Set the color for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
		\param palette Palette for section rectangle in all bars.
	*/
	void setValuePalette(unsigned int index, const QPalette& palette);

	/*! Returns the color for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
	*/
	QPalette valuePalette(unsigned int index);

	/*! Set the color for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
		\param lineWidth Line width for frame for section rectange in all bars.
	*/
	void setValueFrameLineWidth(unsigned int index, int lineWidth);

	/*! Returns the frame line width for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
	*/
	int	frameLineWidth(unsigned int index);

	/*! Set the color for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
		\param style Style for frame for section rectange in all bars.
		Possible values are:
		\li NoFrame	No frame painted.
		\li Plain Plain frame painted (Default).
		\li Raised Raised frame painted.
	*/
	void setValueFrameStyle(unsigned int index, const QwtColumnSymbol::FrameStyle style);

	/*! Returns the frame style for the value section.
		\param index Index of value section. Must be in the range of yValuesCount().
	*/
	QwtColumnSymbol::FrameStyle	frameStyle(unsigned int index);

	/*! Returns a pointer to the x data set.
		The array must have the size from function seriesCount().
		\param index Series index.
	*/
	virtual const double*	xValues() const = 0;

	/*! Returns a pointer to the y data set.
		The array must have the size from function yValuesCount(unsigned int index) for the given index.
		\param index bar index.
	*/
	virtual const double*	yValues(unsigned int index) const = 0;

	/*! Return a list of labels for the bottom x-axis.*/
	virtual QStringList xLabels() const = 0;

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	virtual void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	virtual void writeXML(TiXmlElement * parent) const;

protected:

	/*! Clears internally stored data.
		Use this function to clear the internal state of the AbstractLineSeriesModel
		whenever derived classes reset their content.
		Calling this function implies that all line series have been removed
		and new line series are inserted again.
		A call to clearCachedData() should be place between emitted signals
		of type beginResetModel() and endResetModel().
	*/
	virtual void clearCachedData();

	BarChartInformation		m_barChartInformation;	///< Bar informations


};

} // namespace SCI

#endif // Sci_AbstractBarSeriesModelH
