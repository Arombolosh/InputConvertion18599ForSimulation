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

#include "QtExt_ConstructionGraphicsSceneVertical.h"

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
#include "QtExt_GraphicsTextItemInRect.h"

namespace QtExt {

ConstructionGraphicsSceneVertical::ConstructionGraphicsSceneVertical(bool onScreen, QPaintDevice *device, QObject* parent) :
	ConstructionGraphicsSceneBase(onScreen, device, parent),
	m_yiTop(0),
	m_yiBottom(0)
{
}


void ConstructionGraphicsSceneVertical::calculatePositions() {
	if(m_inputData.empty())
		return;

	int top = m_frame.top() + 5;
	int bottom = m_frame.height() - 5;
	int left = m_frame.left() + 5;
	int right = m_frame.width() - 5;

	if (!m_onScreen) {
		top = static_cast<int>(m_frame.top() + 5 * m_res);
		bottom = static_cast<int>(m_frame.bottom() - 5 * m_res);
		left = static_cast<int>(m_frame.left() + 5 * m_res);
		right = static_cast<int>(m_frame.width() - 5 * m_res);
	}

	std::unique_ptr<QTextDocument> textDocument(new QTextDocument);
	textDocument->documentLayout()->setPaintDevice(m_device);
	textDocument->setDefaultFont(m_axisTitleFont);

	m_innerFrame.setRect(left, top, right - left, bottom - top);

	// calculation x positions
	int axis_bottom, axis_top; // for axes and description
	int m_airLayer;             // for boundary layer

	if (m_onScreen) {
		axis_bottom = 50;
		axis_top = 50;
		m_airLayer = 2 * m_textHeight10;
	} else {
		axis_bottom = static_cast<int>(5 * m_res);
		axis_top = static_cast<int>(5 * m_res);
		m_airLayer = static_cast<int>(10.0 * m_res);
	}
	if(!m_visibleBoundaryLabels) {
		m_airLayer = 0;
	}

	m_yiBottom = m_innerFrame.bottom() - axis_bottom;
	m_yiTop = m_innerFrame.top() + axis_top;

	// calculate width of wall itself in pixdel
//	int pixheight = m_innerFrame.height() - 2 * m_airLayer;
	int pixheight = m_yiBottom - m_yiTop - 2 * m_airLayer;

	// resize vector for x positions
	m_pos.resize(m_inputData.size() + 1);
	m_pos[0] = m_innerFrame.top() + m_airLayer; // left wall boundary

	// calculate scaling factor for wall coordinates in pixel/m
	double dw = pixheight/m_constructionSize; //wallWidth ist die summe aller dicken, also dw sind die pixel pro wanddicke
	// calculate layer thicknesses in pixel and store y positions
	// m_xpos is now a ypos
	// the first layer must be at top - therefore order must be reversed
	std::reverse(m_inputData.begin(), m_inputData.end());
	std::swap(m_leftTopSideLabel, m_rightBottomSideLabel);
	int layerCount = m_inputData.size();
	for (int i=0; i<layerCount; ++i) {
		m_pos[i+1] = m_pos[i] + static_cast<int>(m_inputData[i].m_width * dw);
	}
}

/*! Contains the properties of diagram lables.*/
struct LabelProperties {
	QString				m_text;		///< Label text.
	QRectF				m_brect;	///< Bounding rect.
	bool				m_fit;		///< Indicates if the label fits in the available space.
	QGraphicsTextItem*	m_textItem;	///< Text item for label.
	int					m_index;	///< Label index.
	/*! Standard constructor.*/
	LabelProperties() :
		m_fit(true),
		m_textItem(0),
		m_index(-1)
	{}
};

void ConstructionGraphicsSceneVertical::drawDimensions() {
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

	std::vector<LabelProperties> labelVectLeft;
	std::vector<LabelProperties> labelVectRight;
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
			labelVectRight.push_back(tempLabel);
		} else {
			labelVectLeft.push_back(tempLabel);
		}
	}
	double linespacing = maxTextHeight;

	// calculate dim lines and labels
	std::vector<double> linesXPositions;
	double mainLineXPos = m_innerFrame.right() - linespacing;
	linesXPositions.push_back(mainLineXPos);		// main dim line
	double secondLineXPos = m_innerFrame.right() - 2.1 * linespacing;
	if( n > 2)
		linesXPositions.push_back(secondLineXPos);		// secondary dim line


	// did we have a label too big for the available space?
	if (!labelVectRight.empty() && n > 1) {
		secondLineXPos = m_innerFrame.right() - 3.1 * linespacing;
		linesXPositions.back() = secondLineXPos;	// secondary dim line
	}

	// now update the inner frame i.e. the wall frame
	m_innerFrame.setRight(linesXPositions.back() - linespacing);

	// some helping constants

	double yTop = m_pos.front();
	double yBottom = m_pos.back();

	// set appropriate pen for the dimensioning lines.

	// create text for labels
	for( size_t i=0, count=labelVectLeft.size(); i<count; ++i) {
		int index = labelVectLeft[i].m_index;
		double layer_pixwidth = m_pos[index+1] - m_pos[index];
		double ymiddle = m_pos[index] + layer_pixwidth / 2.0;
		double y_text = ymiddle + labelVectLeft[i].m_brect.width() / 2;

		QGraphicsTextItem* textItem = labelVectLeft[i].m_textItem;
		textItem->setFont(m_tickFont);
		textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		setAlignment(textItem->document(), Qt::AlignVCenter);
		double xpos = secondLineXPos - labelVectLeft[i].m_brect.height();

#if QT_VERSION >= 0x040600
		textItem->setRotation(-90);
#else
		textItem->rotate( -90 );
#endif

#ifdef Q_OS_MAC
		xpos += labelVectLeft[i].m_brect.height() / MACYShift;
#endif

		textItem->setPos(xpos, y_text);
	}
	for( size_t i=0, count=labelVectRight.size(); i<count; ++i) {
		int index = labelVectRight[i].m_index;
		double layer_pixwidth = m_pos[index+1] - m_pos[index];
		double ymiddle = m_pos[index] + layer_pixwidth / 2.0;
		double y_text = ymiddle + labelVectRight[i].m_brect.width() / 2;
		QGraphicsTextItem* textItem = labelVectRight[i].m_textItem;
		setAlignment(textItem->document(), Qt::AlignVCenter);
		textItem->setFont(m_tickFont);
		textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		if( i>0) {
			double yEndBefore = labelVectRight[i-1].m_textItem->pos().y() + labelVectRight[i-1].m_brect.width();
			if( y_text <= yEndBefore) {
				y_text = yEndBefore;
			}
		}
#ifdef Q_OS_MAC
		y_text += labelVectRight[i].m_brect.height() / MACYShift;
#endif
#if QT_VERSION >= 0x040600
		textItem->setRotation(-90);
#else
		textItem->rotate( -90 );
#endif

		textItem->setPos(secondLineXPos, y_text);

		//addLine(xmiddle, yo, x_text + labelVectRight[i].m_brect.width() / 2.0, ypos + 2 * dist, m_internalPens->m_dimlinePen);
		// add line which connects label with dimension line
		double textRectWidth = labelVectRight[i].m_brect.width();
		addLine(secondLineXPos, ymiddle, secondLineXPos + 2 * dist, y_text - textRectWidth / 2.0, m_internalPens->m_dimlinePen);
	}

	// put label on main dim line
	QString wallWidthText = dimLabel(m_constructionSize);
	QGraphicsTextItem* textItem = addText(wallWidthText, m_tickFont);
	int yMiddlePos = static_cast<int>((yTop + yBottom) / 2 + textItem->boundingRect().width() / 2);
	double xPosMainLabel = mainLineXPos - textItem->boundingRect().height();
#ifdef Q_OS_MAC
		xPosMainLabel += textItem->boundingRect().height() / MACYShift;
#endif
#if QT_VERSION >= 0x040600
		textItem->setRotation(-90);
		m_internalStringItems->m_dimensionDescTextItem->setRotation(-90);
#else
		textItem->rotate( -90 );
		m_internalStringItems->m_dimensionDescTextItem->rotate( -90 );
#endif
	textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
	textItem->setPos(xPosMainLabel, yMiddlePos);

	m_internalStringItems->m_dimensionDescTextItem->setVisible(true);
	yMiddlePos = static_cast<int>((yTop + yBottom)/2 + m_internalStringItems->m_dimensionDescTextItem->boundingRect().width() / 2);

	m_internalStringItems->m_dimensionDescTextItem->setPos(mainLineXPos + dist, yMiddlePos);

	// draw dim helping lines

	// main dim line
	addLine(mainLineXPos, yTop - dist, mainLineXPos, yBottom + dist, m_internalPens-> m_dimlinePen);
	// second dim line
	addLine(secondLineXPos, yTop - dist, secondLineXPos, yBottom + dist, m_internalPens->m_dimlinePen);

	int dist_diag = static_cast<int>(std::floor( dist * 1.4 ));
	for (int i=0; i<n; ++i) {
		int xstart = m_pos[i];
		int ystart = secondLineXPos + dist - linespacing;
		if (i==0 || i==n-1) {
			// draw outer lines
			addLine(ystart, xstart, mainLineXPos+dist, xstart, m_internalPens->m_dimlinePen);
			// also draw diagonals at outer lines
			addLine(mainLineXPos + dist_diag, xstart - dist_diag, mainLineXPos - dist_diag, xstart + dist_diag, m_internalPens->m_dimlinePen);
		}
		else {
			// draw inner lines
			addLine(ystart, xstart, secondLineXPos + dist, xstart, m_internalPens->m_dimlinePen);
		}

		// draw upper diagonals
		addLine(secondLineXPos + dist_diag, xstart - dist_diag, secondLineXPos - dist_diag, xstart + dist_diag, m_internalPens->m_dimlinePen);
	}
}

void ConstructionGraphicsSceneVertical::drawWall() {
	if(m_inputData.empty())
		return;

	int rectWidth = m_innerFrame.right() - m_innerFrame.left();

	// Draw material colors
	int markedLayer = m_markedLayer >= m_inputData.size() ? -1 : m_markedLayer;

	for (unsigned int i=1; i<m_pos.size(); ++i) {
		int top = m_pos[i-1];
		int height = m_pos[i] - top;

		QtExt::HatchingType hatching = m_inputData[i-1].m_hatch;
		int hatchDist = std::max(height / 20, (int)(m_internalPens->m_hatchingPen.widthF() * 3));
		if (int(i-1) == markedLayer) {
			hatching = QtExt::HT_LinesObliqueLeftToRight;
			QColor hatchingColor = QtExt::contrastColor(m_inputData[i-1].m_color, Qt::black);
			hatchingColor.setAlpha(150);
			m_internalPens->m_hatchingPen.setColor(hatchingColor);
		}

		QGraphicsRectItem* rectItem = addHatchedRect(m_innerFrame.left(), top, rectWidth, height, hatching, hatchDist, m_internalPens->m_hatchingPen,
						   QPen(m_inputData[i-1].m_color), QBrush(m_inputData[i-1].m_color));
		rectItem->setFlag(QGraphicsItem::ItemIsSelectable, true);
		rectItem->setData(0, i-1);

		if(m_visibleMaterialNames) {
			QGraphicsTextItem* materialName = addText(m_inputData[i-1].m_name, m_tickFont);
			materialName->setZValue(2);
			setAlignment(materialName->document(), Qt::AlignVCenter);
			materialName->document()->setTextWidth(rectWidth);
			materialName->setDefaultTextColor(QtExt::contrastColor(m_inputData[i-1].m_color, Qt::black));
			double textWidth = materialName->document()->idealWidth();

/*
#if QT_VERSION >= 0x040600
			materialName->setRotation(-90);
#else
			materialName->rotate( -90 );
#endif
*/

			//int xpos = left + (width - materialName->boundingRect().height()) / 2.0;
			//int ypos = yb - (rectHeight - textWidth) / 2.0;
			int xpos = m_innerFrame.right() - (rectWidth + textWidth) / 2.0;
			int ypos = top + (height - materialName->boundingRect().height()) / 2.0;
			materialName->setPos(xpos, ypos);

			materialName->setVisible(materialName->boundingRect().height() < height - 2);


		}
	}

	// draw outside wall lines twice as thick

	addLine(m_innerFrame.left(), m_pos.front(), m_innerFrame.right(), m_pos.front(), m_internalPens->m_vborderPen);
	addLine(m_innerFrame.left(), m_pos.back(), m_innerFrame.right(), m_pos.back(), m_internalPens->m_vborderPen);

	addLine(m_innerFrame.left() + m_internalPens->m_hborderPen.width(), m_pos.front(), m_innerFrame.left() + m_internalPens->m_hborderPen.width(), m_pos.back(),
			m_internalPens->m_hborderPen);
	addLine(m_innerFrame.right(), m_pos.front(), m_innerFrame.right(), m_pos.back(), m_internalPens->m_hborderPen);


	// draw inside/outside text

	if(m_visibleBoundaryLabels) {
		QGraphicsTextItem* outsideLeft = addText(m_leftTopSideLabel, m_tickFont);
		setAlignment(outsideLeft->document(), Qt::AlignVCenter);
		//outsideLeft->document()->setTextWidth(m_xpos.front());
		double textHeight = outsideLeft->boundingRect().height();
		outsideLeft->setPos((m_innerFrame.right() + m_innerFrame.left() - outsideLeft->boundingRect().width()) / 2, m_pos.front() - textHeight - textHeight * 0.1);
		outsideLeft->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));


		QGraphicsTextItem* insideRight = addText(m_rightBottomSideLabel, m_tickFont);
		setAlignment(insideRight->document(), Qt::AlignVCenter);
		//int outerBond = m_frame.width() - m_xpos.back();
		//insideRight->document()->setTextWidth(outerBond);
		textHeight = insideRight->boundingRect().height();
		insideRight->setPos((m_innerFrame.right() + m_innerFrame.left() - insideRight->boundingRect().width()) / 2, m_pos.back() + textHeight * 0.1);
		insideRight->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));

	}

}

void ConstructionGraphicsSceneVertical::drawMarkerLines() {
	if(m_lineMarker.empty())
		return;

	const int xLeft = m_innerFrame.left();
	const int xRight = m_innerFrame.right();
	const int yTop = m_pos.front();
	int pixheight = m_pos.back() - m_pos.front();
	double dw = pixheight/m_constructionSize;

	int number = 1;
	qreal lastWidth = 0;
	for(const auto& line : m_lineMarker) {
		QPen pen = line.m_pen;
		pen.setWidthF(pen.widthF() * m_res);
		int pos = (number-1) % 2;
		double reversePos = m_constructionSize - line.m_pos;
		int ypos = yTop + static_cast<int>(reversePos * dw);
		addLine(xLeft, ypos, xRight, ypos, pen);

		// numbering
		QGraphicsTextItem* numberText = addTextInRect(line.m_name, m_axisTitleFont);
		setAlignment(numberText->document(), Qt::AlignVCenter);
		int textHeight = numberText->boundingRect().height();
		numberText->setPos( (xRight - xLeft) / 4 + pos * (lastWidth + 2), ypos - (textHeight / 2));
		numberText->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
		lastWidth = numberText->boundingRect().width();
		++number;
	}
}

void ConstructionGraphicsSceneVertical::drawMarkerArea() {
	if(!m_areaMarker.valid())
		return;

	const int xLeft = m_innerFrame.left();
	const int xRight = m_innerFrame.right();
	const int yTop = m_pos.front();
	int pixheight = m_yiBottom - m_yiTop - 2 * m_airLayer;
	double dw = pixheight/m_constructionSize;

	QPen pen = m_areaMarker.m_framePen;
	pen.setWidthF(pen.widthF() * m_res);
	double reversePosEnd = m_constructionSize - m_areaMarker.m_start;
	double reversePosStart = m_constructionSize - m_areaMarker.m_end;
	int yposStart = yTop + static_cast<int>(reversePosStart * dw);
	int yposEnd = yTop + static_cast<int>(reversePosEnd * dw);

	int hatchDist = std::max((yposEnd - yposStart) / 20, (int)(m_internalPens->m_hatchingPen.widthF() * 3));
	m_internalPens->m_hatchingPen.setColor( Qt::gray);

	QGraphicsRectItem* rectItem = addHatchedRect(xLeft, yposStart, xRight - xLeft, yposEnd - yposStart, QtExt::HT_CrossHatchOblique, hatchDist, m_internalPens->m_hatchingPen,
				   pen, m_areaMarker.m_areaBrush);
	rectItem->setFlag(QGraphicsItem::ItemIsSelectable, false);

}


} // namespace QtExt
