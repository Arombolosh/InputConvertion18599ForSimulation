#include "IC18599EnergyPlusExport.h"

#include "IC18599VicusExport.h"
#include "IC18599ScheduleEditWidget.h"

#include <QFile>
#include <QTextStream>

#include <cmath>


/*! Transliterates German Umlauts and special characters to ASCII.
	EnergyPlus IDF files are ASCII-based and cannot handle Unicode. */
static QString toAscii(const QString &s) {
	QString r = s;
	r.replace(QStringLiteral("ä"), "ae");
	r.replace(QStringLiteral("ö"), "oe");
	r.replace(QStringLiteral("ü"), "ue");
	r.replace(QStringLiteral("Ä"), "Ae");
	r.replace(QStringLiteral("Ö"), "Oe");
	r.replace(QStringLiteral("Ü"), "Ue");
	r.replace(QStringLiteral("ß"), "ss");
	return r;
}


/*! Maps IC18599 day index (0=Mon..6=Sun) to EnergyPlus day name. */
static const char * epDayName(int ic18599Day) {
	static const char *names[] = {
		"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"
	};
	if (ic18599Day >= 0 && ic18599Day <= 6)
		return names[ic18599Day];
	return "Monday";
}


/*! Converts a set of IC18599 day indices to an EnergyPlus For: keyword string.
	If \a isLast is true, uses "AllOtherDays" to catch any remaining day types
	(Holidays, DesignDays, etc.). */
static QString epForString(const std::set<int> &days, bool isLast) {
	if (isLast)
		return "AllOtherDays";
	if (days.size() == 7)
		return "AllDays";
	if (days == std::set<int>{0,1,2,3,4})
		return "WeekDays";
	if (days == std::set<int>{5,6})
		return "Weekends";

	// List individual day names
	QStringList parts;
	for (int d : days)
		parts.append(epDayName(d));
	return parts.join(" ");
}


/*! Compresses 24 hourly values into EnergyPlus Until/value pairs.
	EnergyPlus uses end-points: Until: 7:00 means the value applies
	from the previous time point up to 7:00.
	Consecutive hours with identical values are merged.
	The last entry is always Until: 24:00. */
struct UntilPair {
	int endHour;	///< 1-24
	double value;
};

static std::vector<UntilPair> compressToUntilPairs(const std::vector<double> &hourly)
{
	std::vector<UntilPair> pairs;
	if (hourly.empty()) {
		pairs.push_back({24, 0.0});
		return pairs;
	}

	double current = hourly[0];
	for (int h = 1; h < 24 && h < (int)hourly.size(); ++h) {
		if (std::abs(hourly[(size_t)h] - current) > 1e-8) {
			pairs.push_back({h, current});
			current = hourly[(size_t)h];
		}
	}
	pairs.push_back({24, current});
	return pairs;
}


/*! Formats a double value for IDF output:
	integers without decimals, others with up to 4 significant digits. */
static QString formatVal(double v) {
	if (std::abs(v - std::round(v)) < 1e-8)
		return QString::number((int)std::round(v));
	double rounded = std::round(v * 10000.0) / 10000.0;
	return QString::number(rounded, 'g', 6);
}


/*! Writes a Schedule:Compact object to the stream.
	\param out          Output stream
	\param name         Schedule name
	\param typeLimits   ScheduleTypeLimits name (Fraction, Temperature, Any Number)
	\param groups       DailyCycleGroup data (from IC18599)
	\param channel      Which channel of the group to use
	\param scaleFactor  Multiplier for values (e.g. 1/100 to convert % to fraction)
*/
static void writeCompactSchedule(QTextStream &out,
								 const QString &name,
								 const QString &typeLimits,
								 const std::vector<DailyCycleGroup> &groups,
								 int channel,
								 double scaleFactor)
{
	out << "Schedule:Compact,\n";
	out << "    " << name << ",\n";
	out << "    " << typeLimits << ",\n";
	out << "    Through: 12/31,\n";

	// Collect non-empty groups
	std::vector<size_t> validGroups;
	for (size_t g = 0; g < groups.size(); ++g) {
		if (!groups[g].m_days.empty())
			validGroups.push_back(g);
	}

	if (validGroups.empty()) {
		// Fallback: single group covering all days with zero
		out << "    For: AllDays,\n";
		out << "    Until: 24:00, 0;\n\n";
		return;
	}

	for (size_t vi = 0; vi < validGroups.size(); ++vi) {
		const DailyCycleGroup &grp = groups[validGroups[vi]];
		bool isLastGroup = (vi == validGroups.size() - 1);

		// For single-group schedules covering all 7 days, use AllDays
		QString forStr;
		if (validGroups.size() == 1 && grp.m_days.size() == 7)
			forStr = "AllDays";
		else
			forStr = epForString(grp.m_days, isLastGroup);

		out << "    For: " << forStr << ",\n";

		// Get hourly values for the channel, apply scale factor
		std::vector<double> hourly(24, 0.0);
		if (channel < grp.channelCount()) {
			for (int h = 0; h < 24; ++h)
				hourly[(size_t)h] = grp.m_values[(size_t)channel][(size_t)h] * scaleFactor;
		}

		std::vector<UntilPair> pairs = compressToUntilPairs(hourly);

		for (size_t p = 0; p < pairs.size(); ++p) {
			bool isLastEntry = isLastGroup && (p == pairs.size() - 1);
			out << "    Until: " << pairs[p].endHour << ":00, "
				<< formatVal(pairs[p].value)
				<< (isLastEntry ? ";\n" : ",\n");
		}
	}

	out << "\n";
}


bool exportToEnergyPlus(const QString &idfFilePath,
						const std::vector<VicusExportProfileData> &profiles)
{
	QFile file(idfFilePath);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream out(&file);

	// --- Header ---
	out << "! DIN 18599 Schedules for EnergyPlus\n";
	out << "! Generated by InputConvertion18599ForSimulation\n";
	out << "!\n";
	out << "! This file contains Schedule:Compact definitions and\n";
	out << "! ThermostatSetpoint:DualSetpoint objects for all exported profiles.\n";
	out << "! Copy the relevant sections into your EnergyPlus model.\n\n";

	// --- ScheduleTypeLimits ---
	out << "! ===== Schedule Type Limits =====\n\n";

	out << "ScheduleTypeLimits,\n";
	out << "    Fraction,\n";
	out << "    0.0,\n";
	out << "    1.0,\n";
	out << "    CONTINUOUS;\n\n";

	out << "ScheduleTypeLimits,\n";
	out << "    Temperature,\n";
	out << "    -60,\n";
	out << "    200,\n";
	out << "    CONTINUOUS,\n";
	out << "    Temperature;\n\n";

	out << "ScheduleTypeLimits,\n";
	out << "    Any Number;\n\n";

	out << "ScheduleTypeLimits,\n";
	out << "    Control Type,\n";
	out << "    0,\n";
	out << "    4,\n";
	out << "    DISCRETE;\n\n";

	// --- Profiles ---
	for (const VicusExportProfileData &pd : profiles) {
		QString profName = toAscii(pd.name);

		bool hasPerson = (pd.personPerArea > 1e-8);
		bool hasEquip  = (pd.equipmentPowerPerArea > 1e-8);
		bool hasLight  = (pd.lightingPowerDensity > 1e-8);

		out << "! ===== Profile: " << profName << " =====\n\n";

		// --- Schedules ---
		if (hasPerson) {
			writeCompactSchedule(out,
				profName + " - Occupancy", "Fraction",
				pd.personGroups, 0, 1.0 / 100.0);

			writeCompactSchedule(out,
				profName + " - Activity", "Any Number",
				pd.personGroups, 1, 1.0);
		}

		if (hasEquip) {
			writeCompactSchedule(out,
				profName + " - Equipment", "Fraction",
				pd.equipmentGroups, 0, 1.0 / 100.0);
		}

		if (hasLight) {
			writeCompactSchedule(out,
				profName + " - Lighting", "Fraction",
				pd.lightingGroups, 0, 1.0 / 100.0);
		}

		// Heating/cooling setpoints — always created
		writeCompactSchedule(out,
			profName + " - Heating Setpoint", "Temperature",
			pd.heatingGroups, 0, 1.0);

		writeCompactSchedule(out,
			profName + " - Cooling Setpoint", "Temperature",
			pd.coolingGroups, 0, 1.0);

		// Control type schedule — constant 4 (DualSetpoint)
		out << "Schedule:Compact,\n";
		out << "    " << profName << " - Thermostat Control Type,\n";
		out << "    Control Type,\n";
		out << "    Through: 12/31,\n";
		out << "    For: AllDays,\n";
		out << "    Until: 24:00, 4;\n\n";

		// ThermostatSetpoint:DualSetpoint
		out << "ThermostatSetpoint:DualSetpoint,\n";
		out << "    " << profName << " - DualSetpoint,\n";
		out << "    " << profName << " - Heating Setpoint,\n";
		out << "    " << profName << " - Cooling Setpoint;\n\n";
	}

	file.close();
	return true;
}
