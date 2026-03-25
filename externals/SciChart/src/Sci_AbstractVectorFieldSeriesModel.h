/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_AbstractVectorPlotSeriesModelH
#define Sci_AbstractVectorPlotSeriesModelH

#include "Sci_AbstractCartesianChartModel.h"
#include "Sci_ColorMap.h"
#include <QPen>

class TiXmlElement;

namespace SCI {

/*! Implementation for a data provider model for vector plots.
	Corresponds in many ways to the color grid model, but uses "magnitude" instead of z-values.
	Vectors never have a negative length, but their minimum length can be restricted to show also tiny vectors.
*/
class AbstractVectorFieldSeriesModel : public AbstractCartesianChartModel {
	Q_OBJECT
	Q_DISABLE_COPY(AbstractVectorFieldSeriesModel)
public:
	/*! Data roles for seriesData() and setSeriesData().
		Don't change role order. Always add new roles before NUM_SDR.
		Appearance properties must be added after ColorMap role.
	*/
	enum SeriesDataRole {
		/*! Number of vector samples in the data set. */
		VectorSampleCount,
		/*! Read-only, returns array x coordinates of vectors, size VectorSampleCount. */
		VectorSampleXArray,
		/*! Read-only, returns array y coordinates of vectors, size VectorSampleCount. */
		VectorSampleYArray,
		/*! Read-only, returns array x-vector (velocity) component, size VectorSampleCount. */
		VectorSampleVXArray,
		/*! Read-only, returns array y-vector (velocity) component, size VectorSampleCount. */
		VectorSampleVYArray,

		/*! The color map (Sci::ColorMap), that maps a normalized value to a color. It is used
			to color vector arrows based on their magnitude.
		*/
		ColorMap,
		AutoScale,					///< If disabled, user-defined magnitude values are used for color maps/vector arrow scaling.
		/*! Absolut minimum magnitude for color map range/vector arrow scaling. User defined. This value comes not from the data.
			If not std::numeric_limits<double>::quiet_NaN(), this is used as minimum number instead of DataMinimumZ.
			\note The NAN value is set as default so that there is always a meaningful situation when autoscale is turned off.
				Currently, once changed, it cannot be reset to NAN.
		*/
		MinimumZ,
		/*! Absolut maximum magnitude for color map range/vector arrow scaling. User defined. This value comes not from the data.
			If not std::numeric_limits<double>::quiet_NaN(), this is used as maximum number instead of DataMaximumZ.
			\note The NAN value is set as default so that there is always a meaningful situation when autoscale is turned off.
				Currently, once changed, it cannot be reset to NAN.
		*/
		MaximumZ,
		DataMinimumZ,			///< Minimum magnitude for color map range/vector arrow scaling determined from current data set (not global min/max). Updated whenever data changes. Cannot be set externally.
		DataMaximumZ,			///< Maximum magnitude for color map range/vector arrow scaling determined from current data set (not global min/max). Updated whenever data changes. Cannot be set externally.
		ZValueRange,			///< The effective magnitude-value range (minZ and maxZ) determined depending on value of AutoScale and the Minimum/Maximum data roles above. Cannot be set externally.
		ColorBarVisible,		///< Switchs the visibility of the color bar (turns right axis on/off - axis is only visible when colored arrows is enabled).
		RasterEnabled,			///< Enable raster to collate vectors.
		RasterSize,				///< Size of quadratic raster in pixels (zooming in reveals more detail).
		ColoredArrowsEnabled,	///< Enable colorization of arrows via color map.
		UniformArrowColor,		///< Color (QColor) of uniformely colored arrows.
		IndicatorOrigin,		///< Position of error indicator (0 - OriginHead, 1- OriginTail, 2 - OriginCenter).
		ThinArrow,				///< If true, a thin arrow is drawn instead of a filled one.
		MaxArrowLength,			///< Maximum arrow length in pixels.
		MinArrowLength,			///< Minimum arrow length in pixels (if arrow magnitude > 0).
		ArrowScaleFactor,		///< Arrow scale factor (pixels per magnitude).
		/*! Always last value. */
		NUM_SDR

		/// \note currently NUM_SDR + 1 is reserved as internal role for contour properties (property group), this should be a named property.
	};

	/*! Standard constructor.*/
	explicit AbstractVectorFieldSeriesModel(QObject *parent = 0);

	/*! Returns the type of chart/series. */
	virtual ChartSeriesInfos::SeriesType chartType() const override { return ChartSeriesInfos::VectorFieldSeries; }

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		This function and data() can be reimplemented for editable models.
		\param value Value to be set.
		\param axisPosition Value for axis position, \see AxisPosition
		\param axisDataRole Item data role for axis data, see AxisDataRole
		\return It returns false in case of wrong data or errors. True will be returned if set value is set correctly or no set is necessary.
	*/
	virtual bool setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) override;

	/*! Returns the data stored under the given role for the item referred to by the index.
		\note If you do not have a value to return, return an invalid QVariant instead of returning 0.
		\param seriesIndex Series index.
		\param seriesDataRole Role selects the value kind, \see AbstractChartModel::SeriesDataRole.
	*/
	virtual QVariant seriesData(int seriesIndex, int seriesDataRole) const override;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		The base class implementation returns false. This function and data() must be reimplemented for editable models.
		\param value Value to be set.
		\param seriesIndex Series index.
		\param seriesDataRole Role selects the value kind, \see AbstractChartModel::Role.
	*/
	virtual bool setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) override;

	/*!	Writes the model into the output stream.
		It uses the binary representation.
		\param out	The output stream.
	*/
	virtual void writeSeriesBinary(std::ostream& out) const;

	/*!	Read the model from binary file.*/
	virtual void readSeriesBinary( std::istream& in );

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	virtual void readXML(const TiXmlElement * element) override;

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	virtual void writeXML(TiXmlElement * parent) const override;

	/*! Updates user-defined min/max properties (MinimumZ and MaximumZ roles) by looking at the
		_entire_ data set.
		Function needs to be implemented by derived models.
	*/
	virtual void calculateGlobalMinMax() = 0;


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


private:

	SCI::ColorMap			m_colorMap;
	bool					m_autoScale;
	double					m_minimumZ;
	double					m_maximumZ;
	bool					m_colorBarVisible;
	bool					m_rasterEnabled;
	int						m_rasterSize; // in [pix]
	bool					m_coloredArrowsEnabled;
	QColor					m_uniformArrowColor;
	int						m_indicatorOrigin;
	bool					m_thinArrow;
	double					m_maxArrowLength; // in [pix]
	double					m_minArrowLength; // in [pix]
	double					m_arrowScaleFactor; // in [pix/magnitude]
};

} // namespace SCI

#endif // Sci_AbstractVectorPlotSeriesModelH
