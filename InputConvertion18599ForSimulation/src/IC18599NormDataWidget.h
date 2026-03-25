#ifndef IC18599NormDataWidgetH
#define IC18599NormDataWidgetH

#include <QWidget>

class QTableWidget;
class QLabel;

class IC18599NormDataWidget : public QWidget {
	Q_OBJECT
public:
	explicit IC18599NormDataWidget(QWidget *parent = nullptr);

	/*! Loads norm data from a semicolon-separated CSV file.
		Returns true on success. */
	bool loadCSV(const QString &fname);

	/*! Returns profile names (column headers from CSV, excluding first column). */
	QStringList profileNames() const;

	/*! Returns a numeric value for a given profile and parameter name.
		Searches for paramName in column 0, then returns the value in the profile's column.
		Returns 0.0 if not found. */
	double getProfileValue(const QString &profileName, const QString &paramName) const;

	/*! Returns a string value for a given profile and parameter name.
		Useful for non-numeric values like time strings ("07:00").
		Returns empty string if not found. */
	QString getProfileString(const QString &profileName, const QString &paramName) const;

private:
	void setupUI();

	QTableWidget	*m_tableWidget = nullptr;
	QLabel			*m_infoLabel = nullptr;
};

#endif // IC18599NormDataWidgetH
