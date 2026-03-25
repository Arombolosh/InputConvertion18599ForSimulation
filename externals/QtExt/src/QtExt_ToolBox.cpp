#include "QtExt_ToolBox.h"

#include <QApplication>

#include "QtExt_ClickableLabel.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QDebug>
#include <QPalette>

#include <IBK_Exception.h>


namespace QtExt {

/*! Private container class for a widget with meta data.
	\note The member variables of the tool page are currently not used.
*/
class ToolBox::Page: public QWidget {
public:
	Page(QtExt::ClickableLabel *pageName, QtExt::ClickableLabel *arrowIcon, QtExt::ClickableLabel *icon,
		 QWidget *widget, QFrame *headerframe, QFrame *frame, QWidget *parent):
		m_label(pageName),
		m_arrowIcon(arrowIcon),
		m_icon(icon),
		m_widget(widget),
		m_headerFrame(headerframe),
		m_frame(frame),
		m_parent(parent)
	{
		m_label->setActive(false);
	}

	virtual ~Page() override;

	/*! Stores pointer to label, needed to toggle font weight */
	QtExt::ClickableLabel		*m_label = nullptr;
	/*! Stores pointer to arrow icon, neded to toggle icon. */
	QtExt::ClickableLabel		*m_arrowIcon = nullptr;
	/*! Stores pointer to icon. */
	QtExt::ClickableLabel		*m_icon = nullptr;
	/*! Stores pointer to widget, neded to toggle visibility */
	QWidget						*m_widget = nullptr;
	/*! Stores pointer to headerFrame. */
	QFrame						*m_headerFrame = nullptr;
	/*! Stores pointer to vertical frame, needed to update stylesheet. */
	QFrame						*m_frame = nullptr;
	/*! Parent widget. */
	QWidget						*m_parent = nullptr;
};

ToolBox::Page::~Page() {}

ToolBox::ToolBox(QWidget *parent):
	QWidget(parent), m_layout(new QVBoxLayout(this))
{
	setLayout(m_layout);
	m_layout->setSpacing(2); // 2 pixels between header frames
	m_arrowDown = QIcon(":/gfx/tool-box/arrow-down.svg");
	m_arrowRight = QIcon(":/gfx/tool-box/arrow-right.svg");
}


ToolBox::~ToolBox() {
	for (Page *page: m_pages)
		delete page;
}


void ToolBox::addPage(const QString & headerName, QWidget * widget, QIcon * icon) {

	// Toolbox page is composed of

	// Vertical layout (m_layout) - uses standard spacings
	// - Frame (vFrame with vertical layout vLay)
	//     - Header Frame (with horizontal layout - hLay)
	//       - three ClickableLabel(arrow, icon (optional), label)
	//     - BodyFrame (with vertical layout - vlay)
	//       - horizontal line
	//       - widget
	//     - SpacerWidget (pushed the actual propertywidgets together and leaves an empty space at the bottom

	// create vertical layout that contains the header frame and body frame (with widget)
	QVBoxLayout *vLay = new QVBoxLayout; // no parent pointer here, as we add layouts manually to parent widgets
	vLay->setContentsMargins(0,0,0,0); // no margins in outermost layout
	vLay->setSpacing(2); // 2 pixels between header and body frame should be enough

	// *** Header ***

	// create header layout and add arrow, icon (if existing) and label to it
	QHBoxLayout *hLay = new QHBoxLayout;
	hLay->setContentsMargins(6,6,2,6); // minimal margins inside the header frame box

	ClickableLabel *labelArrow = new ClickableLabel((int)m_pages.size(), "", this);
	labelArrow->setPixmap(m_arrowRight.pixmap(16));
	QSizePolicy pol;
	pol.setHorizontalPolicy(QSizePolicy::Fixed);
	labelArrow->setSizePolicy(pol);
	hLay->addWidget(labelArrow);

	ClickableLabel *labelIcon = nullptr;
	if (icon != nullptr) {
		labelIcon = new ClickableLabel((int)m_pages.size(), "", this);
		labelIcon->setPixmap(icon->pixmap(16));
		labelIcon->setSizePolicy(pol);
		hLay->addWidget(labelIcon);
	}

	ClickableLabel *labelPageName = new ClickableLabel((int)m_pages.size(), headerName, this);
	// label font fixed to 10pt and bold
	QFont f = labelPageName->font();
	f.setPointSize(10);
	f.setBold(true);
	labelPageName->setFont(f);
	hLay->addWidget(labelPageName);

	QFrame * headerFrame = new QFrame;
	headerFrame->setFrameShape(QFrame::Box);
	headerFrame->setObjectName("Header");
	headerFrame->setLayout(hLay);

	vLay->addWidget(headerFrame);


	// *** Body frame ***

	vLay->addWidget(widget);
	// NOTE: leave margin at top so that we have a little space between the content widget and the header

	// add the body frame to our vertical layout to a frame again
	QFrame *vFrame = new QFrame(this);
	vFrame->setObjectName(QString("vFrame%1").arg(m_pages.size()));
	vFrame->setFrameShape(QFrame::Box);
	vFrame->setLayout(vLay);
	m_layout->addWidget(vFrame);

	// *** Spacer ***

	// we need a space item to show, if user collapses currently visible page
	m_spacerWidget = new QWidget(this);
	m_spacerWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	m_layout->addWidget(m_spacerWidget);
	m_spacerWidget->setVisible(false);

	Page *page = new Page(labelPageName, labelArrow, labelIcon, widget, headerFrame, vFrame, this);
	m_pages.push_back(page);

	// add connections
	connect(labelArrow, &ClickableLabel::clicked, this, &ToolBox::onLabelClicked);
	connect(labelPageName, &ClickableLabel::clicked, this, &ToolBox::onLabelClicked);
	if (labelIcon != nullptr)
		connect(labelIcon, &ClickableLabel::clicked, this, &ToolBox::onLabelClicked);

	// update style sheets
	updatePageBackgroundColorFromStyle();

	// set first page as active
	setCurrentIndex(0);
}


QWidget * ToolBox::widget(unsigned int index) const {
	Q_ASSERT(index < m_pages.size());
	return m_pages[index]->m_widget;
}


void ToolBox::setCurrentIndex(unsigned int index) {
	Q_ASSERT(index < m_pages.size());
	m_currentIndex = index;
	// Note: We first collapse everything and AFTER that expand the target widget.
	// Otherwise we might have two widgets visible for a short moment, which could destroy the layout as there is not enough space.
	if (m_enableOpenMultiplePages) {
		if (m_pages[m_currentIndex]->m_widget->isVisibleTo(this)){
			m_pages[m_currentIndex]->m_widget->setVisible(false);
			m_pages[m_currentIndex]->m_arrowIcon->setPixmap(m_arrowRight.pixmap(16));
			m_pages[m_currentIndex]->m_label->setActive(false);
		} else {
			m_pages[m_currentIndex]->m_widget->setVisible(true);
			m_pages[m_currentIndex]->m_arrowIcon->setPixmap(m_arrowDown.pixmap(16));
			m_pages[m_currentIndex]->m_label->setActive(true);
		}
	}
	else {
		for (Page *page: m_pages) {
			page->m_widget->setVisible(false);
			page->m_arrowIcon->setPixmap(m_arrowRight.pixmap(16));
			page->m_label->setActive(false);
		}

		m_pages[m_currentIndex]->m_widget->setVisible(true);
		m_pages[m_currentIndex]->m_arrowIcon->setPixmap(m_arrowDown.pixmap(16));
		m_pages[m_currentIndex]->m_label->setActive(true);
	}

	m_spacerWidget->setVisible(false);
	updatePageBackgroundColorFromStyle();
	updateStretchFactors();
	emit indexChanged(m_currentIndex);
}


void ToolBox::onLabelClicked() {
	unsigned int index = qobject_cast<ClickableLabel*>(sender())->id();
	if (index == m_currentIndex) {
		bool isVisible = m_pages[m_currentIndex]->m_widget->isVisible(); // true if currently visible and being turned off
		m_pages[m_currentIndex]->m_widget->setVisible(!isVisible);
		m_pages[m_currentIndex]->m_arrowIcon->setPixmap(isVisible ? m_arrowRight.pixmap(16) : m_arrowDown.pixmap(16));

		m_spacerWidget->setVisible(isVisible);
		updateStretchFactors();
	}
	else {
		setCurrentIndex(index);
	}
}


unsigned int ToolBox::currentIndex(){
	return m_currentIndex;
}


void ToolBox::updatePageBackgroundColorFromStyle() {
	QColor headerFrameColor = qApp->palette().color(QPalette::Base);
	QColor frameBackground = qApp->palette().color(QPalette::Window);
	QColor headerFrameEdgeColor = qApp->palette().color(QPalette::Shadow);
#define AVERAGE_WINDOW_COLOR
#ifdef AVERAGE_WINDOW_COLOR
	const int OFFSET = 14;
	QColor averagedColor(frameBackground.red()  + OFFSET,
						 frameBackground.green()+ OFFSET,
						 frameBackground.blue() + OFFSET);
	headerFrameColor = averagedColor;
	frameBackground = averagedColor;
#endif
	for (Page *page: m_pages) {
#define WITH_FRAME
#ifdef WITH_FRAME
		if (page->m_widget->isVisibleTo(this))
			page->m_frame->setStyleSheet(QString("QFrame#%1 {border-radius: 5px; background-color: %2; border: 0px solid %3}")
								  .arg(page->m_frame->objectName(), frameBackground.name(), headerFrameEdgeColor.name()));
		else
#endif
			page->m_frame->setStyleSheet(QString("QFrame#%1 {border-radius: 5px; background-color: %2; border: 0px solid %3}")
								  .arg(page->m_frame->objectName(), frameBackground.name(), headerFrameEdgeColor.name()));
		page->m_headerFrame->setStyleSheet(QString("QFrame#Header {border-radius: 5px; background-color: %1; border: 0px solid %3}")
							  .arg(headerFrameColor.name(), headerFrameEdgeColor.name()));
	}
}

void ToolBox::setPageVisibility(int idx, bool isVisible) {
	Q_ASSERT(idx < (int)m_pages.size());
	m_pages[idx]->m_frame->setVisible(isVisible);
}



void ToolBox::updateStretchFactors() {
	for (unsigned int i = 0; i < m_pages.size(); ++i)
		m_layout->setStretchFactor(m_pages[i]->m_frame, m_pages[i]->m_widget->isVisible() ? 1 : 0);
}


} // namespace QtExt
