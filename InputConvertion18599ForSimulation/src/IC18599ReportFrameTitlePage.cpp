#include "IC18599ReportFrameTitlePage.h"

#include "IC18599Report.h"
#include "IC18599Project.h"
#include "IC18599ReportSettings.h"

#include <QtExt_ReportFrameItemTextFrame.h>
#include <QtExt_ReportFrameItemTable.h>

#include <QDate>

IC18599ReportFrameTitlePage::IC18599ReportFrameTitlePage(QtExt::Report *report, QTextDocument *textDocument) :
	QtExt::ReportFrameBase(report, textDocument),
	m_title(textDocument),
	m_subtitle(textDocument),
	m_infoTable(textDocument, true)
{
}


void IC18599ReportFrameTitlePage::update(QPaintDevice *paintDevice, double width) {
	IC18599Report *report = static_cast<IC18599Report*>(m_report);

	// Title
	m_title.setText(QString("<h1 style='font-size:80px;'>%1</h1>")
					.arg(tr("DIN 18599 Input Conversion")));
	addItem(new QtExt::ReportFrameItemTextFrame(&m_title, paintDevice, width,
												BOTTOM_DIST_H1, 0));

	// Subtitle
	m_subtitle.setText(TITLE_H3(tr("Schedule data for dynamic building simulation<hr>")));
	addItem(new QtExt::ReportFrameItemTextFrame(&m_subtitle, paintDevice, width,
												BOTTOM_DIST_H3));

	// Info table: profile name and date
	double tableWidth = 0.5 * width;
	m_infoTable.set(3, 1, tableWidth, 0, 0, QColor(255, 255, 255));
	m_infoTable.setColumnSizeFormat(0, QtExt::CellSizeFormater::Fixed, tableWidth);

	QString profileText = report->m_currentProfile.isEmpty()
							  ? tr("(No profile selected)")
							  : BOLD(report->m_currentProfile);
	m_infoTable.setCellText(0, 0, profileText);
	m_infoTable.setCellText(0, 1, tr("Date: %1").arg(QDate::currentDate().toString(Qt::ISODate)));
	m_infoTable.setCellText(0, 2, tr("Generated with IC18599"));

	addItem(new QtExt::ReportFrameItemTable(&m_infoTable, paintDevice, tableWidth, 0, 0, true));

	QtExt::ReportFrameBase::update(paintDevice, width);
}
