#include "QtExt_ReportPreview.h"
#include "ui_QtExt_ReportPreview.h"

#include <QPrinter>
#include <QPrintPreviewWidget>
#include <QPrintDialog>
#include <QPageSetupDialog>
#include <QFileDialog>
#include <QFontDatabase>
#include <QDebug>
#include <QScreen>
#include <QMetaMethod>

#include <IBK_messages.h>
#include <IBK_Path.h>

#include "QtExt_Report.h"
#include "QtExt_ReportSettingsBase.h"

namespace QtExt {


ReportPreview::ReportPreview(QtExt::Report* report, QtExt::ReportData* data, QWidget *parent) :
	QDialog(parent,
#ifdef Q_OS_LINUX
			Qt::Window),
#else
			Qt::WindowMinMaxButtonsHint |
			Qt::WindowCloseButtonHint |
			Qt::WindowSystemMenuHint),
#endif
	ui(new Ui::ReportPreview),
	m_printer(nullptr),
	m_report(report),
	m_data(data),
	m_reportSettings(report->m_reportSettings),
	m_updateNecessary(false)
{
	ui->setupUi(this);


	// set size to 3/4 of screen size
#if QT_VERSION >= 0x050a00
	QScreen* screen = QApplication::screenAt(QPoint(0,0));
#else
	QList<QScreen *> screens = QApplication::screens();
	QScreen* screen = screens.empty() ? nullptr : screens.front();
#endif
	QRect rect = screen->geometry();
	resize(rect.size()*0.8);

	try {
		// This is the printer used for the preview widget and the printer.
		m_printer = new QPrinter(QPrinter::HighResolution);
		if(m_printer == nullptr) {
		}
		Q_ASSERT(m_printer != nullptr);

		// set default margins
		m_printer->setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);
	}
	catch(std::exception&) {
	}


	// create print preview widget
	m_previewWidget = new QPrintPreviewWidget(m_printer, this);
	m_previewWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// Note: the dummy widget is replaced by the previewWidget in showEvent()
	ui->mainLayout->insertWidget(0, m_previewWidget);
	delete ui->widgetPreview;


//	connect(m_report, SIGNAL(progress(int)), this , SLOT(progress(int)));
	connect(this, &ReportPreview::visibilityChanged, this, &ReportPreview::onVisibilityChanged);

	// populate combo box
	ui->comboBoxZoomOptions->addItem(tr("Fit Page in View"));
	ui->comboBoxZoomOptions->addItem(tr("Fit to Page Width"));
	ui->comboBoxZoomOptions->addItem(tr("Custom Zoom"));
	ui->comboBoxZoomOptions->setCurrentIndex(1);


	// add all checkbox with their corresponding frame ids to the mapping
	m_activatedFrames[ui->checkBoxHeader] = QtExt::ReportSettingsBase::ReportHeader;
	m_activatedFrames[ui->checkBoxFooter] = QtExt::ReportSettingsBase::ReportFooter;

	// connect all checkboxes to the toggled slot
	for (auto it = m_activatedFrames.constBegin();
		it != m_activatedFrames.constEnd(); ++it)
	{
		connect(it.key(), SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged(int)));
	}


	// calculate the width of the dialog part
	int widthBlub = rect.width() * 0.1;
	for ( auto it = m_activatedFrames.begin(); it != m_activatedFrames.end(); ++it ) {
		QCheckBox* box = it.key();
		widthBlub = std::max(widthBlub, box->width());
	}
	ui->toolBox->setFixedWidth(widthBlub);

	ui->checkBoxFooter->setVisible(false);
	ui->checkBoxFooter->setChecked(true);
	ui->checkBoxHeader->setVisible(false);
	ui->checkBoxHeader->setChecked(true);


	ui->fontComboBoxFont->blockSignals(true);
	QFont df = m_reportSettings->m_defaultFont;
	ui->fontComboBoxFont->setFont(df);
	setFontSizes(df);
	ui->fontComboBoxFont->blockSignals(false);


	m_previewWidget->blockSignals(true);
	m_previewWidget->setZoomMode(QPrintPreviewWidget::FitInView);
	m_previewWidget->blockSignals(false);

	// connect the printing signal
	connect( m_previewWidget, SIGNAL(paintRequested(QPrinter*)), this, SLOT(printReport(QPrinter*)));

}

ReportPreview::~ReportPreview() {

	delete ui;
	delete m_printer;
}

void ReportPreview::set(const QString& registrationMessage, const QString& applicationName, const QString& projectName) {

	m_registrationMessage = registrationMessage;
	m_applicationName = applicationName;
	m_projectName = projectName;

}

void ReportPreview::registerCheckBox(QCheckBox* checkBox, int frameType) {
	m_activatedFrames[checkBox] = frameType;
	QMetaObject::Connection connected = connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged(int)), Qt::UniqueConnection);
	Q_ASSERT(connected);
}

void ReportPreview::setFontSizes(QFont font) {
//	ui->comboBoxFontSize->blockSignals(true);
	QFontDatabase database;
	QList<int> fontSizes = database.pointSizes(font.family());
	ui->comboBoxFontSize->clear();
	int currentSize = font.pointSize();
	int currentIndex = -1;
	for(QList<int>::Iterator it=fontSizes.begin(); it != fontSizes.end(); ++it) {
		int size = *it;
		ui->comboBoxFontSize->addItem(QString("%1").arg(size), size);
		if(currentIndex == -1 && size >= currentSize) {
			if(currentSize == size)
				currentIndex = it - fontSizes.begin();
			else {
				it = fontSizes.insert(it, currentSize);
				currentIndex = it - fontSizes.begin() + 1;
			}
		}
	}
	ui->comboBoxFontSize->setCurrentIndex(currentIndex);
//	ui->comboBoxFontSize->blockSignals(false);
}

void ReportPreview::updatePreview() {
//	Logger::instance() << "ReportPreview::updatePreview start";

	// call of updatePreview invokes a emit of paintRequested signal
	// signal paintRequested is connected to printReport function
	m_previewWidget->updatePreview();
}

void ReportPreview::on_pushButtonPrint_clicked() {
	// start the printing on a printer
	QPrintDialog printDialog(m_printer, this);
	if (printDialog.exec() == QDialog::Accepted) {
		printReport(m_printer);
	}
}


void ReportPreview::printReport(QPrinter * printer) {
	const char * const FUNC_ID = "[ReportPreview::printReport]";

	if( !m_updateNecessary)
		return;

	try {
		m_updateNecessary = false;

		QPixmap userLogo;
		m_report->setHeaderFooterData(m_registrationMessage, m_projectName, "", userLogo);
		m_report->set(m_data);

		m_report->print(printer, m_reportSettings->m_defaultFont);
		emit progress(0);
		m_updateNecessary = true;
	}
	catch(std::exception& e) {
		IBK::IBK_Message( IBK::FormatString("Error while printing report.\n%1!")
						  .arg(e.what()),
						  IBK::MSG_ERROR, FUNC_ID, IBK::VL_STANDARD);
	}
}

void ReportPreview::onVisibilityChanged(int frameType, bool visible) {
	if (visible) {
		m_reportSettings->m_frames.insert(frameType);
	}
	else {
		m_reportSettings->m_frames.erase(frameType);
	}
	if( m_updateNecessary)
		updatePreview();
}

void ReportPreview::onReportSettingsChanged() {
	if( m_updateNecessary)
		updatePreview();
}

void ReportPreview::addPage(const QString& name, ReportPreviewPageWidget* widget) {
	ui->toolBox->addItem(widget, name);
	connect(widget, &ReportPreviewPageWidget::visibilityChanged, this, &ReportPreview::onVisibilityChanged);
	connect(widget, &ReportPreviewPageWidget::reportSettingsChanged, this, &ReportPreview::onReportSettingsChanged);
	widget->setSettings(m_reportSettings);
}

void ReportPreview::showEvent(QShowEvent *) {

	m_updateNecessary = false;

	// loop over all checkboxes in the map and set the respective state
	for (auto it = m_activatedFrames.constBegin();
		it != m_activatedFrames.constEnd(); ++it)
	{
		it.key()->setChecked(m_reportSettings->hasFrameId(it.value()) );
	}

	ui->checkBoxPageNumbers->setChecked( m_reportSettings->m_showPageNumbers );

	m_updateNecessary = true;
	updatePreview();
}

void ReportPreview::checkBoxStateChanged(int state) {
	QCheckBox * cb = dynamic_cast<QCheckBox*>(QObject::sender());
	auto it = m_activatedFrames.find(cb);
	if (it == m_activatedFrames.end())
		return;

	cb->setCheckState(static_cast<Qt::CheckState>(state));
	if (state == Qt::Checked) {
		m_reportSettings->m_frames.insert(it.value());
	}
	else {
		m_reportSettings->m_frames.erase(it.value());
	}
	if( m_updateNecessary)
		updatePreview();
}

void ReportPreview::on_pushButtonClose_clicked() {
	close();
}

void ReportPreview::on_pushButtonZoomIn_clicked() {
	ui->comboBoxZoomOptions->setCurrentIndex(2);
	m_previewWidget->zoomIn();
}

void ReportPreview::on_pushButtonZoomOut_clicked() {
	ui->comboBoxZoomOptions->setCurrentIndex(2);
	m_previewWidget->zoomOut();
}

void ReportPreview::on_comboBoxZoomOptions_currentIndexChanged(int index) {
	switch (index) {
		case 0 : m_previewWidget->setZoomMode(QPrintPreviewWidget::FitInView); break;
		case 1 : m_previewWidget->setZoomMode(QPrintPreviewWidget::FitToWidth); break;
		case 2 : m_previewWidget->setZoomMode(QPrintPreviewWidget::CustomZoom); break;
	}
}

void ReportPreview::on_checkBoxPageNumbers_toggled(bool checked) {
	m_reportSettings->m_showPageNumbers = checked;
	m_previewWidget->updatePreview();
}

void ReportPreview::on_pushButtonPageSetup_clicked() {
	// open Print setup dialog
	QPageSetupDialog setupDlg(m_printer, this);
	setupDlg.exec();
	m_previewWidget->updatePreview();
}

void ReportPreview::on_pushButtonWritePdf_clicked() {
	QString fileName = QFileDialog::getSaveFileName((QWidget* )0, "Export PDF", QString(), "*.pdf");
	if (QFileInfo(fileName).suffix().isEmpty()) {
		fileName.append(".pdf");
	}

	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setPageSize(QPageSize(QPageSize::A4));
	printer.setOutputFileName(fileName);
	QMarginsF margins(20, 20, 20, 20);
	printer.setPageMargins(margins, QPageLayout::Millimeter);
	m_updateNecessary = true;
	printReport(&printer);

}

void ReportPreview::on_pushButtonExportPage_clicked() {
	QFileDialog exportDialog(this, "Export file", "./");
	QStringList filters;
	filters << "Pdf file (*.pdf)";
	exportDialog.setNameFilters(filters);
	exportDialog.setDefaultSuffix("pdf");
	exportDialog.setDirectory("./*.pdf");
	exportDialog.exec();
	QStringList files = exportDialog.selectedFiles();

	QString newPath = files[0];

	if (newPath.isEmpty())
		 return;

	QPrinter printer(QPrinter::HighResolution);
	printer.setOutputFormat(QPrinter::PdfFormat);
	printer.setPageSize(QPageSize(QPageSize::A4));
	printer.setPageMargins(QMarginsF(20, 20, 20, 20), QPageLayout::Millimeter);

	IBK::Path tempPath(newPath.toStdString());
	tempPath.withoutExtension().addExtension("pdf");
	newPath = QString::fromStdString(tempPath.str());
	printer.setOutputFileName(newPath);

	m_updateNecessary = true;
	m_report->printPage(&printer, m_previewWidget->currentPage(), this->font());
}

void ReportPreview::on_fontComboBoxFont_currentFontChanged(const QFont &f) {
	m_reportSettings->m_defaultFont = f;
	setFontSizes(f);
	m_report->setDocumentFont(f);
	updatePreview();
}

void ReportPreview::on_comboBoxFontSize_currentIndexChanged(int index) {
	int size = ui->comboBoxFontSize->itemData(index).toInt();
	m_reportSettings->m_defaultFont.setPointSize(size);
	m_report->setDocumentFont(m_reportSettings->m_defaultFont);
	updatePreview();
}


} // namespace QtExt
