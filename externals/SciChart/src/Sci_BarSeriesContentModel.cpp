/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_BarSeriesContentModel.h"

#include <cstring>

#include <IBK_Exception.h>
#include <IBK_FormatString.h>
#include <IBK_UnitList.h>

namespace SCI {

BarSeriesContentModel::BarSeriesContentModel(QObject *parent) :
	AbstractBarSeriesModel(parent)
{
}

int BarSeriesContentModel::seriesCount() const {
	return m_yValues.empty() ? 0 : 1;
}

int BarSeriesContentModel::yValuesCount() const {
	if(m_yValues.empty())
		return 0;

	return (int)m_yValues[0].size();
}

const double*	BarSeriesContentModel::xValues() const {
	Q_ASSERT(!m_xValues.empty());
	return &m_xValues.m_data[0];
}

const double*	BarSeriesContentModel::yValues(unsigned int index) const{
	Q_ASSERT(index<m_yValues.size());
	Q_ASSERT(!m_yValues[index].empty());
	return &m_yValues[index].m_data[0];
}

QStringList BarSeriesContentModel::xLabels() const {
	return m_xLabels;
}

void BarSeriesContentModel::convertXValues() {

	// no data, no conversion
	if (m_xValues.empty())
		return;

	// if properly initialized, the x and y values must have a unit, even though it may not have any data yet
	Q_ASSERT(m_xValues.m_unit.id() != 0);

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
		zeroTime.setTimeSpec(Qt::UTC);
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
		if (m_xValues.size() != m_xValuesConverted.size())
			m_xValuesConverted = m_xValues;
		else {
			std::memcpy(&m_xValuesConverted[0], &m_xValues[0], m_xValues.size()*sizeof(double));
		}
		m_xValuesConverted.m_unit = m_xValues.m_unit;
		m_xValuesConverted.convert( unit );
		if (offset != 0) {
			double * x = &m_xValuesConverted.m_data[0];
			double * xLast = x + m_xValuesConverted.size();
			for (;x<xLast; ++x)
				*x += offset;
		}
	}
	catch (...) {
		// in case of mismatching units do nothing, can happen when mismatching unit was set via setUnit()
	}
}

void BarSeriesContentModel::setValues(const IBK::UnitVector& xvalues, const std::vector<IBK::UnitVector>& yvalues, const QString& name) {
	const char * const FUNC_ID = "[BarSeriesContentModel::setValues]";
	bool addSeries = m_xValues.empty();

	Q_ASSERT(!yvalues.empty());

	// get value units
	IBK::Unit xunit = xvalues.m_unit;
	IBK::Unit yunit = yvalues[0].m_unit;

	// get currently assigned axis units, may be undefined in case of unused axis
	IBK::Unit xAxisUnit;
	if(axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisDateTime).toBool()) {
		xAxisUnit = IBK::Unit("ms");
	}
	else {
		xAxisUnit = IBK::Unit(axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toInt());
	}
	IBK::Unit yAxisUnitLeft = IBK::Unit(axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toInt());

	// check if x-axis is undefined or compatible
	if (xAxisUnit.id() == 0)
		xAxisUnit = xunit; // take x-value unit for axis
	else {
		if (xAxisUnit.base_id() != xunit.base_id())
			throw IBK::Exception(IBK::FormatString("Incompatible x units: x-value unit = %1, x-axis unit = %2")
								 .arg(xunit.name()).arg(xAxisUnit.name()), FUNC_ID);
	}
	// check if y-axis is undefined or compatible
	if (yAxisUnitLeft.id() == 0)
		yAxisUnitLeft = yunit; // take y-value unit for axis
	else {
		if (yAxisUnitLeft.base_id() != yunit.base_id())
			throw IBK::Exception(IBK::FormatString("Incompatible x units: y-value unit = %1, y-axis unit = %2")
								 .arg(yunit.name()).arg(yAxisUnitLeft.name()), FUNC_ID);
	}

	if(addSeries)
		beginInsertSeries(0,0);
	else
		beginChangeSeries(0, 0);

	Q_ASSERT(xvalues.size() == yvalues.size());
	if(xvalues.empty() || yvalues[0].empty()) {
		clear();
		return;
	}

	AbstractCartesianChartModel::setAxisData(xAxisUnit.id(), AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit);
	AbstractCartesianChartModel::setAxisData(yAxisUnitLeft.id(), AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit);
	m_barChartInformation.m_barSymbolInformations.resize(yvalues[0].size());
	bool isDateTimeX = axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisDateTime).toBool();

	m_xValues = xvalues;
	m_xValuesConverted = xvalues;
	if(isDateTimeX) {
		convertXValues();
	}
	else if (xunit != xAxisUnit) {
		m_xValuesConverted.convert(xAxisUnit);
	}
	m_yValues = yvalues;
	if (yunit != yAxisUnitLeft) {
		for(IBK::UnitVector& vect : m_yValues)
			vect.convert(yAxisUnitLeft);
	}

	if(addSeries)
		endInsertSeries();
	else
		endChangeSeries();

	AbstractBarSeriesModel::setSeriesData(name, 0, AbstractBarSeriesModel::SeriesTitleText);
}

void BarSeriesContentModel::setValues(const QStringList& xLabels, const std::vector<IBK::UnitVector>& yvalues, const QString& name) {
	const char * const FUNC_ID = "[BarSeriesContentModel::setValues]";
	bool addSeries = m_xValues.empty();

	Q_ASSERT(!yvalues.empty());

	// get value units
	IBK::Unit xunit;
	IBK::Unit yunit = yvalues[0].m_unit;

	// get currently assigned axis units, may be undefined in case of unused axis
	IBK::Unit yAxisUnitLeft = IBK::Unit(axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toInt());

	// check if y-axis is undefined or compatible
	if (yAxisUnitLeft.id() == 0)
		yAxisUnitLeft = yunit; // take y-value unit for axis
	else {
		if (yAxisUnitLeft.base_id() != yunit.base_id())
			throw IBK::Exception(IBK::FormatString("Incompatible x units: y-value unit = %1, y-axis unit = %2")
								 .arg(yunit.name()).arg(yAxisUnitLeft.name()), FUNC_ID);
	}

	if(addSeries)
		beginInsertSeries(0,0);
	else
		beginChangeSeries(0, 0);

	Q_ASSERT(xLabels.size() == (int)yvalues.size());
	if(xLabels.empty() || yvalues[0].empty()) {
		clear();
		return;
	}

	AbstractCartesianChartModel::setAxisData(yAxisUnitLeft.id(), AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit);
	m_barChartInformation.m_barSymbolInformations.resize(yvalues[0].size());

	m_xLabels = xLabels;
	m_xValues.clear();
	for(int i=0; i<m_xLabels.size(); ++i)
		m_xValues.m_data.push_back(i);

	m_yValues = yvalues;
	if (yunit != yAxisUnitLeft) {
		for(IBK::UnitVector& vect : m_yValues)
			vect.convert(yAxisUnitLeft);
	}

	if(addSeries)
		endInsertSeries();
	else
		endChangeSeries();

	AbstractBarSeriesModel::setSeriesData(name, 0, AbstractBarSeriesModel::SeriesTitleText);
}

void BarSeriesContentModel::clear(){
	beginResetModel();
	clearCachedData();
	m_xValues.clear();
	m_yValues.clear();
	endResetModel();
}

QVariant BarSeriesContentModel::axisData(AxisPosition axisPosition, int axisDataRole) const {
	// special handling for unit and title
	switch (axisDataRole) {
		case AbstractCartesianChartModel::AxisEnabled: {
			switch(axisPosition) {
				case AbstractCartesianChartModel::BottomAxis: {
					return true;
				}
				case AbstractCartesianChartModel::LeftAxis: {
					return true;
				}
				case AbstractCartesianChartModel::RightAxis: {
					return QVariant();
				}
			}
		}
	}
	// all other parameter with default handling
	return AbstractCartesianChartModel::axisData(axisPosition, axisDataRole);
}

bool BarSeriesContentModel::setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) {
	if( axisPosition != BottomAxis && axisPosition != LeftAxis)
		return false;

	// all other parameter with default handling, be disallow signal axisChanged(), because in case of
	// axis unit/datetime changes, we first need the mapper to update its extractors and perform unit conversion
	bool wasBlocked = signalsBlocked();
	blockSignals(true);
	bool result = SCI::AbstractCartesianChartModel::setAxisData(value, axisPosition, axisDataRole);
	blockSignals(wasBlocked);

	if (!result || m_xValues.empty())
		return false;

	if (axisDataRole == AbstractCartesianChartModel::AxisUnit ||
			axisDataRole == AbstractCartesianChartModel::AxisDateTime ||
			axisDataRole == AbstractCartesianChartModel::AxisDateTimeZero)
	{
		if(axisPosition == AbstractCartesianChartModel::BottomAxis) {
			IBK::Unit newXUnit = IBK::Unit(axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());
			IBK::Unit oldXUnit = m_xValues.m_unit;
			if ( newXUnit.base_id() != oldXUnit.base_id() )
				return false;

			bool useDateTimeX = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisDateTime).toBool();
			bool autoscale = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisAutoScale).toBool();
			convertXValues();
			if ( !autoscale && !useDateTimeX) {

				double min = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum).toDouble();
				double max = axisData(SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum).toDouble();

				try {
					IBK::UnitList::instance().convert( oldXUnit, newXUnit, min);
					IBK::UnitList::instance().convert( oldXUnit, newXUnit, max);

					setAxisData(min, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
					setAxisData(max, SCI::AbstractCartesianChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
				}
				catch (...) {
					// can happen, not critical, just don't do anything here
				}
			}
			emit axisChanged(axisPosition, axisDataRole);
			return true;
		}
		else {
			IBK::Unit newYUnit = IBK::Unit(axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit).toInt());
			IBK::Unit oldYUnit;
			bool firstLine = true;
			for( unsigned int i=0; i<m_yValues.size(); ++i) {
				if ( newYUnit.base_id() != m_yValues[i].m_unit.base_id() )
					return false;

				if(oldYUnit.id() == 0)
					oldYUnit = m_yValues[i].m_unit;
				if ( newYUnit != m_yValues[i].m_unit ) {
					if( firstLine)
						AbstractCartesianChartModel::setAxisData(value, axisPosition, axisDataRole);
					m_yValues[i].convert(newYUnit);
					firstLine = false;
				}
			}
			if (oldYUnit.id() != 0) {

				bool autoscale = axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisAutoScale).toBool();
				if ( !autoscale ) {

					double min = axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinimum).toDouble();
					double max = axisData(SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMaximum).toDouble();

					try {
						IBK::UnitList::instance().convert( oldYUnit, newYUnit, min);
						IBK::UnitList::instance().convert( oldYUnit, newYUnit, max);

						setAxisData(min, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMinimum);
						setAxisData(max, SCI::AbstractCartesianChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisMaximum);
					}
					catch (...) {
						// can happen, not critical, just don't do anything here
					}

				}
			}
			emit axisChanged(axisPosition, axisDataRole);
			return true;
		}
	}
	emit axisChanged(axisPosition, axisDataRole);
	return true;
}

QVariant BarSeriesContentModel::seriesData(int seriesIndex, int seriesDataRole) const {
	// test if index is valid
	Q_ASSERT(seriesIndex == 0);

	switch (seriesDataRole) {
		case SeriesSize :
			return (unsigned int)m_xValues.size();
	}

	return AbstractBarSeriesModel::seriesData(seriesIndex, seriesDataRole);
}

bool BarSeriesContentModel::setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) {
	// test if index is valid
	Q_ASSERT(seriesIndex == 0);

	switch (seriesDataRole) {
		case SeriesSize : {
			return false;
		}
	}

	return AbstractBarSeriesModel::setSeriesData(value, seriesIndex, seriesDataRole);
}

} // namespace SCI
