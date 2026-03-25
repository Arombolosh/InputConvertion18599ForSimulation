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

#include "QtExt_ConstructionGraphicsSceneHorizontal.h"

#include <memory>

#include <QAbstractTextDocumentLayout>
#include <QTextBlock>
#include <QTextLayout>
#include <QGraphicsTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QTextDocument>
#include <QApplication>
#include <QStyle>
#include <QWidget>
#include <QPainter>

#include "QtExt_TextFrame.h"

namespace QtExt {


ConstructionGraphicsSceneHorizontal::ConstructionGraphicsSceneHorizontal(bool onScreen, QPaintDevice *device, QObject* parent) :
	ConstructionGraphicsSceneBase(onScreen, device, parent),
	m_xiLeft(0),
	m_xiRight(0)
{
}


void ConstructionGraphicsSceneHorizontal::calculatePositions() {
	if(m_inputData.empty())
		return;

	// get axis text geometry
	int leftAxisMargins = m_textHeight10;
	int rightAxisMargins = m_textHeight10;
	int top = m_frame.top() + 5;
	int bottom = m_frame.height() - 15;

	int frameTop = m_frame.top();
	int frameBottom = m_frame.bottom();

	if (!m_onScreen) {
		top = static_cast<int>(frameTop + 5 * m_res);
		bottom = static_cast<int>(frameBottom - 5 * m_res);
	}

	std::unique_ptr<QTextDocument> textDocument(new QTextDocument);
	textDocument->documentLayout()->setPaintDevice(m_device);
	textDocument->setDefaultFont(m_axisTitleFont);

	// on screen, we always keep space at the top and bottom
	leftAxisMargins = 0;
	rightAxisMargins = 0;

	int left = m_frame.left() + leftAxisMargins;
	int right = m_frame.width() - rightAxisMargins;
	m_innerFrame.setRect(left, top, right - left, bottom - top);

	// calculation x positions
	int axis_left, axis_right; // for axes and description
	int m_airLayer;             // for boundary layer

	if (m_onScreen) {
		axis_left = 40;
		axis_right = 50;
		m_airLayer = static_cast<int>(std::max(m_innerFrame.width()*0.03, 10.0));
	} else {
		axis_left = static_cast<int>(5 * m_res);
		axis_right = static_cast<int>(5 * m_res);
		m_airLayer = static_cast<int>(std::max(m_innerFrame.width()*0.03, 10.0 * m_res));
	}
	if(!m_visibleBoundaryLabels) {
		m_airLayer = 0;
	}

	m_xiLeft = m_innerFrame.left() + axis_left;
	m_xiRight = m_innerFrame.right() - axis_right;

	// calculate width of wall itself in pixdel
	int pixwidth = m_xiRight - m_xiLeft - 2 * m_airLayer;

	// resize vector for x positions
	m_pos.resize(m_inputData.size() + 1);
	m_pos[0] = m_xiLeft + m_airLayer; // left wall boundary

	// calculate scaling factor for wall coordinates in pixel/m
	double dw = pixwidth/m_constructionSize;
	// calculate layer thicknesses in pixel and store x positions
	for (int i=0; i<m_inputData.size(); ++i) {
		m_pos[i+1] = m_pos[i] + static_cast<int>(m_inputData[i].m_width * dw);
	}
}

void ConstructionGraphicsSceneHorizontal::drawDimensions() {
	if(m_inputData.empty())
		return;

#ifdef Q_OS_MAC
		const double MACYShift = 10.0;
#endif

	setFont(m_tickFont);

	// set here the distance in pixels between dim label and dim line
	// this distance is used everywhere as distance
	const double dist = static_cast<int>(std::max(0.8 * m_res, 3.0));

	// store number of x positions
	const int n = (int)m_pos.size();

	// check if all dimensions fit between the helping lines,
	// and while at it, calculate text widths

	std::vector<LabelProperties> labelVectTop;
	std::vector<LabelProperties> labelVectBottom;
	double maxTextHeight(0);
	for (int i=0; i<n-1; ++i) {
		LabelProperties tempLabel;
		tempLabel.m_text = dimLabel(m_inputData[i].m_width);
		tempLabel.m_textItem = addText(tempLabel.m_text, m_tickFont);
		tempLabel.m_brect = tempLabel.m_textItem->boundingRect();
		tempLabel.m_index = i;
		maxTextHeight = std::max(maxTextHeight, tempLabel.m_brect.height());
		int width_available = m_pos[i+1] - m_pos[i] - 2 * dist;
		// check for fit between lines
		if (tempLabel.m_brect.width() > width_available) {
			tempLabel.m_fit = false;
			labelVectBottom.push_back(tempLabel);
		} else {
			labelVectTop.push_back(tempLabel);
		}
	}
	double linespacing = maxTextHeight;

	// calculate dim lines and labels
	std::vector<double> ylines;
	double linePos = m_innerFrame.bottom() - linespacing;
	ylines.push_back(linePos);		// main dim line
	double yo = m_innerFrame.bottom() - 2.1 * linespacing;
	if( n > 2)
		ylines.push_back(yo);		// secondary dim line

	// did we have a label too big for the available space?
	if (!labelVectBottom.empty() && n > 1) {
		yo = m_innerFrame.bottom() - 3.1 * linespacing;
		ylines.back() = yo;	// secondary dim line
	}

	// now update the inner frame i.e. the wall frame
	m_innerFrame.setBottom(ylines.back() - linespacing);

	// some helping constants
	double yu = ylines.front();
	double xl = m_pos.front();
	double xr = m_pos.back();

	// set appropriate pen for the dimensioning lines.
	for (unsigned int i=0, count=labelVectTop.size(); i<count; ++i) {
		int index = labelVectTop[i].m_index;
		double layer_pixwidth = m_pos[index+1] - m_pos[index];
		double xmiddle = m_pos[index] + layer_pixwidth / 2.0;
		double x_text = xmiddle - labelVectTop[i].m_brect.width() / 2;

		QGraphicsTextItem* textItem = labelVectTop[i].m_textItem;
		textItem->setFont(m_tickFont);
		textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		setAlignment(textItem->document(), Qt::AlignVCenter);
		double ypos = yo - labelVectTop[i].m_brect.height();
#ifdef Q_OS_MAC
		ypos += labelVectTop[i].m_brect.height() / MACYShift;
#endif
		textItem->setPos(x_text, ypos);
	}

	for (unsigned int i=0, count=labelVectBottom.size(); i<count; ++i) {
		int index = labelVectBottom[i].m_index;
		double layer_pixwidth = m_pos[index+1] - m_pos[index];
		double xmiddle = m_pos[index] + layer_pixwidth / 2.0;
		double x_text = xmiddle - labelVectBottom[i].m_brect.width() / 2;
		QGraphicsTextItem* textItem = labelVectBottom[i].m_textItem;
		setAlignment(textItem->document(), Qt::AlignVCenter);
		textItem->setFont(m_tickFont);
		textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		double ypos = yo;
		if( i>0) {
			double xEndBefore = labelVectBottom[i-1].m_textItem->pos().x() + labelVectBottom[i-1].m_brect.width();
			if( x_text <= xEndBefore) {
				x_text = xEndBefore;
			}
		}
#ifdef Q_OS_MAC
		ypos += labelVectBottom[i].m_brect.height() / MACYShift;
#endif
		textItem->setPos(x_text, ypos);

		addLine(xmiddle, yo, x_text + labelVectBottom[i].m_brect.width() / 2.0, ypos + 2 * dist, m_internalPens->m_dimlinePen);
	}

	// put label on main dim line
	QString wallWidthText = dimLabel(m_constructionSize);
	QGraphicsTextItem* textItem = addText(wallWidthText, m_tickFont);
	int xw = static_cast<int>((xl + xr) / 2 - textItem->boundingRect().width() / 2);
	double ypos = yu - textItem->boundingRect().height();
#ifdef Q_OS_MAC
		ypos += textItem->boundingRect().height() / MACYShift;
#endif
	textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
	textItem->setPos(xw, ypos);

	m_internalStringItems->m_dimensionDescTextItem->setVisible(true);
	xw = static_cast<int>((xl + xr)/2 - m_internalStringItems->m_dimensionDescTextItem->boundingRect().width() / 2);

	m_internalStringItems->m_dimensionDescTextItem->setPos(xw, yu + dist);

	// draw dim helping lines

	// lower dim line
	addLine(xl - dist, yu, xr + dist, yu, m_internalPens->m_dimlinePen);
	// upper dim line
	addLine(xl - dist, yo, xr + dist, yo, m_internalPens->m_dimlinePen);

	int dist_diag = static_cast<int>(std::floor( dist * 1.4 ));
	for (int i=0; i<n; ++i) {
		int xstart = m_pos[i];
		int ystart = yo + dist - linespacing;
		if (i==0 || i==n-1) {
			// draw outer lines
			addLine(xstart, ystart, xstart, yu+dist, m_internalPens->m_dimlinePen);
			// also draw diagonals at outer lines
			addLine(xstart - dist_diag, yu + dist_diag, xstart + dist_diag, yu - dist_diag, m_internalPens->m_dimlinePen);
		}
		else {
			// draw inner lines
			addLine(xstart, ystart, xstart, yo + dist, m_internalPens->m_dimlinePen);
		}

		// draw upper diagonals
		addLine(xstart - dist_diag, yo + dist_diag, xstart + dist_diag, yo - dist_diag, m_internalPens->m_dimlinePen);
	}
}

void ConstructionGraphicsSceneHorizontal::drawWall() {
	if(m_inputData.empty())
		return;

	const int yt = m_innerFrame.top();
	const int yb = m_innerFrame.bottom();
	const int xl = m_pos.front();
	const int xr = m_pos.back();

	int rectHeight = yb - yt;

	// Draw material colors
	int markedLayer = m_markedLayer >= m_inputData.size() ? -1 : m_markedLayer;

	for (unsigned int i=1; i<m_pos.size(); ++i) {
		int left = m_pos[i-1];
		int width = m_pos[i] - left;

		QtExt::HatchingType hatching = m_inputData[i-1].m_hatch;
		int hatchDist = std::max(width / 20, (int)(m_internalPens->m_hatchingPen.widthF() * 3));
		if (m_inputData[i-1].m_hatch == HT_InsulationHatch)
			hatchDist /= 10;
		if (int(i-1) == markedLayer) {
			hatching = QtExt::HT_LinesObliqueLeftToRight;
			m_internalPens->m_hatchingPen.setColor(m_onScreen ? QtExt::contrastColor(m_inputData[i-1].m_color, Qt::black) : Qt::gray);
		}

		QGraphicsRectItem* rectItem = addHatchedRect(left, yt, width, rectHeight, hatching, hatchDist, m_internalPens->m_hatchingPen,
		m_onScreen ? QPen(m_inputData[i-1].m_color) : m_internalPens->m_vborderPen,
		m_onScreen ? QBrush(m_inputData[i-1].m_color) : QColor(255,255,255));
		rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
		rectItem->setData(0, i-1);

		if (m_visibleMaterialNames) {
			QGraphicsTextItem* materialName = addText(m_inputData[i-1].m_name, m_tickFont);
			materialName->setZValue(2);
			setAlignment(materialName->document(), Qt::AlignVCenter);
			materialName->document()->setTextWidth(rectHeight);
			materialName->setDefaultTextColor(m_onScreen ? QtExt::contrastColor(m_inputData[i-1].m_color, Qt::black) : Qt::black);
			double textWidth = materialName->document()->idealWidth();

#if QT_VERSION >= 0x040600
			materialName->setRotation(-90);
#else
			materialName->rotate( -90 );
#endif

			int xpos = left + (width - materialName->boundingRect().height()) / 2.0;
			int ypos = yb - (rectHeight - textWidth) / 2.0;
			materialName->setPos(xpos, ypos);

			materialName->setVisible(materialName->boundingRect().height() < width - 2);
		}
	}

	// draw outside wall lines twice as thick
	int o = 2;
	addLine(xl, yt - o, xl, yb + o, m_internalPens->m_vborderPen);
	addLine(xr, yt - o, xr, yb + o, m_internalPens->m_vborderPen);

	int w = m_internalPens->m_hborderPen.width();
	addLine(xl, yt - w, xr, yt - w, m_internalPens->m_hborderPen);
	addLine(xl, yb + w, xr, yb + w, m_internalPens->m_hborderPen);

	// draw inside/outside text
	if (m_visibleBoundaryLabels) {
		QGraphicsTextItem* outsideLeft = addText(m_leftTopSideLabel, m_tickFont);
		setAlignment(outsideLeft->document(), Qt::AlignVCenter);
		outsideLeft->document()->setTextWidth(m_pos.front());
		double textWidth = outsideLeft->document()->idealWidth();
		outsideLeft->setPos((m_pos.front() - textWidth) / 2, yt - 2);
		outsideLeft->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));

		QGraphicsTextItem* insideRight = addText(m_rightBottomSideLabel, m_tickFont);
		setAlignment(insideRight->document(), Qt::AlignVCenter);
		int outerBond = m_frame.width() - m_pos.back();
		insideRight->document()->setTextWidth(outerBond);
		textWidth = insideRight->document()->idealWidth();
		insideRight->setPos(m_pos.back() + (outerBond - textWidth) / 2, yt - 2);
		insideRight->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
	}
}

void ConstructionGraphicsSceneHorizontal::drawMarkerLines() {
	if(m_lineMarker.empty())
		return;

	const int yt = m_innerFrame.top();
	const int yb = m_innerFrame.bottom();
	const int xl = m_pos.front();
	int pixwidth = m_pos.back() - m_pos.front();
	double dw = pixwidth/m_constructionSize;

	int number = 1;
	qreal lastHeight = 0;
	double numberPos = 0.75;
	for(const auto& line : m_lineMarker) {
		QPen pen = line.m_pen;
		pen.setWidthF(pen.widthF() * m_res);
		int pos = (number-1) % 2;
		int xpos = xl + static_cast<int>(line.m_pos * dw);
		addLine(xpos, yb, xpos, yt, pen);

		// numbering
		QGraphicsTextItem* numberText = addTextInRect(line.m_name, m_axisTitleFont);
		setAlignment(numberText->document(), Qt::AlignVCenter);
		int textWidth = numberText->document()->idealWidth();
		double textPos = xpos - textWidth / 2;
		if( textPos > m_xiRight - m_airLayer) {
			double diff = textPos - m_xiRight + m_airLayer;
			textPos = xpos - textWidth - diff;
		}
		numberText->setPos(textPos, (yb - yt) * numberPos + yt + pos * (lastHeight + 2));
		numberText->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		lastHeight = numberText->boundingRect().height();
		++number;
	}
}

void ConstructionGraphicsSceneHorizontal::drawMarkerArea() {
	if(!m_areaMarker.valid())
		return;

	const int yt = m_innerFrame.top();
	const int yb = m_innerFrame.bottom();
	const int xl = m_pos.front();
	int pixwidth = m_xiRight - m_xiLeft - 2 * m_airLayer;
	double dw = pixwidth/m_constructionSize;

	QPen pen = m_areaMarker.m_framePen;
	pen.setWidthF(pen.widthF() * m_res);
	int xposStart = xl + static_cast<int>(m_areaMarker.m_start * dw);
	int xposEnd = xl + static_cast<int>(m_areaMarker.m_end * dw);

	int hatchDist = std::max((xposEnd - xposStart) / 20, (int)(m_internalPens->m_hatchingPen.widthF() * 3));
	m_internalPens->m_hatchingPen.setColor( Qt::gray);

	QGraphicsRectItem* rectItem = addHatchedRect(xposStart, yt, xposEnd - xposStart, yb - yt, QtExt::HT_CrossHatchOblique, hatchDist, m_internalPens->m_hatchingPen,
				   pen, m_areaMarker.m_areaBrush);
	rectItem->setFlag(QGraphicsItem::ItemIsSelectable, false);

}


} // namespace QtExt
