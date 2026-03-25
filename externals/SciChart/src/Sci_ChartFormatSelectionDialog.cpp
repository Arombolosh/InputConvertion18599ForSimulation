#include "Sci_ChartFormatSelectionDialog.h"
#include "ui_Sci_ChartFormatSelectionDialog.h"

namespace SCI {

ChartFormatSelectionDialog::ChartFormatSelectionDialog(QWidget *parent) :
	QDialog(parent),
	m_ui(new Ui::ChartFormatSelectionDialog)
{
	m_ui->setupUi(this);
	m_ui->checkBoxAll->setChecked(true);

	connect(m_ui->checkBoxGeneralOptions, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled(bool)));
	connect(m_ui->checkBoxLegendOptions, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled(bool)));
	connect(m_ui->checkBoxAxisX, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled(bool)));
	connect(m_ui->checkBoxAxisYLeft, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled(bool)));
	connect(m_ui->checkBoxAxisYRight, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled(bool)));
	connect(m_ui->checkBoxLineFormats, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled(bool)));
	connect(m_ui->checkBoxColorMap, SIGNAL(toggled(bool)), this, SLOT(onCheckboxToggled(bool)));
}


ChartFormatSelectionDialog::~ChartFormatSelectionDialog() {
	delete m_ui;
}


void ChartFormatSelectionDialog::setLineFormatOptionEnabled(bool enabled) {
	m_ui->checkBoxLineFormats->blockSignals(true);
	m_ui->checkBoxColorMap->blockSignals(true);

	m_ui->checkBoxLineFormats->setEnabled(enabled);
	m_ui->checkBoxLineFormats->setChecked(enabled);
	m_ui->checkBoxColorMap->setEnabled(!enabled);
	m_ui->checkBoxColorMap->setChecked(!enabled);

	m_ui->checkBoxLineFormats->blockSignals(false);
	m_ui->checkBoxColorMap->blockSignals(false);
}


QSet<ChartFormatSelectionDialog::SettingsToTransfer> ChartFormatSelectionDialog::selectedOptions() const {
	QSet<SettingsToTransfer> s;
	if (m_ui->checkBoxAxisX->isChecked()) s.insert(OptionAxisX);
	if (m_ui->checkBoxAxisYLeft->isChecked()) s.insert(OptionAxisYLeft);
	if (m_ui->checkBoxAxisYRight->isChecked()) s.insert(OptionAxisYRight);
	if (m_ui->checkBoxLegendOptions->isChecked()) s.insert(OptionLegend);
	if (m_ui->checkBoxGeneralOptions->isChecked()) s.insert(OptionGeneral);
	if (m_ui->checkBoxLineFormats->isChecked()) s.insert(OptionLineProperties);
	if (m_ui->checkBoxColorMap->isChecked()) s.insert(OptionColorMapProperties);
	return s;
}


void ChartFormatSelectionDialog::on_checkBoxAll_toggled(bool checked) {
	m_ui->checkBoxAxisX->setChecked(checked);
	m_ui->checkBoxAxisYLeft->setChecked(checked);
	m_ui->checkBoxAxisYRight->setChecked(checked);
	m_ui->checkBoxLegendOptions->setChecked(checked);
	m_ui->checkBoxGeneralOptions->setChecked(checked);
	m_ui->checkBoxLineFormats->setChecked(m_ui->checkBoxLineFormats->isEnabled() && checked);
	m_ui->checkBoxColorMap->setChecked(m_ui->checkBoxColorMap->isEnabled() && checked);
}


void ChartFormatSelectionDialog::onCheckboxToggled(bool checked) {
	if (!checked) {
		m_ui->checkBoxAll->blockSignals(true);
		m_ui->checkBoxAll->setChecked(false);
		m_ui->checkBoxAll->blockSignals(false);
	}
}

} // namespace SCI

