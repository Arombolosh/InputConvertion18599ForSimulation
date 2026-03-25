/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_AbstractChartModel.h"

#include <IBK_InputOutput.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

namespace SCI {

AbstractChartModel::AbstractChartModel(QObject *parent) :
	QObject(parent),
	m_writeAll(false)
{
}

AbstractChartModel::~AbstractChartModel() {
}

void AbstractChartModel::clearCachedData() {
	m_chartProperties = ChartInformation();
	if (!m_changes.isEmpty())
		IBK::IBK_Message("Unbalanced changes record.", IBK::MSG_WARNING, "[AbstractChartModel::clearCachedData]", IBK::VL_INFO);
	m_changes.clear();
}

void AbstractChartModel::beginInsertSeries ( int first, int last ) {
	Q_ASSERT(first >= 0);
	Q_ASSERT(last >= first);
	m_changes.push(Change(first,last));

	emit seriesAboutToBeInserted(first, last);
}

void AbstractChartModel::endInsertSeries () {
	Change change = m_changes.pop();
	emit seriesInserted(change.first, change.last);
}

void AbstractChartModel::beginRemoveSeries ( int first, int last ) {
	Q_ASSERT(first >= 0);
	Q_ASSERT(last >= first);
	m_changes.push(Change(first,last));

	emit seriesAboutToBeRemoved(first, last);
}

void AbstractChartModel::endRemoveSeries () {
	Change change = m_changes.pop();
	emit seriesRemoved(change.first, change.last);
}

void AbstractChartModel::beginChangeSeries ( int first, int last ) {
	Q_ASSERT(first >= 0);
	Q_ASSERT(last >= first);
	m_changes.push(Change(first,last));
	emit seriesAboutToBeChanged(first, last);
}

void AbstractChartModel::endChangeSeries () {
	Q_ASSERT(!m_changes.isEmpty());
	Change change = m_changes.pop();
	emit seriesChanged(change.first, change.last);
}

void AbstractChartModel::beginResetModel() {
	emit modelAboutToBeReset();
}

void AbstractChartModel::endResetModel() {
	emit modelReset();
}

void AbstractChartModel::beginChangeModel() {
	emit modelAboutToBeChanged();
}

void AbstractChartModel::endChangeModel() {
	emit modelChanged();
}

QVariant AbstractChartModel::data(int chartDataRole) const {
	switch((ChartDataRole)chartDataRole) {
	case TitleText							: return m_chartProperties.m_titleText;
		// Derivate classes shall substitute variables/placeholders, base class implementation just returns title text
	case Title								: return m_chartProperties.m_titleText;
	case TitleFont							: return m_chartProperties.m_titleFont;
	case LegendVisible						: return m_chartProperties.m_legendVisible;
	case LegendPosition						: return m_chartProperties.m_legendPosition;
	case LegendItemAlignment				: return m_chartProperties.m_legendAlignment;
	case LegendIconStyle					: return m_chartProperties.m_legendIconStyle;
	case LegendIconWidth					: return m_chartProperties.m_legendIconWidth;
	case LegendMaxColumns					: return m_chartProperties.m_legendMaxColumns;
	case LegendItemFrame					: return m_chartProperties.m_legendHasFrame;
	case LegendItemBackgroundColor			: return m_chartProperties.m_legendBackgroundColor;
	case LegendItemBackgroundTransparency	: return m_chartProperties.m_legendBackgroundTransparency;
	case LegendItemOffset					: return m_chartProperties.m_legendOffset;
	case LegendFont							: return m_chartProperties.m_legendFont;
	case LegendSpacing						: return m_chartProperties.m_legendSpacing;
	case AxisY2TitleInverted				: return m_chartProperties.m_axisY2TitleInverted;
	case AxisScalesInside					: return m_chartProperties.m_axisScalesInside;
	case ConstructionLineVisible			: return m_chartProperties.m_constructionLinesVisible;
	case ConstructionLinePen				: return m_chartProperties.m_constructionPen;
	case CanChangeAxisUnit					: return m_chartProperties.m_canChangeAxisUnit;
	case ShowCurrentTimePosition			: return m_chartProperties.m_showCurrentTimePosition;
	case CurrentTimePosition				: return m_chartProperties.m_currentTimePosition;
	case TrackerLegendEnabled				: return m_chartProperties.m_trackerLegendVisible;
	case TrackerLegendNumberFormat			: return m_chartProperties.m_trackerLegendNumberFormat;
	case TrackerLegendNumberPrecision		: return m_chartProperties.m_trackerLegendNumberPrecision;
	case NUM_CDR: ; break; // just to make compiler happy
	}
	return QVariant();
}

bool AbstractChartModel::setData(const QVariant& value, int chartDataRole) {
	switch((ChartDataRole)chartDataRole) {
		case TitleText: {
			QString text = value.toString();
			if (text == m_chartProperties.m_titleText)				return true;
			m_chartProperties.m_titleText = text;
		} break;

		case TitleFont: {
			if (!value.canConvert<QFont>())							return false;
			QFont font = value.value<QFont>();
			if (font == m_chartProperties.m_titleFont)				return true;
			m_chartProperties.m_titleFont = font;
		} break;

		case LegendVisible: {
			bool visible = value.toBool();
			if (visible == m_chartProperties.m_legendVisible)		return true;
			m_chartProperties.m_legendVisible = visible;
		} break;

		case LegendPosition: {
			bool ok;
			int type = value.toInt(&ok);
			if (!ok)												return false;
			if (type == m_chartProperties.m_legendPosition)			return true;
			m_chartProperties.m_legendPosition = (LegendPositionType)type;
		} break;

		case LegendItemAlignment: {
			bool ok;
			int type = value.toInt(&ok);
			if (!ok)												return false;
			if (type == m_chartProperties.m_legendAlignment)		return true;
			m_chartProperties.m_legendAlignment = type;
		} break;

		case LegendIconStyle: {
			bool ok;
			int style = value.toInt(&ok);
			if (!ok)												return false;
			if (style == m_chartProperties.m_legendIconStyle)		return true;
			m_chartProperties.m_legendIconStyle = style;
		} break;

		case LegendIconWidth: {
			bool ok;
			int width = value.toInt(&ok);
			if (!ok)												return false;
			if (width == m_chartProperties.m_legendIconWidth)		return true;
			m_chartProperties.m_legendIconWidth = width;
		} break;

		case LegendMaxColumns: {
			bool ok;
			int cols = value.toInt(&ok);
			if (!ok)												return false;
			if (cols == m_chartProperties.m_legendMaxColumns)		return true;
			m_chartProperties.m_legendMaxColumns = cols;
		} break;

		case LegendItemFrame: {
			bool frame = value.toBool();
			if (frame == m_chartProperties.m_legendHasFrame)		return true;
			m_chartProperties.m_legendHasFrame = frame;
		} break;

		case LegendItemBackgroundColor: {
			if (!value.canConvert<QColor>())						return false;
			QColor col = value.value<QColor>();
			if (col == m_chartProperties.m_legendBackgroundColor)	return true;
			m_chartProperties.m_legendBackgroundColor = col;
		} break;

		case LegendItemBackgroundTransparency: {
			bool ok;
			double trans = value.toDouble(&ok);
			if (!ok)												return false;
			if (trans == m_chartProperties.m_legendBackgroundTransparency)	return true;
			m_chartProperties.m_legendBackgroundTransparency = trans;
		} break;

		case LegendItemOffset: {
			QSize distance = value.toSize();
			if (distance == m_chartProperties.m_legendOffset)		return true;
			m_chartProperties.m_legendOffset = distance;
		} break;

		case LegendFont: {
			if (!value.canConvert<QFont>())							return false;
			QFont font = value.value<QFont>();
			if (font == m_chartProperties.m_legendFont)				return true;
			m_chartProperties.m_legendFont = font;
		} break;

		case LegendSpacing: {
			bool ok;
			int space = value.toInt(&ok);
			if (!ok)												return false;
			if (space == m_chartProperties.m_legendSpacing)			return true;
			m_chartProperties.m_legendSpacing = space;
		} break;

		case TrackerLegendEnabled : {
			bool visible = value.toBool();
			if (visible == m_chartProperties.m_trackerLegendVisible)	return true;
			m_chartProperties.m_trackerLegendVisible = visible;
		} break;

		case TrackerLegendNumberFormat : {
			char format = value.toChar().toLatin1();
			if (format != 'f' && format != 'g' && format != 'e')			return false;
			if (format == m_chartProperties.m_trackerLegendNumberFormat)	return true;
			m_chartProperties.m_trackerLegendNumberFormat = format;
		} break;

		case TrackerLegendNumberPrecision : {
			bool ok;
			int precision = value.toInt(&ok);
			if (!ok)															return false;
			if (precision == m_chartProperties.m_trackerLegendNumberPrecision)	return true;
			m_chartProperties.m_trackerLegendNumberPrecision = precision;
		} break;

		case AxisY2TitleInverted: {
			bool enabled = value.toBool();
			if (enabled == m_chartProperties.m_axisY2TitleInverted)			return true;
			m_chartProperties.m_axisY2TitleInverted = enabled;
		} break;

		case AxisScalesInside: {
			bool enabled = value.toBool();
			if (enabled == m_chartProperties.m_axisScalesInside)			return true;
			m_chartProperties.m_axisScalesInside = enabled;
		} break;

		case ConstructionLineVisible: {
			bool visible = value.toBool();
			if (visible == m_chartProperties.m_constructionLinesVisible)	return true;
			m_chartProperties.m_constructionLinesVisible = visible;
		} break;

		case ConstructionLinePen : {
			if (!value.canConvert<QPen>())									return false;
			QPen newPen = value.value<QPen>();
			if (newPen == m_chartProperties.m_constructionPen)				return true;
			m_chartProperties.m_constructionPen = newPen;
		} break;

		case CanChangeAxisUnit: {
			bool changeable = value.toBool();
			if (changeable == m_chartProperties.m_canChangeAxisUnit)		return true;
			m_chartProperties.m_canChangeAxisUnit = changeable;
		} break;

		case ShowCurrentTimePosition: {
			bool show = value.toBool();
			if (show == m_chartProperties.m_showCurrentTimePosition)		return true;
			m_chartProperties.m_showCurrentTimePosition = show;
		} break;

		case CurrentTimePosition: {
			double pos = value.toDouble();
			if (pos == m_chartProperties.m_currentTimePosition)				return true;
			m_chartProperties.m_currentTimePosition = pos;
		} break;
		case SCI::AbstractChartModel::Title:
		return false; // cannot set title, as this is a derived property.
		case SCI::AbstractChartModel::NUM_CDR: ; // just to make compiler happy
	}
	emit chartChanged(chartDataRole);
	return true;
}


void AbstractChartModel::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("ChartProperties");
	parent->LinkEndChild(e);

	ChartInformation defaultValues;

	if (m_writeAll || m_chartProperties.m_titleText != defaultValues.m_titleText)
		TiXmlElement::appendSingleAttributeElement( e, "TitleText", nullptr, std::string(), m_chartProperties.m_titleText.toStdString() );
	if (m_writeAll || m_chartProperties.m_titleFont != QFont())
		TiXmlElement::appendSingleAttributeElement( e, "TitleFont", nullptr, std::string(), m_chartProperties.m_titleFont.toString().toStdString() );
	if (m_writeAll || m_chartProperties.m_axisY2TitleInverted != defaultValues.m_axisY2TitleInverted)
		TiXmlElement::appendSingleAttributeElement( e, "AxisY2TitleInverted", nullptr, std::string(), IBK::val2string<bool>(m_chartProperties.m_axisY2TitleInverted) );
	if (m_writeAll || m_chartProperties.m_axisScalesInside != defaultValues.m_axisScalesInside)
		TiXmlElement::appendSingleAttributeElement( e, "AxisScalesInside", nullptr, std::string(), IBK::val2string<bool>(m_chartProperties.m_axisScalesInside) );

	TiXmlElement * es = new TiXmlElement("Legend");
	e->LinkEndChild(es);
	if (m_writeAll || m_chartProperties.m_legendVisible != defaultValues.m_legendVisible)
		es->SetAttribute("visible", IBK::val2string<bool>(m_chartProperties.m_legendVisible) );
	if (m_writeAll || m_chartProperties.m_legendHasFrame != defaultValues.m_legendHasFrame)
		es->SetAttribute("hasFrame", IBK::val2string<bool>(m_chartProperties.m_legendHasFrame) );

	if (m_writeAll || m_chartProperties.m_legendPosition != defaultValues.m_legendPosition) {
		switch (m_chartProperties.m_legendPosition) {
			case LegendAtLeft :		TiXmlElement::appendSingleAttributeElement( es, "Position", nullptr, std::string(), "Left" );break;
			case LegendAtRight :	TiXmlElement::appendSingleAttributeElement( es, "Position", nullptr, std::string(), "Right"  );break;
			case LegendAtBottom :	TiXmlElement::appendSingleAttributeElement( es, "Position", nullptr, std::string(), "Bottom" );break;
			case LegendAtTop :		TiXmlElement::appendSingleAttributeElement( es, "Position", nullptr, std::string(), "Top" );break;
			case LegendInChart :	TiXmlElement::appendSingleAttributeElement( es, "Position", nullptr, std::string(), "Inside" );break;
		}
	}

	if (m_writeAll || m_chartProperties.m_legendAlignment != defaultValues.m_legendAlignment)
		TiXmlElement::appendSingleAttributeElement( es, "Alignment", nullptr, std::string(), IBK::val2string(m_chartProperties.m_legendAlignment) );

	if (m_writeAll || m_chartProperties.m_legendOffset.width() != defaultValues.m_legendOffset.width())
		TiXmlElement::appendSingleAttributeElement( es, "OffsetX", nullptr, std::string(), IBK::val2string<double>(m_chartProperties.m_legendOffset.width()) );
	if (m_writeAll || m_chartProperties.m_legendOffset.height() != defaultValues.m_legendOffset.height())
		TiXmlElement::appendSingleAttributeElement( es, "OffsetY", nullptr, std::string(), IBK::val2string<double>(m_chartProperties.m_legendOffset.height()) );
	if (m_writeAll || m_chartProperties.m_legendBackgroundTransparency != defaultValues.m_legendBackgroundTransparency)
		TiXmlElement::appendSingleAttributeElement( es, "BackgroundTransparency", nullptr, std::string(), IBK::val2string<double>(m_chartProperties.m_legendBackgroundTransparency) );
	if (m_writeAll || m_chartProperties.m_legendBackgroundColor != defaultValues.m_legendBackgroundColor)
		TiXmlElement::appendSingleAttributeElement( es, "BackgroundColor", nullptr, std::string(), m_chartProperties.m_legendBackgroundColor.name().toStdString() );
	if (m_writeAll || m_chartProperties.m_legendIconStyle != defaultValues.m_legendIconStyle)
		TiXmlElement::appendSingleAttributeElement( es, "IconStyle", nullptr, std::string(), IBK::val2string(m_chartProperties.m_legendIconStyle) );
	if (m_writeAll || m_chartProperties.m_legendIconWidth != defaultValues.m_legendIconWidth)
		TiXmlElement::appendSingleAttributeElement( es, "IconWidth", nullptr, std::string(), IBK::val2string(m_chartProperties.m_legendIconWidth) );
	if (m_writeAll || m_chartProperties.m_legendMaxColumns != defaultValues.m_legendMaxColumns)
		TiXmlElement::appendSingleAttributeElement( es, "MaxColumns", nullptr, std::string(), IBK::val2string(m_chartProperties.m_legendMaxColumns) );
	if (m_writeAll || m_chartProperties.m_legendSpacing != defaultValues.m_legendSpacing)
		TiXmlElement::appendSingleAttributeElement( es, "Spacing", nullptr, std::string(), IBK::val2string<double>(m_chartProperties.m_legendSpacing) );
	if (m_writeAll || m_chartProperties.m_legendFont != QFont())
		TiXmlElement::appendSingleAttributeElement( es, "Font", nullptr, std::string(), m_chartProperties.m_legendFont.toString().toStdString() );

	// construction lines
	if (m_writeAll || m_chartProperties.m_constructionLinesVisible != defaultValues.m_constructionLinesVisible)
		TiXmlElement::appendSingleAttributeElement( e, "ConstructionLinesVisible", nullptr, std::string(), IBK::val2string<bool>(m_chartProperties.m_constructionLinesVisible) );
	if ( m_writeAll || m_chartProperties.m_constructionPen.width() != QPen().width())
		TiXmlElement::appendSingleAttributeElement( e, "ConstructionLinesPenWidth", nullptr, std::string(), IBK::val2string(m_chartProperties.m_constructionPen.width()) );
	if ( m_writeAll || m_chartProperties.m_constructionPen.style() != QPen().style())
		TiXmlElement::appendSingleAttributeElement( e, "ConstructionLinesPenStyle", nullptr, std::string(), IBK::val2string(m_chartProperties.m_constructionPen.style()) );
	if ( m_writeAll || m_chartProperties.m_constructionPen.color() != QPen().color())
		TiXmlElement::appendSingleAttributeElement( e, "ConstructionLinesPenColor", nullptr, std::string(), m_chartProperties.m_constructionPen.color().name().toStdString() );

	if (m_writeAll || m_chartProperties.m_showCurrentTimePosition != defaultValues.m_showCurrentTimePosition)
		TiXmlElement::appendSingleAttributeElement( e, "ShowCurrentTimePosition", nullptr, std::string(), IBK::val2string<bool>(m_chartProperties.m_showCurrentTimePosition) );

}


void AbstractChartModel::readXML(const TiXmlElement * element) {

	beginChangeModel();

	const char * const FUNC_ID = "[AbstractChartModel::readXML]";
	const TiXmlElement * xmlElem = element->FirstChildElement( "ChartProperties" );

	if (xmlElem != nullptr) {
		try {

			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "TitleText") {
					m_chartProperties.m_titleText = e->GetText();
				}
				else if (ename == "TitleFont") {
					m_chartProperties.m_titleFont.fromString( e->GetText() );
				}
				else if (ename == "AxisY2TitleInverted") {
					m_chartProperties.m_axisY2TitleInverted = IBK::string2val<bool>(e->GetText());
				}
				else if (ename == "AxisScalesInside") {
					m_chartProperties.m_axisScalesInside = IBK::string2val<bool>(e->GetText());
				}
				else if (ename == "Legend") {

					const TiXmlAttribute * visible = TiXmlAttribute::attributeByName(e, "visible");
					if (visible)
						m_chartProperties.m_legendVisible = IBK::string2val<bool>(visible->Value());

					const TiXmlAttribute * hasFrame = TiXmlAttribute::attributeByName(e, "hasFrame");
					if (hasFrame)
						m_chartProperties.m_legendHasFrame = IBK::string2val<bool>(hasFrame->Value());


					const TiXmlElement * legendType = e->FirstChildElement("Position");
					if (legendType) {

						std::string type = legendType->GetText();
						if ( type == "Left" )
							m_chartProperties.m_legendPosition = LegendAtLeft;
						else if ( type == "Right" )
							m_chartProperties.m_legendPosition = LegendAtRight;
						else if ( type == "Bottom" )
							m_chartProperties.m_legendPosition = LegendAtBottom;
						else if ( type == "Top" )
							m_chartProperties.m_legendPosition = LegendAtTop;
						else if ( type == "Inside" )
							m_chartProperties.m_legendPosition = LegendInChart;
						else
							IBK::IBK_Message(IBK::FormatString("Unknown keyword for legend position in 'Position' tag: %1. Bottom position will be used instead.")
											 .arg(type),
											 IBK::MSG_WARNING, FUNC_ID);
					}

					const TiXmlElement * alignment = e->FirstChildElement("Alignment");
					if (alignment)
						m_chartProperties.m_legendAlignment = IBK::string2val<int>(alignment->GetText());

					const TiXmlElement * offsetX = e->FirstChildElement("OffsetX");
					if (offsetX)
						m_chartProperties.m_legendOffset.setWidth( IBK::string2val<int>(offsetX->GetText()) );

					const TiXmlElement * offsetY = e->FirstChildElement("OffsetY");
					if (offsetY)
						m_chartProperties.m_legendOffset.setHeight( IBK::string2val<int>(offsetY->GetText()) );

					const TiXmlElement * backgroundTransparency = e->FirstChildElement("BackgroundTransparency");
					if (backgroundTransparency)
						m_chartProperties.m_legendBackgroundTransparency = IBK::string2val<double>(backgroundTransparency->GetText());

					const TiXmlElement * backgroundColor = e->FirstChildElement("BackgroundColor");
					if (backgroundColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(backgroundColor->GetText());
#else
						col.setNamedColor(backgroundColor->GetText());
#endif
						m_chartProperties.m_legendBackgroundColor = col;
					}

					const TiXmlElement * identifierStyle = e->FirstChildElement("IconStyle");
					if (identifierStyle)
						m_chartProperties.m_legendIconStyle = IBK::string2val<int>(identifierStyle->GetText());

					const TiXmlElement * identifierWidth = e->FirstChildElement("IconWidth");
					if (identifierWidth)
						m_chartProperties.m_legendIconWidth = IBK::string2val<int>(identifierWidth->GetText());

					const TiXmlElement * maxColumns = e->FirstChildElement("MaxColumns");
					if (maxColumns)
						m_chartProperties.m_legendMaxColumns = IBK::string2val<int>(maxColumns->GetText());

					const TiXmlElement * space = e->FirstChildElement("Spacing");
					if (space)
						m_chartProperties.m_legendSpacing = IBK::string2val<int>(space->GetText());

					const TiXmlElement * legendFont = e->FirstChildElement("Font");
					if (legendFont)
						m_chartProperties.m_legendFont.fromString(legendFont->GetText());

					// construction lines
					const TiXmlElement * constructionLinesVisible = xmlElem->FirstChildElement("ConstructionLinesVisible");
					if (constructionLinesVisible)
						m_chartProperties.m_constructionLinesVisible = IBK::string2val<bool>(constructionLinesVisible->GetText());

					const TiXmlElement * constructionLinesPenWidth = xmlElem->FirstChildElement("ConstructionLinesPenWidth");
					if (constructionLinesPenWidth)
						m_chartProperties.m_constructionPen.setWidth(IBK::string2val<int>(constructionLinesPenWidth->GetText()));

					const TiXmlElement * constructionLinesPenStyle = xmlElem->FirstChildElement("ConstructionLinesPenStyle");
					if (constructionLinesPenStyle)
						m_chartProperties.m_constructionPen.setStyle(static_cast<Qt::PenStyle>(IBK::string2val<int>(constructionLinesPenStyle->GetText())));

					const TiXmlElement * constructionLinesPenColor = xmlElem->FirstChildElement("ConstructionLinesPenColor");
					if (constructionLinesPenColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(constructionLinesPenColor->GetText());
#else
						col.setNamedColor(constructionLinesPenColor->GetText());
#endif
						m_chartProperties.m_constructionPen.setColor(col);
					}

				}
				else if (ename == "ShowCurrentTimePosition") {
					m_chartProperties.m_showCurrentTimePosition = IBK::string2val<bool>(e->GetText());
				}
			}
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading chart settings."), FUNC_ID);
		}
	}

	endChangeModel();
}

} // namespace SCI
