#include "IC18599Project.h"

#include <QFileInfo>
#include <QDir>

#include <tinyxml.h>

#include <sstream>


ProfileScheduleData & IC18599Project::getOrCreateProfile(const QString &name) {
	if (!m_profiles.contains(name)) {
		ProfileScheduleData data;
		// Default: weekdays + weekend groups with zero values
		DailyCycleGroup weekdays;
		weekdays.m_days = {0, 1, 2, 3, 4};
		data.m_personGroups.push_back(weekdays);
		data.m_equipmentGroups.push_back(weekdays);
		data.m_lightingGroups.push_back(weekdays);
		DailyCycleGroup weekend;
		weekend.m_days = {5, 6};
		data.m_personGroups.push_back(weekend);
		data.m_equipmentGroups.push_back(weekend);
		data.m_lightingGroups.push_back(weekend);

		// Heating/Cooling default: 20°C weekdays, 18°C weekend (heating) / 26°C (cooling)
		DailyCycleGroup heatingWeekdays;
		heatingWeekdays.m_days = {0, 1, 2, 3, 4};
		heatingWeekdays.m_values = {std::vector<double>(24, 20.0)};
		DailyCycleGroup heatingWeekend;
		heatingWeekend.m_days = {5, 6};
		heatingWeekend.m_values = {std::vector<double>(24, 18.0)};
		data.m_heatingGroups.push_back(heatingWeekdays);
		data.m_heatingGroups.push_back(heatingWeekend);

		DailyCycleGroup coolingWeekdays;
		coolingWeekdays.m_days = {0, 1, 2, 3, 4};
		coolingWeekdays.m_values = {std::vector<double>(24, 26.0)};
		DailyCycleGroup coolingWeekend;
		coolingWeekend.m_days = {5, 6};
		coolingWeekend.m_values = {std::vector<double>(24, 26.0)};
		data.m_coolingGroups.push_back(coolingWeekdays);
		data.m_coolingGroups.push_back(coolingWeekend);

		m_profiles[name] = data;
	}
	return m_profiles[name];
}


// --- Helper: serialize a DailyCycleGroup to XML ---
static TiXmlElement * writeGroupXML(const DailyCycleGroup &grp) {
	TiXmlElement *e = new TiXmlElement("DailyCycleGroup");

	// days attribute: "0,1,2,3,4"
	std::string daysStr;
	for (int d : grp.m_days) {
		if (!daysStr.empty()) daysStr += ",";
		daysStr += std::to_string(d);
	}
	e->SetAttribute("days", daysStr);

	// One <Values> element per channel
	for (const auto &channelVals : grp.m_values) {
		std::stringstream ss;
		for (size_t i = 0; i < channelVals.size(); ++i) {
			if (i > 0) ss << " ";
			ss << channelVals[i];
		}
		TiXmlElement *valElem = new TiXmlElement("Values");
		valElem->LinkEndChild(new TiXmlText(ss.str()));
		e->LinkEndChild(valElem);
	}

	return e;
}


// --- Helper: serialize a schedule (vector of groups) ---
static TiXmlElement * writeScheduleXML(const char *tag, const std::vector<DailyCycleGroup> &groups) {
	TiXmlElement *e = new TiXmlElement(tag);
	for (const DailyCycleGroup &grp : groups)
		e->LinkEndChild(writeGroupXML(grp));
	return e;
}


// --- Helper: parse days attribute ---
static std::set<int> parseDays(const std::string &str) {
	std::set<int> days;
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, ',')) {
		try { days.insert(std::stoi(token)); }
		catch (...) {}
	}
	return days;
}


// --- Helper: parse values text ---
static std::vector<double> parseValues(const std::string &str) {
	std::vector<double> vals;
	std::stringstream ss(str);
	double v;
	while (ss >> v)
		vals.push_back(v);
	// Ensure exactly 24 values
	vals.resize(24, 0.0);
	return vals;
}


// --- Helper: read schedule from XML ---
static std::vector<DailyCycleGroup> readScheduleXML(const TiXmlElement *schedElem) {
	std::vector<DailyCycleGroup> groups;
	const TiXmlElement *grpElem = schedElem->FirstChildElement("DailyCycleGroup");
	while (grpElem) {
		DailyCycleGroup grp;
		const char *daysAttr = grpElem->Attribute("days");
		if (daysAttr)
			grp.m_days = parseDays(daysAttr);

		// Collect all <Values> children (one per channel)
		grp.m_values.clear();
		const TiXmlElement *valElem = grpElem->FirstChildElement("Values");
		while (valElem) {
			if (valElem->GetText())
				grp.m_values.push_back(parseValues(valElem->GetText()));
			valElem = valElem->NextSiblingElement("Values");
		}
		if (grp.m_values.empty())
			grp.m_values.push_back(std::vector<double>(24, 0.0)); // fallback

		groups.push_back(grp);
		grpElem = grpElem->NextSiblingElement("DailyCycleGroup");
	}
	return groups;
}


bool IC18599Project::save(const QString &filePath) const {
	TiXmlDocument doc;
	TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "UTF-8", "");
	doc.LinkEndChild(decl);

	TiXmlElement *root = new TiXmlElement("IC18599Project");
	root->SetAttribute("fileVersion", "1");
	doc.LinkEndChild(root);

	// CsvFile (relative path)
	TiXmlElement *csvElem = new TiXmlElement("CsvFile");
	csvElem->LinkEndChild(new TiXmlText(m_csvFilePath.toStdString()));
	root->LinkEndChild(csvElem);

	// LastProfile
	TiXmlElement *lastProfElem = new TiXmlElement("LastProfile");
	lastProfElem->LinkEndChild(new TiXmlText(m_currentProfile.toStdString()));
	root->LinkEndChild(lastProfElem);

	// Profiles
	TiXmlElement *profilesElem = new TiXmlElement("Profiles");
	root->LinkEndChild(profilesElem);

	for (auto it = m_profiles.constBegin(); it != m_profiles.constEnd(); ++it) {
		TiXmlElement *profElem = new TiXmlElement("Profile");
		profElem->SetAttribute("name", it.key().toStdString());

		profElem->LinkEndChild(writeScheduleXML("PersonSchedule", it->m_personGroups));
		profElem->LinkEndChild(writeScheduleXML("EquipmentSchedule", it->m_equipmentGroups));
		profElem->LinkEndChild(writeScheduleXML("LightingSchedule", it->m_lightingGroups));
		profElem->LinkEndChild(writeScheduleXML("HeatingSchedule", it->m_heatingGroups));
		profElem->LinkEndChild(writeScheduleXML("CoolingSchedule", it->m_coolingGroups));

		profilesElem->LinkEndChild(profElem);
	}

	if (!doc.SaveFile(filePath.toStdString()))
		return false;

	return true;
}


bool IC18599Project::load(const QString &filePath) {
	TiXmlDocument doc;
	if (!doc.LoadFile(filePath.toStdString()))
		return false;

	TiXmlElement *root = doc.RootElement();
	if (!root || std::string(root->Value()) != "IC18599Project")
		return false;

	// CsvFile
	const TiXmlElement *csvElem = root->FirstChildElement("CsvFile");
	if (csvElem && csvElem->GetText())
		m_csvFilePath = QString::fromStdString(csvElem->GetText());

	// LastProfile
	const TiXmlElement *lastProfElem = root->FirstChildElement("LastProfile");
	if (lastProfElem && lastProfElem->GetText())
		m_currentProfile = QString::fromStdString(lastProfElem->GetText());

	// Profiles
	m_profiles.clear();
	const TiXmlElement *profilesElem = root->FirstChildElement("Profiles");
	if (profilesElem) {
		const TiXmlElement *profElem = profilesElem->FirstChildElement("Profile");
		while (profElem) {
			const char *nameAttr = profElem->Attribute("name");
			if (nameAttr) {
				QString name = QString::fromStdString(nameAttr);
				ProfileScheduleData data;

				const TiXmlElement *personElem = profElem->FirstChildElement("PersonSchedule");
				if (personElem)
					data.m_personGroups = readScheduleXML(personElem);

				const TiXmlElement *equipElem = profElem->FirstChildElement("EquipmentSchedule");
				if (equipElem)
					data.m_equipmentGroups = readScheduleXML(equipElem);

				const TiXmlElement *lightElem = profElem->FirstChildElement("LightingSchedule");
				if (lightElem)
					data.m_lightingGroups = readScheduleXML(lightElem);

				const TiXmlElement *heatElem = profElem->FirstChildElement("HeatingSchedule");
				if (heatElem)
					data.m_heatingGroups = readScheduleXML(heatElem);

				const TiXmlElement *coolElem = profElem->FirstChildElement("CoolingSchedule");
				if (coolElem)
					data.m_coolingGroups = readScheduleXML(coolElem);

				m_profiles[name] = data;
			}
			profElem = profElem->NextSiblingElement("Profile");
		}
	}

	m_modified = false;
	return true;
}
