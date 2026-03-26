#ifndef IC18599ReportH
#define IC18599ReportH

#include <QtExt_Report.h>

#include <vector>

#include "IC18599ReportSettings.h"
#include "IC18599ReportFrameProfilePage.h"

class IC18599Project;

class IC18599Report : public QtExt::Report {
	Q_OBJECT
public:
	explicit IC18599Report(IC18599ReportSettings *reportSettings);

	void set(QtExt::ReportData *data) override;
	void setFrames() override;

	/*! Sets profile data for multi-profile report. */
	void setProfileData(const std::vector<ReportProfileData> &profiles);

	IC18599Project				*m_project = nullptr;

private:
	std::vector<ReportProfileData>	m_profiles;
};

#endif // IC18599ReportH
