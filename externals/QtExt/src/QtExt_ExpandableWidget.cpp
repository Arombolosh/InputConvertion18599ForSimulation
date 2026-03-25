#include "QtExt_ExpandableWidget.h"
#include "ui_QtExt_ExpandableWidget.h"

#include <QPropertyAnimation> // For optional animation

namespace QtExt {


ExpandableWidget::ExpandableWidget(QWidget *parent)
	: QWidget(parent), m_isExpanded(false) // Initially collapsed
{
	// --- Create Widgets ---
	m_toggleButton = new QToolButton(this);
	m_titleLabel = new QLabel("title", this);
	m_contentArea = new QFrame(this);

	// --- Configure Widgets ---
	m_toggleButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	m_toggleButton->setArrowType(Qt::RightArrow); // Arrow points right when collapsed
	m_toggleButton->setCheckable(true);
	m_toggleButton->setChecked(m_isExpanded);

	m_titleLabel->setStyleSheet("font-weight: bold;");

	// The content area will have a border to visually separate it
	m_contentArea->setFrameShape(QFrame::NoFrame);
	// contentArea->setFrameShadow(QFrame::Sunken);
	m_contentArea->setVisible(m_isExpanded); // Hide content area initially
	m_contentArea->setLayout(new QVBoxLayout()); // Set a layout for the content area
	m_contentArea->layout()->setContentsMargins(0,0,0,0);

	// --- Create Layouts ---
	m_headerLayout = new QHBoxLayout();
	m_mainLayout = new QVBoxLayout(this); // Set the main layout for this widget

	// --- Assemble Layouts ---
	m_headerLayout->addWidget(m_toggleButton);
	m_headerLayout->addWidget(m_titleLabel);
	m_headerLayout->addStretch(); // Pushes header widgets to the left

	m_mainLayout->addLayout(m_headerLayout);
	m_mainLayout->addWidget(m_contentArea);
	m_mainLayout->addStretch(); // Prevents the widget from expanding vertically when content is hidden

	// Set margins to zero to make it look compact
	m_mainLayout->setContentsMargins(0, 0, 0, 0);
	m_mainLayout->setSpacing(5);

	// --- Connect Signals and Slots ---
	connect(m_toggleButton, &QToolButton::toggled, this, &ExpandableWidget::onToggled);
}

void ExpandableWidget::setContentWidget(QWidget *widget)
{
	// If there's an old widget, delete it
	QLayoutItem *item;
	while ((item = m_contentArea->layout()->takeAt(0)) != nullptr) {
		delete item->widget();
		delete item;
	}

	// Add the new widget to the content area's layout
	if (widget) {
		m_contentArea->layout()->addWidget(widget);
	}
}

void ExpandableWidget::setTitle(const QString &title)
{
	m_titleLabel->setText(title);
}

void ExpandableWidget::onToggled(bool checked)
{
	// Update arrow icon immediately
	m_toggleButton->setArrowType(checked ? Qt::DownArrow : Qt::RightArrow);

	m_contentArea->setVisible(checked);

	// // Stop any previous animation that might be running
	// if (m_animation != nullptr && m_animation->state() == QAbstractAnimation::Running) {
	// 	m_animation->stop();
	// }

	// // Determine the start and end heights for the animation
	// int startHeight = m_contentArea->height();
	// int endHeight = checked ? m_contentArea->layout()->sizeHint().height() : 0;

	// If expanding, we must make the widget visible before the animation starts


	// // Create and configure the animation
	// m_animation = new QPropertyAnimation(m_contentArea, "maximumHeight", this);
	// m_animation->setDuration(300); // Animation duration in milliseconds
	// m_animation->setStartValue(startHeight);
	// m_animation->setEndValue(endHeight);
	// m_animation->setEasingCurve(QEasingCurve::InOutQuad); // A nice smooth curve

	// // Connect a signal to hide the widget completely after it has collapsed
	// if (!checked) {
	// 	connect(m_animation, &QPropertyAnimation::finished, this, [this]() {
	// 		m_contentArea->setVisible(false);
	// 	});
	// }

	// m_animation->start(QAbstractAnimation::DeleteWhenStopped);
}

} // namespace QtExt
