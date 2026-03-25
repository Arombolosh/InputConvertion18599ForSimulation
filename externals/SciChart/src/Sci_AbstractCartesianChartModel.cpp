/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_AbstractCartesianChartModel.h"

#include <IBK_InputOutput.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace SCI {

AbstractCartesianChartModel::AbstractCartesianChartModel(QObject *parent) :
	AbstractChartModel(parent)
{
	AbstractCartesianChartModel::clearCachedData();
}


void AbstractCartesianChartModel::clearCachedData() {
	AbstractChartModel::clearCachedData();

	for( unsigned int i=0; i<3; ++i)
		m_axisInformation[i] = AxisInformation();
	// set meaningful defaults
	m_axisInformation[BottomAxis].m_axisLabelHAlignment = 2; // center
	m_axisInformation[BottomAxis].m_axisLabelVAlignment = 1; // bottom
	m_axisInformation[BottomAxis].m_dateTime = false; // normal value axis

	m_axisInformation[LeftAxis].m_axisLabelHAlignment = 0; // left
	m_axisInformation[LeftAxis].m_axisLabelVAlignment = 2; // center

	m_axisInformation[RightAxis].m_axisLabelHAlignment = 1; // right
	m_axisInformation[RightAxis].m_axisLabelVAlignment = 2; // center

	// Right axis is off by default, and will need to be enabled by user or models when needed (series on right axis
	// or color map axis)
	m_axisInformation[RightAxis].m_axisEnabled = false;
}


QVariant AbstractCartesianChartModel::axisData(AxisPosition axisPosition, int axisDataRole) const {
	if( axisPosition < BottomAxis || axisPosition > RightAxis)
		return QVariant();
	switch(axisDataRole) {
		case AxisEnabled			: return m_axisInformation[axisPosition].m_axisEnabled;
		case AxisTitle				: {
			// implement substitution for $unit placeholder
			QString title = m_axisInformation[axisPosition].m_axisTitleText;
			if (title.contains("$unit")) {
				title.replace(QString("$unit"), QString::fromStdString(m_axisInformation[axisPosition].m_axisUnit.name()) );
			}
			return title;
		}
		case AxisTitleText			: return m_axisInformation[axisPosition].m_axisTitleText;
		case AxisMaximum			: return m_axisInformation[axisPosition].m_axisMaximum;
		case AxisMinimum			: return m_axisInformation[axisPosition].m_axisMinimum;
		case AxisLogarithmic		: return m_axisInformation[axisPosition].m_logarithmicAxis;
		case AxisProportional		: return m_axisInformation[axisPosition].m_axisProportional;
		case AxisAutoScale			: return m_axisInformation[axisPosition].m_axisAutoScale;
		case AxisTitleFont			: return m_axisInformation[axisPosition].m_axisTitleFont;
		case AxisLabelFont			: return m_axisInformation[axisPosition].m_axisLabelFont;
		case AxisLabelHAlignment	: return m_axisInformation[axisPosition].m_axisLabelHAlignment;
		case AxisLabelVAlignment	: return m_axisInformation[axisPosition].m_axisLabelVAlignment;
		case AxisLabelRotation		: return m_axisInformation[axisPosition].m_axisLabelRotation;
		case AxisMaxMinorScale		: return m_axisInformation[axisPosition].m_axisMaxMinorScale;
		case AxisMaxMajorScale		: return m_axisInformation[axisPosition].m_axisMaxMajorScale;
		case AxisUnit				: return m_axisInformation[axisPosition].m_axisUnit.id();
		case AxisGridVisible		: return m_axisInformation[axisPosition].m_gridVisible;
		case AxisGridPen			: return m_axisInformation[0].m_gridPen; // hard-coded 0; only one grid
		case AxisGridPenColor		: return m_axisInformation[0].m_gridPen.color();
		case AxisGridPenWidth		: return m_axisInformation[0].m_gridPen.widthF()*2.0;
		case AxisMinorGridVisible	: return m_axisInformation[axisPosition].m_gridMinorVisible;
		case AxisMinorGridPen		: return m_axisInformation[0].m_gridMinorPen;
		case AxisMinorGridPenColor	: return m_axisInformation[0].m_gridMinorPen.color();
		case AxisMinorGridPenWidth	: return m_axisInformation[0].m_gridMinorPen.widthF()*2.0;
		case AxisTitleSpacing		: return m_axisInformation[axisPosition].m_titleSpacing;
		case AxisLabelSpacing		: return m_axisInformation[axisPosition].m_labelSpacing;
		case AxisDateTime			: return m_axisInformation[axisPosition].m_dateTime;
		case AxisDateTimeZero		: {
			if (m_axisInformation[axisPosition].m_dateTimeZero.isValid())
				return m_axisInformation[axisPosition].m_dateTimeZero;
			else {
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
				QDateTime zeroTime(QDate(2001,1,1), QTime(), QTimeZone("UTC"));
#else
				QDateTime zeroTime(QDate(2001,1,1), QTime(), Qt::UTC);
#endif
				return zeroTime;
			}
		}
		case AxisDateTimeFormats	: return m_axisInformation[axisPosition].m_dateTimeFormats;
	}
	return QVariant();
}


AbstractCartesianChartModel::AxisInformation AbstractCartesianChartModel::axisInformation(int axisPosition) const {
	if (axisPosition < BottomAxis || axisPosition > RightAxis)
		return AxisInformation();

	return m_axisInformation[axisPosition];
}


bool AbstractCartesianChartModel::setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) {
	if( axisPosition < BottomAxis || axisPosition > RightAxis)
		return false;

	bool ok;
	switch(axisDataRole) {
		case AxisEnabled: {
			bool enabled = value.toBool();
			if (enabled == m_axisInformation[axisPosition].m_axisEnabled)				return true;
			m_axisInformation[axisPosition].m_axisEnabled = enabled;
		} break;

		case AxisMaximum: {
			double maximum = value.toDouble(&ok);
			if (!ok)																	return false;
			if (maximum == m_axisInformation[axisPosition].m_axisMaximum)				return true;
			m_axisInformation[axisPosition].m_axisMaximum = maximum;
			// when autoscale is enabled, maximum has no effect and we can skip the axisChanged() signal
			if (m_axisInformation[axisPosition].m_axisAutoScale)						return true;
		} break;

		case AxisMinimum: {
			double minimum = value.toDouble(&ok);
			if (!ok)																	return false;
			if (minimum == m_axisInformation[axisPosition].m_axisMinimum)				return true;
			m_axisInformation[axisPosition].m_axisMinimum = minimum;
			// when autoscale is enabled, minimum has no effect and we can skip the axisChanged() signal
			if (m_axisInformation[axisPosition].m_axisAutoScale)						return true;
		} break;

		case AxisLogarithmic: {
			bool logarithmic = value.toBool();
			if (logarithmic == m_axisInformation[axisPosition].m_logarithmicAxis)		return true;
			m_axisInformation[axisPosition].m_logarithmicAxis = logarithmic;
		} break;

		case AxisProportional: {
			bool proportional = value.toBool();
			if (proportional == m_axisInformation[axisPosition].m_axisProportional)			return true;

			// if enabled, we need to toggle (unset) proportional flag of other axis (x or y depending on which are we)
			// this is done by the axisChanged() slot of the chart
			m_axisInformation[axisPosition].m_axisProportional = proportional;
		} break;

		case AxisAutoScale: {
			bool autoscale = value.toBool();
			if (autoscale == m_axisInformation[axisPosition].m_axisAutoScale)			return true;
			m_axisInformation[axisPosition].m_axisAutoScale = autoscale;
		} break;

		case AxisTitleFont: {
			if (!value.canConvert<QFont>())												return false;
			QFont font = value.value<QFont>();
			if (font ==  m_axisInformation[axisPosition].m_axisTitleFont)				return true;
			m_axisInformation[axisPosition].m_axisTitleFont = font;
		} break;

		case AxisTitleText: {
			QString text = value.toString();
			if (text == m_axisInformation[axisPosition].m_axisTitleText)				return true;
			m_axisInformation[axisPosition].m_axisTitleText = text;
		} break;

		case AxisLabelFont: {
			if (!value.canConvert<QFont>())												return false;
			QFont font = value.value<QFont>();
			if (font ==  m_axisInformation[axisPosition].m_axisLabelFont)				return true;
			m_axisInformation[axisPosition].m_axisLabelFont = font;
		} break;

		case AxisLabelHAlignment: {
			int align = value.toInt(&ok);
			if (!ok)																	return false;
			if( align == m_axisInformation[axisPosition].m_axisLabelHAlignment)			return true;
			m_axisInformation[axisPosition].m_axisLabelHAlignment = align;
		} break;

		case AxisLabelVAlignment: {
			int align = value.toInt(&ok);
			if (!ok)																	return false;
			if (align == m_axisInformation[axisPosition].m_axisLabelVAlignment)			return true;
			m_axisInformation[axisPosition].m_axisLabelVAlignment = align;
		} break;

		case AxisLabelRotation: {
			double rotation = value.toDouble(&ok);
			if (!ok)																	return false;
			if (rotation == m_axisInformation[axisPosition].m_axisLabelRotation)		return true;
			m_axisInformation[axisPosition].m_axisLabelRotation = rotation;
		} break;

		case AxisMaxMinorScale: {
			int minor = value.toInt(&ok);
			if (!ok)																	return false;
			if (minor == m_axisInformation[axisPosition].m_axisMaxMinorScale)			return true;
			m_axisInformation[axisPosition].m_axisMaxMinorScale = minor;
		} break;

		case AxisMaxMajorScale: {
			int major = value.toInt(&ok);
			if (!ok)																	return false;
			if (major == m_axisInformation[axisPosition].m_axisMaxMajorScale)			return true;
			m_axisInformation[axisPosition].m_axisMaxMajorScale = major;
		} break;

		case AxisUnit: {
			unsigned int unitId = value.toUInt(&ok);
			if (!ok)																	return false;
			if (unitId == m_axisInformation[axisPosition].m_axisUnit.id())				return true;
			m_axisInformation[axisPosition].m_axisUnit.set(unitId);
		} break;

		case AxisGridVisible: {
			bool visible = value.toBool();
			if (visible == m_axisInformation[axisPosition].m_gridVisible)				return true;
			m_axisInformation[axisPosition].m_gridVisible = visible;
		} break;

		case AxisGridPen : {
			if( !value.canConvert<QPen>())												return false;
			QPen newPen = value.value<QPen>();
			if( newPen == m_axisInformation[0].m_gridPen)							return true;
			m_axisInformation[0].m_gridPen = newPen; // hard-coded 0 because we have only one grid
		} break;

		case AxisGridPenColor : {
			if( !value.canConvert<QColor>())											return false;
			QColor col = value.value<QColor>();
			if (col == m_axisInformation[0].m_gridPen.color())							return true;
			m_axisInformation[0].m_gridPen.setColor( col ); // hard-coded 0 because we have only one grid
		} break;

		case AxisGridPenWidth : {
			int width = value.toInt(&ok);
			if (!ok)																	return false;
			if (width == m_axisInformation[0].m_gridPen.widthF()*2.0)					return true;
			m_axisInformation[0].m_gridPen.setWidthF( width/2.0 ); // hard-coded 0 because we have only one grid
		} break;

		case AxisMinorGridVisible: {
			bool visible = value.toBool();
			if (visible == m_axisInformation[axisPosition].m_gridMinorVisible)			return true;
			m_axisInformation[axisPosition].m_gridMinorVisible = visible;
		} break;

		case AxisMinorGridPen : {
			if( !value.canConvert<QPen>())												return false;
			QPen newPen = value.value<QPen>();
			if( newPen == m_axisInformation[0].m_gridMinorPen)							return true;
			m_axisInformation[0].m_gridMinorPen = newPen; // hard-coded 0 because we have only one grid
		} break;

		case AxisMinorGridPenColor : {
			if( !value.canConvert<QColor>())											return false;
			QColor col = value.value<QColor>();
			if (col == m_axisInformation[0].m_gridMinorPen.color())						return true;
			m_axisInformation[0].m_gridMinorPen.setColor( col ); // hard-coded 0 because we have only one grid
		} break;

		case AxisMinorGridPenWidth : {
			int width = value.toInt(&ok);
			if (!ok)																	return false;
			if (width == m_axisInformation[0].m_gridMinorPen.widthF()*2.0)				return true;
			m_axisInformation[0].m_gridMinorPen.setWidthF( width/2.0 ); // hard-coded 0 because we have only one grid
		} break;

		case AxisTitleSpacing: {
			int space = value.toInt(&ok);
			if (!ok)																	return false;
			if (space == m_axisInformation[axisPosition].m_titleSpacing)				return true;
			m_axisInformation[axisPosition].m_titleSpacing = space;
		} break;

		case AxisLabelSpacing: {
			int space = value.toInt(&ok);
			if (!ok)																	return false;
			if (space == m_axisInformation[axisPosition].m_labelSpacing)				return true;
			m_axisInformation[axisPosition].m_labelSpacing = space;
		} break;

		case AxisDateTime: {
			bool dateTime = value.toBool();
			if (dateTime == m_axisInformation[axisPosition].m_dateTime)					return true;
			m_axisInformation[axisPosition].m_dateTime = dateTime;
		} break;

		case AxisDateTimeZero: {
			QDateTime zeroTime = value.toDateTime();
			Q_ASSERT_X(zeroTime.timeSpec() == Qt::UTC, "AbstractCartesianChartModel::setAxisData 301", "Timespec doesn't fit to UTC");
			if (zeroTime == m_axisInformation[axisPosition].m_dateTimeZero)				return true;
			m_axisInformation[axisPosition].m_dateTimeZero = zeroTime;
		} break;

		case AxisDateTimeFormats: {
			QStringList formats = value.toStringList();
			// Stringlist in value must have the same size like internal one.
			if (formats.size() !=
					m_axisInformation[axisPosition].m_dateTimeFormats.size())			return false;
			if (formats == m_axisInformation[axisPosition].m_dateTimeFormats)			return true;
			m_axisInformation[axisPosition].m_dateTimeFormats = formats;
		} break;

		default:																		return false;
	}
	emit axisChanged(axisPosition, axisDataRole);
	return true;
}


bool AbstractCartesianChartModel::setAxisData(AxisPosition axisPosition, const AxisInformation& axisinfo) {
	if (axisPosition < BottomAxis || axisPosition > RightAxis)
		return false;

	m_axisInformation[axisPosition] = axisinfo;
	emit axisChanged(axisPosition, AxisTitleText);
	// normally, we should signal here in a loop a change of all properties, but currently, this will
	// update all properties in the property widget.

	return true;
}


void AbstractCartesianChartModel::insertMarker(int markerIndex, const Marker & marker) {
	Q_ASSERT( markerIndex >= 0 && markerIndex <= static_cast<int>(markerCount()) );
	m_markerProperties.insert(markerIndex, marker);
	emit markerInserted(markerIndex, markerIndex);
}


void AbstractCartesianChartModel::removeMarker(int markerIndex) {
	Q_ASSERT( markerIndex >= 0 && markerIndex < static_cast<int>(markerCount()) );
	emit markerAboutToBeRemoved(markerIndex, markerIndex);
	m_markerProperties.removeAt(markerIndex);
}


QVariant AbstractCartesianChartModel::markerData(int markerIndex, int markerDataRole) const {
	// test if index is valid
	Q_ASSERT( markerIndex >= 0 && markerIndex < static_cast<int>(markerCount()) );

	switch ((AbstractCartesianChartModel::MarkerDataRole)markerDataRole) {
		case MarkerData				: return QVariant::fromValue(m_markerProperties[markerIndex]);
		case MarkerType				: return m_markerProperties[markerIndex].m_markerType;
		case MarkerXPos				: return m_markerProperties[markerIndex].m_xPos;
		case MarkerXPosDateTime		: return m_markerProperties[markerIndex].m_xPos;
		case MarkerYPos				: return m_markerProperties[markerIndex].m_yPos;
		case MarkerXAxisID			: return m_markerProperties[markerIndex].m_xAxisID;
		case MarkerYAxisID			: return m_markerProperties[markerIndex].m_yAxisID;
		case MarkerLabel			: return m_markerProperties[markerIndex].m_label;
		case MarkerLabelFont		: return m_markerProperties[markerIndex].m_labelFont;
		case MarkerLabelVAligment	: return (int)m_markerProperties[markerIndex].m_labelAlignment;
		case MarkerLabelAligment	: return (int)m_markerProperties[markerIndex].m_labelAlignment;
		case MarkerLabelOrientation	: return m_markerProperties[markerIndex].m_labelOrientation;
		case MarkerSpacing			: return m_markerProperties[markerIndex].m_spacing;
		case MarkerPen				: return m_markerProperties[markerIndex].m_pen;
		case MarkerPenWidth			: return m_markerProperties[markerIndex].m_pen.width();
		case MarkerPenColor			: return m_markerProperties[markerIndex].m_pen.color();
		case MarkerZOrder			: return m_markerProperties[markerIndex].m_zOrder;
		case NUM_MDR				: ; // just to make compiler happy
	}

	return QVariant();
}


bool AbstractCartesianChartModel::setMarkerData(const QVariant & value, int markerIndex, int markerDataRole) {
	// test if index is valid
	Q_ASSERT( markerIndex >= 0 && markerIndex < static_cast<int>(markerCount()) );

	Marker & m = m_markerProperties[markerIndex];
	bool ok;
	switch (markerDataRole) {

		case MarkerType : {
			unsigned int mt = value.toUInt();
			if (mt == m.m_markerType)				return true;
			if (mt > Marker::MT_Cross)				return false;
			m.m_markerType = (Marker::MarkerType)mt;
		} break;

		case MarkerXPos : {
			double pos = value.toDouble();
			if (pos == m.m_xPos)										return true;
			m.m_xPos = pos;
		} break;

		case MarkerYPos : {
			double pos = value.toDouble();
			if (pos == m.m_yPos)										return true;
			m.m_yPos = pos;
		} break;

		case MarkerXAxisID : {
			double pos = value.toInt();
			if (pos == m.m_xAxisID)										return true;
			m.m_xAxisID = pos;
		} break;

		case MarkerYAxisID : {
			double pos = value.toInt();
			if (pos == m.m_yAxisID)										return true;
			m.m_yAxisID = pos;
		} break;

		case MarkerLabel : {
			QString text = value.toString();
			m.m_label = text;
		} break;

		case MarkerLabelFont : {
			if (!value.canConvert<QFont>())								return false;
			QFont newFont = value.value<QFont>();
			if (newFont == m.m_labelFont)								return true;
			m.m_labelFont = newFont;
		} break;

		case MarkerLabelAligment : {
			unsigned int t = value.toUInt();
			if (t == m.m_labelAlignment)								return true;
			m.m_labelAlignment = (Qt::Alignment)t;
		} break;

		case MarkerLabelOrientation : {
			unsigned int t = value.toUInt();
			if (t == m.m_labelOrientation)								return true;
			m.m_labelOrientation = (Qt::Orientation)t;
		} break;

		case MarkerSpacing : {
			int sp = value.toInt();
			if (sp == m.m_spacing)										return true;
			m.m_spacing = sp;
		} break;

		case MarkerPen : {
			if (!value.canConvert<QPen>())								return false;
			QPen newPen = value.value<QPen>();
			if (newPen == m.m_pen)		return true;
			m.m_pen = newPen;
		} break;

		case MarkerPenColor : {
			if (!value.canConvert<QColor>())							return false;
			QColor col = value.value<QColor>();
			if (col == m.m_pen.color())									return true;
			m.m_pen.setColor( col );
		} break;

		case MarkerPenWidth : {
			int width = value.toInt(&ok);
			if (!ok)													return false;
			if (width == m.m_pen.width())								return true;
			m.m_pen.setWidth( width );
		} break;

		case MarkerZOrder : {
			int sp = value.toInt();
			if (sp == m.m_zOrder)										return true;
			m.m_zOrder = sp;
		} break;

		default :														return false;
	}

	emit markerDataChanged(markerIndex, markerIndex, markerDataRole);
	return true;
}


void AbstractCartesianChartModel::setMarkerData(int markerIndex, const Marker & value) {
	// test if index is valid
	Q_ASSERT( markerIndex >= 0 && markerIndex < static_cast<int>(markerCount()) );
	m_markerProperties[markerIndex] = value;
	emit markerChanged(markerIndex, markerIndex);
}


void AbstractCartesianChartModel::swapMarkerData(int first, int second) {
	Q_ASSERT(first >= 0 && first < m_markerProperties.size());
	Q_ASSERT(second >= 0 && second < m_markerProperties.size());
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
	m_markerProperties.swap(first, second);
#else
	m_markerProperties.swapItemsAt(first, second);
#endif
	emit markerChanged(first, first);
	emit markerChanged(second, second);
}


void AbstractCartesianChartModel::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("AxisProperties");
	parent->LinkEndChild(e);

	AxisInformation defaultValues;

	for (unsigned int i=0; i<3; ++i) {

		if(i == BottomAxis) {
			defaultValues.m_axisLabelHAlignment = 2; // center
			defaultValues.m_axisLabelVAlignment = 1; // bottom
			defaultValues.m_dateTime = false; // normal value axis
		}
		else if(i == LeftAxis) {
			defaultValues.m_axisLabelHAlignment = 0; // left
			defaultValues.m_axisLabelVAlignment = 2; // center
		}
		else  if(i == RightAxis) {
			defaultValues.m_axisLabelHAlignment = 1; // right
			defaultValues.m_axisLabelVAlignment = 2; // center
		}

		TiXmlElement * es = new TiXmlElement("Axis");
		e->LinkEndChild(es);

		es->SetAttribute("index", IBK::val2string(i) );
		if(m_writeAll || m_axisInformation[i].m_axisEnabled != defaultValues.m_axisEnabled)
			es->SetAttribute("enabled", IBK::val2string<bool>(m_axisInformation[i].m_axisEnabled) );
		if(m_writeAll || m_axisInformation[i].m_logarithmicAxis != defaultValues.m_logarithmicAxis)
			es->SetAttribute("logarithmic", IBK::val2string<bool>(m_axisInformation[i].m_logarithmicAxis) );
		if(m_writeAll || m_axisInformation[i].m_axisAutoScale != defaultValues.m_axisAutoScale)
			es->SetAttribute("autoscale", IBK::val2string<bool>(m_axisInformation[i].m_axisAutoScale) );
		if(m_writeAll || m_axisInformation[i].m_axisProportional != defaultValues.m_axisProportional)
			es->SetAttribute("proportional", IBK::val2string<bool>(m_axisInformation[i].m_axisProportional) );

		if (m_writeAll || !m_axisInformation[i].m_axisTitleText.isEmpty() )
			TiXmlElement::appendSingleAttributeElement( es, "TitleText", nullptr, std::string(), m_axisInformation[i].m_axisTitleText.toStdString() );
		if (m_writeAll || m_axisInformation[i].m_axisTitleFont != QFont())
			TiXmlElement::appendSingleAttributeElement( es, "TitleFont", nullptr, std::string(), m_axisInformation[i].m_axisTitleFont.toString().toStdString() );

		if (m_writeAll || m_axisInformation[i].m_axisLabelFont != QFont())
			TiXmlElement::appendSingleAttributeElement( es, "LabelFont", nullptr, std::string(), m_axisInformation[i].m_axisLabelFont.toString().toStdString() );
		if(m_writeAll || m_axisInformation[i].m_axisLabelHAlignment != defaultValues.m_axisLabelHAlignment)
			TiXmlElement::appendSingleAttributeElement( es, "LabelHAlignment", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_axisLabelHAlignment) );
		if(m_writeAll || m_axisInformation[i].m_axisLabelVAlignment != defaultValues.m_axisLabelVAlignment)
			TiXmlElement::appendSingleAttributeElement( es, "LabelVAlignment", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_axisLabelVAlignment) );
		if(m_writeAll || m_axisInformation[i].m_axisLabelRotation != defaultValues.m_axisLabelRotation)
			TiXmlElement::appendSingleAttributeElement( es, "LabelRotation", nullptr, std::string(), IBK::val2string<double>(m_axisInformation[i].m_axisLabelRotation) );

		if(m_writeAll || m_axisInformation[i].m_axisUnit.id() != 0)
			TiXmlElement::appendSingleAttributeElement(es, "IBK:Unit", "name", "AxisUnit", m_axisInformation[i].m_axisUnit.name());

		if(m_writeAll || m_axisInformation[i].m_axisMaximum != defaultValues.m_axisMaximum)
			TiXmlElement::appendSingleAttributeElement( es, "Maximum", nullptr, std::string(), IBK::val2string<double>(m_axisInformation[i].m_axisMaximum, 17) );
		if(m_writeAll || m_axisInformation[i].m_axisMinimum != defaultValues.m_axisMinimum)
			TiXmlElement::appendSingleAttributeElement( es, "Minimum", nullptr, std::string(), IBK::val2string<double>(m_axisInformation[i].m_axisMinimum, 17) );
		if(m_writeAll || m_axisInformation[i].m_axisMaxMinorScale != defaultValues.m_axisMaxMinorScale)
			TiXmlElement::appendSingleAttributeElement( es, "MaxMinorScale", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_axisMaxMinorScale) );
		if(m_writeAll || m_axisInformation[i].m_axisMaxMajorScale != defaultValues.m_axisMaxMajorScale)
			TiXmlElement::appendSingleAttributeElement( es, "MaxMajorScale", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_axisMaxMajorScale) );

		if(m_writeAll || m_axisInformation[i].m_gridVisible != defaultValues.m_gridVisible)
			TiXmlElement::appendSingleAttributeElement( es, "GridVisible", nullptr, std::string(), IBK::val2string<bool>(m_axisInformation[i].m_gridVisible) );

		if (i==0) {
			if(m_writeAll || m_axisInformation[i].m_gridPen.width() != defaultValues.m_gridPen.width())
				TiXmlElement::appendSingleAttributeElement( es, "GridPenWidth", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_gridPen.width()) );
			if(m_writeAll || m_axisInformation[i].m_gridPen.style() != defaultValues.m_gridPen.style())
				TiXmlElement::appendSingleAttributeElement( es, "GridPenStyle", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_gridPen.style()) );
			if(m_writeAll || m_axisInformation[i].m_gridPen.color() != defaultValues.m_gridPen.color())
				TiXmlElement::appendSingleAttributeElement( es, "GridPenColor", nullptr, std::string(), m_axisInformation[i].m_gridPen.color().name().toStdString() );
		}

		if(m_writeAll || m_axisInformation[i].m_gridMinorVisible != defaultValues.m_gridMinorVisible)
			TiXmlElement::appendSingleAttributeElement( es, "MinorGridVisible", nullptr, std::string(), IBK::val2string<bool>(m_axisInformation[i].m_gridMinorVisible) );

		if (i==0) {
			if(m_writeAll || m_axisInformation[i].m_gridMinorPen.width() != defaultValues.m_gridMinorPen.width())
				TiXmlElement::appendSingleAttributeElement( es, "MinorGridPenWidth", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_gridMinorPen.width()) );
			if(m_writeAll || m_axisInformation[i].m_gridMinorPen.style() != defaultValues.m_gridMinorPen.style())
				TiXmlElement::appendSingleAttributeElement( es, "MinorGridPenStyle", nullptr, std::string(), IBK::val2string(m_axisInformation[i].m_gridMinorPen.style()) );
			if(m_writeAll || m_axisInformation[i].m_gridMinorPen.color() != defaultValues.m_gridMinorPen.color())
				TiXmlElement::appendSingleAttributeElement( es, "MinorGridPenColor", nullptr, std::string(), m_axisInformation[i].m_gridMinorPen.color().name().toStdString() );
		}

		if(m_writeAll || m_axisInformation[i].m_labelSpacing != defaultValues.m_labelSpacing)
			TiXmlElement::appendSingleAttributeElement( es, "LabelSpacing", nullptr, std::string(), IBK::val2string<int>(m_axisInformation[i].m_labelSpacing) );
		if(m_writeAll || m_axisInformation[i].m_titleSpacing != defaultValues.m_titleSpacing)
			TiXmlElement::appendSingleAttributeElement( es, "TitleSpacing", nullptr, std::string(), IBK::val2string<int>(m_axisInformation[i].m_titleSpacing) );
		if(m_writeAll || m_axisInformation[i].m_dateTime != defaultValues.m_dateTime)
			TiXmlElement::appendSingleAttributeElement( es, "DateTimeEnabled", nullptr, std::string(), IBK::val2string<bool>(m_axisInformation[i].m_dateTime) );
		if(m_writeAll || m_axisInformation[i].m_dateTimeZero != defaultValues.m_dateTimeZero)
			TiXmlElement::appendSingleAttributeElement( es, "DateTimeZero", nullptr, std::string(), IBK::val2string<qint64>(m_axisInformation[i].m_dateTimeZero.toMSecsSinceEpoch(), 17) );
		if(m_writeAll || m_axisInformation[i].m_dateTimeSpec != defaultValues.m_dateTimeSpec)
			TiXmlElement::appendSingleAttributeElement( es, "DateTimeSpec", nullptr, std::string(), IBK::val2string<int>((int)m_axisInformation[i].m_dateTimeSpec) );
		if(m_writeAll || m_axisInformation[i].m_dateTimeFormats != defaultValues.m_dateTimeFormats)
			TiXmlElement::appendSingleAttributeElement( es, "DateTimeFormats", nullptr, std::string(), m_axisInformation[i].m_dateTimeFormats.join("\n").toStdString() );
	}

	// now the marker

	if (!m_markerProperties.isEmpty()) {
		e = new TiXmlElement("MarkerProperties");
		parent->LinkEndChild(e);

		Marker defaultMarker;

		for (int i=0; i<m_markerProperties.count(); ++i) {
			const Marker & m = m_markerProperties[i];

			TiXmlElement * es = new TiXmlElement("Marker");
			e->LinkEndChild(es);

			if (m_writeAll || m.m_markerType != defaultMarker.m_markerType)
				TiXmlElement::appendSingleAttributeElement( es, "MarkerType", nullptr, std::string(), IBK::val2string(m.m_markerType) );
			if (m_writeAll || m.m_xPos != defaultMarker.m_xPos)
				TiXmlElement::appendSingleAttributeElement( es, "XPos", nullptr, std::string(), IBK::val2string(m.m_xPos));
			if (m_writeAll || m.m_yPos != defaultMarker.m_yPos)
				TiXmlElement::appendSingleAttributeElement( es, "YPos", nullptr, std::string(), IBK::val2string(m.m_yPos));
			if (m_writeAll || m.m_xAxisID != defaultMarker.m_xAxisID)
				TiXmlElement::appendSingleAttributeElement( es, "XAxisID", nullptr, std::string(), IBK::val2string(m.m_xAxisID));
			if (m_writeAll || m.m_yAxisID != defaultMarker.m_yAxisID)
				TiXmlElement::appendSingleAttributeElement( es, "YAxisID", nullptr, std::string(), IBK::val2string(m.m_yAxisID));
			if (m_writeAll || m.m_label != defaultMarker.m_label)
				TiXmlElement::appendSingleAttributeElement( es, "Label", nullptr, std::string(), m.m_label.toStdString() );
			if (m_writeAll || m.m_labelFont != defaultMarker.m_labelFont)
				TiXmlElement::appendSingleAttributeElement( es, "LabelFont", nullptr, std::string(), m.m_labelFont.toString().toStdString() );
			if (m_writeAll || m.m_labelAlignment != defaultMarker.m_labelAlignment)
				TiXmlElement::appendSingleAttributeElement( es, "LabelAlignment", nullptr, std::string(), IBK::val2string(m.m_labelAlignment) );
			if (m_writeAll || m.m_labelOrientation != defaultMarker.m_labelOrientation)
				TiXmlElement::appendSingleAttributeElement( es, "LabelOrientation", nullptr, std::string(), IBK::val2string(m.m_labelOrientation) );
			if (m_writeAll || m.m_spacing != defaultMarker.m_spacing)
				TiXmlElement::appendSingleAttributeElement( es, "Spacing", nullptr, std::string(), IBK::val2string(m.m_spacing) );
			if (m_writeAll || m.m_pen.width() != defaultMarker.m_pen.width())
				TiXmlElement::appendSingleAttributeElement( es, "PenWidth", nullptr, std::string(), IBK::val2string(m.m_pen.width()) );
			if (m_writeAll || m.m_pen.style() != defaultMarker.m_pen.style())
				TiXmlElement::appendSingleAttributeElement( es, "PenStyle", nullptr, std::string(), IBK::val2string(m.m_pen.style()) );
			if (m_writeAll || m.m_pen.color() != defaultMarker.m_pen.color())
				TiXmlElement::appendSingleAttributeElement( es, "PenColor", nullptr, std::string(), m.m_pen.color().name().toStdString() );
			if (m_writeAll || m.m_zOrder != defaultMarker.m_zOrder)
				TiXmlElement::appendSingleAttributeElement( es, "ZOrder", nullptr, std::string(), IBK::val2string(m.m_zOrder) );
		}
	}

	AbstractChartModel::writeXML(parent);
}


void AbstractCartesianChartModel::readXML(const TiXmlElement * element) {
	const char * const FUNC_ID = "[AbstractCartesianChartModel::readXML]";

	/// \todo shouldn't we call a beginModify... function here, just as in AbstractChartModel ??? In any case,
	///       clarify use of these begin/end functions.
	///       or simply wrap the entire read functionality in a beginResetModel() and endResetModel() call, which
	///       must be, however, called from the last child class.... which makes it easy to forget.

	const TiXmlElement * xmlElem = element->FirstChildElement( "AxisProperties" );

	if (xmlElem != nullptr) {
		try {

			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "Axis") {

					const TiXmlAttribute * index = TiXmlAttribute::attributeByName(e, "index");
					if (!index)
						throw IBK::Exception(IBK::FormatString("Expected 'index' attribute in Series."), FUNC_ID);
					int axisIndex = IBK::string2val<int>(index->Value());

					const TiXmlAttribute * enabled = TiXmlAttribute::attributeByName(e, "enabled");
					if (enabled)
						m_axisInformation[axisIndex].m_axisEnabled = IBK::string2val<bool>(enabled->Value());

					const TiXmlAttribute * logarithmic = TiXmlAttribute::attributeByName(e, "logarithmic");
					if (logarithmic)
						m_axisInformation[axisIndex].m_logarithmicAxis = IBK::string2val<bool>(logarithmic->Value());

					const TiXmlAttribute * proportional = TiXmlAttribute::attributeByName(e, "proportional");
					if (proportional)
						m_axisInformation[axisIndex].m_axisProportional = IBK::string2val<bool>(proportional->Value());

					const TiXmlAttribute * autoscale = TiXmlAttribute::attributeByName(e, "autoscale");
					if (autoscale)
						m_axisInformation[axisIndex].m_axisAutoScale = IBK::string2val<bool>(autoscale->Value());

					const TiXmlElement * titleText = e->FirstChildElement("TitleText");
					if (titleText)
						m_axisInformation[axisIndex].m_axisTitleText = titleText->GetText();

					const TiXmlElement * titleFont = e->FirstChildElement("TitleFont");
					if (titleFont)
						m_axisInformation[axisIndex].m_axisTitleFont.fromString(titleFont->GetText());

					const TiXmlElement * labelFont = e->FirstChildElement("LabelFont");
					if (labelFont)
						m_axisInformation[axisIndex].m_axisLabelFont.fromString(labelFont->GetText());

					const TiXmlElement * labelHAlignment = e->FirstChildElement("LabelHAlignment");
					if (labelHAlignment)
						m_axisInformation[axisIndex].m_axisLabelHAlignment = IBK::string2val<int>(labelHAlignment->GetText());

					const TiXmlElement * labelVAlignment = e->FirstChildElement("LabelVAlignment");
					if (labelVAlignment)
						m_axisInformation[axisIndex].m_axisLabelVAlignment = IBK::string2val<int>(labelVAlignment->GetText());

					const TiXmlElement * labelRotation = e->FirstChildElement("LabelRotation");
					if (labelRotation)
						m_axisInformation[axisIndex].m_axisLabelRotation = IBK::string2val<double>(labelRotation->GetText());


					const TiXmlElement * unit = e->FirstChildElement("IBK:Unit");
					if (unit) {
						IBK::Unit u(unit->GetText());
						m_axisInformation[axisIndex].m_axisUnit = u;
					}

					const TiXmlElement * maximum = e->FirstChildElement("Maximum");

					// initialize temporary values with defaults for axis limits (i.e. 0 and 0)
					double maxtemp = 0;
					double mintemp = 0;
					if (maximum)
						maxtemp = IBK::string2val<double>(maximum->GetText());

					const TiXmlElement * minimum = e->FirstChildElement("Minimum");
					if (minimum)
						mintemp = IBK::string2val<double>(minimum->GetText());

					m_axisInformation[axisIndex].m_axisMaximum = maxtemp;
					m_axisInformation[axisIndex].m_axisMinimum = mintemp;

					const TiXmlElement * maxMinorScale = e->FirstChildElement("MaxMinorScale");
					if (maxMinorScale)
						m_axisInformation[axisIndex].m_axisMaxMinorScale = IBK::string2val<int>(maxMinorScale->GetText());

					const TiXmlElement * maxMajorScale = e->FirstChildElement("MaxMajorScale");
					if (maxMajorScale)
						m_axisInformation[axisIndex].m_axisMaxMajorScale = IBK::string2val<int>(maxMajorScale->GetText());


					// *** major grid ***

					const TiXmlElement * gridVisible = e->FirstChildElement("GridVisible");
					if (gridVisible)
						m_axisInformation[axisIndex].m_gridVisible = IBK::string2val<bool>(gridVisible->GetText());

					const TiXmlElement * penWidth = e->FirstChildElement("GridPenWidth");
					if (penWidth)
						m_axisInformation[0].m_gridPen.setWidthF(IBK::string2val<int>(penWidth->GetText())/2.0);

					const TiXmlElement * penStyle = e->FirstChildElement("GridPenStyle");
					if (penStyle)
						m_axisInformation[0].m_gridPen.setStyle(static_cast<Qt::PenStyle>(IBK::string2val<int>(penStyle->GetText())));

					const TiXmlElement * penColor = e->FirstChildElement("GridPenColor");
					if (penColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(penColor->GetText());
#else
						col.setNamedColor(penColor->GetText());
#endif
						m_axisInformation[0].m_gridPen.setColor(col);
					}

					// *** minor grid ***

					const TiXmlElement * minorGridVisible = e->FirstChildElement("MinorGridVisible");
					if (minorGridVisible)
						m_axisInformation[axisIndex].m_gridMinorVisible = IBK::string2val<bool>(minorGridVisible->GetText());

					penWidth = e->FirstChildElement("MinorGridPenWidth");
					if (penWidth)
						m_axisInformation[0].m_gridMinorPen.setWidthF(IBK::string2val<int>(penWidth->GetText()) / 2.0);

					penStyle = e->FirstChildElement("MinorGridPenStyle");
					if (penStyle)
						m_axisInformation[0].m_gridMinorPen.setStyle(static_cast<Qt::PenStyle>(IBK::string2val<int>(penStyle->GetText())));

					penColor = e->FirstChildElement("MinorGridPenColor");
					if (penColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(penColor->GetText());
#else
						col.setNamedColor(penColor->GetText());
#endif
						m_axisInformation[0].m_gridMinorPen.setColor(col);
					}

					const TiXmlElement * titleSpacing = e->FirstChildElement("TitleSpacing");
					if (titleSpacing)
						m_axisInformation[axisIndex].m_titleSpacing = IBK::string2val<int>(titleSpacing->GetText());

					const TiXmlElement * labelSpacing = e->FirstChildElement("LabelSpacing");
					if (labelSpacing)
						m_axisInformation[axisIndex].m_labelSpacing = IBK::string2val<int>(labelSpacing->GetText());

					const TiXmlElement * dateTime = e->FirstChildElement("DateTimeEnabled");
					if (dateTime)
						m_axisInformation[axisIndex].m_dateTime = IBK::string2val<bool>(dateTime->GetText());

					const TiXmlElement * dateTimeZero = e->FirstChildElement("DateTimeZero");
					if (dateTimeZero) {
						QDateTime dateTimeZ;
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
						dateTimeZ.setTimeZone(QTimeZone("UTC"));
#else
						dateTimeZ.setTimeSpec(Qt::UTC);
#endif
						std::string timeText = dateTimeZero->GetText();
						try {
							// new format requires start time to be stored as milli seconds since epoch
							dateTimeZ.setMSecsSinceEpoch( IBK::string2val<qint64>(timeText) );
						}
						catch(IBK::Exception&) {
							// try next version 2.1 formating of time
							dateTimeZ = QDateTime::fromString( QString::fromStdString(dateTimeZero->GetText()));
							if (!dateTimeZ.isValid())
								dateTimeZ = QDateTime(); // invalid start date time -> compute from dataset
						}
						m_axisInformation[axisIndex].m_dateTimeZero = dateTimeZ;
					}

					const TiXmlElement * dateTimeSpec = e->FirstChildElement("DateTimeSpec");
					if (dateTimeSpec)
						m_axisInformation[axisIndex].m_dateTimeSpec = static_cast<Qt::TimeSpec>(IBK::string2val<int>(dateTimeSpec->GetText()));

					const TiXmlElement * dateTimeFormats = e->FirstChildElement("DateTimeFormats");
					if (dateTimeFormats) {
						std::string dfFormats = dateTimeFormats->GetText();
						m_axisInformation[axisIndex].m_dateTimeFormats = QString::fromStdString(dfFormats).split('\n');
					}
				}
			}
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading axis data."), FUNC_ID);
		}
	}

	xmlElem = element->FirstChildElement( "MarkerProperties" );

	if (xmlElem != nullptr) {
		try {
			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "Marker") {
					Marker m;
					const TiXmlElement * markerType = e->FirstChildElement("MarkerType");
					if (markerType)				m.m_markerType = (Marker::MarkerType)IBK::string2val<int>(markerType->GetText());

					const TiXmlElement * pos = e->FirstChildElement("XPos");
					if (pos)					m.m_xPos = IBK::string2val<double>(pos->GetText());

					pos = e->FirstChildElement("YPos");
					if (pos)					m.m_yPos = IBK::string2val<double>(pos->GetText());

					const TiXmlElement * axis = e->FirstChildElement("XAxisID");
					if (axis)					m.m_xAxisID = IBK::string2val<int>(axis->GetText());

					axis = e->FirstChildElement("YAxisID");
					if (axis)					m.m_yAxisID = IBK::string2val<int>(axis->GetText());

					const TiXmlElement * label = e->FirstChildElement("Label");
					if (label)					m.m_label = label->GetText();

					const TiXmlElement * labelFont = e->FirstChildElement("LabelFont");
					if (labelFont)				m.m_labelFont.fromString(labelFont->GetText());

					const TiXmlElement * labelAlignment = e->FirstChildElement("LabelAlignment");
					if (labelAlignment)			m.m_labelAlignment = (Qt::Alignment)IBK::string2val<int>(labelAlignment->GetText());

					const TiXmlElement * labelOrientation = e->FirstChildElement("LabelOrientation");
					if (labelOrientation)		m.m_labelOrientation = (Qt::Orientation)IBK::string2val<double>(labelOrientation->GetText());

					const TiXmlElement * spacing = e->FirstChildElement("Spacing");
					if (spacing)				m.m_spacing = IBK::string2val<int>(spacing->GetText());

					const TiXmlElement * penWidth = e->FirstChildElement("PenWidth");
					if (penWidth)				m.m_pen.setWidth(IBK::string2val<int>(penWidth->GetText()));

					const TiXmlElement * penStyle = e->FirstChildElement("PenStyle");
					if (penStyle)				m.m_pen.setStyle(static_cast<Qt::PenStyle>(IBK::string2val<int>(penStyle->GetText())));

					const TiXmlElement * penColor = e->FirstChildElement("PenColor");
					if (penColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(penColor->GetText());
#else
						col.setNamedColor(penColor->GetText());
#endif
						m.m_pen.setColor(col);
					}

					const TiXmlElement * zOrder = e->FirstChildElement("ZOrder");
					if (zOrder)					m.m_zOrder = IBK::string2val<int>(zOrder->GetText());

					m_markerProperties.append(m);
				}
			}
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading marker data."), FUNC_ID);
		}
	}


	AbstractChartModel::readXML(element);
}


} // namespace SCI
