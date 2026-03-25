#ifndef QtExt_ThumbnailHoverToSelectH
#define QtExt_ThumbnailHoverToSelectH

#include <QLabel>

#include "QtExt_global.h"

namespace QtExt {

/*! Utility class that provides a scaled preview image and a mouse-hover overlay pixmap drawn on top of that.

	You need to set your own thumbnail image via setThumbnail().
	The class provides a default overlay thumbnail, that you can customize with
	setOverlay() or reset with setDefaultOverlay().

	Also, the widget shows a tooltip text, customizable with setTooltip().

	When user clicks on the widget, the signal clicked() is emitted.
*/
class QtExt_EXPORT ThumbnailHoverToSelect : public QLabel {
	Q_OBJECT

public:
	explicit ThumbnailHoverToSelect(QWidget *parent = nullptr);

	/*! Sets the standard thumbnail.
		\param thumbnail The pixmap to show in this thumbnail label.
		\param ratio The aspect ratio to use for transformation.
		\param placeholderText Placeholder text to show, when thumbnail image is invalid.

		\note Thumbnail must be valid, otherwise a default placeholder text is used instead ("No preview available").
	*/
	void setThumbnail(const QPixmap &thumbnail, double ratio = 1.0, const QString placeholderText = tr("No preview available."));

	/*! Set the overlay thumbnail.
		\warning Must be a valid thumbnail. Raises an assertion if overlay is invalid!
	*/
	void setOverlay(const QPixmap &overlay);

	/*! Set standard thumbnail overlay. */
	void setDefaultOverlay();

signals:
	/*! Emitted, when user clicks on this widget. */
	void clicked();

protected:
	/*! When mouse enters the widget, a new label pixmap is composed from given thumbnail and overlay pixmaps. */
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	void enterEvent(QEnterEvent *event) override;
#else
	virtual void enterEvent (QEvent * event ) override;
#endif
	/*! When mouse leaves the widget, the original thumbnail image is restored. */
	void leaveEvent(QEvent *) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void resizeEvent(QResizeEvent *event) override;
	QSize minimumSizeHint() const override;

private:
	/*! Resets the default thumbnail image. */
	void setStandardThumbnail();

	/*! Thumbnail pixmap (the pixmap shown in the label). */
	QPixmap					m_thumbnailPixmap;

	/*! Current Ui pixel ratio for high dpi scaling. */
	double					m_ratio = 1.0;

	/*! Overlay icon, shown when mouse hovers over widget. */
	QPixmap					m_overlayPixmap;

	/*! Placeholder text, shown when m_thumbnailPixmap is invalid. */
	QString					m_placeholderText;
};

}

#endif // QtExt_ThumbnailHoverToSelectH
