/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_AbstractColorGridSeriesModelH
#define Sci_AbstractColorGridSeriesModelH

#include "Sci_AbstractCartesianChartModel.h"
#include "Sci_ColorMap.h"
#include <QPen>

class TiXmlElement;

namespace SCI {

class AbstractColorGridSeriesModel : public AbstractCartesianChartModel {
	Q_OBJECT
	Q_DISABLE_COPY(AbstractColorGridSeriesModel)
public:
	/*! Data roles for seriesData() and setSeriesData().
		Don't change role order. Always add new roles before NUM_SDR.
		Appearance properties must be added after ColorMap role.
	*/
	enum SeriesDataRole {

		XGridArray,			///< Pointer to a array of x grid line positions (void* - double*).
		XGridArraySize,		///< Size of the x grid line array (int).
		YGridArray,			///< Pointer to a array of y grid line positions (void* - double*).
		YGridArraySize,		///< Size of the y grid line array (int).
		XValueSize,			///< X direction size for values and elements (int).
		YValueSize,			///< Y direction size for values and elements (int).
		ValueMatrix,		///< Matrix of values as array with size XValueSize*YValueSize (void* - double*) (quiet_NAN signal gap/missing value).
		/*! The color map (Sci::ColorMap), that maps a normalized value to a color. It is used
			to create color maps for the spectogram chart and color legend.
		*/
		ColorMap,
		AutoScale,					///< If disabled, user-defined z values are used for color maps.
		/*! Absolut minimum z for color map range. User defined. This value comes not from the data.
			If not std::numeric_limits<double>::quiet_NaN(), this is used as minimum number instead of DataMinimumZ.
			\note The NAN value is set as default so that there is always a meaningful situation when autoscale is turned off.
				Currently, once changed, it cannot be reset to NAN.
		*/
		MinimumZ,
		/*! Absolut maximum z for color map range. User defined. This value comes not from the data.
			If not std::numeric_limits<double>::quiet_NaN(), this is used as maximum number instead of DataMaximumZ.
			\note The NAN value is set as default so that there is always a meaningful situation when autoscale is turned off.
				Currently, once changed, it cannot be reset to NAN.
		*/
		MaximumZ,
		DataMinimumZ,			///< Minimum z for color map range determined from current data set (not global min/max). Updated whenever data changes. Cannot be set externally.
		DataMaximumZ,			///< Maximum z for color map range determined from current data set (not global min/max). Updated whenever data changes. Cannot be set externally.
		ZValueRange,			///< The effective z-value range (minZ and maxZ) determined depending on value of AutoScale and the Minimum/Maximum data roles above. Cannot be set externally.
		/*! Data interpolation and color mode of the spectrogram chart.
			Values mean:
			- 0 - raw (no data interpolation, continous color map),
			- 1 - banded (data interpolation, fixed color map)
			- 2 - continuous (data interpolation, continous color map)
		*/
		InterpolationMode,
		DisplayMode,		///< Display mode of the spectrogram chart (bitmask: ImageMode = 1, ContourMode = 2).
		ColorBarVisible,	///< Switchs the visibility of the color bar.
		HasContourIntervalColor,	///< Defines if the contour has interval color or has only the pen color.
		ContourInterval,	///< Holds the contour interval (double).
		ContourPen,			///< Contour pen with style, color and style
		ContourLabelPlacement, ///< Placement flags for contour labels
		/*! Always last value. */
		NUM_SDR

		/// \note currently NUM_SDR + 1 is reserved as internal role for contour properties (property group), this should be a named property.
	};

	/*! Standard constructor.*/
	explicit AbstractColorGridSeriesModel(QObject *parent = 0);

	/*! Returns the type of chart/series. */
	virtual ChartSeriesInfos::SeriesType chartType() const override { return ChartSeriesInfos::ColorGridSeries; }

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
	int						m_interpolationMode; ///< 0 - raw, 1 - banded, 2 - continuous
	int						m_displayMode; ///< Bitmask of options: 0x1 - spectrogram, 0x2 - iso lines, 0x3 - both
	bool					m_colorBarVisible;
	double					m_contourInterval;
	QPen					m_contourPen;
	/*! The label placement flags for contour lines. */
	unsigned int			m_contourLabelPlacement;
	bool					m_contourIntervalColor;

};

} // namespace SCI

#endif // Sci_AbstractColorGridSeriesModelH
