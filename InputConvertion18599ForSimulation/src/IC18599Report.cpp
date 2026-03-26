#include "IC18599Report.h"

#include "IC18599Project.h"
#include "IC18599ReportFrameTitlePage.h"
#include "IC18599ReportFrameProfilePage.h"

IC18599Report::IC18599Report(IC18599ReportSettings *reportSettings) :
	QtExt::Report(reportSettings)
{
}


void IC18599Report::set(QtExt::ReportData *data) {
	QtExt::ReportData1<IC18599Project> *d =
		dynamic_cast<QtExt::ReportData1<IC18599Project>*>(data);
	Q_ASSERT(d != nullptr);
	m_project = d->m_1;
}


void IC18599Report::setFrames() {
	setHeaderFooterData(tr("DIN 18599 Report"), "", "IC18599", QPixmap());

	cleanFrameList();

	// Collect profile names for TOC
	std::vector<QString> profileNames;
	for (const auto &p : m_profiles)
		profileNames.push_back(p.name);

	registerFrame(new IC18599ReportFrameTitlePage(this, m_textDocument, profileNames),
				  IC18599ReportSettings::FrameTitlePage);

	// Two pages per profile: InternalLoads + Setpoints
	for (const auto &profile : m_profiles) {
		registerFrame(new IC18599ReportFrameProfilePage(this, m_textDocument, profile,
								IC18599ReportFrameProfilePage::InternalLoads),
					  IC18599ReportSettings::FrameContent);
		registerFrame(new IC18599ReportFrameProfilePage(this, m_textDocument, profile,
								IC18599ReportFrameProfilePage::Setpoints),
					  IC18599ReportSettings::FrameContent);
	}
}


void IC18599Report::setProfileData(const std::vector<ReportProfileData> &profiles) {
	m_profiles = profiles;
}
