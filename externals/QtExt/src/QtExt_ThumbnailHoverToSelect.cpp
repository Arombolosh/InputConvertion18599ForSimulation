#include "QtExt_ThumbnailHoverToSelect.h"

#include <QColor>
#include <QDebug>
#include <QPainter>
#include <QIcon>
#include <QMouseEvent>

namespace QtExt {

ThumbnailHoverToSelect::ThumbnailHoverToSelect(QWidget *parent)
	: QLabel(parent)
{
	// no padding or margins
	setStyleSheet("QLabel { color: #A0A0A0; background-color: white; border: 0px; padding: 0px; margin: 0px; }");
	setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
	setAlignment(Qt::AlignVCenter | Qt::AlignHCenter);
	setDefaultOverlay();

	m_placeholderText = tr("No preview available.");
	setToolTip(tr("Click to edit!"));
}


void ThumbnailHoverToSelect::setOverlay(const QPixmap & overlay) {
	Q_ASSERT(!m_overlayPixmap.isNull());
	m_overlayPixmap = overlay;
}


void ThumbnailHoverToSelect::setDefaultOverlay() {
	m_overlayPixmap = QPixmap(":/gfx/master/edit.png").scaledToWidth(200, Qt::FastTransformation);
}


void ThumbnailHoverToSelect::setThumbnail(const QPixmap &thumbnail, double ratio, const QString placeholderText) {
	m_thumbnailPixmap = thumbnail;
	m_ratio = ratio;
	m_placeholderText = placeholderText;
	if (!m_thumbnailPixmap.isNull())
		m_thumbnailPixmap.setDevicePixelRatio(m_ratio);
	m_overlayPixmap.setDevicePixelRatio(m_ratio);
	setStandardThumbnail();
}


#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void ThumbnailHoverToSelect::enterEvent(QEnterEvent * event)
#else
void ThumbnailHoverToSelect::enterEvent(QEvent * event)
#endif
{
	// Get the size of this widget
	QSize labelSize = size();
	// Create a new QPixmap to draw on
	// MIND: We need the ratio f
	QPixmap newPixmap(m_ratio * labelSize);
	newPixmap.fill(Qt::transparent); // Fill with transparent color
	newPixmap.setDevicePixelRatio(m_ratio);

	// Create a painter to draw on the pixmap
	QPainter painter(&newPixmap);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);

	// Draw the original pixmap
	QRect labelRect = rect();

	QPixmap targetPixmap = m_thumbnailPixmap.scaled(labelRect.size(), Qt::KeepAspectRatio);
	QRect newTargetRect = targetPixmap.rect();
	newTargetRect.moveTo(0.5 * (labelSize.width()  - targetPixmap.width()),
						 0.5 * (labelSize.height() - targetPixmap.height()));
	painter.drawPixmap(newTargetRect, m_thumbnailPixmap, m_thumbnailPixmap.rect());

	// Create a gradient
	QRectF sceneRect = QRectF(0, 0, labelSize.width(), labelSize.height());
	QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());

	QColor col1("#1875bb");
	col1.setAlpha(200);
	QColor col2 = col1;
	col2.setAlpha(10);

	gradient.setColorAt(0, col2);  // Starting color
	gradient.setColorAt(1, col1);  // Ending color

	// Set the brush for the painter
	painter.setBrush(QBrush(gradient));
	painter.setCompositionMode(QPainter::CompositionMode_Darken);

	// Draw the rectangle with the gradient
	painter.fillRect(sceneRect, QBrush(gradient));

	// now draw the edit icon
	int EDIT_ICON_SIZE = 100;
	QRect editPixmapRect(0, 0, EDIT_ICON_SIZE, EDIT_ICON_SIZE);
	editPixmapRect.moveTo(0.5 * (labelSize.width() - EDIT_ICON_SIZE), 0.5 * (labelSize.height() - EDIT_ICON_SIZE));
	painter.drawPixmap(editPixmapRect, m_overlayPixmap, m_overlayPixmap.rect());

	// Set the new pixmap as the label's background
	setPixmap(newPixmap);
	setMinimumSize(0, 0);

	QWidget::enterEvent(event); // Call base class implementation
}


void ThumbnailHoverToSelect::leaveEvent(QEvent * event) {
	setStandardThumbnail();
	QWidget::leaveEvent(event); // Call base class implementation
}


void ThumbnailHoverToSelect::mouseReleaseEvent(QMouseEvent * event) {
	// only react on left click
	if (event->button() == Qt::LeftButton)
		emit clicked();
	QWidget::mouseReleaseEvent(event);
}


void ThumbnailHoverToSelect::resizeEvent(QResizeEvent *event) {
	QWidget::resizeEvent(event);
	setStandardThumbnail();
}


QSize ThumbnailHoverToSelect::minimumSizeHint() const {
	return QSize(0, QWidget::minimumSizeHint().height());
}


void ThumbnailHoverToSelect::setStandardThumbnail() {
	if (m_thumbnailPixmap.isNull()) {
		setPixmap(QPixmap());
		setText(m_placeholderText);
	}
	else {
		setPixmap(m_thumbnailPixmap.scaled(m_ratio * size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
		setText(QString());
	}
}

} // namespace Qt::Ext
