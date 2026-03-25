#ifndef QtExt_ToolBoxH
#define QtExt_ToolBoxH

#include <QWidget>
#include <QIcon>

#include "QtExt_global.h"

class QVBoxLayout;
class QFrame;
class QSpacerItem;

namespace QtExt {

/*! The ToolBox class implements a widget similar to QToolBox with some more flexibility and custom style.
	Basically, this class holds multiple pages that can be expanded/collapsed. The idea of this class is that only ONE page is expanded at a time.
	Each page contains a header with collapsed/expanded arrow icon. An additional icon can be added to the header as well.
	Header label and icons are all QtExt::ClickableLabels, so they have no click animation but still emit a clicked(id) signal
	that also tells their given id.
*/
class QtExt_EXPORT ToolBox : public QWidget {
	Q_OBJECT

public:
	explicit ToolBox(QWidget *parent = nullptr);
	~ToolBox() override;

	/*! Adds a page with given header name, widget and custom icon. */
	void addPage(const QString & headerName, QWidget * widget, QIcon * icon = nullptr);

	/*! Returns widget according to given index. */
	QWidget * widget(unsigned int index) const;

	/*! Set index from outside */
	void setCurrentIndex(unsigned int index);

	/*! Returns index of currently expanded page */
	unsigned int currentIndex();

	/*! Updates the background color from style. */
	void updatePageBackgroundColorFromStyle();

	/*! Sets the page visibilty defined by idx. */
	void setPageVisibility(int idx, bool isVisible);

private slots:
	/*! Changes arrow icons and visibility of given page, connected to ClickableLabels */
	void onLabelClicked();

signals:
	void indexChanged(unsigned int currentIndex);

public:
	/*! Mode. If true, allows multiple pages to be open */
	bool									m_enableOpenMultiplePages = false;

private:
	class Page;

	/*! The index of the currently active (visible) page. We dont use the isVisible() function for that,
		since the widget might not actually be visible but still be the active page.
	*/
	unsigned int							m_currentIndex = (unsigned int)-1;
	/*! Stores pointer to all pages. */
	std::vector<Page*>						m_pages;
	/*! Stores pointer to layouts. */
	QVBoxLayout								*m_layout = nullptr;
	/*! Collapsed arrow icon */
	QIcon									m_arrowRight;
	/*! Expanded arrow icon */
	QIcon									m_arrowDown;
	/*! Spacer Item. */
	QWidget									*m_spacerWidget = nullptr;

	/*! Updates stretch factors so that visible pages get stretch=1 and collapsed pages get stretch=0. */
	void updateStretchFactors();
};

} // namespace QtExt

#endif // QtExt_ToolBoxH
