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


void IC18599ReportFrameToC::configureHeading() {
	m_heading.setText(TITLE_H3(tr("Table of Contents<hr>")));
}


void IC18599ReportFrameToC::configureTable() {
	int numProfiles = (int)m_profileNames.size();
	bool hasSections = (m_nonResStartIdx > 0 && m_nonResStartIdx < numProfiles);

	// Two section header rows if we have both sections
	int numRows = numProfiles + (hasSections ? 2 : 0);
	m_table.setColumnsRows(2, numRows);

	double width = m_table.tableSize().width();
	double pageColW = width * 0.08;
	double nameColW = width - pageColW;
	m_table.setColumnSizeFormat(0, QtExt::CellSizeFormater::Fixed, nameColW);
	m_table.setColumnSizeFormat(1, QtExt::CellSizeFormater::Fixed, pageColW);

	QColor lightGrey(240, 240, 240);
	QColor darkerGrey(220, 220, 220);
	QColor sectionBg(180, 200, 220);

	int row = 0;
	int profileCounter = 0;

	if (hasSections) {
		// Residential section header
		m_table.setCellText(0, row, BOLD(tr("Residential Buildings")), Qt::AlignLeft);
		m_table.mergeCells(0, row, 2, 1);
		m_table.cell(0, row).setBackgroundColor(sectionBg);
		m_table.cell(1, row).setBackgroundColor(sectionBg);
		++row;
	}

	for (int i = 0; i < numProfiles; ++i) {
		if (hasSections && i == m_nonResStartIdx) {
			// Non-residential section header
			m_table.setCellText(0, row, BOLD(tr("Non-Residential Buildings")), Qt::AlignLeft);
			m_table.mergeCells(0, row, 2, 1);
			m_table.cell(0, row).setBackgroundColor(sectionBg);
			m_table.cell(1, row).setBackgroundColor(sectionBg);
			++row;
		}

		++profileCounter;
		int startPage = 2 + i * 2;
		m_table.setCellText(0, row, QString("%1.  %2").arg(profileCounter).arg(m_profileNames[i]),
							Qt::AlignLeft);
		m_table.setCellText(1, row, QString::number(startPage), Qt::AlignRight);

		QColor bg = (profileCounter % 2 == 1) ? lightGrey : darkerGrey;
		m_table.cell(0, row).setBackgroundColor(bg);
		m_table.cell(1, row).setBackgroundColor(bg);
		++row;
	}
}
