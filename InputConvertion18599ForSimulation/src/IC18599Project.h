#ifndef IC18599ProjectH
#define IC18599ProjectH

#include <QMap>
#include <QString>
#include <QStringList>
#include <vector>

#include "IC18599ScheduleEditWidget.h"  // for DailyCycleGroup

/*! Holds schedule data for one usage profile. */
struct ProfileScheduleData {
	std::vector<DailyCycleGroup>	m_personGroups;
	std::vector<DailyCycleGroup>	m_equipmentGroups;
	std::vector<DailyCycleGroup>	m_lightingGroups;
};


/*! Project data model with XML serialization via TiCPP. */
class IC18599Project {
public:

	/*! Saves the project to an XML file (.ic18599). */
	bool save(const QString &filePath) const;

	/*! Loads the project from an XML file (.ic18599). */
	bool load(const QString &filePath);

	/*! Returns a reference to the profile data, creating it with defaults if needed. */
	ProfileScheduleData & getOrCreateProfile(const QString &name);

	/*! Returns true if profile data exists for the given name. */
	bool hasProfile(const QString &name) const { return m_profiles.contains(name); }

	QString								m_csvFilePath;		///< CSV file path (relative to project file)
	QStringList							m_profileNames;		///< Profile names from CSV header
	QString								m_currentProfile;	///< Currently selected profile
	QMap<QString, ProfileScheduleData>	m_profiles;			///< Edited profile data
	bool								m_modified = false;
};

#endif // IC18599ProjectH
