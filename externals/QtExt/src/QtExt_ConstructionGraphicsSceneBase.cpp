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

#include "QtExt_ConstructionGraphicsSceneBase.h"

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

void setAlignment(QTextDocument* doc, Qt::Alignment align) {
	QTextBlock block = doc->firstBlock();
	if( !block.isValid())
		return;
	QTextLayout* layout = block.layout();
	if(  layout->lineCount() == 0)
		return;
	// set of alignment options
	QTextOption option = layout->textOption();
	option.setAlignment(align);
	layout->setTextOption(option);
}

QColor contrastColor(const QColor& background, const QColor& origin) {
	double r = background.redF();
	double g = background.greenF();
	double b = background.blueF();
	double brightness = std::pow(r,2.2) * 0.2126 +
			  std::pow(g,2.2) * 0.7152 +
			  std::pow(b,2.2) * 0.0722;

	return (brightness > 0.5) ? origin : Qt::white;
}

void ConstructionGraphicsSceneBase::InternalPens::setPens(const QColor& backGround) {

	// boundary layers
	m_layerBoundPen.setWidthF(std::max(p->m_res * 0.6, 2.0));
	m_layerBoundPen.setColor(p->m_onScreen ? QtExt::contrastColor(backGround, Qt::darkGray) : QtExt::contrastColor(backGround, Qt::black));
	m_layerBoundPen.setCapStyle(Qt::FlatCap);


	// dimension lines
	m_dimlinePen.setWidthF(std::max(p->m_res * 0.1, 1.0));
	m_dimlinePen.setColor(p->m_onScreen ? QtExt::contrastColor(backGround, Qt::gray) : QtExt::contrastColor(backGround, Qt::black));

	m_hatchingPen.setWidthF(std::max(p->m_res * 0.1, 1.0));
	m_hatchingPen.setColor(p->m_onScreen ? QtExt::contrastColor(backGround, Qt::white) : Qt::gray);

	// vertical border
	m_vborderPen.setWidthF(p->m_onScreen ? 2 : 0.3 * p->m_res);
	m_vborderPen.setColor(QtExt::contrastColor(backGround, Qt::black));

	// horizontal border
	m_hborderPen.setWidthF(p->m_onScreen ? 1 : 0.2 * p->m_res);
	m_hborderPen.setColor(QtExt::contrastColor(backGround, Qt::darkGray));
	m_hborderPen.setStyle(Qt::DashLine);
}


void ConstructionGraphicsSceneBase::InternalStringItems::setStrings(const QColor& background) {
	// Description for dimension
	QString unit = "mm";
	switch(p->m_distanceUnit) {
		case DU_cm: unit = "cm"; break;
		case DU_mm: unit = "mm"; break;
		case DU_m: unit = "m"; break;
	}

	QString x_label = tr("Layer widths in [%1]").arg(unit);
	m_dimensionDescTextItem = p->addText(x_label, p->m_axisTitleFont);
	m_dimensionDescTextItem->setVisible(false);
	m_dimensionDescTextItem->setDefaultTextColor(contrastColor(background, Qt::black));
}


ConstructionGraphicsSceneBase::ConstructionGraphicsSceneBase(bool onScreen, QPaintDevice *device, QObject* parent) :
	QGraphicsScene(parent),
	m_airLayer(0),
	m_constructionSize(0),
	m_textHeight10(15),
	m_textWidthT10(10),
	m_onScreen(onScreen),
	m_res(1.0),
	m_device(device),
	m_backgroundColor(Qt::white),
	m_internalPens(new InternalPens(this)),
	m_internalStringItems(new InternalStringItems(this)),
	m_visibleDimensions(true),
	m_visibleMaterialNames(true),
	m_visibleBoundaryLabels(true),
	m_markedLayer(-1),
	m_externalChange(false)
{
	Q_ASSERT(m_device);

	m_fontScreen = QFont(); // QFont(fontFamily);
	m_fontPrinter = m_fontScreen;
	m_axisTitleFontScreen = m_fontScreen;
	m_axisTitleFontPrinter = m_fontScreen;

#ifdef Q_OS_WIN
	m_fontScreen.setPointSize(8);
	m_fontPrinter.setPointSize(6);
	m_axisTitleFontScreen.setPointSize(10);
	m_axisTitleFontPrinter.setPointSize(8);
#elif defined(Q_OS_MAC)
	m_fontScreen.setPointSize(8);
	m_fontPrinter.setPointSize(6);
	m_axisTitleFontScreen.setPointSize(10);
	m_axisTitleFontPrinter.setPointSize(8);
	m_fontScreen.setKerning(false);
	m_fontPrinter.setKerning(false);
	m_axisTitleFontScreen.setKerning(false);
	m_axisTitleFontPrinter.setKerning(false);
#else
	m_fontScreen.setPointSize(8);
	m_fontPrinter.setPointSize(6);
	m_axisTitleFontScreen.setPointSize(10);
	m_axisTitleFontPrinter.setPointSize(8);
#endif

	QColor backGrd = QApplication::style()->standardPalette().brush(QPalette::Window).color();
	m_internalPens->setPens(backGrd);
	m_internalStringItems->setStrings(backGrd);

}

ConstructionGraphicsSceneBase::~ConstructionGraphicsSceneBase() {
	delete m_internalPens;
	delete m_internalStringItems;
}

void ConstructionGraphicsSceneBase::clear() {
	QGraphicsScene::clear();
	m_internalPens->setPens(m_backgroundColor);
	m_internalStringItems->setStrings(m_backgroundColor);
	m_inputData.clear();
}

QGraphicsTextItem* ConstructionGraphicsSceneBase::addText(const QString& text, const QFont& font) {
	QGraphicsTextItem* textItem = this->QGraphicsScene::addText(text);
	textItem->document()->documentLayout()->setPaintDevice(m_device);
	setAlignment(textItem->document(), Qt::AlignVCenter);
	textItem->setFont(font);
	textItem->setDefaultTextColor(contrastColor(m_backgroundColor, Qt::black));
	return textItem;
}

QGraphicsTextItem* ConstructionGraphicsSceneBase::addTextInRect(const QString& text, const QFont& font) {
	GraphicsTextItemInRect* textItem = new GraphicsTextItemInRect(text);
	addItem(textItem);
	textItem->document()->documentLayout()->setPaintDevice(m_device);
	setAlignment(textItem->document(), Qt::AlignVCenter);
	textItem->setFont(font);
	textItem->setFillColor(Qt::white);
	textItem->setRectFramePen(QPen(QBrush(Qt::black), 2));
	textItem->setDefaultTextColor(contrastColor(textItem->fillColor(), Qt::black));
	return textItem;
}

double ConstructionGraphicsSceneBase::convertUnit(double val) const {
	switch(m_distanceUnit) {
		case DU_mm: return val * 1000.0;
		case DU_cm: return val * 100.0;
		case DU_m:
		default: return val;
	}
}

QtExt::GraphicsRectItemWithHatch* ConstructionGraphicsSceneBase::addHatchedRect ( qreal x, qreal y, qreal w, qreal h, QtExt::HatchingType hatchType, int hatchDistance,
											const QPen & hatchPen, const QPen & pen, const QBrush & brush ) {

	// rotate hatch pattern when rectangle is wider than tall
	if (w > h) {
		switch (hatchType) {
			case QtExt::HT_LinesHorizontal:		hatchType = QtExt::HT_LinesVertical; break;
			case QtExt::HT_LinesVertical:		hatchType = QtExt::HT_LinesHorizontal; break;
			default: break;
		}
	}

	QtExt::GraphicsRectItemWithHatch* hatchedRect =
				new QtExt::GraphicsRectItemWithHatch(hatchType, hatchDistance);

	hatchedRect->setRect(x, y, w, h);
	if (w > h && hatchType == QtExt::HT_InsulationHatch)
		hatchedRect->setRotated(true);
	if (brush != QBrush()) {
		hatchedRect->setBrush(brush);
	}
	hatchedRect->setPen(pen);
	hatchedRect->setHatchPen(hatchPen);
	addItem(hatchedRect);
	return hatchedRect;
}

// local function, composes dimension number in mm with either 0 or 1 digit after the point
QString ConstructionGraphicsSceneBase::dimLabel(double d) {
	QString text;
	double dInmm = convertUnit(d);
	if (std::fabs(dInmm - std::floor(dInmm)) < 0.1)
		text = QString("%1").arg(std::floor(dInmm + 0.5),0, 'f', 0);
	else
		text = QString("%1").arg(dInmm, 0, 'f', 1);
	return text;
}

void ConstructionGraphicsSceneBase::setup(QRect frame, QPaintDevice *device, double res,
									  const QVector<ConstructionLayer>& layers,
									  const QString & leftTopLabel, const QString & rightBottomLabel,
									  int visibleItems)
{

	int currentVisibilty = 0;
	if(m_visibleDimensions)
		currentVisibilty += VI_Dimensions;
	if(m_visibleMaterialNames)
		currentVisibilty += VI_MaterialNames;
	if(m_visibleBoundaryLabels)
		currentVisibilty += VI_BoundaryLabels;
	m_visibleDimensions = visibleItems & VI_Dimensions;
	m_visibleMaterialNames = visibleItems & VI_MaterialNames;
	m_visibleBoundaryLabels = visibleItems & VI_BoundaryLabels;

	Q_ASSERT(device);
	m_device = device;
	m_res = res;
	// If no change no calculations needed
	bool noCalculationNeeded = frame == m_frame;
	noCalculationNeeded = noCalculationNeeded && layers == m_inputData;
	noCalculationNeeded = noCalculationNeeded && visibleItems == currentVisibilty;
	noCalculationNeeded = noCalculationNeeded && !m_externalChange;

	m_leftTopSideLabel = leftTopLabel;
	m_rightBottomSideLabel = rightBottomLabel;

	if( noCalculationNeeded)
		return;

	m_externalChange = false;

	if (m_onScreen) {
		m_tickFont = m_fontScreen;
		m_axisTitleFont = m_axisTitleFontScreen;
	}
	else {
		m_tickFont = m_fontPrinter;
		m_axisTitleFont = m_axisTitleFontPrinter;
	}

	{
		QtExt::TextFrame testText(0);
		testText.setDefaultFont(m_tickFont);
		testText.setText(QString("T"));
		m_textHeight10 = testText.frameRect(m_device, -1).height();
		m_textWidthT10 = testText.frameRect(m_device, -1).width();
	}

	clear();
	m_inputData = layers;
	m_frame = frame;
	m_internalPens->setPens(m_backgroundColor);
	m_internalStringItems->setStrings(m_backgroundColor);


	// check if we have valid construction data for drawing the construction sketch
	bool valid = true;
	m_constructionSize = 0;
	for (int i=0; i<m_inputData.size(); ++i) {
		double w = m_inputData[i].m_width;
		if (w <= 0) {
			valid = false;
			break;
		}
		m_constructionSize += w;
	}
	if (m_inputData.empty() || !valid)
		return; // nothing to draw

	// update coordinates, normally only necessary when wall construction changes or view is resized
	calculatePositions();
	if(m_visibleDimensions)
		drawDimensions();
	drawWall();
	drawMarkerLines();
	drawMarkerArea();

	setSceneRect(m_frame);
	emit changed(QList<QRectF>() << frame);
}

void ConstructionGraphicsSceneBase::clearLineMarkers() {
	if(!m_lineMarker.empty()) {
		m_lineMarker.clear();
		m_externalChange = true;
	}
}

void ConstructionGraphicsSceneBase::addLinemarker(double pos, const QPen& pen, const QString& name) {
	m_lineMarker.push_back(LineMarker(pos,pen,name));
	m_externalChange = true;
}

void ConstructionGraphicsSceneBase::addLinemarker(const LineMarker& lineMarker) {
	m_lineMarker.push_back(lineMarker);
	m_externalChange = true;
}

void ConstructionGraphicsSceneBase::setBackground(const QColor& bkgColor) {
	if(m_backgroundColor != bkgColor) {
		m_backgroundColor = bkgColor;
		m_externalChange = true;
	}
}

void ConstructionGraphicsSceneBase::markLayer(int layerIndex) {
	if(m_markedLayer != layerIndex) {
		m_markedLayer = layerIndex;
		m_externalChange = true;
	}
}

void ConstructionGraphicsSceneBase::setAreaMarker(const AreaMarker& am) {
	m_areaMarker = am;
}

void ConstructionGraphicsSceneBase::removeAreaMarker() {
	m_areaMarker.m_start = 0;
	m_areaMarker.m_end = 0;
}


} // namespace QtExt
