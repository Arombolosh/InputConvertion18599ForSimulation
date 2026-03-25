/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_LineSeriesContentModelH
#define Sci_LineSeriesContentModelH

#include "Sci_AbstractLineSeriesModel.h"

#include <vector>

#include <IBK_UnitVector.h>
#include <IBK_assert.h>

namespace SCI {

/*! \brief Class for line series models that contains own data.
	The data stored as vectors of IBK::UnitVector pointer. The pointer use is necessary in order to prohibit
	invalidation of data sets while inserting or removing operations.

	This implementation can be used as model for charts when no other data source is available.
	It requires copying of data into the line series model via the addDataSet() function.

	\todo Implement data() and setData() for the model to work.
*/
class LineSeriesContentModel : public AbstractLineSeriesModel {
	Q_OBJECT
	Q_DISABLE_COPY(LineSeriesContentModel)
public:
	explicit LineSeriesContentModel(QObject *parent = nullptr);

	/*! Releases memory from m_xValues and m_yValues. */
	~LineSeriesContentModel() override;

	/*! Should return the number of series in the model.*/
	virtual int seriesCount() const override;

	/*! Returns a pointer to the x data set.
		The array must have the size from series data role SeriesSize.
		\param index Series index.
	*/
	virtual const double*	xValues(unsigned int index) const override;

	/*! Returns a pointer to the y data set.
		The array must have the size from series data role SeriesSize.
		\param index Series index.
	*/
	virtual const double*	yValues(unsigned int index) const override;

	/*! Resets x and y values in data series.
		\param xvalues The x values. Unit of xvalues must match the unit of current x-axis (which must have a valid unit already).
		\param yvalues The y values. Unit of yvalues must match the unit of associated y-axis
			(see AbstractLineSeriesModel::SeriesLeftYAxis property). If the units are not matching, an exception is thrown.
	*/
	void setValues(unsigned int index, const IBK::UnitVector& xvalues, const IBK::UnitVector& yvalues);

	/*! Sets new raw data to replace the current y values in a data series.
		The array must have the size from series data role SeriesSize.
		\param index Series index.
		\param yValues Y-values of data series (already converted to the current unit of the associated y-axis),
			values are copied into the content model.
	*/
	void setYValues(unsigned int index, const double * yValues);

	/*! Adds a new data set. The series is automatically assigned to either the y1 or the y2 axis, depending
		on the unit in the \a yvalues vector and the units of the y1 and y2 axis, if already assigned.
		The unit of the x-axis is set to the unit of the x-values, if not yet used.
		The new series is also given a default color and a solid line style.

		\param xvalues Vector with x-values.
		\param yvalues Vector with y-values.
		\param name Name of the series, used in legend.

		Exceptions are thrown if any of the following conditions are not met:
		- xvalues and yvalues must have valid units
		- x-axis must have undefined/unused unit or unit must be convertible into unit of xvalues vector
		- y1-axis or y2-axis must have undefined/unused unit, or one of the axis units must be convertible into unit of yvalues vector
	*/
	void addDataSet(const IBK::UnitVector& xvalues, const IBK::UnitVector& yvalues, const QString& name);

	/*! Removes the data set identified by index \param index.
		\param index Index of line series, must exist (throws exception if outside seriesCount()).
	*/
	void removeDataSet(unsigned int index);

	/*! Removes all existing data sets and the corresponding series.*/
	void removeAllDataSets();

	/*! Appends a x y value pair to a data set identified by index \param index.
		\param index Index of line series, must exist (throws exception if outside seriesCount()).
		\param xValue Value in xUnit, x-values must be increasing strictly monotonically.
		\param xUnit xUnit, if different from currently used x-axis unit, value is converted accordingly. Axis unit is not modified.
		\param yValue Value in yUnit
		\param yUnit yUnit, if different from currently used y-axis unit (using the y-axis that this series is attached to),
							value is converted accordingly. Axis unit is not modified.

		\throws Exception when unit is not convertible or when x-value is less or equal to last x value in vector.
	*/
	void appendData( unsigned int index, double xValue, const IBK::Unit & xUnit, double yValue, const IBK::Unit & yUnit );

	/*! Clears the content. After calling this function the model has no series and all pointer are not valid.
		\warning This also resets the ENTIRE chart configuration to the defaults.
	*/
	void clear();

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

	virtual void setSeriesData(int seriesIndex, const LineInformation& value) override {
		AbstractLineSeriesModel::setSeriesData(seriesIndex, value);
	}

	/*! Returns the chart meta data stored under the given role.
		Re-implemented to prevent setting AxisEnabled data role.
		\param axisPosition Value for axis position, \see AxisPosition
		\param axisDataRole Item data role for axis data, see AxisDataRole
	*/
	virtual bool setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) override;

private:
	/*! Create the vector for converted x values based on the original ones (m_xvalues). Conversion will be done based on bottom axis settings.*/
	void convertXValues(unsigned int index);

	/*! Vector for original x values. This vector keeps the same until new data are set. It is only used as data source for converted x values.
		\note We use a vector of pointers so that adding new data series won't result in major memory copies
			when std::vector does a re-alloc.
	*/
	std::vector< IBK::UnitVector* >		m_xValues;

	/*! Vector for converted x values. This vector is changed in case of axis unit or date time change.
	   Only this vector will be used for drawing charts.
		\note We use a vector of pointers so that adding new data series won't result in major memory copies
			when std::vector does a re-alloc.
	*/
	std::vector< IBK::UnitVector* >		m_xValuesConverted;

	/*! Vector for y values.
		\note We use a vector of pointers so that adding new data series won't result in major memory copies
			when std::vector does a re-alloc.
	*/
	std::vector< IBK::UnitVector* >		m_yValues;

};

} // namespace SCI

#endif // Sci_LineSeriesContentModelH
