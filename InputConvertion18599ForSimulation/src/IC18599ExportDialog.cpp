#include "IC18599ExportDialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

IC18599ExportDialog::IC18599ExportDialog(const QStringList &profileNames, int nonResidentialStartIndex,
										 QWidget *parent) :
	QDialog(parent)
{
	setWindowTitle(tr("Export PDF Report"));
	setMinimumWidth(400);

	auto *mainLayout = new QVBoxLayout(this);

	mainLayout->addWidget(new QLabel(tr("Select profiles to include:")));

	// Scroll area with checkboxes
	auto *scrollArea = new QScrollArea;
	scrollArea->setWidgetResizable(true);
	auto *scrollWidget = new QWidget;
	auto *scrollLayout = new QVBoxLayout(scrollWidget);

	bool hasSections = (nonResidentialStartIndex > 0 && nonResidentialStartIndex < profileNames.size());
	if (hasSections) {
		auto *resLabel = new QLabel(QString("<b>%1</b>").arg(tr("Residential Buildings")));
		scrollLayout->addWidget(resLabel);
	}

	for (int i = 0; i < profileNames.size(); ++i) {
		if (hasSections && i == nonResidentialStartIndex) {
			auto *nonResLabel = new QLabel(QString("<b>%1</b>").arg(tr("Non-Residential Buildings")));
			nonResLabel->setContentsMargins(0, 8, 0, 0);
			scrollLayout->addWidget(nonResLabel);
		}
		auto *cb = new QCheckBox(profileNames[i]);
		cb->setChecked(true);
		scrollLayout->addWidget(cb);
		m_checkBoxes.push_back(cb);
	}
	scrollLayout->addStretch();
	scrollArea->setWidget(scrollWidget);
	mainLayout->addWidget(scrollArea);

	// Check All / Uncheck All buttons
	auto *checkLayout = new QHBoxLayout;
	auto *checkAllBtn = new QPushButton(tr("Check All"));
	auto *uncheckAllBtn = new QPushButton(tr("Uncheck All"));
	checkLayout->addWidget(checkAllBtn);
	checkLayout->addWidget(uncheckAllBtn);
	checkLayout->addStretch();
	mainLayout->addLayout(checkLayout);

	connect(checkAllBtn, &QPushButton::clicked, this, &IC18599ExportDialog::onCheckAll);
	connect(uncheckAllBtn, &QPushButton::clicked, this, &IC18599ExportDialog::onUncheckAll);

	// Dialog buttons
	auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
	auto *exportBtn = buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
	exportBtn->setDefault(true);
	connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
	mainLayout->addWidget(buttonBox);
}


QStringList IC18599ExportDialog::selectedProfiles() const {
	QStringList result;
	for (QCheckBox *cb : m_checkBoxes) {
		if (cb->isChecked())
			result.append(cb->text());
	}
	return result;
}


void IC18599ExportDialog::onCheckAll() {
	for (QCheckBox *cb : m_checkBoxes)
		cb->setChecked(true);
}


void IC18599ExportDialog::onUncheckAll() {
	for (QCheckBox *cb : m_checkBoxes)
		cb->setChecked(false);
}
