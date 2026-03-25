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

#include "QtExt_AutoUpdater.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPlainTextEdit>
#include <QProcess>
#include <QScreen>
#include <QTextStream>
#include <QUrl>
#include <QVBoxLayout>
#include <QWindow>

#include <IBK_messages.h>
#include <IBK_Version.h>
#include <IBK_Exception.h>
#include <IBK_crypt.h>

#include "QtExt_global.h"
#include "QtExt_AutoUpdateDialog.h"
#include "QtExt_LanguageHandler.h"

#if defined(Q_OS_WIN)
#include "QtExt_Directories.h"
#endif

namespace QtExt {

AutoUpdater::AutoUpdater(QObject *parent) :
	QObject(parent),
	m_networkManager(new QNetworkAccessManager)
{
	connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
			this, SLOT(downloadFinished(QNetworkReply*)));
	connect(m_networkManager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
			this, SLOT(onSslErrors(QNetworkReply*,QList<QSslError>)));
}


// this is only done for Windows
#if defined(Q_OS_WIN)
bool AutoUpdater::installUpdateWhenAvailable(const QString & installerFilePath,
											 const std::string & md5hash) {
	// installerFilePath is the absolute file path to the downloaded installer file
	// for example: "C:\Users\xxx\AppData\Local\PostProcApp\updates\PostProc_2.2.2_win64_2020-05-27.exe"
	if (!QFileInfo::exists(installerFilePath)) {
		IBK::IBK_Message(IBK::FormatString("Update installer '%1' does not exist.").arg(installerFilePath.toStdString()), IBK::MSG_ERROR);
		return false;
	}

	// if we have a md5hash, check if contents match, otherwise ignore update
	if (!md5hash.empty()) {
		QFile f(installerFilePath);
		f.open(QIODevice::ReadOnly);
		// this might be several Megabytes (..100Mb), but that isn't an issue with todays memory availability, right?
		QByteArray downloadedData;
		downloadedData = f.readAll();
		// compute MD5 hash
		std::vector<unsigned char> data(downloadedData.data(), downloadedData.data() + downloadedData.size());
		std::string updateFileMD5 = IBK::md5_str(data);
		if (updateFileMD5 != md5hash) {
			IBK::IBK_Message("Mismatching md5 hash of installer file.", IBK::MSG_ERROR);
			return false;
		}
	}

	// automatic call of installer only on Windows OS
	bool res = QProcess::startDetached(installerFilePath, QStringList());
	if (!res) {
		IBK::IBK_Message("Error installing update.", IBK::MSG_ERROR);
		return false;
	}
	return true;
}
#endif


void AutoUpdater::checkForUpdateInfo(const QString & url, const char * const LONG_VERSION,
									 bool interactive, QString newestRejectedVersion)
{
	m_currentVersion = LONG_VERSION;
	m_interactive = interactive;
	m_newestRejectedVersion = newestRejectedVersion;

	QUrl addy(url);
	QString fullPath = addy.toString();
	int pos = fullPath.lastIndexOf('/');
	m_updateInfoDownloadPath = fullPath.left(pos);

	QNetworkRequest request(addy);
	m_networkManager->get(request);
}


void AutoUpdater::downloadFinished(QNetworkReply *reply) {
	FUNCID(AutoUpdater::downloadFinished);
	QWidget * w = dynamic_cast<QWidget*>(parent());
	if (reply->error()) {
		// maybe we are missing the right libssl and libcrypto libraries? (from VC2019 onward, 1.1.1g should be needed)
//		qDebug()<<"Suppport"<<QSslSocket::supportsSsl();
//		qDebug()<<"sslLibraryVersionString:" << QSslSocket::sslLibraryVersionString()
//				<< "sslLibraryBuildVersionString:" << QSslSocket::sslLibraryBuildVersionString();

		// Note: when QNetworkReply::UnknownNetworkError occurs on Windows while downloading from https,
		//       it is likely that the libraries libeay32.dll and ssleay32.dll are missing in exe directory
		std::string errormsg;
		switch (reply->error()) {
			case QNetworkReply::NoError: break;
			case QNetworkReply::ConnectionRefusedError: errormsg = "ConnectionRefusedError"; break;
			case QNetworkReply::RemoteHostClosedError: errormsg = "RemoteHostClosedError"; break;
			case QNetworkReply::HostNotFoundError: errormsg = "HostNotFoundError"; break;
			case QNetworkReply::TimeoutError: errormsg = "TimeoutError"; break;
			case QNetworkReply::OperationCanceledError: errormsg = "OperationCanceledError"; break;
			case QNetworkReply::SslHandshakeFailedError: errormsg = "SslHandshakeFailedError"; break;
			case QNetworkReply::TemporaryNetworkFailureError: errormsg = "TemporaryNetworkFailureError"; break;
			case QNetworkReply::NetworkSessionFailedError: errormsg = "NetworkSessionFailedError"; break;
			case QNetworkReply::BackgroundRequestNotAllowedError: errormsg = "BackgroundRequestNotAllowedError"; break;
			case QNetworkReply::TooManyRedirectsError: errormsg = "TooManyRedirectsError"; break;
			case QNetworkReply::InsecureRedirectError: errormsg = "InsecureRedirectError"; break;
			case QNetworkReply::UnknownNetworkError: errormsg = "UnknownNetworkError"; break;
			case QNetworkReply::ProxyConnectionRefusedError: errormsg = "ProxyConnectionRefusedError"; break;
			case QNetworkReply::ProxyConnectionClosedError: errormsg = "ProxyConnectionClosedError"; break;
			case QNetworkReply::ProxyNotFoundError: errormsg = "ProxyNotFoundError"; break;
			case QNetworkReply::ProxyTimeoutError: errormsg = "ProxyTimeoutError"; break;
			case QNetworkReply::ProxyAuthenticationRequiredError: errormsg = "ProxyAuthenticationRequiredError"; break;
			case QNetworkReply::UnknownProxyError: errormsg = "UnknownProxyError"; break;
			case QNetworkReply::ContentAccessDenied: errormsg = "ContentAccessDenied"; break;
			case QNetworkReply::ContentOperationNotPermittedError: errormsg = "ContentOperationNotPermittedError"; break;
			case QNetworkReply::ContentNotFoundError: errormsg = "ContentNotFoundError"; break;
			case QNetworkReply::AuthenticationRequiredError: errormsg = "AuthenticationRequiredError"; break;
			case QNetworkReply::ContentReSendError: errormsg = "ContentReSendError"; break;
			case QNetworkReply::ContentConflictError: errormsg = "ContentConflictError"; break;
			case QNetworkReply::ContentGoneError: errormsg = "ContentGoneError"; break;
			case QNetworkReply::UnknownContentError: errormsg = "UnknownContentError"; break;
			case QNetworkReply::ProtocolUnknownError: errormsg = "ProtocolUnknownError"; break;
			case QNetworkReply::ProtocolInvalidOperationError: errormsg = "ProtocolInvalidOperationError"; break;
			case QNetworkReply::ProtocolFailure: errormsg = "ProtocolFailure"; break;
			case QNetworkReply::InternalServerError: errormsg = "InternalServerError"; break;
			case QNetworkReply::OperationNotImplementedError: errormsg = "OperationNotImplementedError"; break;
			case QNetworkReply::ServiceUnavailableError: errormsg = "ServiceUnavailableError"; break;
			case QNetworkReply::UnknownServerError: errormsg = "UnknownServerError"; break;
		}
		IBK::IBK_Message(IBK::FormatString("Network error: %1").arg(errormsg), IBK::MSG_ERROR, FUNC_ID);
		if (m_interactive)
			QMessageBox::critical(w, tr("Connection error"), tr("Could not retrieve update information."));
		else {
			emit updateInfoRetrieved(2, QString());
		}
	}
	else {
		QByteArray newsRaw = reply->readAll();
		// we expect the changelog file to have the following content:
		//win64:Delphin_6.1.7_win64_2025-01-02.exe
		//mac:Delphin_6.1.7_macosx_2025-01-02.dmg
		//linux:Delphin_6.1.7_linux_2025-01-02.7z

		//---HeaderEnd---
		// [lang:de]
		// ... deutscher Update-Info-Text ...
		// [lang:en]
		// ... englischer Update-Info-Text ...

		// when we parse the file, we first try to split the file using "[lang:" as separator.
		// then, we identify the language and remove the "de]\n" or "en]\n" from the start of the changelog and
		// store it in the language specific section

		// extract text
		QMap<QString, QString>	header;
		QString news = QString::fromUtf8(newsRaw);
		int headerEndPos = news.indexOf("---HeaderEnd---");
		if (headerEndPos != -1) {
			m_changeLogText = news.mid(headerEndPos + 15);
			news = news.left(headerEndPos);
		}

		// split changelog text
		QStringList changeLogLangs = m_changeLogText.split("[lang:");
		for (const QString & s : asConst(changeLogLangs)) {
			if (s.size() > 3 && s[2] == ']' && s[3] == '\n') {
				QString langID = s.left(2);
				m_changeLogLanguageText[langID] = s.mid(4);
			}
		}

		// split installer/updater file names
		QStringList lines = news.split('\n');
		for (int i=0; i<lines.size(); ++i) {
			QString  line = lines[i].trimmed();
			QStringList tokens = line.split(":");
			if (tokens.size() == 2)
				header[tokens[0]] = tokens[1];
		}

		// find matching file version
#ifdef _WIN32
#ifdef _WIN64
		QString updateFile = header["win64"];
#else // _WIN64
		QString updateFile = header["win"];
#endif
#else
#ifdef Q_OS_MAC
		QString updateFile = header["mac"];
#else // Q_OS_MAC
		QString updateFile = header["linux"];
#endif
#endif
		// extract version number
		QStringList tokens = updateFile.split("_");
		if (tokens.count() < 2)
			updateFile.clear(); // malformed filename - same as no file
		else {
			m_newVersion = tokens[1];
		}
		if (updateFile.isEmpty()) {
			if (m_interactive)
				showChangelogInfo(w);
			else {
				emit updateInfoRetrieved(1, QString());
			}
		}
		else {
			m_downloadUrl = m_updateInfoDownloadPath + "/" + updateFile;
			if (m_interactive) {
				// only show dialog if a newer version is available
				if (IBK::Version::lesserVersionNumber(m_currentVersion.toStdString(), m_newVersion.toStdString())) {
					// when the new version was already skipped by the user, do not show anything
					QString relevantVersion;
					if (!m_newestRejectedVersion.isEmpty() &&
						IBK::Version::lesserVersionNumber(m_currentVersion.toStdString(), m_newestRejectedVersion.toStdString()))
					{
						relevantVersion = m_newestRejectedVersion;
					}
					else
						relevantVersion = m_currentVersion;

					if (IBK::Version::lesserVersionNumber(relevantVersion.toStdString(), m_newVersion.toStdString())) {
						bool rejected;
						showUpdateDialog(w, m_currentVersion, m_newVersion, m_changeLogText, m_changeLogLanguageText,
										 m_downloadUrl, m_downloadFilePath, m_downloadedFileMD5, rejected);
						if (rejected)
							emit updateRejected(m_newVersion);
					}
				}
				else
					showChangelogInfo(w);
			}
			else {
				emit updateInfoRetrieved(0, m_newVersion);
			}
		}
	}

	reply->deleteLater();
}

void AutoUpdater::onSslErrors(QNetworkReply * reply, QList<QSslError> errors) {
	for (const QSslError & err : errors) {
		qCritical() << err;
	}

	reply->ignoreSslErrors();
}


void AutoUpdater::showChangelogInfo(QWidget * w) {
	QDialog dlg(w, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
	dlg.setWindowTitle(tr("No update available"));
	QVBoxLayout * lay = new QVBoxLayout(w);
	QLabel * label = new QLabel(tr("There is currently no update available for this software and platform/OS."));
	lay->addWidget(label);
	QPlainTextEdit * edit = new QPlainTextEdit(w);
	edit->setReadOnly(true);
	QFont f;
#if defined(Q_OS_MAC) // Q_OS_UNIX
	f.setFamily("Monaco");
#elif defined(Q_OS_UNIX)
	f.setFamily("Monospace");
	f.setPointSize(8); // smaller point size for changelog
#else
	f.setFamily("Consolas");
#endif
	edit->setFont(f);
	edit->setWordWrapMode( QTextOption::NoWrap );
	lay->addWidget(edit);
	QString changes;
	QString langId = QtExt::LanguageHandler::langId();
	if (m_changeLogLanguageText.contains(langId))
		changes = m_changeLogLanguageText[langId];
	else {
		// fall-back to english if language is not present
		if (langId != "en" && m_changeLogLanguageText.contains("en"))
			changes = m_changeLogLanguageText["en"];
	}

	// fall-back to old functionality
	if (changes.isEmpty()) {
		changes = m_changeLogText;
		int pos = changes.indexOf("CHANGES");
		if (pos != -1)
			changes = changes.right(changes.length() - pos);
	}
	edit->setPlainText(changes);
	dlg.setLayout(lay);
	QScreen *screen = w->window()->windowHandle()->screen();
	QRect rec = screen->geometry();
	int height = rec.height();
	int width = rec.width();
	dlg.resize(QSize(width*0.6, height*0.8));
	dlg.exec();
}


void AutoUpdater::showUpdateDialog(QWidget * parent, QString currentVersion, QString newVersion,
								   QString changeLogText, const QMap<QString, QString> & changeLogLanguages,
								   QString downloadUrl, QString &downloadFilePath,
								   std::string & downloadedFileMD5, bool & rejected)
{
	// we expect m_newVersion, m_currentVersion and m_changeLogText to be populated correctly
	AutoUpdateDialog dlg(parent);
	dlg.run(currentVersion, newVersion, changeLogText, changeLogLanguages, downloadUrl);
	rejected = dlg.m_updateRejected;

	// if file was downloaded, we have a valid MD5 hash and local file path, otherwise the local file path
	// will be empty
	downloadedFileMD5 = dlg.m_updateFileMD5;
	downloadFilePath = dlg.m_updateFilePath;
}


} // namespace QtExt
