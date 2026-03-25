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

#include "QtExt_ComboBoxFontSize.h"

#include <QFile>
#include <QTextStream>
#include <QFontDatabase>

namespace QtExt {

ComboBoxFontSize::ComboBoxFontSize(QWidget *parent) :
	QComboBox (parent)
{
	setFont(this->font());
}

void ComboBoxFontSize::setFont(QFont font) {
	QFontDatabase database;
	QList<int> fontSizes = database.pointSizes(font.family());
	clear();
	int currentSize = font.pointSize();
	int currentIndex = -1;
	for(QList<int>::Iterator it=fontSizes.begin(); it != fontSizes.end(); ++it) {
		int size = *it;
		addItem(QString("%1").arg(size), size);
		if(currentIndex == -1 && size >= currentSize) {
			if(currentSize == size)
				currentIndex = it - fontSizes.begin();
			else {
				it = fontSizes.insert(it, currentSize);
				currentIndex = it - fontSizes.begin() + 1;
			}
		}
	}
	setCurrentIndex(currentIndex);
}

int ComboBoxFontSize::currentFontSize() {
	return currentData().toInt();
}

void ComboBoxFontSize::setFontSize(int size) {
	for(int i=0; i<count(); ++i) {
		int s = itemData(i).toInt();
		if( s >= size) {
			setCurrentIndex(i);
			return;
		}
	}
	setCurrentIndex(count() - 1);
}



} // namespace QtExt
