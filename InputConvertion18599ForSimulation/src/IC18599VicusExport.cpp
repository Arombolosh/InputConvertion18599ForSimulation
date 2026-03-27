#include "IC18599VicusExport.h"

#include "IC18599ScheduleEditWidget.h"

#include <tinyxml.h>

#include <cmath>
#include <sstream>
#include <string>


/*! Recursively scans all elements for "id" attributes and tracks the maximum. */
static void scanMaxId(const TiXmlElement *elem, unsigned int &maxId) {
	if (!elem)
		return;
	unsigned int val = 0;
	if (elem->QueryUnsignedAttribute("id", &val) == TIXML_SUCCESS) {
		if (val > maxId)
			maxId = val;
	}
	for (const TiXmlElement *child = elem->FirstChildElement();
		 child != nullptr;
		 child = child->NextSiblingElement())
	{
		scanMaxId(child, maxId);
	}
}


/*! Converts 24 hourly values to compressed TimePoints + Values strings.
	Merges consecutive hours with identical values.
	Example: [0,0,0,0,0,0,0,50,50,50,50,50,50,50,50,50,50,50,0,0,0,0,0,0]
	       → timePoints="0,7,18", values="0,50,0"
*/
static void compressHourlyValues(const std::vector<double> &hourly,
								 std::string &timePointsStr,
								 std::string &valuesStr)
{
	std::vector<int> tp;
	std::vector<double> vals;

	if (hourly.empty()) {
		timePointsStr = "0";
		valuesStr = "0";
		return;
	}

	tp.push_back(0);
	vals.push_back(hourly[0]);
	for (int h = 1; h < 24 && h < (int)hourly.size(); ++h) {
		if (std::abs(hourly[(size_t)h] - vals.back()) > 1e-8) {
			tp.push_back(h);
			vals.push_back(hourly[(size_t)h]);
		}
	}

	// Build comma-separated strings
	std::ostringstream tpSS, valSS;
	for (size_t i = 0; i < tp.size(); ++i) {
		if (i > 0) tpSS << ",";
		tpSS << tp[i];
	}
	for (size_t i = 0; i < vals.size(); ++i) {
		if (i > 0) valSS << ",";
		// Output integer if value is whole, otherwise 2 decimal places
		if (std::abs(vals[i] - std::round(vals[i])) < 1e-8)
			valSS << (int)std::round(vals[i]);
		else
			valSS << std::round(vals[i] * 100.0) / 100.0;
	}

	timePointsStr = tpSS.str();
	valuesStr = valSS.str();
}


/*! VICUS ScheduledDayType constants (from NANDRAD_Schedule.h). */
static const int VICUS_ST_ALLDAYS  = 0;
static const int VICUS_ST_WEEKDAY  = 1;
static const int VICUS_ST_WEEKEND  = 2;
// Individual days: Mon=3 .. Sun=9 (offset +3 from IC18599 day index 0..6)


/*! Builds a comma-separated VICUS DayTypes string from a set of IC18599 day indices.
	Uses aggregate constants (AllDays, Weekday, Weekend) when possible. */
static std::string dayTypesString(const std::set<int> &days) {
	if (days.size() == 7)
		return std::to_string(VICUS_ST_ALLDAYS);
	if (days == std::set<int>{0,1,2,3,4})
		return std::to_string(VICUS_ST_WEEKDAY);
	if (days == std::set<int>{5,6})
		return std::to_string(VICUS_ST_WEEKEND);

	// Mixed: list individual day types
	std::ostringstream ss;
	bool first = true;
	for (int d : days) {
		if (!first) ss << ",";
		ss << (d + 3);  // IC18599 day 0=Mon → VICUS day 3=Mon
		first = false;
	}
	return ss.str();
}


/*! Creates a VICUS Schedule element and appends it to the container.
	\param container   Parent XML element (Schedules)
	\param id          Unique element ID
	\param name        Display name
	\param groups      DailyCycleGroup data
	\param channel     Which channel of the group to use
	\param scaleFactor Multiplier for values (e.g. 1/100 to convert % → fraction)
*/
static void writeVicusSchedule(TiXmlElement *container,
							   unsigned int id,
							   const std::string &name,
							   const std::vector<DailyCycleGroup> &groups,
							   int channel,
							   double scaleFactor)
{
	TiXmlElement schedElem("Schedule");
	schedElem.SetAttribute("id", (int)id);
	schedElem.SetAttribute("displayName", name);

	TiXmlElement useLinear("UseLinearInterpolation");
	useLinear.InsertEndChild(TiXmlText("false"));
	schedElem.InsertEndChild(useLinear);

	TiXmlElement haveAnnual("HaveAnnualSchedule");
	haveAnnual.InsertEndChild(TiXmlText("false"));
	schedElem.InsertEndChild(haveAnnual);

	TiXmlElement periods("Periods");
	TiXmlElement interval("ScheduleInterval");
	interval.SetAttribute("intervalStartDay", "0");

	TiXmlElement dailyCycles("DailyCycles");

	for (const DailyCycleGroup &grp : groups) {
		if (grp.m_days.empty())
			continue;

		TiXmlElement dc("DailyCycle");

		TiXmlElement dtElem("DayTypes");
		dtElem.InsertEndChild(TiXmlText(dayTypesString(grp.m_days)));
		dc.InsertEndChild(dtElem);

		// Get hourly values for requested channel, apply scale factor
		std::vector<double> hourly(24, 0.0);
		if (channel < grp.channelCount()) {
			for (int h = 0; h < 24; ++h)
				hourly[(size_t)h] = grp.m_values[(size_t)channel][(size_t)h] * scaleFactor;
		}

		std::string tpStr, valStr;
		compressHourlyValues(hourly, tpStr, valStr);

		TiXmlElement tpElem("TimePoints");
		tpElem.InsertEndChild(TiXmlText(tpStr));
		dc.InsertEndChild(tpElem);

		TiXmlElement valElem("Values");
		valElem.InsertEndChild(TiXmlText(valStr));
		dc.InsertEndChild(valElem);

		dailyCycles.InsertEndChild(dc);
	}

	interval.InsertEndChild(dailyCycles);
	periods.InsertEndChild(interval);
	schedElem.InsertEndChild(periods);

	container->InsertEndChild(schedElem);
}


/*! Creates a VICUS InternalLoad element and appends it to the container. */
static void writeVicusInternalLoad(TiXmlElement *container,
								   unsigned int id,
								   const std::string &name,
								   const std::string &category,
								   const std::string &methodTag,
								   const std::string &methodValue,
								   unsigned int scheduleId1,
								   const std::string &schedRef1Tag,
								   unsigned int scheduleId2,
								   const std::string &schedRef2Tag,
								   const std::string &paramName,
								   const std::string &paramUnit,
								   double paramValue,
								   double convectiveHeatFactor)
{
	TiXmlElement elem("InternalLoad");
	elem.SetAttribute("id", (int)id);
	elem.SetAttribute("displayName", name);

	TiXmlElement catElem("Category");
	catElem.InsertEndChild(TiXmlText(category));
	elem.InsertEndChild(catElem);

	TiXmlElement methodElem(methodTag.c_str());
	methodElem.InsertEndChild(TiXmlText(methodValue));
	elem.InsertEndChild(methodElem);

	// Schedule reference 1
	TiXmlElement ref1(schedRef1Tag.c_str());
	ref1.InsertEndChild(TiXmlText(std::to_string(scheduleId1)));
	elem.InsertEndChild(ref1);

	// Schedule reference 2 (optional)
	if (!schedRef2Tag.empty() && scheduleId2 > 0) {
		TiXmlElement ref2(schedRef2Tag.c_str());
		ref2.InsertEndChild(TiXmlText(std::to_string(scheduleId2)));
		elem.InsertEndChild(ref2);
	}

	// IBK:Parameter elements
	auto addParam = [&](const std::string &pName, const std::string &pUnit, double pVal) {
		TiXmlElement p("IBK:Parameter");
		p.SetAttribute("name", pName);
		p.SetAttribute("unit", pUnit);
		std::ostringstream ss;
		if (std::abs(pVal - std::round(pVal)) < 1e-8)
			ss << (int)std::round(pVal);
		else
			ss << pVal;
		p.InsertEndChild(TiXmlText(ss.str()));
		elem.InsertEndChild(p);
	};

	addParam(paramName, paramUnit, paramValue);
	addParam("ConvectiveHeatFactor", "---", convectiveHeatFactor);
	addParam("LatentHeatFactor", "---", 0);
	addParam("LossHeatFactor", "---", 0);

	container->InsertEndChild(elem);
}


/*! Creates a VICUS ZoneControlThermostat element and appends it to the container. */
static void writeVicusThermostat(TiXmlElement *container,
								 unsigned int id,
								 const std::string &name,
								 unsigned int heatingSchedId,
								 unsigned int coolingSchedId)
{
	TiXmlElement elem("ZoneControlThermostat");
	elem.SetAttribute("id", (int)id);
	elem.SetAttribute("displayName", name);

	TiXmlElement cv("ControlValue");
	cv.InsertEndChild(TiXmlText("AirTemperature"));
	elem.InsertEndChild(cv);

	TiXmlElement ct("ControllerType");
	ct.InsertEndChild(TiXmlText("Analog"));
	elem.InsertEndChild(ct);

	TiXmlElement hRef("IdHeatingSetpointSchedule");
	hRef.InsertEndChild(TiXmlText(std::to_string(heatingSchedId)));
	elem.InsertEndChild(hRef);

	TiXmlElement cRef("IdCoolingSetpointSchedule");
	cRef.InsertEndChild(TiXmlText(std::to_string(coolingSchedId)));
	elem.InsertEndChild(cRef);

	TiXmlElement tol("IBK:Parameter");
	tol.SetAttribute("name", "Tolerance");
	tol.SetAttribute("unit", "K");
	tol.InsertEndChild(TiXmlText("0.1"));
	elem.InsertEndChild(tol);

	container->InsertEndChild(elem);
}


/*! Creates a VICUS ZoneTemplate element and appends it to the container.
	IDs of 0 are treated as "not present" and their reference element is omitted. */
static void writeVicusZoneTemplate(TiXmlElement *container,
								   unsigned int id,
								   const std::string &name,
								   unsigned int personLoadId,
								   unsigned int equipLoadId,
								   unsigned int lightLoadId,
								   unsigned int thermostatId)
{
	TiXmlElement elem("ZoneTemplate");
	elem.SetAttribute("id", (int)id);
	elem.SetAttribute("displayName", name);

	if (personLoadId > 0) {
		TiXmlElement ip("IntLoadPerson");
		ip.InsertEndChild(TiXmlText(std::to_string(personLoadId)));
		elem.InsertEndChild(ip);
	}

	if (equipLoadId > 0) {
		TiXmlElement ie("IntLoadEquipment");
		ie.InsertEndChild(TiXmlText(std::to_string(equipLoadId)));
		elem.InsertEndChild(ie);
	}

	if (lightLoadId > 0) {
		TiXmlElement il("IntLoadLighting");
		il.InsertEndChild(TiXmlText(std::to_string(lightLoadId)));
		elem.InsertEndChild(il);
	}

	if (thermostatId > 0) {
		TiXmlElement tc("ControlThermostat");
		tc.InsertEndChild(TiXmlText(std::to_string(thermostatId)));
		elem.InsertEndChild(tc);
	}

	container->InsertEndChild(elem);
}


/*! Finds or creates a child element with the given name. */
static TiXmlElement * findOrCreateChild(TiXmlElement *parent, const char *childName) {
	TiXmlElement *child = parent->FirstChildElement(childName);
	if (!child) {
		TiXmlElement newChild(childName);
		parent->InsertEndChild(newChild);
		child = parent->FirstChildElement(childName);
	}
	return child;
}


bool exportToVicus(const QString &vicusFilePath,
				   const std::vector<VicusExportProfileData> &profiles)
{
	std::string filePath = vicusFilePath.toStdString();

	// Load .vicus file
	TiXmlDocument doc(filePath);
	if (!doc.LoadFile())
		return false;

	// Navigate to VicusProject/Project/EmbeddedDatabase
	TiXmlElement *root = doc.RootElement();
	if (!root)
		return false;

	// Accept both "VicusProject" root or "Project" as direct root
	TiXmlElement *project = nullptr;
	if (std::string(root->Value()) == "VicusProject")
		project = root->FirstChildElement("Project");
	else if (std::string(root->Value()) == "Project")
		project = root;

	if (!project)
		return false;

	TiXmlElement *embeddedDB = findOrCreateChild(project, "EmbeddedDatabase");

	// Find or create container elements
	TiXmlElement *schedulesContainer = findOrCreateChild(embeddedDB, "Schedules");
	TiXmlElement *intLoadsContainer = findOrCreateChild(embeddedDB, "InternalLoads");
	TiXmlElement *thermostatsContainer = findOrCreateChild(embeddedDB, "ZoneControlThermostats");
	TiXmlElement *zoneTemplatesContainer = findOrCreateChild(embeddedDB, "ZoneTemplates");

	// Scan all existing IDs in the document
	unsigned int maxId = 0;
	scanMaxId(root, maxId);
	unsigned int nextId = maxId + 1;

	// Generate elements for each profile
	for (const VicusExportProfileData &pd : profiles) {
		std::string profName = pd.name.toStdString();

		bool hasPerson = (pd.personPerArea > 1e-8);
		bool hasEquip  = (pd.equipmentPowerPerArea > 1e-8);
		bool hasLight  = (pd.lightingPowerDensity > 1e-8);

		// Allocate IDs only for elements that will be created
		unsigned int idOccupancySched	= hasPerson ? nextId++ : 0;
		unsigned int idActivitySched	= hasPerson ? nextId++ : 0;
		unsigned int idEquipmentSched	= hasEquip  ? nextId++ : 0;
		unsigned int idLightingSched	= hasLight  ? nextId++ : 0;
		unsigned int idHeatingSched		= nextId++;  // always created
		unsigned int idCoolingSched		= nextId++;  // always created
		unsigned int idPersonLoad		= hasPerson ? nextId++ : 0;
		unsigned int idEquipLoad		= hasEquip  ? nextId++ : 0;
		unsigned int idLightLoad		= hasLight  ? nextId++ : 0;
		unsigned int idThermostat		= nextId++;  // always created
		unsigned int idZoneTemplate		= nextId++;  // always created

		// --- Schedules ---
		if (hasPerson) {
			// 1. Occupancy schedule (person ch0, % → fraction: scale 1/100)
			writeVicusSchedule(schedulesContainer, idOccupancySched,
							   profName + " - Occupancy", pd.personGroups, 0, 1.0 / 100.0);
			// 2. Activity schedule (person ch1, W/Person → W/Person: scale 1.0)
			writeVicusSchedule(schedulesContainer, idActivitySched,
							   profName + " - Activity", pd.personGroups, 1, 1.0);
		}

		if (hasEquip) {
			// 3. Equipment schedule (equipment ch0, % → fraction: scale 1/100)
			writeVicusSchedule(schedulesContainer, idEquipmentSched,
							   profName + " - Equipment", pd.equipmentGroups, 0, 1.0 / 100.0);
		}

		if (hasLight) {
			// 4. Lighting schedule (lighting ch0, % → fraction: scale 1/100)
			writeVicusSchedule(schedulesContainer, idLightingSched,
							   profName + " - Lighting", pd.lightingGroups, 0, 1.0 / 100.0);
		}

		// 5. Heating setpoint schedule (°C → °C: scale 1.0) — always created
		writeVicusSchedule(schedulesContainer, idHeatingSched,
						   profName + " - Heating Setpoint", pd.heatingGroups, 0, 1.0);

		// 6. Cooling setpoint schedule (°C → °C: scale 1.0) — always created
		writeVicusSchedule(schedulesContainer, idCoolingSched,
						   profName + " - Cooling Setpoint", pd.coolingGroups, 0, 1.0);

		// --- Internal Loads ---
		if (hasPerson) {
			// 7. Person internal load
			writeVicusInternalLoad(intLoadsContainer, idPersonLoad,
								   profName + " - Person",
								   "Person",
								   "PersonCountMethod", "PersonPerArea",
								   idOccupancySched, "IdOccupancySchedule",
								   idActivitySched, "IdActivitySchedule",
								   "PersonPerArea", "Person/m2", pd.personPerArea,
								   0.5);
		}

		if (hasEquip) {
			// 8. Equipment internal load
			writeVicusInternalLoad(intLoadsContainer, idEquipLoad,
								   profName + " - Equipment",
								   "ElectricEquiment",
								   "PowerMethod", "PowerPerArea",
								   idEquipmentSched, "IdPowerManagementSchedule",
								   0, "",
								   "PowerPerArea", "W/m2", pd.equipmentPowerPerArea,
								   0.8);
		}

		if (hasLight) {
			// 9. Lighting internal load
			writeVicusInternalLoad(intLoadsContainer, idLightLoad,
								   profName + " - Lighting",
								   "Lighting",
								   "PowerMethod", "PowerPerArea",
								   idLightingSched, "IdPowerManagementSchedule",
								   0, "",
								   "PowerPerArea", "W/m2", pd.lightingPowerDensity,
								   0.5);
		}

		// --- Thermostat --- (always created)
		// 10. Zone control thermostat
		writeVicusThermostat(thermostatsContainer, idThermostat,
							 profName + " - Thermostat",
							 idHeatingSched, idCoolingSched);

		// --- Zone Template --- (always created, references only non-zero loads)
		// 11. Zone template referencing non-skipped items
		writeVicusZoneTemplate(zoneTemplatesContainer, idZoneTemplate,
							   profName,
							   idPersonLoad, idEquipLoad, idLightLoad, idThermostat);
	}

	// Save modified document
	return doc.SaveFile(filePath);
}


bool exportVicusDatabaseFiles(const QString &outputDir,
							  const std::vector<VicusExportProfileData> &profiles)
{
	// Create three documents with root elements
	TiXmlDocument schedDoc;
	schedDoc.InsertEndChild(TiXmlDeclaration("1.0", "UTF-8", ""));
	TiXmlElement schedRoot("Schedules");

	TiXmlDocument loadsDoc;
	loadsDoc.InsertEndChild(TiXmlDeclaration("1.0", "UTF-8", ""));
	TiXmlElement loadsRoot("InternalLoads");

	TiXmlDocument thermoDoc;
	thermoDoc.InsertEndChild(TiXmlDeclaration("1.0", "UTF-8", ""));
	TiXmlElement thermoRoot("ZoneControlThermostats");

	unsigned int nextId = 1;

	for (const VicusExportProfileData &pd : profiles) {
		std::string profName = pd.name.toStdString();

		bool hasPerson = (pd.personPerArea > 1e-8);
		bool hasEquip  = (pd.equipmentPowerPerArea > 1e-8);
		bool hasLight  = (pd.lightingPowerDensity > 1e-8);

		// Allocate IDs only for elements that will be created
		unsigned int idOccupancySched	= hasPerson ? nextId++ : 0;
		unsigned int idActivitySched	= hasPerson ? nextId++ : 0;
		unsigned int idEquipmentSched	= hasEquip  ? nextId++ : 0;
		unsigned int idLightingSched	= hasLight  ? nextId++ : 0;
		unsigned int idHeatingSched		= nextId++;
		unsigned int idCoolingSched		= nextId++;
		unsigned int idPersonLoad		= hasPerson ? nextId++ : 0;
		unsigned int idEquipLoad		= hasEquip  ? nextId++ : 0;
		unsigned int idLightLoad		= hasLight  ? nextId++ : 0;
		unsigned int idThermostat		= nextId++;

		// --- Schedules ---
		if (hasPerson) {
			writeVicusSchedule(&schedRoot, idOccupancySched,
							   profName + " - Occupancy", pd.personGroups, 0, 1.0 / 100.0);
			writeVicusSchedule(&schedRoot, idActivitySched,
							   profName + " - Activity", pd.personGroups, 1, 1.0);
		}
		if (hasEquip) {
			writeVicusSchedule(&schedRoot, idEquipmentSched,
							   profName + " - Equipment", pd.equipmentGroups, 0, 1.0 / 100.0);
		}
		if (hasLight) {
			writeVicusSchedule(&schedRoot, idLightingSched,
							   profName + " - Lighting", pd.lightingGroups, 0, 1.0 / 100.0);
		}
		writeVicusSchedule(&schedRoot, idHeatingSched,
						   profName + " - Heating Setpoint", pd.heatingGroups, 0, 1.0);
		writeVicusSchedule(&schedRoot, idCoolingSched,
						   profName + " - Cooling Setpoint", pd.coolingGroups, 0, 1.0);

		// --- Internal Loads ---
		if (hasPerson) {
			writeVicusInternalLoad(&loadsRoot, idPersonLoad,
								   profName + " - Person",
								   "Person",
								   "PersonCountMethod", "PersonPerArea",
								   idOccupancySched, "IdOccupancySchedule",
								   idActivitySched, "IdActivitySchedule",
								   "PersonPerArea", "Person/m2", pd.personPerArea,
								   0.5);
		}
		if (hasEquip) {
			writeVicusInternalLoad(&loadsRoot, idEquipLoad,
								   profName + " - Equipment",
								   "ElectricEquiment",
								   "PowerMethod", "PowerPerArea",
								   idEquipmentSched, "IdPowerManagementSchedule",
								   0, "",
								   "PowerPerArea", "W/m2", pd.equipmentPowerPerArea,
								   0.8);
		}
		if (hasLight) {
			writeVicusInternalLoad(&loadsRoot, idLightLoad,
								   profName + " - Lighting",
								   "Lighting",
								   "PowerMethod", "PowerPerArea",
								   idLightingSched, "IdPowerManagementSchedule",
								   0, "",
								   "PowerPerArea", "W/m2", pd.lightingPowerDensity,
								   0.5);
		}

		// --- Thermostat ---
		writeVicusThermostat(&thermoRoot, idThermostat,
							 profName + " - Thermostat",
							 idHeatingSched, idCoolingSched);
	}

	// Insert root elements into documents and save
	schedDoc.InsertEndChild(schedRoot);
	loadsDoc.InsertEndChild(loadsRoot);
	thermoDoc.InsertEndChild(thermoRoot);

	std::string dir = outputDir.toStdString();
	if (!dir.empty() && dir.back() != '/')
		dir += '/';

	bool ok = true;
	ok = schedDoc.SaveFile(dir + "db_schedules_18599.xml") && ok;
	ok = loadsDoc.SaveFile(dir + "db_internalLoads_18599.xml") && ok;
	ok = thermoDoc.SaveFile(dir + "db_zoneControlThermostat_18599.xml") && ok;
	return ok;
}
