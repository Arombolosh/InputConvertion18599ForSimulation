#ifndef IC18599VicusExportH
#define IC18599VicusExportH

#include <QString>
#include <vector>

struct DailyCycleGroup;

/*! Data collected for one profile to be exported to VICUS. */
struct VicusExportProfileData {
	QString								name;
	std::vector<DailyCycleGroup>		personGroups;			///< ch0: occupancy %, ch1: activity W/Pers
	std::vector<DailyCycleGroup>		equipmentGroups;		///< ch0: utilization %
	std::vector<DailyCycleGroup>		lightingGroups;			///< ch0: availability %
	std::vector<DailyCycleGroup>		heatingGroups;			///< ch0: setpoint °C
	std::vector<DailyCycleGroup>		coolingGroups;			///< ch0: setpoint °C
	double								personPerArea = 0;		///< 1/maxOccupancyDensity [Person/m²]
	double								equipmentPowerPerArea = 0;	///< [W/m²]
	double								lightingPowerDensity = 0;	///< [W/m²] computed from p_j,lx formula
};

/*! Injects IC18599 schedule data into an existing .vicus file.
	Creates Schedule, InternalLoad, ZoneControlThermostat and ZoneTemplate
	elements in the EmbeddedDatabase section.
	\return true on success. */
bool exportToVicus(const QString &vicusFilePath,
				   const std::vector<VicusExportProfileData> &profiles);

/*! Exports IC18599 schedule data as standalone VICUS database files.
	Creates three XML files in the given output directory:
	  - db_schedules_18599.xml
	  - db_internalLoads_18599.xml
	  - db_zoneControlThermostat_18599.xml
	\return true on success. */
bool exportVicusDatabaseFiles(const QString &outputDir,
							  const std::vector<VicusExportProfileData> &profiles);

#endif // IC18599VicusExportH
