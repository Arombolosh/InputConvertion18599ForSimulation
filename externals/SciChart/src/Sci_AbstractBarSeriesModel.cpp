/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_AbstractBarSeriesModel.h"

#include <QByteArray>
#include <QDataStream>

#include <IBK_StringUtils.h>
#include <IBK_assert.h>

#include <tinyxml.h>

namespace SCI {


AbstractBarSeriesModel::AbstractBarSeriesModel(QObject *parent) :
	AbstractCartesianChartModel(parent)
{
}

QVariant AbstractBarSeriesModel::seriesData(int seriesIndex, int seriesDataRole) const {
	// test if index is valid
	if( seriesIndex < 0 || seriesIndex >= static_cast<int>(seriesCount()) )
		return QVariant();

	switch (seriesDataRole) {

		case SeriesTitleText :
		case SeriesTitle :
			return m_barChartInformation.m_title;
		case LayoutPolicy:
			return m_barChartInformation.m_layoutPolicy;
		case LayoutHint:
			return m_barChartInformation.m_layoutHint;
		case Spacing:
			return m_barChartInformation.m_spacing;
		case Margin:
			return m_barChartInformation.m_margin;
		case BaseLine:
			return m_barChartInformation.m_baseLine;
		case BarChartStyle:
			return m_barChartInformation.m_barChartStyle;
		case Orientation:
			return m_barChartInformation.m_orientation;
	}

	return QVariant();
}

bool AbstractBarSeriesModel::setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) {
	// test if index is valid
	if( seriesIndex != 0 )
		return false;

	switch (seriesDataRole) {
		case SeriesTitleText : {
			QString text = value.toString();
			m_barChartInformation.m_title = text;
			break;
		}
		case LayoutPolicy : {
			QwtPlotAbstractBarChart::LayoutPolicy layoutPolicy = static_cast<QwtPlotAbstractBarChart::LayoutPolicy>(value.toInt());
			if( layoutPolicy == m_barChartInformation.m_layoutPolicy)
				return true;

			m_barChartInformation.m_layoutPolicy = layoutPolicy;
			break;
		}
		case LayoutHint : {
			bool canConvert;
			double layoutHint = value.toDouble(&canConvert);
			if (!canConvert)
				return false;

			if( layoutHint == m_barChartInformation.m_layoutHint)
				return true;

			m_barChartInformation.m_layoutHint = layoutHint;
			break;
		}
		case Spacing : {
			bool canConvert;
			int spacing = value.toInt(&canConvert);
			if (!canConvert)
				return false;

			if( spacing == m_barChartInformation.m_spacing)
				return true;

			m_barChartInformation.m_spacing = spacing;
			break;
		}
		case Margin : {
			bool canConvert;
			int margin = value.toInt(&canConvert);
			if (!canConvert)
				return false;

			if( margin == m_barChartInformation.m_margin)
				return true;

			m_barChartInformation.m_margin = margin;
			break;
		}
		case BaseLine : {
			bool canConvert;
			double baseLine = value.toDouble(&canConvert);
			if (!canConvert)
				return false;

			if( baseLine == m_barChartInformation.m_baseLine)
				return true;

			m_barChartInformation.m_baseLine = baseLine;
			break;
		}
		case BarChartStyle : {
			QwtPlotMultiBarChart::ChartStyle style = static_cast<QwtPlotMultiBarChart::ChartStyle>(value.toInt());
			if( style == m_barChartInformation.m_barChartStyle)
				return true;

			m_barChartInformation.m_barChartStyle = style;
			break;
		}
		case Orientation : {
			Qt::Orientation orientation = static_cast<Qt::Orientation>(value.toInt());
			if( orientation == m_barChartInformation.m_orientation)
				return true;

			m_barChartInformation.m_orientation = orientation;
			break;
		}

		default: return false;
	}

	emit seriesViewChanged(0, 0, seriesDataRole);
	return true;
}

void AbstractBarSeriesModel::setSeriesData(const BarChartInformation& value) {
	m_barChartInformation = value;
	emit seriesChanged(0, 0); // we emit seriesChanged, because we change several properties at once
}

void AbstractBarSeriesModel::setValueColor(unsigned int index, const QColor& color) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	m_barChartInformation.m_barSymbolInformations[index].m_palette.setColor(QPalette::Window, color);
	emit seriesViewChanged(0, 0, Information);
}

QColor AbstractBarSeriesModel::valueColor(unsigned int index) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	return m_barChartInformation.m_barSymbolInformations[index].m_palette.color(QPalette::Window);
}

void AbstractBarSeriesModel::setValuePalette(unsigned int index, const QPalette& palette) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	m_barChartInformation.m_barSymbolInformations[index].m_palette = palette;
	emit seriesViewChanged(0, 0, Information);
}

QPalette AbstractBarSeriesModel::valuePalette(unsigned int index) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	return m_barChartInformation.m_barSymbolInformations[index].m_palette;
}

void AbstractBarSeriesModel::setValueFrameLineWidth(unsigned int index, int lineWidth) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	m_barChartInformation.m_barSymbolInformations[index].m_lineWidth = lineWidth;
	emit seriesViewChanged(0, 0, Information);
}

int	AbstractBarSeriesModel::frameLineWidth(unsigned int index) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	return m_barChartInformation.m_barSymbolInformations[index].m_lineWidth;
}

void AbstractBarSeriesModel::setValueFrameStyle(unsigned int index, QwtColumnSymbol::FrameStyle style) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	m_barChartInformation.m_barSymbolInformations[index].m_frameStyle = style;
	emit seriesViewChanged(0, 0, Information);
}

QwtColumnSymbol::FrameStyle	AbstractBarSeriesModel::frameStyle(unsigned int index) {
	Q_ASSERT(index < m_barChartInformation.m_barSymbolInformations.size());
	return m_barChartInformation.m_barSymbolInformations[index].m_frameStyle;
}

void AbstractBarSeriesModel::clearCachedData() {
	m_barChartInformation = BarChartInformation();
	AbstractCartesianChartModel::clearCachedData();
}

void AbstractBarSeriesModel::writeXML(TiXmlElement * parent) const {

	TiXmlElement * es = new TiXmlElement("BarSeriesProperties");
	parent->LinkEndChild(es);

	BarChartInformation		defaultChartValues;
	BarSymbolInformation	defaultSymbolValues;


	if ( m_writeAll || !m_barChartInformation.m_title.isEmpty() )
		TiXmlElement::appendSingleAttributeElement( es, "TitleText", NULL, std::string(), m_barChartInformation.m_title.toStdString() );

	if ( m_writeAll || m_barChartInformation.m_layoutPolicy != defaultChartValues.m_layoutPolicy )
		TiXmlElement::appendSingleAttributeElement( es, "LayoutPolicy", NULL, std::string(), IBK::val2string(m_barChartInformation.m_layoutPolicy) );

	if ( m_writeAll || m_barChartInformation.m_layoutHint != defaultChartValues.m_layoutHint )
		TiXmlElement::appendSingleAttributeElement( es, "LayoutHint", NULL, std::string(), IBK::val2string(m_barChartInformation.m_layoutHint) );

	if ( m_writeAll || m_barChartInformation.m_spacing != defaultChartValues.m_spacing )
		TiXmlElement::appendSingleAttributeElement( es, "Spacing", NULL, std::string(), IBK::val2string(m_barChartInformation.m_spacing) );

	if ( m_writeAll || m_barChartInformation.m_margin != defaultChartValues.m_margin )
		TiXmlElement::appendSingleAttributeElement( es, "Margin", NULL, std::string(), IBK::val2string(m_barChartInformation.m_margin) );

	if ( m_writeAll || m_barChartInformation.m_baseLine != defaultChartValues.m_baseLine )
		TiXmlElement::appendSingleAttributeElement( es, "BaseLine", NULL, std::string(), IBK::val2string(m_barChartInformation.m_baseLine) );

	if ( m_writeAll || m_barChartInformation.m_barChartStyle != defaultChartValues.m_barChartStyle )
		TiXmlElement::appendSingleAttributeElement( es, "BarChartStyle", NULL, std::string(), IBK::val2string(m_barChartInformation.m_barChartStyle) );

	if ( m_writeAll || m_barChartInformation.m_orientation != defaultChartValues.m_orientation )
		TiXmlElement::appendSingleAttributeElement( es, "Orientation", NULL, std::string(), IBK::val2string(m_barChartInformation.m_orientation) );

	for( size_t i=0; i<m_barChartInformation.m_barSymbolInformations.size(); ++i) {

		TiXmlElement * sectionProperty = new TiXmlElement("SectionProperty");
		es->LinkEndChild(sectionProperty);

		if ( m_writeAll || m_barChartInformation.m_barSymbolInformations[i].m_frameStyle != defaultSymbolValues.m_frameStyle )
			TiXmlElement::appendSingleAttributeElement( sectionProperty, "FrameStyle", NULL, std::string(), IBK::val2string(m_barChartInformation.m_barSymbolInformations[i].m_frameStyle) );
		if ( m_writeAll || m_barChartInformation.m_barSymbolInformations[i].m_lineWidth != defaultSymbolValues.m_lineWidth )
			TiXmlElement::appendSingleAttributeElement( sectionProperty, "LineWidth", NULL, std::string(), IBK::val2string(m_barChartInformation.m_barSymbolInformations[i].m_lineWidth) );

		if ( m_writeAll || m_barChartInformation.m_barSymbolInformations[i].m_palette != defaultSymbolValues.m_palette ) {
			QByteArray buffer;
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
			QDataStream palStream(&buffer, QDataStream::WriteOnly);
#else
			QDataStream palStream(&buffer, QIODevice::WriteOnly);
#endif
			palStream.setVersion(QDataStream::Qt_4_8);
			palStream << m_barChartInformation.m_barSymbolInformations[i].m_palette;
			std::string palStr = buffer.toStdString();
			TiXmlElement::appendSingleAttributeElement( sectionProperty, "Palette", NULL, std::string(), palStr);
		}
	}

	AbstractCartesianChartModel::writeXML(parent);
}

void AbstractBarSeriesModel::readXML(const TiXmlElement * element) {

	const char * const FUNC_ID = "[AbstractBarSeriesModel::readXML]";
	const TiXmlElement * xmlElem = element->FirstChildElement( "BarSeriesProperties" );

	if (xmlElem != NULL) {

		try {

			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "BarSeriesProperties") {

					const TiXmlElement * titleText = e->FirstChildElement("TitleText");
					if (titleText)
						m_barChartInformation.m_title = titleText->GetText();

					const TiXmlElement * layoutPolicy = e->FirstChildElement("LayoutPolicy");
					if (layoutPolicy)
						m_barChartInformation.m_layoutPolicy = static_cast<QwtPlotAbstractBarChart::LayoutPolicy>(IBK::string2val<int>(layoutPolicy->GetText()));

					const TiXmlElement * layoutHint = e->FirstChildElement("LayoutHint");
					if (layoutHint)
						m_barChartInformation.m_layoutHint = IBK::string2val<double>(layoutHint->GetText());

					const TiXmlElement * spacing = e->FirstChildElement("Spacing");
					if (spacing)
						m_barChartInformation.m_spacing = IBK::string2val<int>(spacing->GetText());

					const TiXmlElement * margin = e->FirstChildElement("Margin");
					if (margin)
						m_barChartInformation.m_margin = IBK::string2val<int>(margin->GetText());

					const TiXmlElement * baseLine = e->FirstChildElement("BaseLine");
					if (baseLine)
						m_barChartInformation.m_baseLine = IBK::string2val<double>(baseLine->GetText());

					const TiXmlElement * barChartStyle = e->FirstChildElement("BarChartStyle");
					if (barChartStyle)
						m_barChartInformation.m_barChartStyle = static_cast<QwtPlotMultiBarChart::ChartStyle>(IBK::string2val<int>(barChartStyle->GetText()));
				}

				const TiXmlElement * orientation = e->FirstChildElement("Orientation");
				if (orientation)
					m_barChartInformation.m_orientation = static_cast<Qt::Orientation>(IBK::string2val<int>(orientation->GetText()));

				// read sub-elements
				m_barChartInformation.m_barSymbolInformations.clear();
				for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {
					std::string ename = e->Value();
					if (ename == "SectionProperty") {

						BarSymbolInformation symbolInfo;

						const TiXmlElement * frameStyle = e->FirstChildElement("FrameStyle");
						if (frameStyle)
							symbolInfo.m_frameStyle = static_cast<QwtColumnSymbol::FrameStyle>(IBK::string2val<int>(frameStyle->GetText()));

						const TiXmlElement * lineWidth = e->FirstChildElement("LineWidth");
						if (lineWidth)
							symbolInfo.m_lineWidth = IBK::string2val<int>(lineWidth->GetText());

						std::string palStr;
						const TiXmlElement * paletteElem = e->FirstChildElement("Palette");
						if (paletteElem) {
							palStr = paletteElem->GetText();

							QByteArray buffer(QByteArray::fromStdString(palStr));
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
							QDataStream palStream(&buffer, QDataStream::ReadOnly);
#else
							QDataStream palStream(&buffer, QIODevice::ReadOnly);
#endif

							palStream.setVersion(QDataStream::Qt_4_8);
							QPalette palette;
							palStream >> palette;
						}

						m_barChartInformation.m_barSymbolInformations.push_back(symbolInfo);

					}
				}
			}
		}
		catch (IBK::Exception & ex) {
			throw IBK::Exception(ex, IBK::FormatString("Error reading chart series."), FUNC_ID);
		}
	}

	AbstractCartesianChartModel::readXML(element);

}

} // namespace SCI
