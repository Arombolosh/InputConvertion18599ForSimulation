#include "IC18599ReportFrameToC.h"

#include "IC18599Report.h"
#include "IC18599ReportSettings.h"

IC18599ReportFrameToC::IC18599ReportFrameToC(QtExt::Report *report, QTextDocument *textDocument,
											 const std::vector<QString> &profileNames,
											 int nonResidentialStartIndex) :
	QtExt::ReportFrameLongTable(report, textDocument),
	m_profileNames(profileNames),
	m_nonResStartIdx(nonResidentialStartIndex)
{
	m_headingSpace = BOTTOM_DIST_H3;
}


void IC18599ReportFrameToC::update(QPaintDevice *paintDevice, double width) {
	m_tableWidth = width;
	QtExt::ReportFrameLongTable::update(paintDevice, width);
}


void IC18599ReportFrameToC::configureHeading() {
	m_heading.setText(TITLE_H3(tr("Table of Contents<hr>")));
}


void IC18599ReportFrameToC::configureTable() {
	int numProfiles = (int)m_profileNames.size();
	bool hasSections = (m_nonResStartIdx > 0 && m_nonResStartIdx < numProfiles);

	int numRows = numProfiles + (hasSections ? 2 : 0) + 1;  // +1 for header row

	m_table.setColumnsRows(2, numRows, 1);
	m_table.setOuterFrameWidth(0);
	m_table.setInnerFrameWidth(0);

	double pageColW = m_tableWidth * 0.12;
	double nameColW = m_tableWidth - pageColW;
	m_table.setColumnSizeFormat(0, QtExt::CellSizeFormater::Fixed, nameColW);
	m_table.setColumnSizeFormat(1, QtExt::CellSizeFormater::Fixed, pageColW);

	QColor lightGrey(240, 240, 240);
	QColor darkerGrey(220, 220, 220);
	QColor sectionBg(180, 200, 220);

	// Header row
	m_table.setCellText(0, 0, BOLD(tr("Profile")), Qt::AlignLeft);
	m_table.setCellText(1, 0, BOLD(tr("Page")), Qt::AlignRight);

	int row = 1;
	int profileCounter = 0;

	// Title page = 1, ToC = 2 pages (fits up to ~80 entries)
	int pageOffset = 3;

	if (hasSections) {
		m_table.setCellText(0, row, BOLD(tr("Residential Buildings")), Qt::AlignLeft);
		m_table.cell(0, row).setBackgroundColor(sectionBg);
		m_table.cell(1, row).setBackgroundColor(sectionBg);
		++row;
	}

	for (int i = 0; i < numProfiles; ++i) {
		if (hasSections && i == m_nonResStartIdx) {
			m_table.setCellText(0, row, BOLD(tr("Non-Residential Buildings")), Qt::AlignLeft);
			m_table.cell(0, row).setBackgroundColor(sectionBg);
			m_table.cell(1, row).setBackgroundColor(sectionBg);
			++row;
		}

		++profileCounter;
		int startPage = pageOffset + i * 2;
		m_table.setCellText(0, row, QString("%1.  %2").arg(profileCounter).arg(m_profileNames[i]),
							Qt::AlignLeft);
		m_table.setCellText(1, row, QString::number(startPage), Qt::AlignRight);

		QColor bg = (profileCounter % 2 == 1) ? lightGrey : darkerGrey;
		m_table.cell(0, row).setBackgroundColor(bg);
		m_table.cell(1, row).setBackgroundColor(bg);
		++row;
	}
}
