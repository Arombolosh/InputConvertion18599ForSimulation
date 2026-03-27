#ifndef IC18599NormDataWidgetH
#define IC18599NormDataWidgetH

#include <QWidget>

class QTableWidget;
class QLabel;

/*! Row indices into the DIN 18599 CSV data (0-based, relative to first data row).
	The CSV format has a fixed row order defined by the standard. */
enum NormParamRow {
	RowDailyUsageStart       = 1,   // "tägliche Nutzungszeit Start"
	RowDailyUsageEnd         = 2,   // "tägliche Nutzungszeit Ende"
	RowAnnualUsageDays       = 3,   // "jährliche Nutzungszeit"
	RowHeatingSetpoint       = 12,  // "Raum-Solltemperatur Heizung"
	RowCoolingSetpoint       = 13,  // "Raum-Solltemperatur Kühlung"
	RowTempReduction         = 16,  // "Temperaturabsenkung reduzierter Betrieb"
	RowMaintainedIlluminance  = 26,  // "Wartungswert der Beleuchtungsstärke"
	RowReductionFactorKA     = 28,  // "Minderungsfaktor kA"
	RowRoomIndexK            = 30,  // "Raumindex k"
	RowAdjustmentFactorKVB   = 32,  // "Anpassungfaktor Beleuchtung vertikaler Flächen kVB"
	RowMaxOccupancyDensity   = 34,  // "maximale Belegungsdichte"
	RowFullUseHoursPersons   = 36,  // "Vollnutzungstunden Personen"
	RowFullUseHoursEquipment = 37,  // "Vollnutzungstunden Geräte"
	RowPersonActivitySensible = 38, // "Aktivität Personen (sensible)"
	RowEquipmentPower        = 39   // "Geräteleistung"
};

class IC18599NormDataWidget : public QWidget {
	Q_OBJECT
public:
	explicit IC18599NormDataWidget(QWidget *parent = nullptr);

	/*! Loads norm data from a semicolon-separated CSV file.
		Returns true on success. */
	bool loadCSV(const QString &fname);

	/*! Returns profile names (column headers from CSV, excluding first column). */
	QStringList profileNames() const;

	/*! Returns a numeric value for a given profile and data row index.
		The dataRowIndex is 0-based relative to the first data row (after header).
		Returns 0.0 if not found. */
	double getProfileValueByRow(const QString &profileName, int dataRowIndex) const;

	/*! Returns a string value for a given profile and data row index.
		Useful for non-numeric values like time strings ("07:00").
		Returns empty string if not found. */
	QString getProfileStringByRow(const QString &profileName, int dataRowIndex) const;

private:
	void setupUI();

	/*! Finds the column index for a profile name in m_rawData header row.
		Uses normalized comparison (underscores→spaces, collapsed whitespace).
		Returns -1 if not found. */
	int findProfileColumn(const QString &profileName) const;

	QTableWidget		*m_tableWidget = nullptr;
	QLabel				*m_infoLabel = nullptr;
	QVector<QStringList>	m_rawData;		///< All CSV rows including header, for row-index based access
};

#endif // IC18599NormDataWidgetH
