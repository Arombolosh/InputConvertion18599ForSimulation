#ifndef IC18599ReportFrameProfilePageH
#define IC18599ReportFrameProfilePageH

#include <QtExt_ReportFrameBase.h>
#include <QtExt_TextFrame.h>
#include <QtExt_Table.h>

#include <QCoreApplication>
#include <QString>

#include <vector>
#include <set>

#include "IC18599ScheduleEditWidget.h"  // for DailyCycleGroup

/*! Data for one profile's report charts and tables. */
struct ReportProfileData {
	QString name;
	int profileIndex = 0;				///< 1-based index for enumeration in report
	// Weekly result curves (168 values each)
	std::vector<double> personResultWeek;		///< [W/m2]
	std::vector<double> equipResultWeek;		///< [W/m2]
	std::vector<double> lightResultWeek;		///< [-] fraction
	std::vector<double> heatingSetpointWeek;	///< [C]
	std::vector<double> coolingSetpointWeek;	///< [C]
	// Day groups for table headers and per-group values
	std::vector<DailyCycleGroup> personGroups;
	std::vector<DailyCycleGroup> equipGroups;
	std::vector<DailyCycleGroup> lightGroups;
	std::vector<DailyCycleGroup> heatingGroups;
	std::vector<DailyCycleGroup> coolingGroups;
};


/*! Report frame for one profile page.
	PageType::InternalLoads = Person + Equipment + Lighting charts + table
	PageType::Setpoints = Heating + Cooling charts + table
*/
class IC18599ReportFrameProfilePage : public QtExt::ReportFrameBase {
	Q_DECLARE_TR_FUNCTIONS(IC18599ReportFrameProfilePage)
public:
	enum PageType {
		InternalLoads,
		Setpoints
	};

	IC18599ReportFrameProfilePage(QtExt::Report *report, QTextDocument *textDocument,
								  const ReportProfileData &data, PageType pageType);

	void update(QPaintDevice *paintDevice, double width) override;

private:
	/*! Returns a day-group label like "Mo - Fr" from a set of day indices. */
	static QString dayGroupLabel(const std::set<int> &days);

	ReportProfileData		m_data;
	PageType				m_pageType;
	QtExt::TextFrame		m_heading;
};

#endif // IC18599ReportFrameProfilePageH
