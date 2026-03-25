/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_AbstractVectorFieldSeriesModel.h"

#include <limits>
#include <cmath>

#include <IBK_InputOutput.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace SCI {

AbstractVectorFieldSeriesModel::AbstractVectorFieldSeriesModel(QObject *parent) :
	AbstractCartesianChartModel(parent),
	m_autoScale(true),
	m_minimumZ(std::numeric_limits<double>::quiet_NaN()),
	m_maximumZ(std::numeric_limits<double>::quiet_NaN()),
	m_colorBarVisible(true),
	m_rasterEnabled(true),
	m_rasterSize(15),
	m_coloredArrowsEnabled(true),
	m_uniformArrowColor(Qt::black),
	m_indicatorOrigin(2),
	m_thinArrow(true),
	m_maxArrowLength(50),
	m_minArrowLength(0),
	m_arrowScaleFactor(1)
{
	setData(false, LegendVisible);
}


bool AbstractVectorFieldSeriesModel::setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) {
	bool success = SCI::AbstractCartesianChartModel::setAxisData(value, axisPosition, axisDataRole);
	if (!success)
		return false;
	switch (axisDataRole) {
		case SCI::AbstractCartesianChartModel::AxisUnit :
			if (seriesCount() > 0)
				emit seriesViewChanged(0, 0, RasterSize); // signal change of raster size, so that raster in plot can be updated
		break;
		default: ;
	}
	return true;
}


QVariant AbstractVectorFieldSeriesModel::seriesData(int seriesIndex, int seriesDataRole) const {
	// test if index is valid
	if( seriesIndex < 0 || seriesIndex >= static_cast<int>(seriesCount()) )
		return QVariant();

	switch (seriesDataRole) {
		case ColorMap : {
			QVariant res;
			res.setValue(m_colorMap);
			return res;
		}

		case AutoScale: {
			return m_autoScale;
		}

		case MinimumZ: {
			return m_minimumZ;
		}

		case MaximumZ: {
			return m_maximumZ;
		}

		case DataMinimumZ: {
			unsigned int vectorSampleCount = this->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleCount).toUInt();
			const double* vx = static_cast<const double*>(this->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleVXArray).value<void*>());
			const double* vy = static_cast<const double*>(this->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleVYArray).value<void*>());
			// no data available yet?
			if (vectorSampleCount == 0 || vx == NULL || vy == NULL)
				return 0;

			double minZ = std::numeric_limits<double>::quiet_NaN();
			for (unsigned int i=0; i<vectorSampleCount; ++i) {
				// compute magnitude
				double mag = vx[i]*vx[i] + vy[i]*vy[i];

				// if minZ is still nan, store value
				if (minZ != minZ)
					minZ = mag;
				else if (mag < minZ)
					minZ = mag;
			}

			// round down
			return std::floor( std::sqrt(minZ) );
		}

		case DataMaximumZ: {

			unsigned int vectorSampleCount = this->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleCount).toUInt();
			const double* vx = static_cast<const double*>(this->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleVXArray).value<void*>());
			const double* vy = static_cast<const double*>(this->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleVYArray).value<void*>());
			// no data available yet?
			if (vectorSampleCount == 0 || vx == NULL || vy == NULL)
				return 0;

			double maxZ = std::numeric_limits<double>::quiet_NaN();
			for (unsigned int i=0; i<vectorSampleCount; ++i) {
				// compute magnitude
				double mag = vx[i]*vx[i] + vy[i]*vy[i];

				// if maxZ is still nan, store value
				if (maxZ != maxZ)
					maxZ = mag;
				else if (mag > maxZ)
					maxZ = mag;
			}

			// round up
			return std::ceil( std::sqrt(maxZ) );
		}

		case ZValueRange: {
			if ( m_autoScale ) {
				// when we use auto-scale, we need the current data minimum and maximum values
				double dataMin = seriesData(0, AbstractVectorFieldSeriesModel::DataMinimumZ).toDouble();
				double dataMax = seriesData(0, AbstractVectorFieldSeriesModel::DataMaximumZ).toDouble();
				// in case of invalid data (NAN) we simply set a default 0..1 interval.
				// in case of missing/invalid data, ensure a valid interval
				if (dataMin >= dataMax)
					dataMax = dataMin + 1;
				Range range(dataMin, dataMax);
				QVariant res;
				res.setValue(range);
				return res;
			}
			// user-scale, apply rules for selecting meaningful min/max values
			double userMin = m_minimumZ;
			double userMax = m_maximumZ;

			// if user has not given valid min value, use dataMin
			if (userMin != userMin)
				userMin = seriesData(0, AbstractVectorFieldSeriesModel::DataMinimumZ).toDouble();
			if (userMax != userMax)
				userMax = seriesData(0, AbstractVectorFieldSeriesModel::DataMaximumZ).toDouble();
			// ensure that userMax is larger than userMin
			if (userMin >= userMax)
				userMax = userMin + 1;
			Range range(userMin, userMax);
			QVariant res;
			res.setValue(range);
			return res;
		}

		case ColorBarVisible:			return m_colorBarVisible;
		case RasterEnabled: 			return m_rasterEnabled;
		case RasterSize:				return m_rasterSize;
		case ColoredArrowsEnabled:		return m_coloredArrowsEnabled;
		case UniformArrowColor:			return m_uniformArrowColor;
		case IndicatorOrigin:			return m_indicatorOrigin;
		case ThinArrow:					return m_thinArrow;
		case MaxArrowLength:			return m_maxArrowLength;
		case MinArrowLength:			return m_minArrowLength;
		case ArrowScaleFactor:			return m_arrowScaleFactor;

	}

	return QVariant();
}


bool AbstractVectorFieldSeriesModel::setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) {
	const char * const FUNC_ID = "[AbstractVectorPlotSeriesModel::setSeriesData]";
	// test if index is valid
	if ( seriesIndex < 0 || seriesIndex > 1 ) {
		IBK::IBK_Message( IBK::FormatString("Invalid series index #%1.").arg(seriesIndex), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	switch (seriesDataRole) {
		case ColorMap : {
			if( value.canConvert<SCI::ColorMap>()) {
				m_colorMap = value.value<SCI::ColorMap>();
			}
		} break;

		case AutoScale: {
			m_autoScale = value.toBool();
			seriesDataRole = ZValueRange;
		} break;
		case MinimumZ: {
			m_minimumZ = value.toDouble();
			seriesDataRole = ZValueRange;
		} break;
		case MaximumZ: {
			m_maximumZ = value.toDouble();
			seriesDataRole = ZValueRange;
		} break;
		case ColorBarVisible: {
			m_colorBarVisible = value.toBool();
			emit axisChanged(AbstractChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
		} break;
		case RasterEnabled:						m_rasterEnabled = value.toBool();			break;
		case RasterSize:						m_rasterSize = (int)value.toDouble();			break;
		case ColoredArrowsEnabled: {
			m_coloredArrowsEnabled = value.toBool();
			emit axisChanged(AbstractChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
		} break;
		case UniformArrowColor: {
			if (!value.canConvert<QColor>())
				return false;
			m_uniformArrowColor = value.value<QColor>();
		} break;
		case IndicatorOrigin:					m_indicatorOrigin = value.toInt();			break;
		case ThinArrow:							m_thinArrow = value.toBool();				break;
		case MaxArrowLength:					m_maxArrowLength = value.toDouble();		break;
		case MinArrowLength:					m_minArrowLength = value.toDouble();		break;
		case ArrowScaleFactor:					m_arrowScaleFactor = value.toDouble();		break;
		default:
			IBK::IBK_Message( IBK::FormatString("Invalid data role #%1.").arg(seriesDataRole), IBK::MSG_ERROR, FUNC_ID);
			return false;
	}

	// only emit change signal, if we have a series
	if (seriesCount() > 0)
		emit seriesViewChanged(0, 0, seriesDataRole);
	return true;
}


void AbstractVectorFieldSeriesModel::beginInsertSeries ( int first, int last) {
	// prepare for insertion of series
	AbstractChartModel::beginInsertSeries(first, last);
}


void AbstractVectorFieldSeriesModel::beginRemoveSeries ( int first, int last ) {
	// prepare for removal of series
	AbstractChartModel::beginRemoveSeries(first, last);
}


void AbstractVectorFieldSeriesModel::clearCachedData() {
	/// \todo clear other data
	AbstractCartesianChartModel::clearCachedData();
}


void AbstractVectorFieldSeriesModel::writeSeriesBinary(std::ostream& /*out*/) const {
	/// \todo implement
}


void AbstractVectorFieldSeriesModel::readSeriesBinary( std::istream& /*in*/ ) {
	/// \todo implement
}


void AbstractVectorFieldSeriesModel::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("VectorPlotProperties");
	parent->LinkEndChild(e);

	if ( m_writeAll || m_colorBarVisible != true)
		TiXmlElement::appendSingleAttributeElement( e, "ColorBarVisible", NULL, std::string(), IBK::val2string<bool>(m_colorBarVisible) );

	if ( m_writeAll || m_autoScale != true)
		TiXmlElement::appendSingleAttributeElement( e, "AutoScale", NULL, std::string(), IBK::val2string<bool>(m_autoScale) );

	if ( m_writeAll || m_minimumZ == m_minimumZ) {  // is not NaN
		if (m_writeAll && m_minimumZ != m_minimumZ)
			TiXmlElement::appendSingleAttributeElement( e, "MinimumZ", NULL, std::string(), "NaN" );
		else
			TiXmlElement::appendSingleAttributeElement( e, "MinimumZ", NULL, std::string(), IBK::val2string<double>(m_minimumZ) );
	}

	if ( m_writeAll || m_maximumZ == m_maximumZ) {  // is not NaN
		if (m_writeAll && m_maximumZ != m_maximumZ)
			TiXmlElement::appendSingleAttributeElement( e, "MaximumZ", NULL, std::string(), "NaN" );
		else
			TiXmlElement::appendSingleAttributeElement( e, "MaximumZ", NULL, std::string(), IBK::val2string<double>(m_maximumZ) );
	}

	if ( m_writeAll || m_rasterEnabled == false)
		TiXmlElement::appendSingleAttributeElement( e, "RasterEnabled", NULL, std::string(), IBK::val2string<bool>(m_rasterEnabled) );
	TiXmlElement::appendSingleAttributeElement( e, "RasterSize", NULL, std::string(), IBK::val2string(m_rasterSize) );
	if ( m_writeAll || m_coloredArrowsEnabled == false)
		TiXmlElement::appendSingleAttributeElement( e, "ColoredArrowsEnabled", NULL, std::string(), IBK::val2string<bool>(m_coloredArrowsEnabled) );
	if ( m_writeAll || m_uniformArrowColor != Qt::black)
		TiXmlElement::appendSingleAttributeElement( e, "UniformArrowColor", NULL, std::string(), m_uniformArrowColor.name().toStdString() );
	if ( m_writeAll || m_indicatorOrigin != 2)
		TiXmlElement::appendSingleAttributeElement( e, "IndicatorOrigin", NULL, std::string(), IBK::val2string<int>(m_indicatorOrigin) );
	if ( m_writeAll || !m_thinArrow)
		TiXmlElement::appendSingleAttributeElement( e, "ThinArrow", NULL, std::string(), IBK::val2string<bool>(m_thinArrow) );
	if (m_writeAll || m_maxArrowLength != 50)
		TiXmlElement::appendSingleAttributeElement( e, "MaximumArrowLength", NULL, std::string(), IBK::val2string<double>(m_maxArrowLength) );
	if (m_writeAll || m_minArrowLength != 0)
		TiXmlElement::appendSingleAttributeElement( e, "MinimumArrowLength", NULL, std::string(), IBK::val2string<double>(m_minArrowLength) );
	if (m_writeAll || m_arrowScaleFactor != 1)
		TiXmlElement::appendSingleAttributeElement( e, "ArrowScaleFactor", NULL, std::string(), IBK::val2string<double>(m_arrowScaleFactor) );

	m_colorMap.writeXML( e );

	AbstractCartesianChartModel::writeXML(parent);
}


void AbstractVectorFieldSeriesModel::readXML(const TiXmlElement * element) {

	const char * const FUNC_ID = "[AbstractVectorPlotSeriesModel::readXML]";
	const TiXmlElement * xmlElem = element->FirstChildElement( "VectorPlotProperties" );

	if (xmlElem != NULL) {

		try {

			const TiXmlElement * colorBar = xmlElem->FirstChildElement("ColorBar");
			if (colorBar)
				m_colorBarVisible = IBK::string2val<bool>(colorBar->GetText());

			const TiXmlElement * autoScale = xmlElem->FirstChildElement("AutoScale");
			if (autoScale)
				m_autoScale = IBK::string2val<bool>(autoScale->GetText());

			const TiXmlElement * min = xmlElem->FirstChildElement("MinimumZ");
			if (min) {
				std::string textValue = min->GetText();
				if (textValue == "NaN")
					m_minimumZ = std::numeric_limits<double>::quiet_NaN();
				else
					m_minimumZ = IBK::string2val<double>(textValue);
			}

			const TiXmlElement * max = xmlElem->FirstChildElement("MaximumZ");
			if (max) {
				std::string textValue = max->GetText();
				if (textValue == "NaN")
					m_maximumZ = std::numeric_limits<double>::quiet_NaN();
				else
					m_maximumZ = IBK::string2val<double>(textValue);
			}

			const TiXmlElement * rasterEnabled = xmlElem->FirstChildElement("RasterEnabled");
			if (rasterEnabled)
				m_rasterEnabled = IBK::string2val<bool>(rasterEnabled->GetText());

			const TiXmlElement * rasterSize = xmlElem->FirstChildElement("RasterSize");
			if (rasterSize)
				m_rasterSize = IBK::string2val<int>(rasterSize->GetText());

			const TiXmlElement * coloredArrowsEnabled = xmlElem->FirstChildElement("ColoredArrowsEnabled");
			if (coloredArrowsEnabled)
				m_coloredArrowsEnabled = IBK::string2val<bool>(coloredArrowsEnabled->GetText());

			const TiXmlElement * uniformArrowColor = xmlElem->FirstChildElement("UniformArrowColor");
			if (uniformArrowColor) {
				QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
				col = QColor::fromString(uniformArrowColor->GetText());
#else
				col.setNamedColor(uniformArrowColor->GetText());
#endif
				m_uniformArrowColor = col;
			}

			const TiXmlElement * indicatorOrigin = xmlElem->FirstChildElement("IndicatorOrigin");
			if (indicatorOrigin)
				m_indicatorOrigin = IBK::string2val<int>(indicatorOrigin->GetText());

			const TiXmlElement * thinArrow = xmlElem->FirstChildElement("ThinArrow");
			if (thinArrow)
				m_thinArrow = IBK::string2val<bool>(thinArrow->GetText());

			const TiXmlElement * maxArrowLength = xmlElem->FirstChildElement("MaximumArrowLength");
			if (maxArrowLength)
				m_maxArrowLength = IBK::string2val<double>(maxArrowLength->GetText());

			const TiXmlElement * minArrowLength = xmlElem->FirstChildElement("MinimumArrowLength");
			if (minArrowLength)
				m_minArrowLength = IBK::string2val<double>(minArrowLength->GetText());

			const TiXmlElement * arrowScaleFactor = xmlElem->FirstChildElement("ArrowScaleFactor");
			if (arrowScaleFactor)
				m_arrowScaleFactor = IBK::string2val<double>(arrowScaleFactor->GetText());

			m_colorMap.readXML( xmlElem );

		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading chart settings."), FUNC_ID);
		}
	}

	AbstractCartesianChartModel::readXML(element);
}


} // namespace SCI
