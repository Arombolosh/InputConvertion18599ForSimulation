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

#ifndef QtExt_BrowseFilenameWidgetH
#define QtExt_BrowseFilenameWidgetH

#include <QWidget>
#include <IBK_Path.h>

#include "QtExt_global.h"

class QLineEdit;
class QToolButton;
class QRadioButton;

namespace QtExt {

namespace Ui {
	class BrowseFilenameWidget;
}

/*! A widget that provides a line edit for entering a file name/path manually and a browse button that opens
	a file selection dialog to manually select the file.

	Optional: Also relative/absolute switching can be enabled
	but then a placeholder map has to be passed from the current tool (\sa onPlaceHoldersUpdated)!

	\note Connect placeholder updates with onPlaceHoldersUpdated(), when using relative / absolute switching and pass
	the placeholder map as an argument

	No rel/abs switching:
	\code
	// to select existing files only
	filenameWidget->setup("myfile.txt", true, true, tr("Text files (*.txt);;All files (*.*)"));

	// to existing directories
	filenameWidget->setup("myfile.txt", false, true, QString());

	// to select directories that may not yet exist
	filenameWidget->setup("myfile.txt", false, false, QString());
	\endcode

	With rel/abs switching:
	\code
	filenameWidget>setup("", true, true, tr("FMU file (*.fmu)"), SVSettings::instance().m_dontUseNativeDialogs,
											   tr("Supply FMU:"), true, &project().m_placeholders);
	connect(&SVProjectHandler::instance(), &SVProjectHandler::placeholdersUpdated, filenameWidget,
			&QtExt::BrowseFilenameWidget::onPlaceHoldersUpdated);
	\endcode
*/
class QtExt_EXPORT BrowseFilenameWidget : public QWidget {
	Q_OBJECT

public:
	explicit BrowseFilenameWidget(QWidget *parent = nullptr);

	/*! Sets up line edit.
		\param filename	Current name of file
		\param filenameMode If true, tool button requests a file, otherwise a directory.
		\param fileMustExist If true, the file must exist when browsing for the file/directory.
		\param filter filter, only applicable in filename mode
		\param dontUseNativeFilenameDialog if true the Qt File-Browser is used.
		\param labelText If a label inside the widget shall be shown, set the label text here
		\param relAbsSwitching If true a relative/absolute switching is enabled
		\param placeholders Placeholder map, needed to enabled relative/absolute switching
	*/
	void setup(const QString & filename, bool filenameMode, bool fileMustExist, const QString & filter,
			   bool dontUseNativeFilenameDialog, QString labelText = "", bool relAbsSwitching = false,
			   const std::map< std::string, IBK::Path > *placeholders = nullptr);

	/*! Sets a filename in the line edit. */
	void setFilename(const QString & filename);

	/*! Returns the filename currently held in the line edit. */
	QString filename(bool abs = false) const;

	/*! When set to read-only, button is disabled and line edit is made read-only. */
	void setReadOnly(bool readOnly);

	/*! Updates the Ui. */
	void updateUi();

	/*! Size hint. */
	QSize sizeHint() const override;

	/*! The line edit (to set tab order). */
	QLineEdit		*lineEdit();

	/*! The tool button (to set tab order). */
	QToolButton		*toolBtn();

signals:
	/*! Emitted when filename in line edit has changed. */
	void editingFinished();

	/*! Emitted, when return was pressed in line edit (to complete the editing of the filename). */
	void returnPressed();

public slots:
	void onPlaceHoldersUpdated(const std::map<std::string, IBK::Path> &placeholders);

private slots:
	void onToolBtnClicked();

	/*! Radio button rel. */
	void onAbsToggled(bool rel);

private:
	Ui::BrowseFilenameWidget			*m_ui;

	/*! If true, tool button requests a file, otherwise a directory. */
	bool								m_filenameMode;
	/*! If true, the file must exist when browsing for the file/directory. */
	bool								m_fileMustExist;
	/*! Filter to use in file dialog (only applicable for filename-mode). */
	QString								m_filter;
	/*! Filename string. */
	QString								m_filename;
	/*! If true, the Qt-own file dialog is used. */
	bool								m_dontUseNativeFilenameDialog;
	/*! Project file path, used to generate relative filepath, when
		mode has switched (rel/abs). */
	std::map< std::string, IBK::Path >	m_placeholders;
	/*! Is relative or absolute mode active?. */
	bool								m_absMode = true;
	/*! Relative / absolute switching is enabled. */
	bool								m_relAbsSwitchingEnabled = false;

};

} // namespace QtExt

#endif // QtExt_BrowseFilenameWidgetH
