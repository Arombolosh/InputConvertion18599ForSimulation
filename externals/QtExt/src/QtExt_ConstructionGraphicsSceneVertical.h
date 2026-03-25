/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#ifndef QtExt_ConstructionGraphicsSceneVerticalH
#define QtExt_ConstructionGraphicsSceneVerticalH

#include "QtExt_ConstructionGraphicsSceneBase.h"
#include "QtExt_GraphicsRectItemWithHatch.h"
#include "QtExt_ConstructionLayer.h"

namespace QtExt {

/*! \brief Graphicsscene for showing a simple one dimensional wall structure.
	All necessary informations are given by setup.
*/
class QtExt_EXPORT ConstructionGraphicsSceneVertical : public ConstructionGraphicsSceneBase
{
	Q_OBJECT
public:

	/*! Default constructor.
		\param fontFamily Font family used for all fonts.
		\param onScreen Is true if painting is on screen.
		\param device Used paint device.
		\param parent Parent widget.
	*/
	ConstructionGraphicsSceneVertical(bool onScreen, QPaintDevice *device, QObject* parent = 0);

private:

	/*! Updates m_xpos, m_innerFrame, m_xiLeft, m_xiRight and m_fontScale based on the input data.*/
	void calculatePositions();

	/*! Calculates and draws the dimensions.*/
	void drawDimensions();

	/*! Draws the wall including hashing.*/
	void drawWall();

	/*! Draw marker lines then exist.*/
	void drawMarkerLines();

	/*! Draw a marker area rect.*/
	void drawMarkerArea();

	/*! Y-coordinate (top) of inner frame.*/
	int					m_yiTop;
	/*! Y-coordinate (bottom) of inner frame.*/
	int					m_yiBottom;
};

} // namespace QtExt

#endif // QtExt_ConstructionGraphicsSceneVerticalH
