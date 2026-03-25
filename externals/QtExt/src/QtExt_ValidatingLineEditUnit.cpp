#include "QtExt_ValidatingLineEditUnit.h"
#include "ui_QtExt_ValidatingLineEditUnit.h"

#include <IBK_UnitList.h>

namespace QtExt {

ValidatingLineEditUnit::ValidatingLineEditUnit(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ValidatingLineEditUnit)
{
	ui->setupUi(this);
	connect(ui->lineEdit, SIGNAL(editingFinishedSuccessfully()), this, SIGNAL(editingFinishedSuccessfully()));
	ui->labelUnit->setText(tr("unknown"));
}

ValidatingLineEditUnit::~ValidatingLineEditUnit() {
	emit unregister(this);
	delete ui;
}

QLineEdit *ValidatingLineEditUnit::edit() {
	return ui->lineEdit;
}

void ValidatingLineEditUnit::setup(const IBK::Unit &unit, double minVal, double maxVal, const QString &toolTip,
								   bool minValueAllowed, bool maxValueAllowed)
{
	changeUnit(unit);
	ValidatingInputBase::setup(minVal, maxVal, toolTip, minValueAllowed, maxValueAllowed);
}

bool ValidatingLineEditUnit::isValid() const {
	return ui->lineEdit->isValid();
}

void ValidatingLineEditUnit::setValidator(ValidatorBase* validator) {
	ui->lineEdit->setValidator(validator);
}

bool ValidatingLineEditUnit::isValidNumber(double & val) const {
	return ui->lineEdit->isValidNumber(val);
}

void ValidatingLineEditUnit::setEnabled(bool enabled) {
	ui->lineEdit->setEnabled(enabled);
	ui->labelUnit->setEnabled(enabled);
}

void ValidatingLineEditUnit::setReadOnly(bool readOnly) {
	ui->lineEdit->setReadOnly(readOnly);
}


void ValidatingLineEditUnit::setEmptyAllowed(bool allowEmpty, const QString & placeholderText) {
	ui->lineEdit->setEmptyAllowed(allowEmpty, placeholderText);
}

void ValidatingLineEditUnit::setFromParameter(const IBK::Parameter &p) {
	ui->lineEdit->setFromParameter(p, m_unit);
}

void ValidatingLineEditUnit::setFromParameterOrClear(const IBK::Parameter &p) {
	ui->lineEdit->setFromParameterOrClear(p, m_unit);
}

void ValidatingLineEditUnit::setFromParameterOrDefault(const IBK::Parameter &p, const IBK::Parameter &defaultPara) {
	if (p.empty())
		setFromParameter(defaultPara);
	else {
		Q_ASSERT(p.IO_unit.base_id() == defaultPara.IO_unit.base_id()); // if p is given, we expect a matching unit
		setFromParameter(p);
	}
}

void ValidatingLineEditUnit::setValue(double value) {
	ui->lineEdit->setValue(value);
}

double ValidatingLineEditUnit::value(const IBK::Unit& unit) const {
	if(unit == IBK::Unit())
		return ui->lineEdit->value();

	if(unit.base_id() != m_unit.base_id())
		throw IBK::Exception(
				IBK::FormatString("Mismatching units, cannot relate unit if parameter '%1' to requested target unit '%2'.")
				.arg(unit.name()).arg(m_unit.name()), "[ValidatingLineEdit::setFromParameter]");

	double value = ui->lineEdit->value();
	IBK::UnitList::instance().convert(m_unit, unit, value);
	return value;
}

IBK::Parameter ValidatingLineEditUnit::toParameter(const std::string &name) {
	if (ui->lineEdit->text().trimmed().isEmpty() && !ui->lineEdit->placeholderText().isEmpty()) {
		return IBK::Parameter();
	}
	double val;
	if (!isValidNumber(val))
		return IBK::Parameter();

	return IBK::Parameter(name, val, m_unit);
}

void ValidatingLineEditUnit::setText(const QString& str) {
	ui->lineEdit->setText(str);
}

void ValidatingLineEditUnit::check() {
	QString t = ui->lineEdit->text();
	setText(t);
}

void ValidatingLineEditUnit::changeUnit(const IBK::Unit &unit) {
	if(m_unit != IBK::Unit()) {
		if(unit.base_id() != m_unit.base_id())
			throw IBK::Exception(
					IBK::FormatString("Mismatching units, cannot relate unit if parameter '%1' to requested target unit '%2'.")
					.arg(unit.name()).arg(m_unit.name()), "[ValidatingLineEdit::setFromParameter]");
		if(ui->lineEdit->text() != ui->lineEdit->placeholderText()) {
			double value = ui->lineEdit->value();
			IBK::UnitList::instance().convert(m_unit, unit, value);
			setValue(value);
		}
	}
	m_unit = unit;
	ui->labelUnit->setText(QString::fromStdString(m_unit.name()));
}

} // namespace QtExt
