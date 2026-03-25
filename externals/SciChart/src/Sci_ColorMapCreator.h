/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ColorMapCreatorH
#define Sci_ColorMapCreatorH

class QwtColorMap;

namespace SCI {

class ColorMap;

/*! A pure factory class to construction QwtColorMap objects on the heap.
*/
class ColorMapCreator {
public:

	/*! Create and returns a colormap of current type (SCI::LinearColorMap or SCI::AlphaColorMap).
		Caller takes ownership.
		\param colMap The color map with color stops for normalized linear maps or alpha map color value.
		\param bandedColors Indicates in case of linear color maps whether fixed colors per interval shall be used
			or, if false, colors shall be interpolated between steps.
	*/
	static QwtColorMap*	createColorMap(const ColorMap & colMap, bool bandedColors);
};

} // namespace SCI


#endif // Sci_ColorMapCreatorH
