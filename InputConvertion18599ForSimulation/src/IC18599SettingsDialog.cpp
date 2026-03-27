#include "IC18599SettingsDialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

#include <QtExt_LanguageHandler.h>

IC18599SettingsDialog::IC18599SettingsDialog(QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Settings"));
	setMinimumWidth(300);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);

	QFormLayout *formLayout = new QFormLayout;
	m_langCombo = new QComboBox(this);
	m_langCombo->addItem("English", "en");
	m_langCombo->addItem("Deutsch", "de");

	// Pre-select current language
	QString currentLang = QtExt::LanguageHandler::langId();
	int idx = m_langCombo->findData(currentLang);
	if (idx >= 0)
		m_langCombo->setCurrentIndex(idx);

	formLayout->addRow(tr("Language:"), m_langCombo);
	mainLayout->addLayout(formLayout);

	mainLayout->addWidget(new QLabel(tr("Changes take effect after restart."), this));

	QDialogButtonBox *buttons = new QDialogButtonBox(
		QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
	mainLayout->addWidget(buttons);
}


QString IC18599SettingsDialog::selectedLangId() const {
	return m_langCombo->currentData().toString();
}
