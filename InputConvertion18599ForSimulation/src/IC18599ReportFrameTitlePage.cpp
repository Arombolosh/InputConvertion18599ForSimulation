#include "IC18599ReportFrameTitlePage.h"

#include "IC18599Report.h"
#include "IC18599Project.h"
#include "IC18599ReportSettings.h"

#include <QtExt_ReportFrameItemTextFrame.h>
#include <QtExt_ReportFrameItemTable.h>

IC18599ReportFrameTitlePage::IC18599ReportFrameTitlePage(QtExt::Report *report, QTextDocument *textDocument,
													   const std::vector<QString> &profileNames) :
	QtExt::ReportFrameBase(report, textDocument),
	m_title(textDocument),
	m_subtitle(textDocument),
	m_tocHeading(textDocument),
	m_tocTable(textDocument, true),
	m_profileNames(profileNames)
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

	// Table of contents — lists profile names with enumeration
	if (!m_profileNames.empty()) {
		m_tocHeading.setText(TITLE_H3(tr("Table of Contents<hr>")));
		addItem(new QtExt::ReportFrameItemTextFrame(&m_tocHeading, paintDevice, width,
													BOTTOM_DIST_H3, 0));

		// 2-column table: name (left) | page number (right)
		// Alternating row background for visual tracking.
		int numRows = (int)m_profileNames.size();
		m_tocTable.set(numRows, 2, width, 0, 0, QColor(255, 255, 255));

		double pageColW = width * 0.08;
		double nameColW = width - pageColW;
		m_tocTable.setColumnSizeFormat(0, QtExt::CellSizeFormater::Fixed, nameColW);
		m_tocTable.setColumnSizeFormat(1, QtExt::CellSizeFormater::Fixed, pageColW);

		QColor lightGrey(240, 240, 240);
		QColor darkerGrey(220, 220, 220);

		for (int i = 0; i < numRows; ++i) {
			int startPage = 2 + i * 2;
			m_tocTable.setCellText(0, i, QString("%1.  %2").arg(i + 1).arg(m_profileNames[i]),
								   Qt::AlignLeft);
			m_tocTable.setCellText(1, i, QString::number(startPage), Qt::AlignRight);

			QColor bg = (i % 2 == 0) ? lightGrey : darkerGrey;
			m_tocTable.cell(0, i).setBackgroundColor(bg);
			m_tocTable.cell(1, i).setBackgroundColor(bg);
		}

		addItem(new QtExt::ReportFrameItemTable(&m_tocTable, paintDevice, width, 0, 0, true));
	}

	QtExt::ReportFrameBase::update(paintDevice, width);
}
