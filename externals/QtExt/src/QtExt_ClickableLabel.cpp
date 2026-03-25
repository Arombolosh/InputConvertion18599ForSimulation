#include "QtExt_ClickableLabel.h"

#include <QMouseEvent>

namespace QtExt {

ClickableLabel::ClickableLabel(QWidget * /*parent*/)
{
}

ClickableLabel::ClickableLabel(const QString & text, QWidget* parent):
	QLabel(text, parent)
{
}


ClickableLabel::ClickableLabel(int id, const QString & text, QWidget* parent):
	QLabel(text, parent),
	m_id(id)
{
}


void ClickableLabel::setActive(bool active) {
	m_active = active;
	updateStyle();
}


void ClickableLabel::mousePressEvent(QMouseEvent * event) {
	if (event->button() == Qt::LeftButton) {
		emit clicked();
	}
	QLabel::mousePressEvent(event); // to catch QUrl-clicks
}


#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
void ClickableLabel::enterEvent(QEnterEvent * ev) {
#else
void ClickableLabel::enterEvent(QEvent * ev) {
#endif
	QLabel::enterEvent(ev);
	if (!m_active) {
		QFont fnt = font();
		fnt.setBold(true);
		setFont(fnt);
	}
}


void ClickableLabel::leaveEvent(QEvent *ev) {
	QLabel::leaveEvent(ev);
	updateStyle();
}

void ClickableLabel::updateStyle(){
	if (m_active) {
		QFont fnt = font();
		fnt.setBold(true);
		setFont(fnt);
		// QLabel::setStyleSheet(m_activeStyleSheet);
	}
	else {
		QFont fnt = font();
		fnt.setBold(false);
		setFont(fnt);
		// QLabel::setStyleSheet(m_normalStyleSheet);
	}
}


} // namespace Qt_Ext
