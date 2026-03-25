#ifndef ConstructionViewHoverToSelectH
#define ConstructionViewHoverToSelectH

#include "QtExt_ConstructionView.h"

namespace QtExt {

/*! Slightly adapted version of ConstructionView to be used as view only, whithout editing of actual construction.
	It shows an editing icon and patch when hovered on and emits a signal when clicked on.
*/
class QtExt_EXPORT ConstructionViewHoverToSelect: public ConstructionView {
	Q_OBJECT
public:
	explicit ConstructionViewHoverToSelect(QWidget *parent = nullptr);

	void setHoverProperties(const QPixmap & icon, QColor color);

	/*! Draws edit icon if scene is empty. */
	void updateEditIcon();

	/*! Set to read only. */
	void setReadOnly(bool readOnly) {
		m_isReadOnly = readOnly;
	}

signals:
	/*! Signal is emitted when is not read-only and scene was clicked */
	void sceneClicked();

protected:

#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
	void enterEvent(QEnterEvent * event) override;
#else
	virtual void enterEvent (QEvent * event ) override;
#endif
	void leaveEvent(QEvent * event) override;
	void mouseReleaseEvent(QMouseEvent * event) override;

private:
	/*! Read only mode */
	bool					m_isReadOnly = false;
	/*! Editing icon */
	QPixmap					m_icon = QPixmap(":/gfx/master/editscene_green.png");
	QColor					m_hoverColor = "#a08918";
	/*! Pointer to QGraphicsItems. */
	QGraphicsPixmapItem		*m_iconItem = nullptr;
	QGraphicsRectItem		*m_transparentRect = nullptr;
};

} // namespace QtExt

#endif // ConstructionViewHoverToSelectH
