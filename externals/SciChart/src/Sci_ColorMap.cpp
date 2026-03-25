/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_ColorMap.h"

#include <tinyxml.h>

#include <IBK_StringUtils.h>

namespace SCI {

ColorMap::ColorMap() :
	m_type(Linear),
	m_alphaColor(Qt::black)
{
	// standard color map goes from red (hsv color 0) to blue (hsv color 240)
	setLinearMap(Qt::red, Qt::blue, 10);
}


void ColorMap::setAlphaMap(QColor alphaColor) {
	m_type = Alpha;
	m_alphaColor = alphaColor;
	m_linearColorStops.clear();
	m_originalLinearColorStops.clear();
}


void ColorMap::setLinearMap(QColor color1, QColor color2, unsigned int steps) {
	FUNCID("ColorMap::setLinearMap");
	m_type = Linear;
	if (steps < 1)
		throw IBK::Exception("Invalid number of steps, must be > 1.", FUNC_ID);
	// first get hue and saturation values from colors
	double h1 = color1.hueF();
	double h2 = color2.hueF();
	if (h2 == -1)
		h2 = h1;
	else if (h1 == -1)
		h1 = h2;
	double s1 = color1.saturationF();
	double s2 = color2.saturationF();
	double v1 = color1.valueF();
	double v2 = color2.valueF();
	m_linearColorStops.clear();
	m_linearColorStops.push_back(ColorStop(0, color1) );

	// example: steps = 4
	// pos 0    : color1 - hue 0
	// pos 0.25          - hue 90
	// pos 0.50          - hue 180
	// pos 0.75          - hue 270
	// pos 1    : color2 - hue 360
	// delta-hue = 360/4

	// add all steps between color1 and color2 (we add color2 manually at the end
	// to avoid rounding errors)
	for (unsigned int i=0; i<steps-1; ++i ) {
		double pos = (i+1.0)/steps;
		QColor c = QColor::fromHsvF( pos*h2 + (1-pos)*h1, pos*s2 + (1-pos)*s1, pos*v2 + (1-pos)*v1);
		m_linearColorStops.push_back(ColorStop(pos, c) );
	}
	m_linearColorStops.push_back(ColorStop(1.0, color2) );
	m_originalLinearColorStops.clear();
}


void ColorMap::readXML(const TiXmlElement * element) {
	FUNCID("ColorMap::readXML");
	try {
		// type
		const TiXmlElement * typeE = element->FirstChildElement("Type");
		if (typeE){
			std::string m = typeE->GetText();
			if ( m == "LinearMap" ) {
				m_type = Linear;
			}
			else if ( m == "AlphaMap" ) {
				m_type = Alpha;
			}
		}

		const TiXmlElement * xmlElem = element->FirstChildElement( "ColorStops" );
		m_linearColorStops.clear();
		if (xmlElem != NULL) {

			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "ColorStop") {

					const TiXmlAttribute * index = TiXmlAttribute::attributeByName(e, "index");
					if (!index)
						throw IBK::Exception(IBK::FormatString("Expected 'index' attribute in Series."), FUNC_ID);

					const TiXmlAttribute * position = TiXmlAttribute::attributeByName(e, "position");
					if (!position)
						throw IBK::Exception(IBK::FormatString("Expected 'position' attribute in Series."), FUNC_ID);

					double positionValue = IBK::string2val<double>(position->Value());

					const TiXmlAttribute * color = TiXmlAttribute::attributeByName(e, "color");
					if (!color)
						throw IBK::Exception(IBK::FormatString("Expected 'color' attribute in Series."), FUNC_ID);

					QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
					col = QColor::fromString(color->Value());
#else
					col.setNamedColor(color->Value());
#endif

					m_linearColorStops.push_back( ColorStop( positionValue, col ) );
				}
			}
		}
		xmlElem = element->FirstChildElement( "OriginalColorStops" );
		m_originalLinearColorStops.clear(); // we always clear the original color map and only keep it, if we have it in file
		if (xmlElem != NULL) {

			// read sub-elements
			for (const TiXmlElement * e = xmlElem->FirstChildElement(); e; e = e->NextSiblingElement()) {

				std::string ename = e->Value();
				if (ename == "ColorStop") {

					const TiXmlAttribute * index = TiXmlAttribute::attributeByName(e, "index");
					if (!index)
						throw IBK::Exception(IBK::FormatString("Expected 'index' attribute in Series."), FUNC_ID);

					const TiXmlAttribute * position = TiXmlAttribute::attributeByName(e, "position");
					if (!position)
						throw IBK::Exception(IBK::FormatString("Expected 'position' attribute in Series."), FUNC_ID);

					double positionValue = IBK::string2val<double>(position->Value());

					const TiXmlAttribute * color = TiXmlAttribute::attributeByName(e, "color");
					if (!color)
						throw IBK::Exception(IBK::FormatString("Expected 'color' attribute in Series."), FUNC_ID);

					QColor col = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
					col = QColor::fromString(color->Value());
#else
					col.setNamedColor(color->Value());
#endif

					m_originalLinearColorStops.push_back( ColorStop( positionValue, col ) );
				}
			}
			// in case of color map template files, we only have the original color stops and so we
			// duplicate it in the linear color stops.
			if (m_linearColorStops.isEmpty())
				m_linearColorStops = m_originalLinearColorStops;
		}

		// for alpha map
		const TiXmlElement * color = element->FirstChildElement("AlphaColor");
		if (color != NULL) {
			m_alphaColor = QColor();
#if QT_VERSION >= QT_VERSION_CHECK(6,4,0)
			m_alphaColor = QColor::fromString(color->GetText());
#else
			m_alphaColor.setNamedColor(color->GetText());
#endif
		}
	}
	catch (IBK::Exception & ex) {
		throw IBK::Exception(ex, IBK::FormatString("Error reading ColorMap."), FUNC_ID);
	}
}


void ColorMap::writeXML(TiXmlElement * parent) const {

	switch ( m_type ) {
		case Linear : TiXmlElement::appendSingleAttributeElement( parent, "Type", NULL, std::string(), "LinearMap" ); break;
		case Alpha : TiXmlElement::appendSingleAttributeElement( parent, "Type", NULL, std::string(), "AlphaMap" ); break;
	}

	TiXmlElement * e = new TiXmlElement("ColorStops");
	parent->LinkEndChild(e);

	// We may have a complete color map that our m_linearColorStops is based on. Or we may just have data
	// in m_linearColorStops. In the first case, we dump out the entire color map but store as attribute the number of steps
	// we want to have in the linear color map.

	// when we write the color map, we write always the currently used color map
	for ( int i=0; i<m_linearColorStops.size(); ++i ) {

		QColor col = m_linearColorStops.at(i).m_color;
		double value = m_linearColorStops.at(i).m_pos;

		TiXmlElement * es = new TiXmlElement("ColorStop");
		e->LinkEndChild(es);

		es->SetAttribute("index", IBK::val2string(i) );
		es->SetAttribute("position", IBK::val2string<double>(value));
		es->SetAttribute("color", col.name().toStdString() );
	}

	if (!m_originalLinearColorStops.isEmpty()) {
		TiXmlElement * e = new TiXmlElement("OriginalColorStops");
		parent->LinkEndChild(e);

		// We may have a complete color map that our m_linearColorStops is based on. Or we may just have data
		// in m_linearColorStops. In the first case, we dump out the entire color map but store as attribute the number of steps
		// we want to have in the linear color map.

		// when we write the color map, we write always the currently used color map
		for ( int i=0; i<m_originalLinearColorStops.size(); ++i ) {

			QColor col = m_originalLinearColorStops.at(i).m_color;
			double value = m_originalLinearColorStops.at(i).m_pos;

			TiXmlElement * es = new TiXmlElement("ColorStop");
			e->LinkEndChild(es);

			es->SetAttribute("index", IBK::val2string(i) );
			es->SetAttribute("position", IBK::val2string<double>(value));
			es->SetAttribute("color", col.name().toStdString() );
		}
	}

	TiXmlElement::appendSingleAttributeElement( parent, "AlphaColor", NULL, std::string(), m_alphaColor.name().toStdString() );
}


void ColorMap::interpolateColorMap(unsigned int steps) {
	FUNCID(ColorMap::interpolateColorMap);
	// this only works if our complete color map has at least 2 colors
	if (m_originalLinearColorStops.size() < 2)
		throw IBK::Exception("Invalid complete color map!", FUNC_ID);
	m_linearColorStops.clear();
	m_linearColorStops.push_back(ColorStop(0, m_originalLinearColorStops.front().m_color) );
	for (unsigned int i=0; i<steps-1; ++i ) {
		double pos = (i+1.0)/steps;
		// lookup color stops in original color map and interpolate between these values
		unsigned int idx = std::distance(m_originalLinearColorStops.begin(), std::lower_bound(m_originalLinearColorStops.begin(), m_originalLinearColorStops.end(), pos));
		unsigned int idx2 = std::min<unsigned int>(idx + 1, m_originalLinearColorStops.size()-1);
		QColor c;
		if (idx == idx2) {
			c = m_originalLinearColorStops[idx].m_color;
		}
		else {
			// now interpolate the colors
			double w = (pos - m_originalLinearColorStops[idx].m_pos)/(m_originalLinearColorStops[idx2].m_pos - m_originalLinearColorStops[idx].m_pos);
			QColor c1 = m_originalLinearColorStops[idx].m_color;
			QColor c2 = m_originalLinearColorStops[idx2].m_color;
			c = QColor::fromRgbF( (1-w)*c1.redF() + w*c2.redF(),
								  (1-w)*c1.greenF() + w*c2.greenF(),
								  (1-w)*c1.blueF() + w*c2.blueF()
								);
		}
		m_linearColorStops.push_back(ColorStop(pos, c) );
	}
	m_linearColorStops.push_back(ColorStop(1, m_originalLinearColorStops.back().m_color) );
}



} // namespace SCI
