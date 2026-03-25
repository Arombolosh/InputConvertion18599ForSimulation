/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_LineSeriesContentModel.h"

#include <IBK_UnitList.h>

#include "Sci_PlotCurve.h"

namespace SCI {

LineSeriesContentModel::LineSeriesContentModel(QObject *parent) :
	AbstractLineSeriesModel(parent)
{
}

LineSeriesContentModel::~LineSeriesContentModel() {
	// we require that the model is first removed from any charts, so we do not
	// have to worry about any signal connections

	// clean up allocated memory
	for( std::vector< IBK::UnitVector* >::iterator it=m_xValues.begin(); it!=m_xValues.end(); ++it) {
		delete *it;
	}
	for( std::vector< IBK::UnitVector* >::iterator it=m_xValuesConverted.begin(); it!=m_xValuesConverted.end(); ++it) {
		delete *it;
	}
	for( std::vector< IBK::UnitVector* >::iterator it=m_yValues.begin(); it!=m_yValues.end(); ++it) {
		delete *it;
	}
}


void LineSeriesContentModel::clear() {
	beginResetModel();
	clearCachedData();
	for( std::vector<IBK::UnitVector*>::iterator it=m_xValues.begin(); it!=m_xValues.end(); ++it) {
		delete *it;
	}
	m_xValues.clear();
	for( std::vector< IBK::UnitVector* >::iterator it=m_xValuesConverted.begin(); it!=m_xValuesConverted.end(); ++it) {
		delete *it;
	}
	m_xValuesConverted.clear();
	for( std::vector< IBK::UnitVector* >::iterator it=m_yValues.begin(); it!=m_yValues.end(); ++it) {
		delete *it;
	}
	m_yValues.clear();
	endResetModel();
}


int LineSeriesContentModel::seriesCount() const {
	return (int)m_xValues.size();
}


QVariant LineSeriesContentModel::seriesData(int seriesIndex, int seriesDataRole) const {
	// test if index is valid
	Q_ASSERT(seriesIndex >= 0 && seriesIndex < static_cast<int>(m_xValues.size()));

	switch (seriesDataRole) {
		case SeriesSize :
			return static_cast<int>(m_xValues[seriesIndex]->size());
	}

	return AbstractLineSeriesModel::seriesData(seriesIndex, seriesDataRole);
}


bool LineSeriesContentModel::setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) {
	// test if index is valid
	Q_ASSERT(seriesIndex >= 0 && seriesIndex <= static_cast<int>(m_xValues.size()));

	switch (seriesDataRole) {
		case SeriesSize : {
			return false;
		}
	}

	return AbstractLineSeriesModel::setSeriesData(value, seriesIndex, seriesDataRole);
}

void LineSeriesContentModel::convertXValues(unsigned int index) {
	Q_ASSERT(index < m_xValues.size());


	// no data, no conversion
	if (m_xValues[index]->empty())
		return;

	// if properly initialized, the x and y values must have a unit, even though it may not have any data yet
	IBK_ASSERT(m_xValues[index]->m_unit.id() != 0);

	bool useDateTimeX = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTime).toBool();

	IBK::Unit unit;
	double offset;
	if ( useDateTimeX ) {
		// DateTime axis always uses ms
		unit = IBK::Unit("ms");
		QDateTime zeroTime = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeZero).toDateTime();
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		zeroTime.setTimeZone(QTimeZone("UTC"));
#else
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		zeroTime.setTimeZone(QTimeZone("UTC"));
#else
		zeroTime.setTimeSpec(Qt::UTC);
#endif
#endif
		offset = zeroTime.toMSecsSinceEpoch();
	}
	else {
		unit = IBK::Unit(axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());
		offset = 0;
	}

	Q_ASSERT(unit.id() != 0);

	// convert to x-coordinate target unit and update unit again
	try {
		if (m_xValues[index]->size() != m_xValuesConverted[index]->size())
			*m_xValuesConverted[index] = *m_xValues[index];
		else {
			std::memmove(&(*m_xValuesConverted[index])[0], &(*m_xValues[index])[0], m_xValues[index]->size()*sizeof(double));
		}
		m_xValuesConverted[index]->m_unit = m_xValues[index]->m_unit;
		m_xValuesConverted[index]->convert( unit );
		if (offset != 0) {
			double * x = &m_xValuesConverted[index]->m_data[0];
			double * xLast = x + m_xValuesConverted[index]->size();
			for (;x<xLast; ++x)
				*x += offset;
		}
	}
	catch (...) {
		// in case of mismatching units do nothing, can happen when mismatching unit was set via setUnit()
	}
}

bool LineSeriesContentModel::setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) {
	// all other parameter with default handling, be disallow signal axisChanged(), because in case of
	// axis unit/datetime changes, we first need the mapper to update its extractors and perform unit conversion
	bool wasBlocked = signalsBlocked();
	blockSignals(true);
	bool result = SCI::AbstractCartesianChartModel::setAxisData(value, axisPosition, axisDataRole);
	blockSignals(wasBlocked);

	// Only make conversions if data are present
	if (result && !m_xValues.empty()) {
		// unit change triggers update of target unit in extractors, followed by a model update
		if (axisDataRole == AbstractCartesianChartModel::AxisUnit ||
			axisDataRole == AbstractCartesianChartModel::AxisDateTime ||
			axisDataRole == AbstractCartesianChartModel::AxisDateTimeZero)
		{
			IBK::Unit xUnit = IBK::Unit(axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());
			IBK::Unit y1Unit = IBK::Unit(axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());
			IBK::Unit y2Unit = IBK::Unit(axisData(SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());
			bool useDateTimeX = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTime).toBool();

			// store old x unit, for axis limit conversion
			IBK::Unit oldXUnit = m_xValues[0]->m_unit;
			IBK::Unit oldY1Unit, oldY2Unit;

			for (unsigned int i=0, size=(unsigned int)m_xValues.size(); i<size; ++i) {

				if ( seriesData(i, SCI::AbstractLineSeriesModel::SeriesLeftYAxis).toBool()) {
					oldY1Unit = m_yValues[i]->m_unit;
				}
				else {
					oldY2Unit = m_yValues[i]->m_unit;
				}

				convertXValues(i);

				// values assigned to y1 axis?
				if (seriesData(i, SCI::AbstractLineSeriesModel::SeriesLeftYAxis).toBool()) {
					m_yValues[i]->convert(y1Unit);
				}
				else {
					// must be y2 axis then
					m_yValues[i]->convert(y2Unit);
				}
			}
			// scale axis limits
			if (oldXUnit.id() != 0) {
				bool autoscale = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale).toBool();
				if ( !autoscale && !useDateTimeX) {

					double min = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum).toDouble();
					double max = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum).toDouble();

					try {
						IBK::UnitList::instance().convert( oldXUnit, xUnit, min);
						IBK::UnitList::instance().convert( oldXUnit, xUnit, max);

						setAxisData(min, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
						setAxisData(max, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
					}
					catch (...) {
						// can happen, not critical, just don't do anything here
					}
				}
			}
			if (oldY1Unit.id() != 0) {

				bool autoscale = axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale).toBool();
				if ( !autoscale ) {

					double min = axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinimum).toDouble();
					double max = axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMaximum).toDouble();

					try {
						IBK::UnitList::instance().convert( oldY1Unit, y1Unit, min);
						IBK::UnitList::instance().convert( oldY1Unit, y1Unit, max);

						setAxisData(min, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
						setAxisData(max, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
					}
					catch (...) {
						// can happen, not critical, just don't do anything here
					}

				}
			}
			if (oldY2Unit.id() != 0) {

				bool autoscale = axisData(SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisAutoScale).toBool();
				if ( !autoscale ) {

					double min = axisData(SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMinimum).toDouble();
					double max = axisData(SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMaximum).toDouble();

					try {
						IBK::UnitList::instance().convert( oldY2Unit, y2Unit, min);
						IBK::UnitList::instance().convert( oldY2Unit, y2Unit, max);

						setAxisData(min, SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
						setAxisData(max, SCI::AbstractCartesianChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
					}
					catch (...) {
						// can happen, not critical, just don't do anything here
					}

				}
			}
		}
		// finally emit the axisChanged signal to notify listing view
		emit axisChanged(axisPosition, axisDataRole);
		return true;
	}
	return false;
}

const double*	LineSeriesContentModel::xValues(unsigned int index) const {
	Q_ASSERT(index<m_xValuesConverted.size());
	return &(*m_xValuesConverted[index])[0];
}


const double*	LineSeriesContentModel::yValues(unsigned int index) const {
	Q_ASSERT(index<m_yValues.size());
	return &(*m_yValues[index])[0];
}


void LineSeriesContentModel::setValues(unsigned int index, const IBK::UnitVector& xvalues,
									   const IBK::UnitVector& yvalues)
{
	/// \todo fix this
	Q_ASSERT(index<m_yValues.size());
	beginChangeSeries(index, index);
	m_xValues[index]->m_data = xvalues.m_data;
	m_xValuesConverted[index]->m_data = xvalues.m_data;
	convertXValues(index);
	m_yValues[index]->m_data = yvalues.m_data;
	endChangeSeries();
}


void LineSeriesContentModel::setYValues(unsigned int index, const double * yValues) {
	/// \todo fix this
	Q_ASSERT(index<m_yValues.size());
	beginChangeSeries(index, index);
	std::size_t s = m_yValues[index]->size();
	double * targetMem = &(m_yValues[index]->m_data[0]);
	std::memcpy(targetMem, yValues, s*sizeof(double));
	// signal chart that series has changed
	endChangeSeries();
}


void LineSeriesContentModel::addDataSet(const IBK::UnitVector& xvalues,
										const IBK::UnitVector& yvalues,
										const QString& name)
{
	const char * const FUNC_ID = "[LineSeriesContentModel::addDataSet]";

	// units in both vectors must be valid
	if (xvalues.m_unit.id() == 0)
		throw IBK::Exception("Undefined x-unit in xvalues vector.", FUNC_ID);
	if (yvalues.m_unit.id() == 0)
		throw IBK::Exception("Undefined y-unit in yvalues vector.", FUNC_ID);

	// get value units
	IBK::Unit xunit = xvalues.m_unit;
	IBK::Unit yunit = yvalues.m_unit;

	// get currently assigned axis units, may be undefined in case of unused axis
	IBK::Unit xAxisUnit;
	if(axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisDateTime).toBool()) {
		xAxisUnit = IBK::Unit("ms");
	}
	else {
		xAxisUnit = IBK::Unit(axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toInt());
	}
	IBK::Unit yAxisUnitLeft = IBK::Unit(axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toInt());
	IBK::Unit yAxisUnitRight = IBK::Unit(axisData(AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisUnit).toInt());

	// check if x-axis is undefined or compatible
	if (xAxisUnit.id() == 0)
		xAxisUnit = xunit; // take x-value unit for axis
	else {
		if (xAxisUnit.base_id() != xunit.base_id())
			throw IBK::Exception(IBK::FormatString("Incompatible x units: x-value unit = %1, x-axis unit = %2")
								 .arg(xunit.name()).arg(xAxisUnit.name()), FUNC_ID);
	}

	bool leftYAxis = true; // assume left axis

	// check if y1-axis is undefined, and if yes, assign series to this axis
	if (yAxisUnitLeft.id() == 0) {
		yAxisUnitLeft = yunit;
	}
	else {
		if (yAxisUnitLeft.base_id() != yunit.base_id()) { // left y axis compatible?
			if (yAxisUnitRight.id() == 0) { // right y axis undefined
				yAxisUnitRight = yunit;
				leftYAxis = false;
			}
			else {
				if (yAxisUnitRight.base_id() != yunit.base_id()) { // right y axis compatible?
					throw IBK::Exception(IBK::FormatString("Incompatible y unit: y-value unit = %1 cannot be converted "
														   "to either of the left y axis (unit = %2) or right y axis (unit = %3)")
										 .arg(yunit.name()).arg(yAxisUnitLeft.name()).arg(yAxisUnitRight.name()), FUNC_ID);
				}
				leftYAxis = false;
			}
		}
	}


	// at this point we start modifying the chart

	// first update the axis properties
	bool isDateTimeX = axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisDateTime).toBool();
//	if(!isDateTimeX)
		AbstractCartesianChartModel::setAxisData(xAxisUnit.id(), AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit);
	AbstractCartesianChartModel::setAxisData(yAxisUnitLeft.id(), AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit);
	AbstractCartesianChartModel::setAxisData(yAxisUnitRight.id(), AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisUnit);


	beginInsertSeries((int)m_xValues.size(), (int)m_xValues.size()); // m_xValues.size() == seriesCount() --> add one series with this index
	// now we have one more line information structure to set model parameters

	// add xvalues
	m_xValues.push_back(new IBK::UnitVector(xvalues));
	m_xValuesConverted.push_back(new IBK::UnitVector(xvalues));
	if(isDateTimeX) {
		convertXValues((unsigned int)(m_xValues.size() - 1));
	}
	else if (xunit != xAxisUnit) {
		m_xValuesConverted.back()->convert(xAxisUnit);
	}
	// add yvalues
	m_yValues.push_back(new IBK::UnitVector(yvalues));
	if (leftYAxis) {
		if (yunit != yAxisUnitLeft)
			m_yValues.back()->convert(yAxisUnitLeft);
	}
	else {
		if (yunit != yAxisUnitRight)
			m_yValues.back()->convert(yAxisUnitRight);
	}

	int seriesIndex = (int)m_xValues.size()-1;

	// disallow sending seriesViewChanged() signal
	blockSignals(true);

	// convenience configuration of chart series, note that each call may emit the seriesViewChanged signal
	// and trigger a view update
	AbstractLineSeriesModel::setSeriesData(name, seriesIndex, AbstractLineSeriesModel::SeriesTitleText);
	AbstractLineSeriesModel::setSeriesData(leftYAxis, seriesIndex, AbstractLineSeriesModel::SeriesLeftYAxis);
	AbstractLineSeriesModel::setSeriesData(AbstractLineSeriesModel::colorFromIndex(seriesIndex), seriesIndex, AbstractLineSeriesModel::SeriesColor);
	AbstractLineSeriesModel::setSeriesData(SCI::PlotCurve::Lines, seriesIndex, AbstractLineSeriesModel::SeriesLineStyle);
	blockSignals(false);

	endInsertSeries(); // this will emit the seriesInserted() signal which in turn calls
					   // SCI::Chart::insertLineSeriesFromModel() for the newly inserted series
}

void LineSeriesContentModel::removeDataSet(unsigned int index) {
	const char * const FUNC_ID = "[LineSeriesContentModel::removeDataSet]";
	Q_ASSERT((int)m_yValues.size() == seriesCount());

	if (index >= m_yValues.size())
		throw IBK::Exception( IBK::FormatString("Series index %1 out of range [seriesCount=%2]").arg(index).arg(seriesCount()), FUNC_ID);

	int seriesIndex = static_cast<int>(index);
	beginRemoveSeries(seriesIndex, seriesIndex);
	m_xValues.erase(m_xValues.begin() + index);
	m_xValuesConverted.erase(m_xValuesConverted.begin() + index);
	m_yValues.erase(m_yValues.begin() + index);
	endRemoveSeries();
}

void LineSeriesContentModel::removeAllDataSets() {
	int scount = seriesCount();
	if(scount == 0)
		return;

	Q_ASSERT((int)m_yValues.size() == scount);

	beginRemoveSeries(0, scount-1);
	m_xValues.clear();
	m_xValuesConverted.clear();
	m_yValues.clear();
	endRemoveSeries();
}


void LineSeriesContentModel::appendData(unsigned int index, double xValue, const IBK::Unit & xUnit, double yValue, const IBK::Unit & yUnit ){

	const char* const FUNC_ID = "[LineSeriesContentModel::appendData]";
	Q_ASSERT((int)m_yValues.size() == seriesCount());

	if (index>=m_yValues.size())
		throw IBK::Exception( IBK::FormatString("Series index %1 out of range [seriesCount=%2]").arg(index).arg(seriesCount()), FUNC_ID);

	beginChangeSeries(index, index);

	IBK::UnitVector* xvect = m_xValues[index];
	IBK::UnitVector* xvectConverted = m_xValuesConverted[index];
	IBK::UnitVector* yvect = m_yValues[index];

	if(xUnit.base_id() != xvect->m_unit.base_id())
		throw IBK::Exception("X value unit not compatible", FUNC_ID);

	if(yUnit.base_id() != yvect->m_unit.base_id())
		throw IBK::Exception("Y value unit not compatible", FUNC_ID);


	double xValueConverted = xValue;
	if( axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisDateTime).toBool()) {
		IBK::UnitList::instance().convert(xUnit, xvect->m_unit, xValueConverted);
		QDateTime zeroTime = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTimeZero).toDateTime();
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
		zeroTime.setTimeZone(QTimeZone("UTC"));
#else
		zeroTime.setTimeSpec(Qt::UTC);
#endif
		xValueConverted += zeroTime.toMSecsSinceEpoch();
	}
	else {
		IBK::UnitList::instance().convert(xUnit, xvect->m_unit, xValueConverted);
	}
	IBK::UnitList::instance().convert(yUnit, yvect->m_unit, yValue);

	if (!xvect->m_data.empty()) {
		if (xValue <= xvect->m_data.back())
			throw IBK::Exception( IBK::FormatString("X value is not monotonically increasing (last x value = %1, new value = %2)")
								  .arg(xvect->m_data.back())
								  .arg(xValue), FUNC_ID);
	}
	xvect->m_data.push_back(xValue);
	xvectConverted->m_data.push_back(xValueConverted);
	yvect->m_data.push_back(yValue);

	// signal chart that series has changed
	endChangeSeries();
}

} // namespace SCI
