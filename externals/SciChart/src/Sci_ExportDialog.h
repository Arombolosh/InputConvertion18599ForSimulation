/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ExportDialogH
#define Sci_ExportDialogH

#include <QDialog>

namespace SCI {

namespace Ui {
	class ExportDialog;
}

class Chart;
class Export2VectorWidget;

/*! Dialog for exporting chart to file.
	Call exportChart() to show the dialog instead of exec().
*/
class ExportDialog : public QDialog {
	Q_OBJECT

public:
	explicit ExportDialog(QWidget *parent = 0);
	~ExportDialog();

	/*! Main exec function for the dialog, call instead of exec().
		\param chart Pointer to the chart to export.
	*/
	bool exportChart(Chart * chart);

	/*! This function is only used for scripted export of diagram files. */
	bool exportChart(Chart * chart, std::string filename, int b, int h);

	/*! Gives access to the vector export widget (only needed for the history of dimensions used) */
	Export2VectorWidget * export2VectorWidget();

	/*! Holds last export directory.
		Users of this dialog should initialize this path with some meaningful default and
		store this path in user settings when exportChart() returns successfully.
	*/
	static QString m_lastExportDirectory;

private slots:
	void onChartComponentToggled();
	void on_pushButtonSave_clicked();
	void on_pushButtonCopyToClipboard_clicked();
	void on_tabWidget_currentChanged(int index);
	void onExportButtonsEnabled(bool enabled);

	void on_lineEditExportFilename_returnPressed();

private:
	Ui::ExportDialog	*m_ui;
};

} // namespace SCI

#endif // Sci_ExportDialogH
