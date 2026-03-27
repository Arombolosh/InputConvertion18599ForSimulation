#ifndef IC18599MainWindowH
#define IC18599MainWindowH

#include <QMainWindow>

#include <QMap>

#include "IC18599Project.h"

class QComboBox;

/*! Hourly distribution data for one profile (from TSV file). */
struct HourlyDistribution {
	std::vector<double> person;		///< 24 values (0-1 fractions) for person occupancy
	std::vector<double> equipment;	///< 24 values (0-1 fractions) for equipment utilization
};

namespace Ui {
	class IC18599MainWindow;
}

class IC18599MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit IC18599MainWindow(QWidget *parent = nullptr);
	~IC18599MainWindow() override;

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void onNewProject();
	void onOpenProject();
	void onSaveProject();
	void onSaveProjectAs();
	void onExportPDFReport();
	void onLoadCSV();
	void onProfileChanged(const QString &profileName);
	void onScheduleModified();
	void onQuit();
	void onLanguageSettings();

private:
	void populateProfileComboBox();
	void storeCurrentProfileData();
	void loadProfileDataToWidgets(const QString &profileName);
	void updateWindowTitle();
	void recalculateResults();
	bool maybeSave();
	bool loadDistributionTSV(const QString &tsvPath);

	Ui::IC18599MainWindow					*m_ui = nullptr;
	IC18599Project							m_project;
	QString									m_projectFilePath;
	QComboBox								*m_profileComboBox = nullptr;
	QMap<QString, HourlyDistribution>		m_distributions;	///< TSV distribution data, keyed by normalized profile name
};

#endif // IC18599MainWindowH
