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

#include "QtExt_ReportFrameItemImage.h"

#include <QPixmap>
#include <QPainter>

namespace QtExt {

ReportFrameItemImage::ReportFrameItemImage(const QPixmap* pixmap, QPaintDevice* paintDevice, double width, double spaceAfter, double spaceBefore, bool canPageBreakAfter) :
	ReportFrameItemBase(paintDevice, width, spaceAfter, spaceBefore, canPageBreakAfter),
	m_image(pixmap)
{
}

void ReportFrameItemImage::setCurrentRect() {
	QPixmap pixmap = m_image->scaledToWidth(m_width);

	// Height should not be bigger then width of image
	if (pixmap.height() > m_width)
		pixmap = m_image->scaledToHeight(m_width);

	m_currentRect = pixmap.rect();
}

void ReportFrameItemImage::drawItem(QPainter* painter, QPointF& pos) {
	QRect painterRect(pos.x(), pos.y(), m_currentRect.width(), m_currentRect.height());
	painter->drawPixmap(painterRect, *m_image);
}

} // namespace QtExt
