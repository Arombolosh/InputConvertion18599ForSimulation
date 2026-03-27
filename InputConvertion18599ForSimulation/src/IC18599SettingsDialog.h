#ifndef IC18599SettingsDialogH
#define IC18599SettingsDialogH

#include <QDialog>

class QComboBox;

class IC18599SettingsDialog : public QDialog {
	Q_OBJECT
public:
	explicit IC18599SettingsDialog(QWidget *parent = nullptr);

	/*! Returns the selected language ID ("en" or "de"). */
	QString selectedLangId() const;

private:
	QComboBox *m_langCombo = nullptr;
};

#endif // IC18599SettingsDialogH
