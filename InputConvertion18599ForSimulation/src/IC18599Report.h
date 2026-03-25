#ifndef IC18599ReportH
#define IC18599ReportH

#include <QtExt_Report.h>

#include "IC18599ReportSettings.h"

class IC18599Project;

class IC18599Report : public QtExt::Report {
	Q_OBJECT
public:
	explicit IC18599Report(IC18599ReportSettings *reportSettings);

	void set(QtExt::ReportData *data) override;
	void setFrames() override;

	IC18599Project		*m_project = nullptr;
	QString				m_currentProfile;
};

#endif // IC18599ReportH
