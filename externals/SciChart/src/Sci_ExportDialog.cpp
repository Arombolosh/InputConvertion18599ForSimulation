/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_ExportDialog.h"
#include "ui_Sci_ExportDialog.h"

#include <QMessageBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>

#include <IBK_StringUtils.h>

#include <qwt_plot_renderer.h>

namespace SCI {

QString ExportDialog::m_lastExportDirectory;

ExportDialog::ExportDialog(QWidget *parent) :
	QDialog(parent, Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint),
	m_ui(new Ui::ExportDialog)
{
	m_ui->setupUi(this);
	m_ui->tab->layout()->setContentsMargins(0,0,0,0);
	m_ui->tab_2->layout()->setContentsMargins(0,0,0,0);

	m_ui->checkBox_canvasBackground->setChecked(false);

	connect(m_ui->checkBox_title, &QCheckBox::toggled, this, &ExportDialog::onChartComponentToggled);
	connect(m_ui->checkBox_background, &QCheckBox::toggled, this, &ExportDialog::onChartComponentToggled);
	connect(m_ui->checkBox_canvasBackground, &QCheckBox::toggled, this, &ExportDialog::onChartComponentToggled);
	connect(m_ui->checkBox_canvasFrame, &QCheckBox::toggled, this, &ExportDialog::onChartComponentToggled);
	connect(m_ui->checkBox_legend, &QCheckBox::toggled, this, &ExportDialog::onChartComponentToggled);
	onChartComponentToggled();

	// We disable the configuration of the chart - user expactations are not clear, and all relevant properties
	// can be set in the model anyway!
	m_ui->groupBox_RenderFlags->setVisible(false);

	connect(m_ui->widgetExportBitmap, SIGNAL(exportButtonsEnabled(bool)),
			this, SLOT(onExportButtonsEnabled(bool)));
	connect(m_ui->widgetExportVector, SIGNAL(exportButtonsEnabled(bool)),
			this, SLOT(onExportButtonsEnabled(bool)));
}


ExportDialog::~ExportDialog() {
	delete m_ui;
}


bool ExportDialog::exportChart(Chart * chart) {
	m_ui->widgetExportBitmap->setChart(chart);
	m_ui->widgetExportVector->setChart(chart);

	return (exec() == QDialog::Accepted);
}


bool ExportDialog::exportChart(Chart * chart, std::string filename, int b, int h) {
	FUNCID(ExportDialog::exportChart);

	m_ui->widgetExportVector->setChart(chart);
	m_ui->widgetExportBitmap->setChart(chart);

	std::string file, fileEnding;
	int resolution = 300;

	std::vector<std::string> tokens;
	if ( IBK::explode(filename, tokens, '@', true)>1 ) {
		filename = tokens[0];
		tokens[0] = IBK::replace_string(tokens[0], ".", " ");
		std::stringstream strmFilename(tokens[0]);
		std::stringstream strmResolution(tokens[1]);
		if ( !(strmFilename >> file >> fileEnding) || fileEnding == "pdf" || fileEnding == "svg" || fileEnding == "emf")
			throw IBK::Exception(IBK::FormatString("Resolution can be only set to Bitmap graphics '%1' --generate-chart").arg(filename), FUNC_ID);
		else if ( !(strmResolution >> resolution) || resolution <= 0 )
			throw IBK::Exception(IBK::FormatString("Resolution with value '%1' needs to be greater then 0 --generate-chart").arg(resolution), FUNC_ID);
	}

	fileEnding = filename.substr(filename.find_last_of(".") + 1);

	if(fileEnding == "png" || fileEnding == "jpg" || fileEnding == "bmp") {
		m_ui->widgetExportBitmap->setDimension(b, h, resolution);
		m_ui->widgetExportBitmap->saveToFile(QFileInfo(QString::fromStdString(filename)).absoluteFilePath());
	} else {
		m_ui->widgetExportVector->setDimension(b, h);
		m_ui->widgetExportVector->saveToFile(QFileInfo(QString::fromStdString(filename)).absoluteFilePath());
	}
	return true; // signal that no chart has been exported
}



Export2VectorWidget * ExportDialog::export2VectorWidget() {
	return m_ui->widgetExportVector;
}


void ExportDialog::onChartComponentToggled() {
	QwtPlotRenderer::DiscardFlags discardFlags = QwtPlotRenderer::DiscardNone;
	if( !m_ui->checkBox_background->isChecked())
		discardFlags = discardFlags | QwtPlotRenderer::DiscardBackground;
	if( !m_ui->checkBox_canvasBackground->isChecked())
		discardFlags = discardFlags | QwtPlotRenderer::DiscardCanvasBackground;
	if( !m_ui->checkBox_canvasFrame->isChecked())
		discardFlags = discardFlags | QwtPlotRenderer::DiscardCanvasFrame;
	if( !m_ui->checkBox_legend->isChecked())
		discardFlags = discardFlags | QwtPlotRenderer::DiscardLegend;
	if( !m_ui->checkBox_title->isChecked())
		discardFlags = discardFlags | QwtPlotRenderer::DiscardTitle;

	m_ui->widgetExportBitmap->setDiscardFlags(discardFlags);
	m_ui->widgetExportVector->setDiscardFlags(discardFlags);
	m_ui->checkBox_canvasBackground->setEnabled(m_ui->checkBox_background->isChecked());
}


void ExportDialog::on_pushButtonSave_clicked() {
	// ask user to select suitable file name
	QString filter;
	if (m_ui->tabWidget->currentIndex() == 0)
		filter = tr("PNG (*.png);;Bitmap (*.bmp);;Gif (*.gif);;JPeg(*.jpg)");
	else
		filter = tr("PDF (*.pdf);;SVG (*.svg);;EMF (*.emf)");

	QString selectedFilter;
	QString fname = QFileDialog::getSaveFileName(this, tr("Select export filename"), m_lastExportDirectory, filter, &selectedFilter,
												 QFileDialog::DontUseNativeDialog);
	if (fname.isEmpty())
		return; // user cancelled the dialog

	QString extension = QFileInfo(fname).suffix().toLower();
	QString ext = selectedFilter.mid(selectedFilter.indexOf("(*.") +3);
	ext = ext.left(ext.indexOf(')'));
	if (extension != ext) {
		fname +="." + ext;
	}

	bool success;
	if (m_ui->tabWidget->currentIndex() == 0) {
		success = m_ui->widgetExportBitmap->saveToFile(fname);
	}
	else {
		success = m_ui->widgetExportVector->saveToFile(fname);
	}

	if (success) {
		m_lastExportDirectory = QFileInfo(fname).dir().absolutePath();
		accept();
	}
}


void ExportDialog::on_pushButtonCopyToClipboard_clicked() {
	if (m_ui->tabWidget->currentIndex() == 0) {
		if (m_ui->widgetExportBitmap->copyToClipboard())
			accept();
	}
	else {
		if (m_ui->widgetExportVector->copyToClipboard())
			accept();
	}
}


void ExportDialog::on_tabWidget_currentChanged(int) {
	if (m_ui->tabWidget->currentIndex() == 0)
		m_ui->widgetExportBitmap->updateDimensions();
	else
		m_ui->widgetExportVector->updateDimensions();
}


void ExportDialog::onExportButtonsEnabled(bool enabled) {
	m_ui->pushButtonCopyToClipboard->setEnabled(enabled);
	m_ui->pushButtonSave->setEnabled(enabled);
}


void ExportDialog::on_lineEditExportFilename_returnPressed() {
	on_pushButtonSave_clicked();
}

} // namespace SCI


