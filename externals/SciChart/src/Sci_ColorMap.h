/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ColorMapH
#define Sci_ColorMapH

#include <QMetaType>
#include <QColor>
#include <QVector>
#include <QRgb>

class TiXmlElement;

namespace SCI {

/*! A class that holds all data that describes a color map.
	Basically it holds the information on how to generate the color map (color1, color2, steps), and
	the final color map with computed steps.

	Typical ways to set create a color map are use of the convenience functions setAlphaMap() and
	setLinearMap(). But you can also just directly change the m_linearColorStops. Mind, that at least
	the stop positions 0 and 1 need to be present in the linear map.

	You may define a full colormap with many steps in m_completeLinearColorStops and then use the convenience
	function interpolateColorMap() to generate a sub-sampled color map with less steps (useful for banded color maps).

	\note The color map is independent of the actual value range of the data to be visualized and
	only needs to be updated when the color mapping to the normalized range (0,1) changes.
*/
class ColorMap {
public:
	/*! Specify colormap type.*/
	enum ColorMapType {
		/*! Several stop points with associated colors and linear gradient in RGB space between stop points. */
		Linear,
		/*! One color, value maps to alpha (transparency). */
		Alpha
	};

	/*! Contains color stops for linear colormap.*/
	struct ColorStop {
		double	m_pos;		///< Position value, must be between 0 and 1.
		QColor	m_color;	///< Color for this position.

		/*! Standard constructor. */
		ColorStop() :
			m_pos(0),
			m_color(Qt::black)
		{
		}

		/*! Value constructor.
			\arg val Position value.
			\arg col Color for position.
		*/
		ColorStop(double val, QColor col) :
			m_pos(val),
			m_color(col)
		{
		}

		/*! Comparison operator to allow ordered search through color maps. */
		bool operator<(double pos) const {
			return m_pos < pos;
		}
	};

	/*! Standard constructor.*/
	ColorMap();

	/*! Sets type to Alpha and stores alphaColor, clears m_linearColorStops. */
	void setAlphaMap(QColor alphaColor);

	/*! Sets type to linear, and generates a color map between color1 and color2 usings the number of desired steps.
		color1 matches position 0, color2 matches position 1. For example, if steps = 10, colors will
		be generated for positions 0.1, 0.2, 0.3, ..., 0.9.

		Currently, only the HSV wheel color generation algorithm is implemented.
	*/
	void setLinearMap(QColor color1, QColor color2, unsigned int steps);

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	void writeXML(TiXmlElement * parent) const;

	/*! Builds an interpolated color map with the given number of steps based on the m_completeLinearColorStops. */
	void interpolateColorMap(unsigned int steps);

	ColorMapType			m_type;							///< Type of colormap.
	QColor					m_alphaColor;					///< Color for alpha colormap.
	QVector<ColorStop>		m_originalLinearColorStops;		///< Color stops of original, full linear colormap.
	QVector<ColorStop>		m_linearColorStops;				///< Color stops of interpolated linear colormap (usually a subset of the original color map).
};

} // namespace SCI

Q_DECLARE_METATYPE(SCI::ColorMap)

#endif // Sci_ColorMapH
