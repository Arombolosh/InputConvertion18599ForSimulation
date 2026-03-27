#ifndef IC18599EnergyPlusExportH
#define IC18599EnergyPlusExportH

#include <QString>
#include <vector>

struct VicusExportProfileData;

/*! Exports IC18599 schedule data as an EnergyPlus IDF file.
	Creates Schedule:Compact objects for all profiles, plus
	ScheduleTypeLimits and ThermostatSetpoint:DualSetpoint objects.
	\return true on success. */
bool exportToEnergyPlus(const QString &idfFilePath,
						const std::vector<VicusExportProfileData> &profiles);

#endif // IC18599EnergyPlusExportH
