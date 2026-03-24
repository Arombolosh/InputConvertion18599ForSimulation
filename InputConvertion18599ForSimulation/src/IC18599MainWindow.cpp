#include "IC18599MainWindow.h"
#include "IC18599NormDataWidget.h"
#include "IC18599ScheduleEditWidget.h"

#include <QAction>
#include <QCloseEvent>
#include <QComboBox>
#include <QLabel>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QStatusBar>
#include <QTabWidget>
#include <QToolBar>

IC18599MainWindow::IC18599MainWindow(QWidget *parent) :
	QMainWindow(parent)
{
	setWindowTitle(tr("DIN 18599 - Konvertierung für dynamische Simulation"));
	resize(1200, 800);
	setupUI();
}


void IC18599MainWindow::setupUI() {
	// --- Toolbar with profile ComboBox ---
	m_toolBar = addToolBar(tr("Profil"));
	m_toolBar->setMovable(false);
	m_toolBar->addWidget(new QLabel(tr("Profil: "), this));
	m_profileComboBox = new QComboBox(this);
	m_profileComboBox->setMinimumWidth(250);
	m_profileComboBox->setEnabled(false);
	m_toolBar->addWidget(m_profileComboBox);
	connect(m_profileComboBox, &QComboBox::currentTextChanged,
			this, &IC18599MainWindow::onProfileChanged);

	// --- Tab widget as central widget ---
	m_tabWidget = new QTabWidget(this);
	setCentralWidget(m_tabWidget);

	// Tab 1: Norm data
	m_normDataWidget = new IC18599NormDataWidget(this);
	m_tabWidget->addTab(m_normDataWidget, tr("Eingangsdaten Norm"));

	// Tab 2: Personen-Zeitplan
	m_personScheduleWidget = new IC18599ScheduleEditWidget(
		tr("Personen - Anwesenheitszeitplan"), QColor(70, 130, 180), this);
	m_tabWidget->addTab(m_personScheduleWidget, tr("Zeitplan Personen"));
	connect(m_personScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);

	// Tab 3: Geräte-Zeitplan
	m_equipmentScheduleWidget = new IC18599ScheduleEditWidget(
		tr("Geräte - Auslastungszeitplan"), QColor(180, 120, 60), this);
	m_tabWidget->addTab(m_equipmentScheduleWidget, tr("Zeitplan Geräte"));
	connect(m_equipmentScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);

	// Tab 4: Beleuchtung-Zeitplan
	m_lightingScheduleWidget = new IC18599ScheduleEditWidget(
		tr("Beleuchtung - Verfügbarkeitszeitplan"), QColor(200, 180, 50), this);
	m_tabWidget->addTab(m_lightingScheduleWidget, tr("Zeitplan Beleuchtung"));
	connect(m_lightingScheduleWidget, &IC18599ScheduleEditWidget::valuesChanged,
			this, &IC18599MainWindow::onScheduleModified);

	// Setup result charts
	m_personScheduleWidget->setResultChartCount(2);
	m_equipmentScheduleWidget->setResultChartCount(1);
	m_lightingScheduleWidget->setResultChartCount(1);

	// Tab: Log
	m_logWidget = new QPlainTextEdit(this);
	m_logWidget->setReadOnly(true);
	m_logWidget->setFont(QFont("Monospace", 10));
	m_tabWidget->addTab(m_logWidget, tr("Log"));

	// --- Menu ---
	QMenu *fileMenu = menuBar()->addMenu(tr("&Datei"));

	QAction *newAct = new QAction(tr("&Neues Projekt"), this);
	newAct->setShortcut(QKeySequence::New);
	connect(newAct, &QAction::triggered, this, &IC18599MainWindow::onNewProject);
	fileMenu->addAction(newAct);

	QAction *openAct = new QAction(tr("Projekt &öffnen..."), this);
	openAct->setShortcut(QKeySequence::Open);
	connect(openAct, &QAction::triggered, this, &IC18599MainWindow::onOpenProject);
	fileMenu->addAction(openAct);

	QAction *saveAct = new QAction(tr("Projekt &speichern"), this);
	saveAct->setShortcut(QKeySequence::Save);
	connect(saveAct, &QAction::triggered, this, &IC18599MainWindow::onSaveProject);
	fileMenu->addAction(saveAct);

	QAction *saveAsAct = new QAction(tr("Projekt speichern &unter..."), this);
	saveAsAct->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_S));
	connect(saveAsAct, &QAction::triggered, this, &IC18599MainWindow::onSaveProjectAs);
	fileMenu->addAction(saveAsAct);

	fileMenu->addSeparator();

	QAction *loadAct = new QAction(tr("Normdaten &laden (CSV)..."), this);
	connect(loadAct, &QAction::triggered, this, &IC18599MainWindow::onLoadCSV);
	fileMenu->addAction(loadAct);

	fileMenu->addSeparator();

	QAction *quitAct = new QAction(tr("&Beenden"), this);
	quitAct->setShortcut(QKeySequence::Quit);
	connect(quitAct, &QAction::triggered, this, &IC18599MainWindow::onQuit);
	fileMenu->addAction(quitAct);

	// Statusbar
	statusBar()->showMessage(tr("Bereit. Bitte CSV-Datei mit Normdaten laden."));

	m_logWidget->appendPlainText(tr("Programm gestartet."));
}


void IC18599MainWindow::onNewProject() {
	if (!maybeSave())
		return;

	m_project = IC18599Project();
	m_projectFilePath.clear();
	m_profileComboBox->clear();
	m_profileComboBox->setEnabled(false);
	m_personScheduleWidget->setGroups({});
	m_equipmentScheduleWidget->setGroups({});
	m_lightingScheduleWidget->setGroups({});
	updateWindowTitle();
	m_logWidget->appendPlainText(tr("Neues Projekt erstellt."));
	statusBar()->showMessage(tr("Neues Projekt. Bitte CSV-Datei laden."));
}


void IC18599MainWindow::onOpenProject() {
	if (!maybeSave())
		return;

	QString fname = QFileDialog::getOpenFileName(this,
		tr("Projekt öffnen"),
		QString(),
		tr("IC18599 Projektdateien (*.ic18599);;Alle Dateien (*)"));
	if (fname.isEmpty())
		return;

	IC18599Project proj;
	if (!proj.load(fname)) {
		m_logWidget->appendPlainText(tr("Fehler beim Laden: %1").arg(fname));
		QMessageBox::warning(this, tr("Fehler"), tr("Projektdatei konnte nicht geladen werden."));
		return;
	}

	m_project = proj;
	m_projectFilePath = fname;

	// Resolve CSV path relative to project file
	QFileInfo projInfo(fname);
	QString csvPath = projInfo.dir().filePath(m_project.m_csvFilePath);

	if (!m_normDataWidget->loadCSV(csvPath)) {
		m_logWidget->appendPlainText(tr("Warnung: CSV konnte nicht geladen werden: %1").arg(csvPath));
	}
	else {
		m_project.m_profileNames = m_normDataWidget->profileNames();
		m_logWidget->appendPlainText(tr("Normdaten geladen: %1").arg(csvPath));
	}

	populateProfileComboBox();

	// Restore last selected profile
	int idx = m_profileComboBox->findText(m_project.m_currentProfile);
	if (idx >= 0)
		m_profileComboBox->setCurrentIndex(idx);

	updateWindowTitle();
	m_logWidget->appendPlainText(tr("Projekt geladen: %1").arg(fname));
	statusBar()->showMessage(tr("Projekt: %1").arg(fname));
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
		m_logWidget->appendPlainText(tr("Projekt gespeichert: %1").arg(m_projectFilePath));
		statusBar()->showMessage(tr("Gespeichert: %1").arg(m_projectFilePath));
	}
	else {
		m_logWidget->appendPlainText(tr("Fehler beim Speichern: %1").arg(m_projectFilePath));
		QMessageBox::warning(this, tr("Fehler"), tr("Projekt konnte nicht gespeichert werden."));
	}
}


void IC18599MainWindow::onSaveProjectAs() {
	QString fname = QFileDialog::getSaveFileName(this,
		tr("Projekt speichern unter"),
		QString(),
		tr("IC18599 Projektdateien (*.ic18599);;Alle Dateien (*)"));
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


void IC18599MainWindow::onLoadCSV() {
	QString fname = QFileDialog::getOpenFileName(this,
		tr("DIN 18599 Normdaten laden"),
		QString(),
		tr("CSV-Dateien (*.csv);;Alle Dateien (*)"));
	if (fname.isEmpty())
		return;

	bool ok = m_normDataWidget->loadCSV(fname);
	if (ok) {
		m_project.m_csvFilePath = fname;  // store absolute, relativize on save
		m_project.m_profileNames = m_normDataWidget->profileNames();
		populateProfileComboBox();
		m_logWidget->appendPlainText(tr("Normdaten geladen: %1").arg(fname));
		statusBar()->showMessage(tr("Datei: %1").arg(fname));
		m_tabWidget->setCurrentIndex(0);
	}
	else {
		m_logWidget->appendPlainText(tr("Fehler beim Laden: %1").arg(fname));
		statusBar()->showMessage(tr("Fehler beim Laden."));
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

	statusBar()->showMessage(tr("Profil: %1").arg(profileName));
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
	data.m_personGroups = m_personScheduleWidget->groups();
	data.m_equipmentGroups = m_equipmentScheduleWidget->groups();
	data.m_lightingGroups = m_lightingScheduleWidget->groups();
}


void IC18599MainWindow::loadProfileDataToWidgets(const QString &profileName) {
	ProfileScheduleData &data = m_project.getOrCreateProfile(profileName);
	m_personScheduleWidget->setGroups(data.m_personGroups);
	m_equipmentScheduleWidget->setGroups(data.m_equipmentGroups);
	m_lightingScheduleWidget->setGroups(data.m_lightingGroups);
}


void IC18599MainWindow::updateWindowTitle() {
	QString title = tr("DIN 18599 - Konvertierung für dynamische Simulation");
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
		tr("Ungespeicherte Änderungen"),
		tr("Das Projekt wurde geändert. Möchten Sie speichern?"),
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
	double fullUseHoursPerson = m_normDataWidget->getProfileValue(prof, "Vollnutzungstunden Personen");
	double activity = m_normDataWidget->getProfileValue(prof, QString::fromUtf8("Aktivität Personen (sensible)"));

	std::vector<double> personWeek = m_personScheduleWidget->weekValues(); // 0-100
	double sumPerson = 0;
	for (double v : personWeek) sumPerson += v / 100.0;
	double normFactorPerson = (sumPerson > 0) ? fullUseHoursPerson / sumPerson : 0;

	std::vector<double> occupancyWeek(168), activityWeek(168);
	for (int i = 0; i < 168; ++i) {
		occupancyWeek[i] = (personWeek[i] / 100.0) * normFactorPerson;
		activityWeek[i] = activity;
	}
	m_personScheduleWidget->setResultData(0, occupancyWeek, "[-]", QColor(80, 150, 80));
	m_personScheduleWidget->setResultData(1, activityWeek, "[W/Pers]", QColor(200, 80, 80));

	// --- Equipment ---
	double fullUseHoursEquip = m_normDataWidget->getProfileValue(prof, QString::fromUtf8("Vollnutzungstunden Geräte"));

	std::vector<double> equipWeek = m_equipmentScheduleWidget->weekValues();
	double sumEquip = 0;
	for (double v : equipWeek) sumEquip += v / 100.0;
	double normFactorEquip = (sumEquip > 0) ? fullUseHoursEquip / sumEquip : 0;

	std::vector<double> equipOccupancy(168);
	for (int i = 0; i < 168; ++i)
		equipOccupancy[i] = (equipWeek[i] / 100.0) * normFactorEquip;
	m_equipmentScheduleWidget->setResultData(0, equipOccupancy, "[-]", QColor(180, 140, 80));

	// --- Lighting ---
	std::vector<double> lightWeek = m_lightingScheduleWidget->weekValues();
	std::vector<double> lightFraction(168);
	for (int i = 0; i < 168; ++i)
		lightFraction[i] = lightWeek[i] / 100.0;
	m_lightingScheduleWidget->setResultData(0, lightFraction, "[-]", QColor(200, 180, 50));
}
