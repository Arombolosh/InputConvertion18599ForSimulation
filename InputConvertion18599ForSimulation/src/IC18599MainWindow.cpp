#include "IC18599MainWindow.h"
#include "ui_IC18599MainWindow.h"

#include "IC18599NormDataWidget.h"
#include "IC18599ScheduleEditWidget.h"
#include "IC18599Report.h"
#include "IC18599ReportSettings.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QFile>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPrinter>
#include <QTextStream>

/*! Returns default day groupings based on "jährliche Nutzungszeit" from norm data.
	First set = active days, second set = inactive days.
	- 365: single group Mon-Sun          — 24/7 buildings (hospitals, hotels, data centers)
	- 300: Mon-Sat + Sun                 — businesses open on Saturdays
	- 250: Mon-Fri + Sat-Sun             — standard office
	- 230: Mon-Fri + Sat-Sun             — industrial halls (some weeks off, same weekly pattern)
	- 200: Mon-Thu + Fri-Sun             — classrooms (4-day week)
	- 150: Tue-Thu + Mon,Fri-Sun         — midweek usage (auditoriums, congress)
*/
static std::vector<std::set<int>> defaultDayTemplate(int usageDays) {
	if (usageDays >= 365)
		return { {0,1,2,3,4,5,6} };
	if (usageDays >= 300)
		return { {0,1,2,3,4,5}, {6} };
	if (usageDays > 0 && usageDays <= 150)
		return { {1,2,3}, {0,4,5,6} };
	if (usageDays > 150 && usageDays <= 200)
		return { {0,1,2,3}, {4,5,6} };
	// 230, 250, or unknown: Mon-Fri + Sat-Sun
	return { {0,1,2,3,4}, {5,6} };
}


/*! Parses a time string like "07:00" and returns the hour (0-23). */
static int parseHour(const QString &timeStr) {
	QStringList parts = timeStr.split(':');
	if (!parts.isEmpty()) {
		bool ok;
		int h = parts[0].toInt(&ok);
		if (ok && h >= 0 && h <= 23)
			return h;
	}
	return 0;
}


/*! Returns true if hour falls in the active time window [startHour, endHour).
	Handles overnight wrapping (e.g. start=21, end=8).
	If startHour == endHour, all hours are considered active (24h usage). */
static bool isActiveHour(int hour, int startHour, int endHour) {
	if (startHour == endHour)
		return true;  // 24h usage (e.g. 00:00-00:00)
	if (startHour < endHour)
		return hour >= startHour && hour < endHour;
	// Wraps midnight (e.g. 21:00-08:00)
	return hour >= startHour || hour < endHour;
}


/*! Normalizes a profile name for matching between TSV (underscores) and
	display names (spaces, line-wrapped with newlines). */
static QString normalizeProfileName(const QString &name) {
	QString n = name;
	n.replace('_', ' ');
	n.replace('\n', ' ');
	return n.simplified();	// collapses whitespace and trims
}


IC18599MainWindow::IC18599MainWindow(QWidget *parent) :
	QMainWindow(parent),
	m_ui(new Ui::IC18599MainWindow)
{
	m_ui->setupUi(this);

	// --- Toolbar: add profile label + combo box ---
	m_ui->m_toolBar->addWidget(new QLabel(tr("Profile: "), this));
	m_profileComboBox = new QComboBox(this);
	m_profileComboBox->setMinimumWidth(250);
	m_profileComboBox->setEnabled(false);
	m_ui->m_toolBar->addWidget(m_profileComboBox);

	// --- Configure promoted schedule widgets ---
	m_ui->m_personScheduleWidget->configure(
		tr("Person Schedule"),
		{ {tr("Occupancy"), "[%]", QColor(70, 130, 180), 0.0, 100.0, 0.0},
		  {tr("Activity"),  "[W/Pers]", QColor(200, 80, 80), 0.0, 200.0, 80.0} });
	m_ui->m_equipmentScheduleWidget->configure(
		tr("Equipment - Utilization Schedule"), QColor(180, 120, 60));
	m_ui->m_lightingScheduleWidget->configure(
		tr("Lighting - Availability Schedule"), QColor(200, 180, 50));
	m_ui->m_heatingScheduleWidget->configure(
		tr("Heating - Temperature Setpoint"), QColor(200, 80, 80),
		QString::fromUtf8("[°C]"), 0.0, 50.0, 20.0);
	m_ui->m_coolingScheduleWidget->configure(
		tr("Cooling - Temperature Setpoint"), QColor(70, 130, 200),
		QString::fromUtf8("[°C]"), 0.0, 50.0, 26.0);

	// Setup result charts
	m_ui->m_personScheduleWidget->setResultChartCount(1);
	m_ui->m_equipmentScheduleWidget->setResultChartCount(1);
	m_ui->m_lightingScheduleWidget->setResultChartCount(1);

	// --- Connections ---
	connect(m_profileComboBox, &QComboBox::currentTextChanged,
			this, &IC18599MainWindow::onProfileChanged);

	connect(m_ui->m_personScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);
	connect(m_ui->m_equipmentScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);
	connect(m_ui->m_lightingScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);
	connect(m_ui->m_heatingScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);
	connect(m_ui->m_coolingScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);

	// Menu actions
	connect(m_ui->actionNew, &QAction::triggered, this, &IC18599MainWindow::onNewProject);
	connect(m_ui->actionOpen, &QAction::triggered, this, &IC18599MainWindow::onOpenProject);
	connect(m_ui->actionSave, &QAction::triggered, this, &IC18599MainWindow::onSaveProject);
	connect(m_ui->actionSaveAs, &QAction::triggered, this, &IC18599MainWindow::onSaveProjectAs);
	connect(m_ui->actionExportPDF, &QAction::triggered, this, &IC18599MainWindow::onExportPDFReport);
	connect(m_ui->actionLoadCSV, &QAction::triggered, this, &IC18599MainWindow::onLoadCSV);
	connect(m_ui->actionQuit, &QAction::triggered, this, &IC18599MainWindow::onQuit);

	// Status bar
	statusBar()->showMessage(tr("Ready. Please load a CSV file with norm data."));
	m_ui->m_logWidget->appendPlainText(tr("Program started."));
}


IC18599MainWindow::~IC18599MainWindow() {
	delete m_ui;
}


void IC18599MainWindow::onNewProject() {
	if (!maybeSave())
		return;

	m_project = IC18599Project();
	m_projectFilePath.clear();
	m_profileComboBox->clear();
	m_profileComboBox->setEnabled(false);
	m_ui->m_personScheduleWidget->setGroups({});
	m_ui->m_equipmentScheduleWidget->setGroups({});
	m_ui->m_lightingScheduleWidget->setGroups({});
	m_ui->m_heatingScheduleWidget->setGroups({});
	m_ui->m_coolingScheduleWidget->setGroups({});
	updateWindowTitle();
	m_ui->m_logWidget->appendPlainText(tr("New project created."));
	statusBar()->showMessage(tr("New project. Please load a CSV file."));
}


void IC18599MainWindow::onOpenProject() {
	if (!maybeSave())
		return;

	QString fname = QFileDialog::getOpenFileName(this,
		tr("Open Project"),
		QString(),
		tr("IC18599 Project Files (*.ic18599);;All Files (*)"));
	if (fname.isEmpty())
		return;

	IC18599Project proj;
	if (!proj.load(fname)) {
		m_ui->m_logWidget->appendPlainText(tr("Error loading: %1").arg(fname));
		QMessageBox::warning(this, tr("Error"), tr("Project file could not be loaded."));
		return;
	}

	m_project = proj;
	m_projectFilePath = fname;

	// Resolve CSV path relative to project file
	QFileInfo projInfo(fname);
	QString csvPath = projInfo.dir().filePath(m_project.m_csvFilePath);

	if (!m_ui->m_normDataWidget->loadCSV(csvPath)) {
		m_ui->m_logWidget->appendPlainText(tr("Warning: CSV could not be loaded: %1").arg(csvPath));
	}
	else {
		m_project.m_profileNames = m_ui->m_normDataWidget->profileNames();
		m_ui->m_logWidget->appendPlainText(tr("Norm data loaded: %1").arg(csvPath));

		// Try to load hourly distribution TSV from same directory
		QFileInfo csvAbsInfo(csvPath);
		QString tsvPath = csvAbsInfo.dir().filePath("PersonAndEquipmentLoadDistribution.tsv");
		if (loadDistributionTSV(tsvPath))
			m_ui->m_logWidget->appendPlainText(tr("Distribution data loaded."));
	}

	populateProfileComboBox();

	// Restore last selected profile
	int idx = m_profileComboBox->findText(m_project.m_currentProfile);
	if (idx >= 0)
		m_profileComboBox->setCurrentIndex(idx);

	updateWindowTitle();
	m_ui->m_logWidget->appendPlainText(tr("Project loaded: %1").arg(fname));
	statusBar()->showMessage(tr("Project: %1").arg(fname));
}


void IC18599MainWindow::onSaveProject() {
	if (m_projectFilePath.isEmpty()) {
		onSaveProjectAs();
		return;
	}

	storeCurrentProfileData();
	if (m_project.save(m_projectFilePath)) {
		m_project.m_modified = false;
		updateWindowTitle();
		m_ui->m_logWidget->appendPlainText(tr("Project saved: %1").arg(m_projectFilePath));
		statusBar()->showMessage(tr("Saved: %1").arg(m_projectFilePath));
	}
	else {
		m_ui->m_logWidget->appendPlainText(tr("Error saving: %1").arg(m_projectFilePath));
		QMessageBox::warning(this, tr("Error"), tr("Project could not be saved."));
	}
}


void IC18599MainWindow::onSaveProjectAs() {
	QString fname = QFileDialog::getSaveFileName(this,
		tr("Save Project As"),
		QString(),
		tr("IC18599 Project Files (*.ic18599);;All Files (*)"));
	if (fname.isEmpty())
		return;

	// Ensure .ic18599 extension
	if (!fname.endsWith(".ic18599", Qt::CaseInsensitive))
		fname += ".ic18599";

	// Store CSV path relative to project file
	if (!m_project.m_csvFilePath.isEmpty()) {
		QFileInfo projInfo(fname);
		QFileInfo csvInfo(m_project.m_csvFilePath);
		// If CSV path is absolute, make it relative
		if (csvInfo.isAbsolute())
			m_project.m_csvFilePath = projInfo.dir().relativeFilePath(m_project.m_csvFilePath);
	}

	m_projectFilePath = fname;
	onSaveProject();
}


void IC18599MainWindow::onExportPDFReport() {
	QString fname = QFileDialog::getSaveFileName(this,
		tr("Export PDF Report"),
		QString(),
		tr("PDF Files (*.pdf);;All Files (*)"));
	if (fname.isEmpty())
		return;

	if (!fname.endsWith(".pdf", Qt::CaseInsensitive))
		fname += ".pdf";

	// Store current profile data before generating report
	storeCurrentProfileData();

	IC18599ReportSettings settings;
	IC18599Report report(&settings);

	QString profile = m_project.m_currentProfile;
	QtExt::ReportData2<IC18599Project, QString> data(&m_project, &profile);
	report.set(&data);
	report.setFrames();

	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setOutputFileName(fname);
	printer.setPageSize(QPageSize(QPageSize::A4));

	QFont font = settings.m_defaultFont;
	report.print(&printer, font);

	m_ui->m_logWidget->appendPlainText(tr("PDF report exported: %1").arg(fname));
	statusBar()->showMessage(tr("Report exported: %1").arg(fname));
}


void IC18599MainWindow::onLoadCSV() {
	QString fname = QFileDialog::getOpenFileName(this,
		tr("Load DIN 18599 Norm Data"),
		QString(),
		tr("CSV Files (*.csv);;All Files (*)"));
	if (fname.isEmpty())
		return;

	bool ok = m_ui->m_normDataWidget->loadCSV(fname);
	if (ok) {
		m_project.m_csvFilePath = fname;  // store absolute, relativize on save
		m_project.m_profileNames = m_ui->m_normDataWidget->profileNames();

		// Try to load hourly distribution TSV from same directory
		QFileInfo csvInfo(fname);
		QString tsvPath = csvInfo.dir().filePath("PersonAndEquipmentLoadDistribution.tsv");
		if (loadDistributionTSV(tsvPath))
			m_ui->m_logWidget->appendPlainText(tr("Distribution data loaded: %1").arg(tsvPath));

		populateProfileComboBox();
		m_ui->m_logWidget->appendPlainText(tr("Norm data loaded: %1").arg(fname));
		statusBar()->showMessage(tr("File: %1").arg(fname));
		m_ui->m_tabWidget->setCurrentIndex(0);
	}
	else {
		m_ui->m_logWidget->appendPlainText(tr("Error loading: %1").arg(fname));
		statusBar()->showMessage(tr("Error loading."));
	}
}


void IC18599MainWindow::onProfileChanged(const QString &profileName) {
	if (profileName.isEmpty())
		return;

	// Store data from previous profile
	if (!m_project.m_currentProfile.isEmpty())
		storeCurrentProfileData();

	// Switch to new profile
	m_project.m_currentProfile = profileName;
	loadProfileDataToWidgets(profileName);
	recalculateResults();

	statusBar()->showMessage(tr("Profile: %1").arg(profileName));
}


void IC18599MainWindow::onScheduleModified() {
	m_project.m_modified = true;
	updateWindowTitle();
	recalculateResults();
}


void IC18599MainWindow::onQuit() {
	close();
}


void IC18599MainWindow::closeEvent(QCloseEvent *event) {
	if (maybeSave())
		event->accept();
	else
		event->ignore();
}


void IC18599MainWindow::populateProfileComboBox() {
	// Block signals to avoid triggering onProfileChanged during population
	m_profileComboBox->blockSignals(true);
	m_profileComboBox->clear();
	for (const QString &name : m_project.m_profileNames)
		m_profileComboBox->addItem(name);
	m_profileComboBox->setEnabled(m_profileComboBox->count() > 0);
	m_profileComboBox->blockSignals(false);

	// Select first profile
	if (m_profileComboBox->count() > 0) {
		m_profileComboBox->setCurrentIndex(0);
		onProfileChanged(m_profileComboBox->currentText());
	}
}


void IC18599MainWindow::storeCurrentProfileData() {
	if (m_project.m_currentProfile.isEmpty())
		return;

	ProfileScheduleData &data = m_project.getOrCreateProfile(m_project.m_currentProfile);
	data.m_personGroups = m_ui->m_personScheduleWidget->groups();
	data.m_equipmentGroups = m_ui->m_equipmentScheduleWidget->groups();
	data.m_lightingGroups = m_ui->m_lightingScheduleWidget->groups();
	data.m_heatingGroups = m_ui->m_heatingScheduleWidget->groups();
	data.m_coolingGroups = m_ui->m_coolingScheduleWidget->groups();
}


void IC18599MainWindow::loadProfileDataToWidgets(const QString &profileName) {
	bool isNew = !m_project.hasProfile(profileName);
	ProfileScheduleData &data = m_project.getOrCreateProfile(profileName);

	if (isNew) {
		// Determine day template from norm data
		int usageDays = (int)m_ui->m_normDataWidget->getProfileValue(
			profileName, QString::fromUtf8("jährliche Nutzungszeit"));
		auto tmpl = defaultDayTemplate(usageDays);

		// Parse daily start/end times
		QString startStr = m_ui->m_normDataWidget->getProfileString(
			profileName, QString::fromUtf8("tägliche Nutzungszeit Start"));
		QString endStr = m_ui->m_normDataWidget->getProfileString(
			profileName, QString::fromUtf8("tägliche Nutzungszeit Ende"));
		int startHour = parseHour(startStr);
		int endHour = parseHour(endStr);

		// Activity norm value for person channel 1
		double activityNorm = m_ui->m_normDataWidget->getProfileValue(
			profileName, QString::fromUtf8("Aktivität Personen (sensible)"));
		if (activityNorm <= 0) activityNorm = 80.0;  // fallback

		// Check for TSV distribution data
		QString normName = normalizeProfileName(profileName);
		bool hasDist = m_distributions.contains(normName);

		if (hasDist) {
			const HourlyDistribution &dist = m_distributions[normName];

			// Person channel 0: occupancy from distribution (fraction * 100 → %)
			std::vector<double> personOccActive(24);
			for (int h = 0; h < 24; ++h)
				personOccActive[(size_t)h] = dist.person[(size_t)h] * 100.0;
			std::vector<double> personOccInactive(24, 0.0);

			// Person channel 1: activity = norm value when occupied, 0 when not
			std::vector<double> activityActive(24, 0.0);
			for (int h = 0; h < 24; ++h) {
				if (dist.person[(size_t)h] > 0)
					activityActive[(size_t)h] = activityNorm;
			}
			std::vector<double> activityInactive(24, 0.0);

			data.m_personGroups.clear();
			for (size_t i = 0; i < tmpl.size(); ++i) {
				DailyCycleGroup g;
				g.m_days = tmpl[i];
				g.m_values = {
					(i == 0) ? personOccActive : personOccInactive,
					(i == 0) ? activityActive  : activityInactive
				};
				data.m_personGroups.push_back(g);
			}

			// Equipment from distribution (fraction * 100 → %)
			std::vector<double> equipActive(24);
			for (int h = 0; h < 24; ++h)
				equipActive[(size_t)h] = dist.equipment[(size_t)h] * 100.0;
			std::vector<double> equipInactive(24, 0.0);

			data.m_equipmentGroups.clear();
			for (size_t i = 0; i < tmpl.size(); ++i) {
				DailyCycleGroup g;
				g.m_days = tmpl[i];
				g.m_values = { (i == 0) ? equipActive : equipInactive };
				data.m_equipmentGroups.push_back(g);
			}
		}
		else {
			// Fallback: flat time-based pattern from start/end hours
			std::vector<double> activeProfile(24, 0.0);
			for (int h = 0; h < 24; ++h) {
				if (isActiveHour(h, startHour, endHour))
					activeProfile[(size_t)h] = 100.0;
			}
			std::vector<double> inactiveProfile(24, 0.0);

			// Person schedule: 2 channels
			std::vector<double> activityActive(24, 0.0);
			for (int h = 0; h < 24; ++h) {
				if (isActiveHour(h, startHour, endHour))
					activityActive[(size_t)h] = activityNorm;
			}
			std::vector<double> activityInactive(24, 0.0);

			data.m_personGroups.clear();
			for (size_t i = 0; i < tmpl.size(); ++i) {
				DailyCycleGroup g;
				g.m_days = tmpl[i];
				g.m_values = {
					(i == 0) ? activeProfile : inactiveProfile,
					(i == 0) ? activityActive : activityInactive
				};
				data.m_personGroups.push_back(g);
			}

			data.m_equipmentGroups.clear();
			for (size_t i = 0; i < tmpl.size(); ++i) {
				DailyCycleGroup g;
				g.m_days = tmpl[i];
				g.m_values = { (i == 0) ? activeProfile : inactiveProfile };
				data.m_equipmentGroups.push_back(g);
			}
		}

		// Lighting: always uses flat time-based pattern (no TSV data)
		std::vector<double> lightActive(24, 0.0);
		for (int h = 0; h < 24; ++h) {
			if (isActiveHour(h, startHour, endHour))
				lightActive[(size_t)h] = 100.0;
		}
		std::vector<double> lightInactive(24, 0.0);

		data.m_lightingGroups.clear();
		for (size_t i = 0; i < tmpl.size(); ++i) {
			DailyCycleGroup g;
			g.m_days = tmpl[i];
			g.m_values = { (i == 0) ? lightActive : lightInactive };
			data.m_lightingGroups.push_back(g);
		}

		// Read norm setpoints for heating/cooling
		double heatingSetpoint = m_ui->m_normDataWidget->getProfileValue(
			profileName, QString::fromUtf8("Raum-Solltemperatur Heizung"));
		double coolingSetpoint = m_ui->m_normDataWidget->getProfileValue(
			profileName, QString::fromUtf8("Raum-Solltemperatur Kühlung"));
		double tempReduction = m_ui->m_normDataWidget->getProfileValue(
			profileName, QString::fromUtf8("Temperaturabsenkung reduzierter Betrieb"));
		if (heatingSetpoint <= 0) heatingSetpoint = 21.0;  // fallback
		if (coolingSetpoint <= 0) coolingSetpoint = 24.0;
		double heatingReduced = heatingSetpoint - tempReduction;

		// Build heating 24h profiles
		std::vector<double> heatingActive(24, heatingReduced);
		std::vector<double> heatingInactive(24, heatingReduced);
		for (int h = 0; h < 24; ++h) {
			if (isActiveHour(h, startHour, endHour))
				heatingActive[(size_t)h] = heatingSetpoint;
		}

		// Build cooling 24h profiles (inactive = setpoint + reduction, i.e. allow higher temps)
		double coolingReduced = coolingSetpoint + tempReduction;
		std::vector<double> coolingActive(24, coolingReduced);
		std::vector<double> coolingInactive(24, coolingReduced);
		for (int h = 0; h < 24; ++h) {
			if (isActiveHour(h, startHour, endHour))
				coolingActive[(size_t)h] = coolingSetpoint;
		}

		// Heating: active days get time-based setpoint, inactive days stay at reduced temp
		data.m_heatingGroups.clear();
		for (size_t i = 0; i < tmpl.size(); ++i) {
			DailyCycleGroup g;
			g.m_days = tmpl[i];
			g.m_values = { (i == 0) ? heatingActive : heatingInactive };
			data.m_heatingGroups.push_back(g);
		}

		// Cooling: active days get time-based setpoint, inactive days stay at max (no cooling)
		data.m_coolingGroups.clear();
		for (size_t i = 0; i < tmpl.size(); ++i) {
			DailyCycleGroup g;
			g.m_days = tmpl[i];
			g.m_values = { (i == 0) ? coolingActive : coolingInactive };
			data.m_coolingGroups.push_back(g);
		}
	}

	m_ui->m_personScheduleWidget->setGroups(data.m_personGroups);
	m_ui->m_equipmentScheduleWidget->setGroups(data.m_equipmentGroups);
	m_ui->m_lightingScheduleWidget->setGroups(data.m_lightingGroups);
	m_ui->m_heatingScheduleWidget->setGroups(data.m_heatingGroups);
	m_ui->m_coolingScheduleWidget->setGroups(data.m_coolingGroups);
}


void IC18599MainWindow::updateWindowTitle() {
	QString title = tr("DIN 18599 - Conversion for Dynamic Simulation");
	if (!m_projectFilePath.isEmpty()) {
		QFileInfo fi(m_projectFilePath);
		title = fi.fileName() + " - " + title;
	}
	if (m_project.m_modified)
		title = "* " + title;
	setWindowTitle(title);
}


bool IC18599MainWindow::maybeSave() {
	if (!m_project.m_modified)
		return true;

	QMessageBox::StandardButton ret = QMessageBox::warning(this,
		tr("Unsaved Changes"),
		tr("The project has been modified. Do you want to save?"),
		QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

	if (ret == QMessageBox::Save) {
		onSaveProject();
		return !m_project.m_modified;  // false if save failed
	}
	else if (ret == QMessageBox::Cancel) {
		return false;
	}
	return true;  // Discard
}


void IC18599MainWindow::recalculateResults() {
	QString prof = m_project.m_currentProfile;
	if (prof.isEmpty())
		return;

	// --- Persons ---
	double fullUseHoursPerson = m_ui->m_normDataWidget->getProfileValue(prof, "Vollnutzungstunden Personen");
	double areaPerPerson = m_ui->m_normDataWidget->getProfileValue(prof, QString::fromUtf8("maximale Belegungsdichte"));
	if (areaPerPerson <= 0) areaPerPerson = 1.0;  // avoid division by zero

	std::vector<double> personWeek = m_ui->m_personScheduleWidget->weekValues(0); // occupancy 0-100
	std::vector<double> activityWeek = m_ui->m_personScheduleWidget->weekValues(1); // activity W/Pers
	double sumPerson = 0;
	for (double v : personWeek) sumPerson += v / 100.0;
	double normFactorPerson = (sumPerson > 0) ? fullUseHoursPerson / sumPerson : 0;

	// Result: normalized occupancy * activity / area → [W/m²]
	std::vector<double> personHeatGain(168);
	for (int i = 0; i < 168; ++i)
		personHeatGain[i] = (personWeek[i] / 100.0) * normFactorPerson * activityWeek[i] / areaPerPerson;
	m_ui->m_personScheduleWidget->setResultData(0, personHeatGain,
		tr("Person Heat Gain"), QString::fromUtf8("[W/m²]"), QColor(80, 150, 80));

	// --- Equipment ---
	double equipPower = m_ui->m_normDataWidget->getProfileValue(prof, QString::fromUtf8("Geräteleistung"));

	std::vector<double> equipWeek = m_ui->m_equipmentScheduleWidget->weekValues();

	// Result: schedule fraction * equipment power / area → [W/m²]
	std::vector<double> equipHeatGain(168);
	for (int i = 0; i < 168; ++i)
		equipHeatGain[i] = (equipWeek[i] / 100.0) * equipPower / areaPerPerson;
	m_ui->m_equipmentScheduleWidget->setResultData(0, equipHeatGain,
		tr("Equipment Heat Gain"), QString::fromUtf8("[W/m²]"), QColor(180, 140, 80));

	// --- Lighting ---
	std::vector<double> lightWeek = m_ui->m_lightingScheduleWidget->weekValues();
	std::vector<double> lightFraction(168);
	for (int i = 0; i < 168; ++i)
		lightFraction[i] = lightWeek[i] / 100.0;
	m_ui->m_lightingScheduleWidget->setResultData(0, lightFraction,
		tr("Availability (fraction)"), "[-]", QColor(200, 180, 50));
}


bool IC18599MainWindow::loadDistributionTSV(const QString &tsvPath) {
	QFile file(tsvPath);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream in(&file);
	in.setEncoding(QStringConverter::Utf8);

	QVector<QStringList> rows;
	while (!in.atEnd()) {
		QString line = in.readLine();
		rows.append(line.split('\t'));
	}
	file.close();

	if (rows.size() < 2)
		return false;

	// Row 0: header with profile names (first column is "Profile")
	const QStringList &header = rows[0];
	int numProfiles = header.size() - 1;

	// Find section start rows by marker text
	int personSection = -1, equipSection = -1;
	for (int r = 1; r < rows.size(); ++r) {
		if (rows[r].isEmpty())
			continue;
		const QString &marker = rows[r][0];
		if (marker.contains("Personenzeitplan"))
			personSection = r;
		else if (marker.contains("zeitplan") && marker.contains("Ger"))
			equipSection = r;
	}

	if (personSection < 0 || equipSection < 0)
		return false;

	// Data starts 2 rows after section marker (skip marker + "Stunde" summary row)
	int personDataStart = personSection + 2;
	int equipDataStart = equipSection + 2;

	m_distributions.clear();

	for (int c = 1; c <= numProfiles; ++c) {
		QString profName = normalizeProfileName(header[c]);
		HourlyDistribution dist;
		dist.person.resize(24, 0.0);
		dist.equipment.resize(24, 0.0);

		for (int h = 0; h < 24; ++h) {
			int pr = personDataStart + h;
			if (pr < rows.size() && c < rows[pr].size()) {
				bool ok;
				double val = rows[pr][c].toDouble(&ok);
				if (ok)
					dist.person[(size_t)h] = val;
			}

			int er = equipDataStart + h;
			if (er < rows.size() && c < rows[er].size()) {
				bool ok;
				double val = rows[er][c].toDouble(&ok);
				if (ok)
					dist.equipment[(size_t)h] = val;
			}
		}

		m_distributions[profName] = dist;
	}

	return true;
}
