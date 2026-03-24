#ifndef IC18599MainWindowH
#define IC18599MainWindowH

#include <QMainWindow>

#include "IC18599Project.h"

class QComboBox;
class QTabWidget;
class QPlainTextEdit;
class QToolBar;
class IC18599NormDataWidget;
class IC18599ScheduleEditWidget;

class IC18599MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit IC18599MainWindow(QWidget *parent = nullptr);

protected:
	void closeEvent(QCloseEvent *event) override;

private slots:
	void onNewProject();
	void onOpenProject();
	void onSaveProject();
	void onSaveProjectAs();
	void onLoadCSV();
	void onProfileChanged(const QString &profileName);
	void onScheduleModified();
	void onQuit();

private:
	void setupUI();
	void populateProfileComboBox();
	void storeCurrentProfileData();
	void loadProfileDataToWidgets(const QString &profileName);
	void updateWindowTitle();
	void recalculateResults();
	bool maybeSave();

	IC18599Project				m_project;
	QString						m_projectFilePath;

	QToolBar					*m_toolBar = nullptr;
	QComboBox					*m_profileComboBox = nullptr;
	QTabWidget					*m_tabWidget = nullptr;
	IC18599NormDataWidget		*m_normDataWidget = nullptr;
	IC18599ScheduleEditWidget	*m_personScheduleWidget = nullptr;
	IC18599ScheduleEditWidget	*m_equipmentScheduleWidget = nullptr;
	IC18599ScheduleEditWidget	*m_lightingScheduleWidget = nullptr;
	QPlainTextEdit				*m_logWidget = nullptr;
};

#endif // IC18599MainWindowH
