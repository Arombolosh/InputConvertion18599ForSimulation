/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_AbstractColorGridSeriesModel.h"

#include <limits>
#include <cmath>

#include <IBK_InputOutput.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

#include "Sci_PlotSpectrogram.h"

namespace SCI {

AbstractColorGridSeriesModel::AbstractColorGridSeriesModel(QObject *parent) :
	AbstractCartesianChartModel(parent),
	m_autoScale(true),
	m_minimumZ(std::numeric_limits<double>::quiet_NaN()),
	m_maximumZ(std::numeric_limits<double>::quiet_NaN()),
	m_interpolationMode(1), // banded
	m_displayMode(1), // spectrogram
	m_colorBarVisible(true),
	m_contourInterval(std::numeric_limits<double>::quiet_NaN()),
	m_contourLabelPlacement(PlotSpectrogram::AT_MAX_Y),
	m_contourIntervalColor(false)
{
}


QVariant AbstractColorGridSeriesModel::seriesData(int seriesIndex, int seriesDataRole) const {
	// test if index is valid
	if( seriesIndex < 0 || seriesIndex >= static_cast<int>(seriesCount()) )
		return QVariant();

	switch (seriesDataRole) {
		case ColorMap : {
			QVariant res;
			res.setValue(m_colorMap);
			return res;
		}

		case AutoScale:				return m_autoScale;
		case MinimumZ:				return m_minimumZ;
		case MaximumZ:				return m_maximumZ;

		case DataMinimumZ: {
			unsigned int xValueSize = this->seriesData(0, AbstractColorGridSeriesModel::XValueSize).toUInt();
			unsigned int yValueSize = this->seriesData(0, AbstractColorGridSeriesModel::YValueSize).toUInt();
			const double* values = static_cast<const double*>(this->seriesData(0, AbstractColorGridSeriesModel::ValueMatrix).value<void*>());
			// no data available yet?
			if (values == NULL || xValueSize == 0 || yValueSize == 0)
				return 0;

			// Note: cannot use std::min_element() because of possible nan values in range
			double minZ = std::numeric_limits<double>::quiet_NaN();
			for (unsigned i=0, size=xValueSize * yValueSize; i<size; ++i) {
				// check if value is nan and continue if so
				double val = values[i];
				if (val != val)
					continue;
				// if minZ is still nan, store value
				if (minZ != minZ)
					minZ = val;
				else if (values[i] < minZ)
					minZ = values[i];
			}

			// if range only holds nan, return 0
			if (minZ != minZ)
				return 0;

			// round down
			return std::floor( minZ );
		}

		case DataMaximumZ: {
			int xValueSize = this->seriesData(0, AbstractColorGridSeriesModel::XValueSize).toInt();
			int yValueSize = this->seriesData(0, AbstractColorGridSeriesModel::YValueSize).toInt();
			const double* values = static_cast<const double*>(this->seriesData(0, AbstractColorGridSeriesModel::ValueMatrix).value<void*>());
			// no data available yet?
			if (values == NULL || xValueSize == 0 || yValueSize == 0)
				return 0;

			// Note: cannot use std::min_element() because of possible nan values in range
			double maxZ = std::numeric_limits<double>::quiet_NaN();
			for (unsigned i=0, size=xValueSize * yValueSize; i<size; ++i) {
				// check if value is nan and continue if so
				double val = values[i];
				if (val != val)
					continue;
				// if maxZ is still nan, store value
				if (maxZ != maxZ)
					maxZ = val;
				else if (values[i] > maxZ)
					maxZ = values[i];
			}

			// if range only holds nan, return 0
			if (maxZ != maxZ)
				return 0;

			// round down
			return std::ceil( maxZ );
		}

		case ZValueRange: {
			if ( m_autoScale ) {
				// when we use auto-scale, we need the current data minimum and maximum values
				double dataMin = seriesData(0, AbstractColorGridSeriesModel::DataMinimumZ).toDouble();
				double dataMax = seriesData(0, AbstractColorGridSeriesModel::DataMaximumZ).toDouble();
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
				userMin = seriesData(0, AbstractColorGridSeriesModel::DataMinimumZ).toDouble();
			if (userMax != userMax)
				userMax = seriesData(0, AbstractColorGridSeriesModel::DataMaximumZ).toDouble();
			// ensure that userMax is larger than userMin
			if (userMin >= userMax)
				userMax = userMin + 1;
			Range range(userMin, userMax);
			QVariant res;
			res.setValue(range);
			return res;
		}

		case InterpolationMode: 			return m_interpolationMode;
		case DisplayMode:					return m_displayMode;
		case ColorBarVisible:				return m_colorBarVisible;

		case ContourInterval: {

			// when contourInterval is NAN this means "undefined"
			// and we treat this as:
			// - If we have a valid color map, compute a contour interval that
			//   matches the color bands of the color map.
			// - If we do not have a valid color map, split the value range into
			//   10 contours.
			// - If the range is also invalid, just return 1.
			if ( m_contourInterval != m_contourInterval ) {

				// get color map, if any
				QVariant val = seriesData(0, AbstractColorGridSeriesModel::ColorMap);
				Q_ASSERT( val.canConvert<SCI::ColorMap>() );
				SCI::ColorMap cmap = val.value<SCI::ColorMap>();

				val = seriesData(0, AbstractColorGridSeriesModel::ZValueRange);
				Q_ASSERT( val.canConvert<Range>() );
				Range interval = val.value<Range>();
				// check for invalid interval (e.g. no valid data in range, or selection
				// of only void elements, or ...
				if (interval.first != interval.first)
					return 1;
				if (interval.second != interval.second)
					return 1;

				// not a linear color map? use 10 contour lines
				if (cmap.m_type != SCI::ColorMap::Linear)
					return (interval.second - interval.first)*0.1;

				int bands = cmap.m_linearColorStops.size() - 1;
				if (bands <= 0)
					return 1; // bad color map
				else
					return (interval.second - interval.first)/bands;
			}

			return m_contourInterval;
		}

		case ContourLabelPlacement: 		return m_contourLabelPlacement;
		case ContourPen:					return m_contourPen;
		case HasContourIntervalColor: 		return m_contourIntervalColor;
	}

	return QVariant();
}


bool AbstractColorGridSeriesModel::setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) {
	const char * const FUNC_ID = "[AbstractColorGridSeriesModel::setSeriesData]";
	// test if index is valid
	if ( seriesIndex < 0 || seriesIndex > 1 ) {
		IBK::IBK_Message( IBK::FormatString("Invalid series index #%1.").arg(seriesIndex), IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	switch ((SeriesDataRole)seriesDataRole) {
		case SCI::AbstractColorGridSeriesModel::XGridArray:
		case SCI::AbstractColorGridSeriesModel::XGridArraySize:
		case SCI::AbstractColorGridSeriesModel::YGridArray:
		case SCI::AbstractColorGridSeriesModel::YGridArraySize:
		case SCI::AbstractColorGridSeriesModel::XValueSize:
		case SCI::AbstractColorGridSeriesModel::YValueSize:
		case SCI::AbstractColorGridSeriesModel::ValueMatrix:
		case SCI::AbstractColorGridSeriesModel::DataMinimumZ:
		case SCI::AbstractColorGridSeriesModel::DataMaximumZ:
		case SCI::AbstractColorGridSeriesModel::ZValueRange:
		case SCI::AbstractColorGridSeriesModel::NUM_SDR:
			IBK::IBK_Message( IBK::FormatString("Invalid data role #%1 for setSeriesData().").arg(seriesDataRole), IBK::MSG_ERROR, FUNC_ID);
			return false;

		case ColorMap :
			if (!value.canConvert<SCI::ColorMap>())
				return false;
			m_colorMap = value.value<SCI::ColorMap>();
		break;

		case AutoScale:
			m_autoScale = value.toBool();
			seriesDataRole = ZValueRange;
		break;

		case MinimumZ:
			m_minimumZ = value.toDouble();
			seriesDataRole = ZValueRange;
		break;

		case MaximumZ:
			m_maximumZ = value.toDouble();
			seriesDataRole = ZValueRange;
		break;

		case InterpolationMode:
			m_interpolationMode = value.toInt();
		break;

		case DisplayMode:
			m_displayMode = value.toInt();
		break;

		case ColorBarVisible:
			m_colorBarVisible = value.toBool();
			emit axisChanged(AbstractChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
		break;

		case ContourInterval:
			m_contourInterval = value.toDouble();
		break;

		case ContourLabelPlacement:
			m_contourLabelPlacement = value.toUInt();
		break;

		case ContourPen:
			if (!value.canConvert<QPen>())
				return false;
			m_contourPen = value.value<QPen>();
		break;

		case HasContourIntervalColor:
			m_contourIntervalColor = value.toBool();
		break;
	}

	// onyl emit change signal, if we have a series
	if (seriesCount() > 0)
		emit seriesViewChanged(0, 0, seriesDataRole);
	return true;
}


void AbstractColorGridSeriesModel::beginInsertSeries ( int first, int last) {
	// prepare for insertion of series
	AbstractChartModel::beginInsertSeries(first, last);
}


void AbstractColorGridSeriesModel::beginRemoveSeries ( int first, int last ) {
	// prepare for removal of series
	AbstractChartModel::beginRemoveSeries(first, last);
}


void AbstractColorGridSeriesModel::clearCachedData() {
	/// \todo clear other data
	AbstractCartesianChartModel::clearCachedData();
}


void AbstractColorGridSeriesModel::writeSeriesBinary(std::ostream& /*out*/) const {
	/// \todo implement
}


void AbstractColorGridSeriesModel::readSeriesBinary( std::istream& /*in*/ ) {
	/// \todo implement
}


void AbstractColorGridSeriesModel::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("ColorGridProperties");
	parent->LinkEndChild(e);

	if ( m_writeAll || m_colorBarVisible != true)
		TiXmlElement::appendSingleAttributeElement( e, "ColorBarVisible", NULL, std::string(), IBK::val2string<bool>(m_colorBarVisible) );

	if ( m_writeAll || m_autoScale != true)
		TiXmlElement::appendSingleAttributeElement( e, "AutoScale", NULL, std::string(), IBK::val2string<bool>(m_autoScale) );

	if (m_writeAll || m_minimumZ == m_minimumZ) {  // is not NaN
		if (m_writeAll && m_minimumZ != m_minimumZ)
			TiXmlElement::appendSingleAttributeElement( e, "MinimumZ", NULL, std::string(), "NaN" );
		else
			TiXmlElement::appendSingleAttributeElement( e, "MinimumZ", NULL, std::string(), IBK::val2string<double>(m_minimumZ) );
	}

	if (m_writeAll || m_maximumZ == m_maximumZ) {  // is not NaN
		if (m_writeAll && m_maximumZ != m_maximumZ)
			TiXmlElement::appendSingleAttributeElement( e, "MaximumZ", NULL, std::string(), "NaN" );
		else
			TiXmlElement::appendSingleAttributeElement( e, "MaximumZ", NULL, std::string(), IBK::val2string<double>(m_maximumZ) );
	}

	if ( m_writeAll || m_interpolationMode != 1)
		TiXmlElement::appendSingleAttributeElement( e, "InterpolationMode", NULL, std::string(), IBK::val2string<int>(m_interpolationMode) );
	if ( m_writeAll || m_displayMode != 1)
		TiXmlElement::appendSingleAttributeElement( e, "DisplayMode", NULL, std::string(), IBK::val2string(m_displayMode) );

	if (m_writeAll || m_contourInterval == m_contourInterval) {  // is not NaN
		if (m_writeAll && m_contourInterval != m_contourInterval)
			TiXmlElement::appendSingleAttributeElement( e, "ContourInterval", NULL, std::string(), "NaN" );
		else
			TiXmlElement::appendSingleAttributeElement( e, "ContourInterval", NULL, std::string(), IBK::val2string<double>(m_contourInterval) );
	}

	if ( m_writeAll || m_contourPen.width() != QPen().width())
		TiXmlElement::appendSingleAttributeElement( e, "ContourPenWidth", NULL, std::string(), IBK::val2string(m_contourPen.width()) );
	if ( m_writeAll || m_contourPen.style() != QPen().style())
		TiXmlElement::appendSingleAttributeElement( e, "ContourPenStyle", NULL, std::string(), IBK::val2string(m_contourPen.style()) );
	if ( m_writeAll || m_contourPen.color() != QPen().color())
		TiXmlElement::appendSingleAttributeElement( e, "ContourPenColor", NULL, std::string(), m_contourPen.color().name().toStdString() );
	if ( m_writeAll || m_contourIntervalColor != false)
		TiXmlElement::appendSingleAttributeElement( e, "ContourIntervalColor", NULL, std::string(), IBK::val2string<bool>(m_contourIntervalColor) );
	if ( m_writeAll || m_contourLabelPlacement != SCI::PlotSpectrogram::AT_MAX_Y)
		TiXmlElement::appendSingleAttributeElement( e, "ContourLabelPlacement", NULL, std::string(), IBK::val2string<bool>(m_contourLabelPlacement) );

	m_colorMap.writeXML( e );

	AbstractCartesianChartModel::writeXML(parent);
}


void AbstractColorGridSeriesModel::readXML(const TiXmlElement * element) {

	const char * const FUNC_ID = "[AbstractColorGridSeriesModel::readXML]";
	const TiXmlElement * xmlElem = element->FirstChildElement( "ColorGridProperties" );

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

			const TiXmlElement * interpolationMode = xmlElem->FirstChildElement("InterpolationMode");
			if (interpolationMode)
				m_interpolationMode = IBK::string2val<int>(interpolationMode->GetText());

			const TiXmlElement * display = xmlElem->FirstChildElement("DisplayMode");
			if (display)
				m_displayMode = IBK::string2val<int>(display->GetText());

			const TiXmlElement * contourInterval = xmlElem->FirstChildElement("ContourInterval");
			if (contourInterval) {
				std::string textValue = contourInterval->GetText();
				if (textValue == "NaN")
					m_contourInterval = std::numeric_limits<double>::quiet_NaN();
				else
					m_contourInterval = IBK::string2val<double>(textValue);
			}

			const TiXmlElement * cpenWidth = xmlElem->FirstChildElement("ContourPenWidth");
			if (cpenWidth)
				m_contourPen.setWidth(IBK::string2val<int>(cpenWidth->GetText()));

			const TiXmlElement * cpenStyle = xmlElem->FirstChildElement("ContourPenStyle");
			if (cpenStyle)
				m_contourPen.setStyle(static_cast<Qt::PenStyle>(IBK::string2val<int>(cpenStyle->GetText())));

			const TiXmlElement * cpenColor = xmlElem->FirstChildElement("ContourPenColor");
			if (cpenColor) {
				QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
				col = QColor::fromString(cpenColor->GetText());
#else
				col.setNamedColor(cpenColor->GetText());
#endif
				m_contourPen.setColor(col);
			}

			const TiXmlElement * contourIntervalColor = xmlElem->FirstChildElement("ContourIntervalColor");
			if (contourIntervalColor)
				m_contourIntervalColor = IBK::string2val<bool>(contourIntervalColor->GetText());

			const TiXmlElement * labelPlacement = xmlElem->FirstChildElement("ContourLabelPlacement");
			if (labelPlacement)
				m_contourLabelPlacement = IBK::string2val<unsigned int>(labelPlacement->GetText());

			m_colorMap.readXML( xmlElem );

		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading chart settings."), FUNC_ID);
		}
	}

	AbstractCartesianChartModel::readXML(element);
}


} // namespace SCI
