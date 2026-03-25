/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_BarSeriesContentModelH
#define Sci_BarSeriesContentModelH

#include <QObject>

#include "Sci_AbstractBarSeriesModel.h"

#include <IBK_UnitVector.h>


namespace SCI {

class BarSeriesContentModel : public AbstractBarSeriesModel {
	Q_OBJECT
	Q_DISABLE_COPY(BarSeriesContentModel)
public:
	explicit BarSeriesContentModel(QObject *parent = 0);

	/*! Returns number of bars. */
	virtual int seriesCount() const override;

	/*! Returns number of values (sections) for the bars. */
	virtual int yValuesCount() const override;

	/*! Returns a pointer to the x data set.
		The array must have the size from series data role SeriesSize.
		\param index Series index.
	*/
	virtual const double*	xValues() const override;

	/*! Returns a pointer to the y data set.
		The array must have the size from series data role SeriesSize.
		\param index Series index.
	*/
	virtual const double*	yValues(unsigned int index) const override;

	/*! Return a list of labels for the bottom x-axis.*/
	virtual QStringList xLabels() const override;

	/*! Resets y values and x data labels in data series.
		\param xLabels Vector of labels for bottom axis.
		\param yValues Vector of y values. If internal UnitVector contains only one value a single bar chart will be created otherwise a multi bar chart.
		\param name Chart title.
		Size of vector xLabels and yValues must be the same.
		Only the first unit in yValues is used.
	*/
	void setValues(const QStringList& xLabels, const std::vector<IBK::UnitVector>& yValues, const QString& name);

	/*! Clears the content. After calling this function the model has no series and all pointer are not valid.*/
	void clear();

	/*! Returns the chart meta data stored under the given role.
		Well-behaved models will at least return a suitable axis title.
		\param axisPosition Value for axis position, \see AxisPosition
		\param axisDataRole Item data role for axis data, see AxisDataRole
	*/
	QVariant axisData(AxisPosition axisPosition, int axisDataRole) const override;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		This function and data() must be reimplemented for editable models.
		Default implementation does nothing.
		\param value Value to be set.
		\param axisPosition Value for axis position, \see AxisPosition
		\param axisDataRole Item data role for axis data, see AxisDataRole
	*/
	virtual bool setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) override;

	/*! Returns the data stored under the given role for the item referred to by the index.
		\note If you do not have a value to return, return an invalid QVariant instead of returning 0.
		\param index Series index.
		\param role Role selects the value kind, \see AbstractChartModel::SeriesDataRole.
	*/
	virtual QVariant seriesData(int seriesIndex, int seriesDataRole) const override;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		The base class implementation returns false. This function and data() must be reimplemented for editable models.
		\param index Series index.
		\param value Value to be set.
		\param role Role selects the value kind, \see AbstractChartModel::Role.
	*/
	virtual bool setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) override;

private:
	IBK::UnitVector					m_xValues;				///< Internal use as index vector
	IBK::UnitVector					m_xValuesConverted;		///< For converting x-values into datTime (not used yet)
	/*! Vector of values for bar chart. External vector represents number of bars entries in the the series.
		If internal vector has size one its a normal bar chart. Otherwise its a multicolumn or stacked bar chart.
	*/
	std::vector<IBK::UnitVector>	m_yValues;
	QStringList						m_xLabels;				///< Labels for bottom x-axis.

	/*! Convert x-values depending on state of dataTime.
		Cannot used currently.
	*/
	void convertXValues();

	/*! Resets x and y values in data series. Current variant of BarSeries doesn't work with real x-values (only indices).7
		This function is for later use.
		\param xvalues Vector of data for x values. Description can be set with setBarTitles.
		\param yvalues Vector of y values. If internal UnitVector contains only one value a single bar chart will be created otherwise a multi bar chart.
		\param name Chart title.
		Size of vector xvalues and yvalues must be the same.
		All units in yvalues must be the same. Only the first unit is used.
	*/
	void setValues(const IBK::UnitVector& xvalues, const std::vector<IBK::UnitVector>& yvalues, const QString& name);
};

} // namespace SCI

#endif // Sci_BarSeriesContentModelH
