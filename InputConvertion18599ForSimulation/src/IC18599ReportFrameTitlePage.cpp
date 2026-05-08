#include "IC18599ReportFrameTitlePage.h"

#include "IC18599Report.h"
#include "IC18599ReportSettings.h"

#include <QtExt_ReportFrameItemTextFrame.h>

IC18599ReportFrameTitlePage::IC18599ReportFrameTitlePage(QtExt::Report *report, QTextDocument *textDocument) :
	QtExt::ReportFrameBase(report, textDocument),
	m_title(textDocument),
	m_subtitle(textDocument)
{
}


void IC18599ReportFrameTitlePage::update(QPaintDevice *paintDevice, double width) {
	// Title
	m_title.setText(QString("<h1 style='font-size:80px;'>%1</h1>")
					.arg(tr("DIN 18599 Input Conversion")));
	addItem(new QtExt::ReportFrameItemTextFrame(&m_title, paintDevice, width,
												BOTTOM_DIST_H1, 0));

	// Subtitle
	m_subtitle.setText(TITLE_H3(tr("Schedule data for dynamic building simulation<hr>")));
	addItem(new QtExt::ReportFrameItemTextFrame(&m_subtitle, paintDevice, width,
												BOTTOM_DIST_H3));

	QtExt::ReportFrameBase::update(paintDevice, width);
}
