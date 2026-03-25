#ifndef ExpandableWidgetH
#define ExpandableWidgetH


#include <QWidget>
#include <QFrame>
#include <QToolButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "QtExt_global.h"

// class QPropertyAnimation;

namespace QtExt {

class QtExt_EXPORT ExpandableWidget : public QWidget
{
	Q_OBJECT

public:
	/*! Constructor: sets an optional title and parent */
	explicit ExpandableWidget(QWidget *parent = nullptr);

	/*! Sets the widget to be shown/hidden in the content area */
	void setContentWidget(QWidget *widget);

	/*! Public method to change the title text */
	void setTitle(const QString &title);

private slots:
	/*! Slot to handle the button click and toggle the content's visibility */
	void onToggled(bool checked);

private:
	QHBoxLayout*		m_headerLayout;
	QVBoxLayout*		m_mainLayout;
	QToolButton*		m_toggleButton;
	QLabel*				m_titleLabel;
	QFrame*				m_contentArea;
	// QPropertyAnimation	*m_animation = nullptr;

	/*! A flag to keep track of the expanded/collapsed state */
	bool		m_isExpanded;
};

} // namespace QtExt

#endif // ExpandableWidgetH
