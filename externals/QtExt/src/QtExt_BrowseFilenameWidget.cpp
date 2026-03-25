/*	QtExt - Qt-based utility classes and functions (extends Qt library)

	Copyright (c) 2014-today, Institut für Bauklimatik, TU Dresden, Germany

	Primary authors:
	  Heiko Fechner    <heiko.fechner -[at]- tu-dresden.de>
	  Andreas Nicolai

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.

	Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
	der GNU General Public License, wie von der Free Software Foundation,
	Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
	veröffentlichten Version, weiter verteilen und/oder modifizieren.

	Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
	OHNE JEDE GEWÄHR,; sogar ohne die implizite
	Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#include "QtExt_BrowseFilenameWidget.h"
#include "ui_QtExt_BrowseFilenameWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QRadioButton>
#include <QToolButton>

#include <IBK_Exception.h>
#include <IBK_Path.h>

namespace QtExt {

BrowseFilenameWidget::BrowseFilenameWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::BrowseFilenameWidget),
	m_filenameMode(true),
	m_fileMustExist(true)
{
	m_ui->setupUi(this);

	connect(m_ui->toolButtonSetFile, &QToolButton::clicked, this, &QtExt::BrowseFilenameWidget::onToolBtnClicked);
	connect(m_ui->checkBoxRelative, &QCheckBox::toggled, this, &QtExt::BrowseFilenameWidget::onAbsToggled);

	// Hides rel/abs switching
	updateUi();
}


void BrowseFilenameWidget::setup(const QString &filename, bool filenameMode, bool fileMustExist, const QString & filter,
								 bool dontUseNativeFilenameDialog, QString labelText, bool relAbsSwitching, const std::map<std::string, IBK::Path> *placeholders) {
	m_filenameMode = filenameMode;
	m_fileMustExist = fileMustExist;
	m_filter = filter;
	m_ui->lineEditFilename->setText(filename);
	m_dontUseNativeFilenameDialog = dontUseNativeFilenameDialog;
	m_ui->widgetLabel->setVisible(!labelText.isEmpty());
	m_ui->label->setText(labelText);

	m_relAbsSwitchingEnabled = relAbsSwitching;
	if (placeholders != nullptr)
		m_placeholders = *placeholders;

	setMinimumHeight(sizeHint().height());

	updateUi();
}


void BrowseFilenameWidget::setReadOnly(bool readOnly) {
	m_ui->toolButtonSetFile->setEnabled(!readOnly);
	m_ui->lineEditFilename->setReadOnly(readOnly);
	m_ui->checkBoxRelative->setEnabled(!readOnly);
}


void BrowseFilenameWidget::setFilename(const QString & filename) {
	// replace potentially existing relative path paths
	// but that does not mean we always want an absolute path!!!
	m_filename = filename;
	m_absMode = !IBK::Path(m_filename.toStdString()).hasPlaceholder();
	updateUi();
}


QString BrowseFilenameWidget::filename(bool abs) const {
	IBK::Path file(m_filename.toStdString());
	if (abs) {
		file = file.withReplacedPlaceholders(m_placeholders);
		file = file.absolutePath();
	}
	QString f = QString::fromStdString(file.str());
	return f;
}

QLineEdit *BrowseFilenameWidget::lineEdit() {
	return m_ui->lineEditFilename;
}


QToolButton *BrowseFilenameWidget::toolBtn() {
	return m_ui->toolButtonSetFile;
}


void BrowseFilenameWidget::onAbsToggled(bool rel) {
	m_absMode = !rel;
	updateUi();
	emit editingFinished();
}


void BrowseFilenameWidget::updateUi() {

	// check if we can create a relative path
	IBK::Path file(m_filename.toStdString());
	std::string errStr;
	bool canCreateRelPath = false;
	if (m_placeholders.count("Project Directory") > 0)
	{
		if (file.hasPlaceholder())
			file = file.withReplacedPlaceholders(m_placeholders);

		canCreateRelPath = file.canCreateRelativePath(m_placeholders.at("Project Directory"), errStr);
	}

	// only enable checkbox in this case
	m_ui->widgetRelative->setVisible(m_relAbsSwitchingEnabled);
	m_ui->checkBoxRelative->setEnabled(canCreateRelPath);

	// if empty, clear and return here
	if (m_filename.isEmpty()) {
		m_ui->lineEditFilename->blockSignals(true);
		m_ui->lineEditFilename->setText("");
		m_ui->lineEditFilename->blockSignals(false);
		return;
	}

	try {
		// Only do the complex testing, if we are in rel/abs mode
		if (m_relAbsSwitchingEnabled) {
			if (m_placeholders.empty() ||
					m_placeholders.find("Project Directory") == m_placeholders.end()) {
				m_ui->checkBoxRelative->setEnabled(false);
				qCritical() << "Cannot enable relative/absolute path switching.";
			}
			else if (m_absMode) { // Absolute mode
				if (file.hasPlaceholder())
					file = file.withReplacedPlaceholders(m_placeholders);
				m_filename = QString::fromStdString(file.absolutePath().str());
			}
			else if (canCreateRelPath) { // Relative Mode
				if (file.hasPlaceholder())
					file = file.withReplacedPlaceholders(m_placeholders);

				file = file.relativePath(m_placeholders.at("Project Directory"));
				m_filename = QString::fromStdString("${Project Directory}/" + file.str());
			}
		}
	} catch (IBK::Exception &ex) {
		QMessageBox::critical(this, tr("Error with relative path"),
							  tr("Could not switch to relative path and taking absolute path\n(%1)").arg(ex.what()));
		m_absMode = true;
	}

	m_ui->checkBoxRelative->blockSignals(true);
	m_ui->checkBoxRelative->setChecked(!m_absMode);
	m_ui->checkBoxRelative->blockSignals(false);

	m_ui->lineEditFilename->blockSignals(true);
	m_ui->lineEditFilename->setText(m_filename);
	m_ui->lineEditFilename->blockSignals(false);

	sizeHint();
}

QSize BrowseFilenameWidget::sizeHint() const {
	QSize s = QWidget::sizeHint();
	if (m_relAbsSwitchingEnabled)
		s.setHeight(50); // ToDo Stephan: Check if this is a good workaround!
	return s;
}


void BrowseFilenameWidget::onPlaceHoldersUpdated(const std::map< std::string, IBK::Path > &placeholders) {
	m_placeholders = placeholders;
	updateUi();
}


void BrowseFilenameWidget::onToolBtnClicked() {
	QString newFileName;
	blockSignals(true);

	// get current filename and remove possible "?x" part
	QString currentFileName = filename();
	int pos = currentFileName.lastIndexOf('?');
	if (pos != -1) {
		currentFileName = currentFileName.left(pos);
	}

	if (m_filenameMode) {
		if (m_fileMustExist) {
			newFileName = QFileDialog::getOpenFileName(nullptr, tr("Select filename"), currentFileName, m_filter, nullptr,
											  m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
																			  );
		}
		else {
			newFileName = QFileDialog::getSaveFileName(this, tr("Select filename"), currentFileName, m_filter, nullptr,
											  m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
																			  );
		}
	}
	else {
		if (m_fileMustExist) {
			newFileName = QFileDialog::getExistingDirectory(this, tr("Select filename"), currentFileName,
												   m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
																				   );
		}
		else {
			newFileName = QFileDialog::getSaveFileName(this, tr("Select directory"), currentFileName, QString(), nullptr,
											  m_dontUseNativeFilenameDialog ? QFileDialog::DontUseNativeDialog : QFileDialog::Options()
																			  );
		}
	}
	blockSignals(false);
	if (!newFileName.isEmpty()) {
		m_filename = newFileName;
		updateUi();
		emit editingFinished();
	}
}

} // namespace QtExt
