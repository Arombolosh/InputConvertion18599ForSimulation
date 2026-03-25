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

#include "QtExt_AutoUpdateDialog.h"
#include "ui_QtExt_AutoUpdateDialog.h"

#include <QPushButton>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QScreen>

#include <IBK_messages.h>
#include <IBK_FormatString.h>
#include <IBK_crypt.h>
#include <QFileDialog>

#include "QtExt_LanguageHandler.h"

namespace QtExt {

AutoUpdateDialog::AutoUpdateDialog(QWidget *parent) :
	QDialog(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
	m_updateRejected(false),
	m_ui(new Ui::AutoUpdateDialog),
	m_networkManager(new QNetworkAccessManager(this))
{
	m_ui->setupUi(this);

#if defined(Q_OS_MAC) // Q_OS_UNIX

const char * const FIXED_FONT_FAMILY = "Monaco";

#elif defined(Q_OS_UNIX)

const char * const FIXED_FONT_FAMILY = "Monospace";

#else

const char * const FIXED_FONT_FAMILY = "Consolas";

#endif

	// create push-button for downloading the update
	m_ui->textBrowserChangeLog->setFontFamily( FIXED_FONT_FAMILY );
	m_ui->textBrowserChangeLog->setWordWrapMode(QTextOption::NoWrap);
	m_ui->progressBarDownload->setVisible(false);

	QScreen *screen = QGuiApplication::primaryScreen();
	QRect  screenGeometry = screen->geometry();
	int height = screenGeometry.height();
	int width = screenGeometry.width();
	resize(QSize(width*0.6, height*0.8));

	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(onDownloadFinished(QNetworkReply*)));
}


AutoUpdateDialog::~AutoUpdateDialog() {
	delete m_ui;
}


int AutoUpdateDialog::run(const QString & currentVersion, const QString & newVersion, const QString & changeLogText,
						  const QMap<QString, QString> & changeLogLangs,
						  const QString & downloadUrl)
{
	m_ui->labelCurrentVersionNumber->setText(currentVersion);
	m_ui->labelNewVersionNumber->setText(newVersion);

	m_updateFileMD5.clear();
	m_updateFilePath.clear();

	QString changes;
	QString langId = QtExt::LanguageHandler::langId();
	if (changeLogLangs.contains(langId))
		changes = changeLogLangs[langId];
	else {
		// fall-back to english if language is not present
		if (langId != "en" && changeLogLangs.contains("en"))
			changes = changeLogLangs["en"];
	}

	// fall-back to old functionality
	if (changes.isEmpty())  {
		m_ui->textBrowserChangeLog->setPlainText(changeLogText);
		m_ui->textBrowserChangeLog->setWordWrapMode(QTextOption::NoWrap);
	}
	else {
		m_ui->textBrowserChangeLog->setHtml(changes);
		m_ui->textBrowserChangeLog->setWordWrapMode(QTextOption::WordWrap);
	}
	m_downloadUrl = downloadUrl;
	return exec();
}


void AutoUpdateDialog::on_pushButtonDownload_clicked() {

	// select a new folder for downloaded update file
	QString dir = QFileDialog::getExistingDirectory(
				this,
				tr("Select folder"),
				m_updateFilePath,
				QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
				);

	// aborted by user
	if (dir.isEmpty())
		return;

	m_updateFilePath = dir; // for now it is only the path, we apppend the actual file name in onDownloadFinished()

	IBK::IBK_Message(IBK::FormatString("Downloading update installer to '%1'.").arg(m_updateFilePath.toStdString()),
					 IBK::MSG_PROGRESS, "[AutoUpdateDialog::on_pushButtonDownload_clicked]", IBK::VL_STANDARD);
	QUrl addy(m_downloadUrl);
	QNetworkReply * reply = m_networkManager->get( QNetworkRequest(addy) );
	connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
	m_ui->pushButtonDownload->setEnabled(false);
	m_ui->progressBarDownload->setVisible(true);
	m_ui->progressBarDownload->setValue(0);
}


void AutoUpdateDialog::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
	if (bytesTotal == -1) {
		m_ui->progressBarDownload->setRange(0,0);
		m_ui->progressBarDownload->setValue(0);
	}
	else {
		m_ui->progressBarDownload->setMaximum(bytesTotal);
		m_ui->progressBarDownload->setValue(bytesReceived);
	}
}


void AutoUpdateDialog::onDownloadFinished(QNetworkReply* reply) {
	m_ui->progressBarDownload->setVisible(false);
	m_ui->pushButtonDownload->setEnabled(true);

	if (reply->error() != QNetworkReply::NoError ) {
		QMessageBox::critical(this, tr("Download error"), tr("Couldn't download the update."));
	}
	else {
		// store downloaded file in selected location
		QString targetFile = m_updateFilePath + "/" + QFileInfo(m_downloadUrl).fileName();
		targetFile = QDir::toNativeSeparators(targetFile);
		QDir baseDir(m_updateFilePath);
		if (!baseDir.exists())
			baseDir.mkpath(m_updateFilePath);
		m_updateFilePath = targetFile; // now updateFilePath holds full path to downloaded file
		QFile f(targetFile);
		bool success = f.open(QIODevice::WriteOnly);
		if (success) {
			// this might be several Megabytes (..100Mb), but that isn't an issue with todays memory availability, right?
			QByteArray downloadedData = reply->readAll();
			f.write(downloadedData);
			// compute MD5 hash
			std::vector<unsigned char> data(downloadedData.data(), downloadedData.data() + downloadedData.size());
			m_updateFileMD5 = IBK::md5_str(data);
			f.close();
		}

		// now check if the file was actually written or if it was moved by
		// virus scan software etc.
		if (!success || !QFileInfo::exists(targetFile) || QFileInfo(targetFile).size() == 0) {
			QMessageBox::information(this, tr("Download complete"), tr("The update installer could not be stored "
																	   "on the machine, possibly through access restrictions. "
																	   "Please download and install the update manually."));
		}
		else {

#if defined(Q_OS_WIN)

			QMessageBox msgBox(QMessageBox::Information, tr("Download complete"),
							   tr("The software updater has downloaded the update installer (to '%1').\n"
								   "You must restart the program to apply the update!").arg(targetFile), QMessageBox::Ok | QMessageBox::Close, this);
			msgBox.button(QMessageBox::Ok)->setText(tr("Close the software and install update"));

			int res = msgBox.exec();

			if (res == QMessageBox::Ok) {
				qApp->closeAllWindows();
			}

#elif defined(Q_OS_MAC)
			QMessageBox::information(this, tr("Download complete"), tr("The software updater has downloaded the new software package in %1.\n"
																	   "The disk image will now be mounted (you may need to enable third party "
																	   "app downloads in the security settings).")
									 .arg(targetFile) );
			// open DMG
			QString openCommand("open");
			QStringList args  = QStringList() << targetFile;
			QProcess::execute(openCommand, args);
#elif defined(Q_OS_UNIX)
			QMessageBox::information(this, tr("Download complete"), tr("The software updater has downloaded the new software package in %1.")
									 .arg(targetFile) );
#endif
		}
	}

	reply->deleteLater();
}


void AutoUpdateDialog::on_pushButtonSkipUpdate_clicked() {
	int res = QMessageBox::question(this, tr("Skip update"),
									tr("Would you like to skip this update and and not be reminded again for this version?"));
	if (res == QMessageBox::Yes) {
		m_updateRejected = true;
		reject();
	}
}


} // namespace QtExt


