/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_ColorMapCreator.h"

#include <qwt_color_map.h>

#include "Sci_ColorMap.h"

#include <IBK_StringUtils.h>
#include <IBK_messages.h>

namespace SCI {

// *** ColorMapCreator ***

QwtColorMap* ColorMapCreator::createColorMap(const ColorMap & colMap, bool bandedColors) {
	const char * const FUNC_ID = "[ColorMapCreator::createColorMap]";
	switch (colMap.m_type) {
		case ColorMap::Linear : {
			QwtLinearColorMap * cmap = new QwtLinearColorMap();
			// Note: LinearColorMap is a QwtLinearColorMap with value blanking
			if (bandedColors)
				cmap->setMode(QwtLinearColorMap::FixedColors);
			else
				cmap->setMode(QwtLinearColorMap::ScaledColors);

			// Note: cmap now holds already first and last color stop
			// sanity check, must have at least two colors in linear color stops
			if (colMap.m_linearColorStops.size() < 2) {
				IBK::IBK_Message("Not enough color stops in color map.", IBK::MSG_ERROR, FUNC_ID);
				return cmap; // use default color map as safe-default
			}
			// ensure that color positions are 0 and 1 for first and last color stop
			if (colMap.m_linearColorStops.first().m_pos != 0 || colMap.m_linearColorStops.last().m_pos != 1) {
				IBK::IBK_Message(IBK::FormatString("Invalid color map, first position must be 0 (got %1), last position must be 1 (got %2).")
								 .arg(colMap.m_linearColorStops.first().m_pos)
								 .arg(colMap.m_linearColorStops.last().m_pos), IBK::MSG_ERROR, FUNC_ID);
				return cmap; // use default color map as safe-default
			}
			// set new interval
			cmap->setColorInterval(colMap.m_linearColorStops.first().m_color, colMap.m_linearColorStops.last().m_color);
			// insert all steps in-between, the QwtLinearColorMap sorts the new steps in between the outer steps
			for (int i=1; i<colMap.m_linearColorStops.size()-1; ++i ) {
				double currentVal = colMap.m_linearColorStops[i].m_pos;
				cmap->addColorStop( currentVal, colMap.m_linearColorStops[i].m_color );
			}
			return cmap;
		} break;

		case ColorMap::Alpha : {
			QwtAlphaColorMap* result = new QwtAlphaColorMap(colMap.m_alphaColor);
			// Note: AlphaColorMap is a QwtAlphaColorMap with value blanking
			return result;
		} break;
		default:;
	}
	return NULL;
}

} // namespace SCI
