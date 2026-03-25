#include "IC18599Report.h"

#include "IC18599Project.h"
#include "IC18599ReportFrameTitlePage.h"

IC18599Report::IC18599Report(IC18599ReportSettings *reportSettings) :
	QtExt::Report(reportSettings)
{
}


void IC18599Report::set(QtExt::ReportData *data) {
	QtExt::ReportData2<IC18599Project, QString> *d =
		dynamic_cast<QtExt::ReportData2<IC18599Project, QString>*>(data);
	Q_ASSERT(d != nullptr);
	m_project = d->m_1;
	m_currentProfile = *d->m_2;
}


void IC18599Report::setFrames() {
	setHeaderFooterData(tr("DIN 18599 Report"), "", "IC18599", QPixmap());

	cleanFrameList();

	registerFrame(new IC18599ReportFrameTitlePage(this, m_textDocument),
				  IC18599ReportSettings::FrameTitlePage);
}
