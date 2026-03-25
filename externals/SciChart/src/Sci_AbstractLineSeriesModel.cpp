/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_AbstractLineSeriesModel.h"

#include <IBK_InputOutput.h>
#include <IBK_Color.h>
#include <IBK_StringUtils.h>

#include <tinyxml.h>

namespace SCI {

const int NUM_DEFAULT_COLORS = 14;
const QColor DEFAULT_COLORS[NUM_DEFAULT_COLORS] = {
	"#000000",
	"#e41a1c",
	"#377eb8",
	"#4daf4a",
	"#984ea3",
	"#ff7f00",
	"#e3e333",
	"#a65628",
	"#f781bf",
	"#4b1f6f",
	"#ff950e",
	"#c5000b",
	"#0084d1",
	"#004586"
};

QColor colorFromDefault(unsigned int index) {
	index = index % NUM_DEFAULT_COLORS;
	return DEFAULT_COLORS[index];
}

AbstractLineSeriesModel::AbstractLineSeriesModel(QObject *parent) :
	AbstractCartesianChartModel(parent)
{
}

QVariant AbstractLineSeriesModel::seriesData(int seriesIndex, int seriesDataRole) const {
	// test if index is valid
	Q_ASSERT( seriesIndex >= 0 && seriesIndex < static_cast<int>(seriesCount()) );

	switch (seriesDataRole) {
		case SeriesTitleText		: // fall-through, AbstractLineSeriesModel doesn't know how to handle placeholders
		case SeriesTitle			: return m_lineInformation[seriesIndex].m_title;
		case SeriesPen				: return m_lineInformation[seriesIndex].m_pen;
		case SeriesColor			: return m_lineInformation[seriesIndex].m_pen.color();
		case SeriesWidth			: return m_lineInformation[seriesIndex].m_pen.width();
		case SeriesLeftYAxis		: return m_lineInformation[seriesIndex].m_leftAxis;
		case SeriesLineStyle		: return m_lineInformation[seriesIndex].m_lineStyle;
		case SeriesMarkerStyle		: return m_lineInformation[seriesIndex].m_markerStyle;
		case SeriesInverted			: return m_lineInformation[seriesIndex].m_inverted;
		case SeriesFitted			: return m_lineInformation[seriesIndex].m_fitted;
		case SeriesInLegend			: return m_lineInformation[seriesIndex].m_inLegend;
		case SeriesMarkerSize		: return m_lineInformation[seriesIndex].m_markerSize;
		case SeriesMarkerFilled		: return m_lineInformation[seriesIndex].m_markerFilled;
		case SeriesMarkerColor		: return m_lineInformation[seriesIndex].m_markerColor;
		case SeriesBrush			: return m_lineInformation[seriesIndex].m_brush;
	}

	return QVariant();
}


bool AbstractLineSeriesModel::setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) {
	// test if index is valid
	Q_ASSERT( seriesIndex >= 0 && seriesIndex < static_cast<int>(seriesCount()) );

	bool ok;
	switch (seriesDataRole) {
		case SeriesTitleText : {
			QString text = value.toString();
			m_lineInformation[seriesIndex].m_title = text;
		} break;

		case SeriesPen : {
			if( !value.canConvert<QPen>())								return false;
			QPen newPen = value.value<QPen>();
			if( newPen == m_lineInformation[seriesIndex].m_pen)			return true;
			m_lineInformation[seriesIndex].m_pen = newPen;
		} break;

		case SeriesColor : {
			if( !value.canConvert<QColor>())							return false;
			QColor col = value.value<QColor>();
			if( col == m_lineInformation[seriesIndex].m_pen.color())	return true;
			m_lineInformation[seriesIndex].m_pen.setColor( col );
		} break;

		case SeriesWidth : {
			int width = value.toInt(&ok);
			if (!ok)													return false;
			if( width == m_lineInformation[seriesIndex].m_pen.width())	return true;
			m_lineInformation[seriesIndex].m_pen.setWidth( width );
		} break;

		case SeriesLeftYAxis : {
			bool left = value.toBool();
			if( left == m_lineInformation[seriesIndex].m_leftAxis)		return true;
			m_lineInformation[seriesIndex].m_leftAxis = left;
			if (m_lineInformation[seriesIndex].m_leftAxis) {
				// check if there is still any line attached to the right axis - if not, disable this axis
				bool useRight = false;
				for (int i=0;i<m_lineInformation.size(); ++i)
					if (!m_lineInformation[i].m_leftAxis) {
						useRight = true;
						break;
					}
				if (!useRight) // enable right axis
					setAxisData(false, AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
				// get currently assigned axis unit
				int unit = axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toInt();
				if (unit == 0) {
					// unit still undefined, get unit from right axis and set it as well
					int unitY2 = axisData(AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisUnit).toInt();
					setAxisData(unitY2, AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit);
				}
			}
			else {
				// enable right axis
				setAxisData(true, AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
				// get currently assigned axis unit
				int unit = axisData(AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisUnit).toInt();
				if (unit == 0) {
					// unit still undefined, get unit from right axis and set it as well
					int unitY1 = axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toInt();
					setAxisData(unitY1, AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisUnit);
				}
			}
			// if moving of series from one axis to the other left an axis without any series,
			// set its unit to undefined
		} break;

		case SeriesLineStyle : {
			int style = value.toInt();
			if( style == m_lineInformation[seriesIndex].m_lineStyle)				return true;
			m_lineInformation[seriesIndex].m_lineStyle = style;
		} break;

		case SeriesMarkerStyle : {
			int style = value.toInt();
			if( style == m_lineInformation[seriesIndex].m_markerStyle)				return true;
			m_lineInformation[seriesIndex].m_markerStyle = style;
		} break;

		case SeriesMarkerSize : {
			unsigned int msize = value.toUInt();
			if (msize == m_lineInformation[seriesIndex].m_markerSize)				return true;
			m_lineInformation[seriesIndex].m_markerSize = msize;
		} break;

		case SeriesMarkerFilled : {
			bool markerFilled = value.toBool();
			if( markerFilled == m_lineInformation[seriesIndex].m_markerFilled)		return true;
			m_lineInformation[seriesIndex].m_markerFilled = markerFilled;
		} break;

		case SeriesMarkerColor : {
			if( !value.canConvert<QColor>())							return false;
			QColor col = value.value<QColor>();
			if( col == m_lineInformation[seriesIndex].m_markerColor)				return true;
			m_lineInformation[seriesIndex].m_markerColor = col;
		} break;

		case SeriesInverted : {
			bool inverted = value.toBool();
			if( inverted == m_lineInformation[seriesIndex].m_inverted)				return true;
			m_lineInformation[seriesIndex].m_inverted = inverted;
		} break;

		case SeriesFitted : {
			bool fitted = value.toBool();
			if( fitted == m_lineInformation[seriesIndex].m_fitted)					return true;
			m_lineInformation[seriesIndex].m_fitted = fitted;
		} break;

		case SeriesInLegend : {
			bool inLegend = value.toBool();
			if( inLegend == m_lineInformation[seriesIndex].m_inLegend)				return true;
			m_lineInformation[seriesIndex].m_inLegend = inLegend;
		} break;

		case SeriesBrush : {
			if( !value.canConvert<QBrush>())								return false;
			QBrush newBrush = value.value<QBrush>();
			if( newBrush == m_lineInformation[seriesIndex].m_brush)			return true;
			m_lineInformation[seriesIndex].m_brush = newBrush;
		} break;

		default :																	return false;
	}

	emit seriesViewChanged(seriesIndex, seriesIndex, seriesDataRole);
	return true;
}


void AbstractLineSeriesModel::setSeriesData(int seriesIndex, const LineInformation& value) {
	Q_ASSERT( seriesIndex >= 0 && seriesIndex < static_cast<int>(seriesCount()) );
	m_lineInformation[seriesIndex] = value;
	emit seriesChanged(seriesIndex, seriesIndex);
}


void AbstractLineSeriesModel::swapSeriesData(int first, int second) {
	Q_ASSERT(first >= 0 && first < m_lineInformation.size());
	Q_ASSERT(second >= 0 && second < m_lineInformation.size());
#if QT_VERSION < QT_VERSION_CHECK(5, 13, 0)
	m_lineInformation.swap(first, second);
#else
	m_lineInformation.swapItemsAt(first, second);
#endif
}


void AbstractLineSeriesModel::beginInsertSeries ( int first, int last) {
	// prepare for insertion of series
	AbstractChartModel::beginInsertSeries(first, last);

	// increase vector with line information
	for( int i=first; i<=last; ++i) {
		m_lineInformation.insert(i, LineInformation());
	}
}


void AbstractLineSeriesModel::beginRemoveSeries ( int first, int last ) {
	// prepare for removal of series
	AbstractChartModel::beginRemoveSeries(first, last);

	// remove selected series
	for (int i=first; i<=last; ++i) {
		// Mind: in this loop always remove the first, since when I remove series
		//       at index 2, the series 3 becomes the new series 2
		m_lineInformation.removeAt(first);
	}
}


void AbstractLineSeriesModel::clearCachedData() {
	m_lineInformation.clear();
	AbstractCartesianChartModel::clearCachedData();
}


void AbstractLineSeriesModel::writeXML(TiXmlElement * parent) const {

	TiXmlElement * e = new TiXmlElement("SeriesProperties");
	parent->LinkEndChild(e);

	LineInformation defaultValues;

	for (int i=0; i<m_lineInformation.count(); ++i) {

		TiXmlElement * es = new TiXmlElement("Series");
		e->LinkEndChild(es);

		if( m_writeAll || m_lineInformation[i].m_leftAxis != defaultValues.m_leftAxis)
			es->SetAttribute("leftAxis", IBK::val2string<bool>(m_lineInformation[i].m_leftAxis) );
		if( m_writeAll || m_lineInformation[i].m_inverted != defaultValues.m_inverted)
			es->SetAttribute("inverted", IBK::val2string<bool>(m_lineInformation[i].m_inverted) );
		if( m_writeAll || m_lineInformation[i].m_fitted != defaultValues.m_fitted)
			es->SetAttribute("fitted", IBK::val2string<bool>(m_lineInformation[i].m_fitted) );
		if( m_writeAll || m_lineInformation[i].m_inLegend != defaultValues.m_inLegend)
			es->SetAttribute("inLegend", IBK::val2string<bool>(m_lineInformation[i].m_inLegend) );

		if ( m_writeAll || !m_lineInformation[i].m_title.isEmpty() )
			TiXmlElement::appendSingleAttributeElement( es, "TitleText", nullptr, std::string(), m_lineInformation[i].m_title.toStdString() );

		if( m_writeAll || m_lineInformation[i].m_lineStyle != defaultValues.m_lineStyle)
			TiXmlElement::appendSingleAttributeElement( es, "LineStyle", nullptr, std::string(), IBK::val2string(m_lineInformation[i].m_lineStyle) );
		if( m_writeAll || m_lineInformation[i].m_markerStyle != defaultValues.m_markerStyle)
			TiXmlElement::appendSingleAttributeElement( es, "MarkerStyle", nullptr, std::string(), IBK::val2string(m_lineInformation[i].m_markerStyle) );
		if( m_writeAll || m_lineInformation[i].m_pen.width() != defaultValues.m_pen.width())
			TiXmlElement::appendSingleAttributeElement( es, "PenWidth", nullptr, std::string(), IBK::val2string(m_lineInformation[i].m_pen.width()) );
		if( m_writeAll || m_lineInformation[i].m_pen.style() != defaultValues.m_pen.style())
			TiXmlElement::appendSingleAttributeElement( es, "PenStyle", nullptr, std::string(), IBK::val2string(m_lineInformation[i].m_pen.style()) );
		if( m_writeAll || m_lineInformation[i].m_pen.color() != defaultValues.m_pen.color())
			TiXmlElement::appendSingleAttributeElement( es, "PenColor", nullptr, std::string(), m_lineInformation[i].m_pen.color().name().toStdString() );
		if( m_writeAll || m_lineInformation[i].m_markerColor != defaultValues.m_markerColor)
			TiXmlElement::appendSingleAttributeElement( es, "MarkerColor", nullptr, std::string(), m_lineInformation[i].m_markerColor.name().toStdString() );
		if( m_writeAll || m_lineInformation[i].m_markerSize != defaultValues.m_markerSize) {
			QString text = QString("%1").arg(m_lineInformation[i].m_markerSize);
			TiXmlElement::appendSingleAttributeElement( es, "MarkerSize", nullptr, std::string(), text.toStdString() );
		}
		if( m_writeAll || m_lineInformation[i].m_markerFilled != defaultValues.m_markerFilled)
			TiXmlElement::appendSingleAttributeElement( es, "MarkerFilled", nullptr, std::string(), IBK::val2string<bool>(m_lineInformation[i].m_markerFilled) );
		if( m_writeAll || m_lineInformation[i].m_brush.style() != defaultValues.m_brush.style())
			TiXmlElement::appendSingleAttributeElement( es, "BrushStyle", nullptr, std::string(), IBK::val2string(m_lineInformation[i].m_brush.style()) );
		if( m_writeAll || m_lineInformation[i].m_brush.color() != defaultValues.m_brush.color())
			TiXmlElement::appendSingleAttributeElement( es, "BrushColor", nullptr, std::string(), m_lineInformation[i].m_brush.color().name().toStdString() );
	}

	AbstractCartesianChartModel::writeXML(parent);
}


void AbstractLineSeriesModel::readXML(const TiXmlElement * element) {

	const char * const FUNC_ID = "[AbstractLineSeriesModel::readXML]";
	const TiXmlElement * xmlElem = element->FirstChildElement( "SeriesProperties" );
	if (xmlElem != nullptr) {
		try {

			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "Series") {

					// create temporaray LineInformation data structure
					LineInformation lineInfo;

					const TiXmlAttribute * leftAxis = TiXmlAttribute::attributeByName(e, "leftAxis");
					if (leftAxis)
						lineInfo.m_leftAxis = IBK::string2val<bool>(leftAxis->Value());

					const TiXmlAttribute * inverted = TiXmlAttribute::attributeByName(e, "inverted");
					if (inverted)
						lineInfo.m_inverted = IBK::string2val<bool>(inverted->Value());

					const TiXmlAttribute * fitted = TiXmlAttribute::attributeByName(e, "fitted");
					if (fitted)
						lineInfo.m_fitted = IBK::string2val<bool>(fitted->Value());

					const TiXmlElement * titleText = e->FirstChildElement("TitleText");
					if (titleText)
						lineInfo.m_title = titleText->GetText();

					const TiXmlElement * lineStyle = e->FirstChildElement("LineStyle");
					if (lineStyle)
						lineInfo.m_lineStyle = IBK::string2val<int>(lineStyle->GetText());

					const TiXmlElement * markerStyle = e->FirstChildElement("MarkerStyle");
					if (markerStyle)
						lineInfo.m_markerStyle = IBK::string2val<int>(markerStyle->GetText());

					const TiXmlElement * penWidth = e->FirstChildElement("PenWidth");
					if (penWidth)
						lineInfo.m_pen.setWidth(IBK::string2val<int>(penWidth->GetText()));

					const TiXmlElement * penStyle = e->FirstChildElement("PenStyle");
					if (penStyle)
						lineInfo.m_pen.setStyle(static_cast<Qt::PenStyle>(IBK::string2val<int>(penStyle->GetText())));

					const TiXmlElement * penColor = e->FirstChildElement("PenColor");
					if (penColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(penColor->GetText());
#else
						col.setNamedColor(penColor->GetText());
#endif
						lineInfo.m_pen.setColor(col);
					}

					const TiXmlElement * markerColor = e->FirstChildElement("MarkerColor");
					if (markerColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(markerColor->GetText());
#else
						col.setNamedColor(markerColor->GetText());
#endif
						lineInfo.m_markerColor = col;
					}

					const TiXmlElement * markerFilled = e->FirstChildElement("MarkerFilled");
					if (markerFilled)
						lineInfo.m_markerFilled = IBK::string2val<bool>(markerFilled->GetText());

					const TiXmlElement * markerSize = e->FirstChildElement("MarkerSize");
					if (markerSize) {
						unsigned int msize = 6;
						try {
							msize = IBK::string2val<unsigned int>(markerSize->GetText());
						}
						catch(...) {}
						lineInfo.m_markerSize = msize;
					}

					const TiXmlAttribute * inLegend = TiXmlAttribute::attributeByName(e, "inLegend");
					if (inLegend)
						lineInfo.m_inLegend = IBK::string2val<bool>(inLegend->Value());

					const TiXmlElement * brushStyle = e->FirstChildElement("BrushStyle");
					if (brushStyle)
						lineInfo.m_brush.setStyle(static_cast<Qt::BrushStyle>(IBK::string2val<int>(brushStyle->GetText())));

					const TiXmlElement * brushColor = e->FirstChildElement("BrushColor");
					if (brushColor) {
						QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
						col = QColor::fromString(brushColor->GetText());
#else
						col.setNamedColor(brushColor->GetText());
#endif
						lineInfo.m_brush.setColor(col);
					}

					// successfully read line data, now store line information

					m_lineInformation.push_back(lineInfo);
				}
			}
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading chart series."), FUNC_ID);
		}
	}

	AbstractCartesianChartModel::readXML(element);
}


// *** private functions ***

QColor AbstractLineSeriesModel::colorFromIndex(unsigned int index) {
	return colorFromDefault(index);
}




} // namespace SCI
