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
	OHNE JEDE GEWäHR,; sogar ohne die implizite
	GewÃ¤hr der MARKTFäHIGKEIT oder EIGNUNG FüR EINEN BESTIMMTEN ZWECK.
	Siehe die GNU General Public License für weitere Einzelheiten.

	Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
*/

#ifndef QtExt_ReportPreviewH
#define QtExt_ReportPreviewH

#include <type_traits>

#include <QDialog>
#include <QMap>

#include "QtExt_ReportPreviewPageWidget.h"
#include "QtExt_global.h"

class QPrintPreviewWidget;
class QCheckBox;
class QPrinter;

namespace QtExt {

class Report;
class ReportSettingsBase;
class ReportData;

namespace Ui {
	class ReportPreview; 
}

class QtExt_EXPORT ReportPreview : public QDialog {
	Q_OBJECT
	Q_DISABLE_COPY(ReportPreview)

public:
	/*! Constructor of ReportPreviewbase.
		\param report Pointer to object based from QtExt::Report.
		The ReportPreview class object takes ownership of the report object.
	*/
	ReportPreview(QtExt::Report* report, QtExt::ReportData* data, QWidget *parent = 0);

	~ReportPreview();

	/*! Set basic properties.*/
	void set(const QString& registrationMessage, const QString& applicationName, const QString& projectName);

	/*! Register checkbox.*/
	void registerCheckBox(QCheckBox* checkBox, int frameType);

	/*! Add a new property page with including widget.*/
	template<class PageWidget>
	void addPage(const QString& name) {
		PageWidget* widget = new PageWidget(this);
		static_assert(std::is_base_of<ReportPreviewPageWidget, PageWidget>::value, "Widget is not based of ReportPreviewPageWidget");
		ReportPreviewPageWidget* pageWidget = static_cast<ReportPreviewPageWidget*>(widget);
		addPage(name, pageWidget);
	}

public slots:
	/*! Can be called from MainWindow to signal updating of the preview information.*/
	void updatePreview();

	/*! This slot is activated from the print button in this dialog, or from
		the print action in the main window.
		*/
	void on_pushButtonPrint_clicked();

	/*! Prints the report onto the selected printer.
		This slot is called from the print preview widget.
		*/
	void printReport(QPrinter * printer);

protected:
	/*! Overloaded to set the checkbox states.*/
	virtual void showEvent(QShowEvent *e);

private slots:

	/*! Triggered when user checks/unchecks any of the check boxes*/
	void checkBoxStateChanged(int);

	/*! Close button implementation.*/
	void on_pushButtonClose_clicked();

	/*! Set zoom level in preview.*/
	void on_comboBoxZoomOptions_currentIndexChanged(int index);

	/*! Set if page numbers are shown.*/
	void on_checkBoxPageNumbers_toggled(bool checked);

	/*! Open page setup dialog.*/
	void on_pushButtonPageSetup_clicked();

	/*! Write complete preview to pdf file.*/
	void on_pushButtonWritePdf_clicked();

	/*! Change the font type for the complete preview.*/
	void on_fontComboBoxFont_currentFontChanged(const QFont &f);

	/*! Change the normal font size for the complete preview (HTML flag \normal).*/
	void on_comboBoxFontSize_currentIndexChanged(int index);

	/*! Exports current page to pdf.*/
	void on_pushButtonExportPage_clicked();

	/*! Zoom-in in preview.*/
	void on_pushButtonZoomIn_clicked();

	/*! Zoom-out in preview.*/
	void on_pushButtonZoomOut_clicked();

	/*! Change visibilty of a frame and updates the preview.*/
	void onVisibilityChanged(int frameType, bool visible);

	/*! Update the preview in case of exernal change in report settings.
		Is connected with a signal type reportSettingsChanged from one of the property widgets.
	*/
	void onReportSettingsChanged();

protected:
	Ui::ReportPreview *ui;

private:
	void addPage(const QString& name, ReportPreviewPageWidget* widget);

	/*! Fill font size combo with size list according given font.*/
	void setFontSizes(QFont font);

	/*! Widget for print preview.*/
	QPrintPreviewWidget *		m_previewWidget;

	/*! Pointer to the printer object used in the dialog.
		The printer object is created on first use and kept
		for the livetime of the print dialog. Thus, subsequent
		printing tasks retain the selected print style.
	*/
	QPrinter *					m_printer;

	/*! Report to be printed.*/
	QtExt::Report*				m_report;

	/*! Data for report.*/
	QtExt::ReportData*			m_data;

	/*! Settings for report view.*/
	QtExt::ReportSettingsBase*	m_reportSettings;

	/*! Links a checkbox to the corresponding frame id.*/
	QMap<QCheckBox*,int>		m_activatedFrames;

	/*! If false updatePreview will not be called.*/
	bool						m_updateNecessary;

	/*! Name of the calling application.*/
	QString						m_applicationName;

	/*! Name of the curent project.*/
	QString						m_projectName;

	/*! Message for registration of calling application.*/
	QString						m_registrationMessage;


signals:
	/*! Sends progress of creating or updating report.
		It is used by a progress bar in main form.*/
	void progress(int);

	/*" Can be sent in case the visibility of a frame is changed.*/
	void visibilityChanged(int frameType, bool visible);

	friend class ReportPreviewPageWidget;
};


} // namespace QtExt

#endif // QtExt_ReportPreviewH
