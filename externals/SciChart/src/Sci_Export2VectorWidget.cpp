#include "Sci_Export2VectorWidget.h"
#include "ui_Sci_Export2VectorWidget.h"

#include <QApplication>
#include <QBuffer>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>
#include <QPainter>
#include <QPdfWriter>
#include <QScreen>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
#include <QWindow>
#else
#include <QDesktopWidget>
#endif
#include <qwt_plot_renderer.h>

#include "Sci_Chart.h"
#include "Sci_ChartRenderWidget.h"
#include "Sci_ExportDialog.h"

#ifdef SCICHART_HAS_EMFENGINE
#include <EmfEngine.h>
#endif // SCICHART_HAS_EMFENGINE

#include <QSvgGenerator>

namespace SCI {

Export2VectorWidget::Export2VectorWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::Export2VectorWidget),
	m_chart(nullptr),
	m_dimensionHistory(nullptr)
{
	m_ui->setupUi(this);
	m_ui->frame->layout()->setContentsMargins(0,0,0,0);
	m_ui->lineEditMMX->setup(0, std::numeric_limits<double>::max(), tr("A positive number is required."));
	m_ui->lineEditMMY->setup(0, std::numeric_limits<double>::max(), tr("A positive number is required."));

	on_checkBoxDataReduction_toggled(false);
}


Export2VectorWidget::~Export2VectorWidget() {
	delete m_ui;
}


void Export2VectorWidget::setChart(SCI::Chart * chart) {
	if (m_chart == NULL) {
		double mmX = 160;
		double mmY = 120;
		m_ui->lineEditMMX->setText( QString("%L1").arg(mmX) );
		m_ui->lineEditMMY->setText( QString("%L1").arg(mmY) );

		if (mmY != 0)
			m_aspectRatio = mmX/mmY;
		else
			m_aspectRatio = 1;
	}
	m_chart = chart;
	m_ui->widgetVectorPreview->m_chart = NULL; // disables preview, will be updated in updateDimensions()
	updateDimensions();
}


void Export2VectorWidget::setDimension(double mmX, double mmY) {
	m_ui->lineEditMMX->setText( QString("%L1").arg(mmX) );
	m_ui->lineEditMMY->setText( QString("%L1").arg(mmY) );
}


void Export2VectorWidget::setDimensionHistory(QList<DimensionHistory> * dimensionHistory) {
	Q_ASSERT(dimensionHistory);
	m_dimensionHistory = dimensionHistory;
	m_ui->comboBoxLastPlotDimensions->blockSignals(true);
	m_ui->comboBoxLastPlotDimensions->clear();
	m_ui->comboBoxLastPlotDimensions->addItem(""); // first is always empty
	for (int i=0; i<dimensionHistory->count(); ++i) {
		QString text(tr("%1x%2").arg((*dimensionHistory)[i].xMM, (*dimensionHistory)[i].yMM));
		m_ui->comboBoxLastPlotDimensions->addItem(text);
	}
	m_ui->comboBoxLastPlotDimensions->blockSignals(false);
}


void Export2VectorWidget::setDiscardFlags(QwtPlotRenderer::DiscardFlags discardFlags) {
	m_discardFlags = discardFlags;
	m_ui->widgetVectorPreview->m_discardFlags = discardFlags;
	updateDimensions(); // updates the preview
}


void Export2VectorWidget::updateDimensionHistory() {
	if (m_dimensionHistory != nullptr) {
		// store and append to combo box
		QString xMM = m_ui->lineEditMMX->text();
		QString yMM = m_ui->lineEditMMY->text();

		int idxHistory = 0;
		for (; idxHistory<m_dimensionHistory->count(); ++idxHistory) {
			if (xMM == (*m_dimensionHistory)[idxHistory].xMM &&
				yMM == (*m_dimensionHistory)[idxHistory].yMM)
			{
				(*m_dimensionHistory)[idxHistory].lastUsed = QDateTime::currentDateTime();
				break;
			}
		}
		if (idxHistory == m_dimensionHistory->count()) {
			DimensionHistory h;
			h.xMM = xMM;
			h.yMM = yMM;
			h.lastUsed = QDateTime::currentDateTime();
			m_dimensionHistory->append(h);
			setDimensionHistory(m_dimensionHistory);
		}
	}
}


bool Export2VectorWidget::saveToFile(const QString & fname) {
	if (!m_ui->lineEditMMX->isValid() || !m_ui->lineEditMMY->isValid()) {
		QMessageBox::critical(this, tr("Export error"), tr("Invalid graphics dimensions"));
		return false;
	}
	QSizeF sizeMM(m_ui->lineEditMMX->value(), m_ui->lineEditMMY->value());
	if (sizeMM.width() > 800 || sizeMM.height() > 800) {
		QMessageBox::critical(this, tr("Export chart"), tr("Dimensions too large!"));
		return false;
	}

	updateDimensionHistory();

	QwtPlotRenderer renderer;
	if (!m_ui->checkBoxNoTransparency->isChecked()) {
		// do not draw plot and canvas background
		renderer.setDiscardFlags(m_discardFlags | QwtPlotRenderer::DiscardCanvasBackground | QwtPlotRenderer::DiscardBackground);
	}
	else {
		QwtPlotRenderer::DiscardFlags discardFlags = m_discardFlags;
		// turn off discard flags for background
		discardFlags = discardFlags & ~QwtPlotRenderer::DiscardBackground;
		renderer.setDiscardFlags(discardFlags);
	}

	renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);


#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
	QScreen *screen = QWidget::windowHandle()->screen();
	int resolution = screen->logicalDotsPerInch(); // should be taken from current screen
#else
	QDesktopWidget * deskWidget = QApplication::desktop();
	int screenNr = deskWidget->screenNumber(this);
	QScreen *screen = QGuiApplication::primaryScreen();
	QList<QScreen *> screens = QApplication::screens();
	if (screenNr < screens.count())
		screen = screens[screenNr];
	int resolution = screen->logicalDotsPerInch(); // should be taken from current screen
	//qDebug() << "Using screen #" << screenNr << " with DPI " << resolution;
#endif
	const double mmToInch = 1.0 / 25.4;
	const QSizeF size = sizeMM * mmToInch * resolution;
	const QRectF documentRect( 0.0, 0.0, size.width(), size.height() );
//	qDebug() << "Document size = " << size;

	QString title = m_chart->title().text();
	if (title.isEmpty())
		title = QFileInfo(fname).baseName();

	// set grid pen width in chart model to 0.5 px, and reset after export to 1
	m_chart->prepareForVectorExport(!(m_discardFlags & QwtPlotRenderer::DiscardLegend), m_ui->checkBoxDataReduction->isChecked());

	const QString extension = QFileInfo(fname).suffix();
	const QString fmt = extension.toLower();
	//qDebug() << "Exporting chart to " << fname;
	if ( fmt == "pdf" ) {
		QPdfWriter pdfWriter( fname );
		pdfWriter.setPageSize( QPageSize(sizeMM, QPageSize::Millimeter) );
		pdfWriter.setTitle( title );
		pdfWriter.setPageMargins( QMarginsF() );
		pdfWriter.setResolution( resolution );
#if QT_VERSION >= 0x051000
		pdfWriter.setPdfVersion(QPdfWriter::PdfVersion_A1b);
#endif

		QPainter painter( &pdfWriter );
		renderer.render( m_chart, &painter, documentRect );
	}
	else if (fmt == "svg") {
		QSvgGenerator generator;
		generator.setFileName(fname);
		generator.setSize(size.toSize());
		generator.setViewBox(documentRect);
		QPainter painter;
		painter.begin(&generator);
		renderer.render(m_chart, &painter, documentRect);
		painter.end();
	}
#ifdef SCICHART_HAS_EMFENGINE
	else if (fmt == "emf") {
//		double pdpi = m_chart->physicalDpiX(); // native resolution
//		double ldpi = resolution; // screen resolution
//		double fact = pdpi/ldpi;
//		qDebug() << "EMF Export: physicalDPI = " <<pdpi << " logicalDPI = " << ldpi << " Enlargement factor = " << fact;
//		// render to EMF
//		QSize adaptedSize = fact*size.toSize();
//		qDebug() << "EMF-size (pixel) = " << adaptedSize;
		EmfPaintDevice emf(size.toSize(), fname);
		QPainter p(&emf);
		QRectF documentRect( 0.0, 0.0, size.width(), size.height() ); // must be the same as given to the EmfPaintDevice
		renderer.render(m_chart, &p, documentRect);
//		qDebug() << "Export to EMF file complete";
	}
#endif

	// reset line width in model
	m_chart->resetAfterVectorExport();

	return true;
}


bool Export2VectorWidget::copyToClipboard() {
	// render to EMF and copy to clipboard
	QwtPlotRenderer renderer;
	if (!m_ui->checkBoxNoTransparency->isChecked()) {
		// do not draw plot and canvas background
		renderer.setDiscardFlags(m_discardFlags | QwtPlotRenderer::DiscardCanvasBackground | QwtPlotRenderer::DiscardBackground);
	}
	else {
		QwtPlotRenderer::DiscardFlags discardFlags = m_discardFlags;
		// turn off discard flags for background
		discardFlags = discardFlags & ~QwtPlotRenderer::DiscardBackground;
		renderer.setDiscardFlags(discardFlags);
	}
	renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);

	QSizeF sizeMM(m_ui->lineEditMMX->value(), m_ui->lineEditMMY->value());
	if (sizeMM.width() > 800 || sizeMM.height() > 800) {
		QMessageBox::critical(this, tr("Export chart"), tr("Dimensions too large!"));
		return false;
	}
	updateDimensionHistory();

	QScreen *screen = QGuiApplication::primaryScreen();
	int resolution = screen->logicalDotsPerInch(); // should be taken from current screen
	const double mmToInch = 1.0 / 25.4;
	const QSizeF size = sizeMM * mmToInch * resolution;

	// set grid pen width in chart model to 0.5 px, and reset after export to 1
	m_chart->prepareForVectorExport(!(m_discardFlags & QwtPlotRenderer::DiscardLegend), m_ui->checkBoxDataReduction->isChecked());

#ifdef SCICHART_HAS_EMFENGINE

//	double pdpi = m_chart->physicalDpiX();
//	double ldpi = m_chart->logicalDpiX();
//	double fact = pdpi/ldpi;
//	qDebug() << "EMF Export: physicalDPI = " <<pdpi << " logicalDPI = " << ldpi << " Enlargement factor = " << fact;
	// render to EMF
//	QSize adaptedSize = fact*size.toSize();
//	qDebug() << "EMF-size (pixel) = " << adaptedSize;
	EmfPaintDevice emf(size.toSize());
//	EmfPaintDevice emf(adaptedSize);
	QPainter p(&emf);
//	qDebug() << emf.logicalDpiX() << " " << emf.physicalDpiX();
//	QRectF documentRect( 0.0, 0.0, adaptedSize.width(), adaptedSize.height() );
	QRectF documentRect( 0.0, 0.0, size.width(), size.height() );

	renderer.render(m_chart, &p, documentRect); // will automatically copy to clipboard once QPainter is gone

#else // SCICHART_HAS_EMFENGINE

	QSvgGenerator generator;
	generator.setSize(size.toSize());
	QRectF documentRect( 0,0,size.width(), size.height() );
	generator.setViewBox(documentRect);

	QBuffer b;
	generator.setOutputDevice(&b);
	QPainter painter;
	painter.begin(&generator);
	renderer.render(m_chart, &painter, documentRect);
	painter.end();
	QMimeData * d = new QMimeData();
	d->setData("image/svg+xml",b.buffer());
	QApplication::clipboard()->setMimeData(d);
	QMessageBox::information(this, QString(), tr("Data has been copied to the clipboard."));
#endif // SCICHART_HAS_EMFENGINE

	// reset line width in model
	m_chart->resetAfterVectorExport();

	return true;
}


void Export2VectorWidget::updateDimensions() {
	// conversion factor pix/mm
	bool ok1 = m_ui->lineEditMMX->isValid();
	bool ok2 = m_ui->lineEditMMY->isValid();
	double mmx = m_ui->lineEditMMX->value();
	double mmy = m_ui->lineEditMMY->value();
	// update preview pixmap

	if (ok1 && ok2 && mmy != 0 && m_chart != NULL) {
		// update size and aspect ratio for preview
		m_aspectRatio = (double)mmx/(double)mmy;
		m_ui->widgetVectorPreview->m_aspectRatio = m_aspectRatio;

		// clip to maximum allowed size
		mmx = qMin(800.0, mmx);
		mmy = qMin(800.0, mmy);
		m_ui->widgetVectorPreview->m_sizeMM = QSizeF(mmx,mmy);

		emit exportButtonsEnabled(true);
		m_ui->widgetVectorPreview->m_chart = m_chart; // enables preview
	}
	else {
		emit exportButtonsEnabled(false);
		m_ui->widgetVectorPreview->m_chart = NULL; // disables preview
	}
	m_ui->widgetVectorPreview->update();
}


void Export2VectorWidget::on_lineEditMMX_editingFinishedSuccessfully() {
	if (m_ui->checkBoxAspectRatio->isChecked()) {
		m_ui->lineEditMMY->blockSignals(true);
		double mmx = m_ui->lineEditMMX->value();
		if (m_aspectRatio != 0) {
			double mmy = mmx/m_aspectRatio;
			m_ui->lineEditMMY->setText( QString("%L1").arg(mmy ) );
		}
		m_ui->lineEditMMY->blockSignals(false);
	}
	updateDimensions();
}

void Export2VectorWidget::on_lineEditMMY_editingFinishedSuccessfully() {
	// if aspect ratio is set, fix also x mm
	if (m_ui->checkBoxAspectRatio->isChecked()) {
		m_ui->lineEditMMX->blockSignals(true);
		double mmy = m_ui->lineEditMMY->value();
		double mmx = mmy*m_aspectRatio;
		m_ui->lineEditMMX->setText( QString("%L1").arg(mmx ) );
	}
	m_ui->lineEditMMX->blockSignals(false);
	updateDimensions();
}


void Export2VectorWidget::on_checkBoxDataReduction_toggled(bool checked) {
	m_ui->widgetVectorPreview->m_useDataReduction = checked;
	m_ui->widgetVectorPreview->update();
}


void Export2VectorWidget::on_comboBoxLastPlotDimensions_currentIndexChanged(int index) {
	if (m_dimensionHistory == nullptr)
		return;
	if (index == 0)
		return;
	Q_ASSERT(index-1 < m_ui->comboBoxLastPlotDimensions->count());
	m_ui->lineEditMMX->blockSignals(true);
	m_ui->lineEditMMY->blockSignals(true);
	m_ui->lineEditMMX->setText( (*m_dimensionHistory)[index-1].xMM );
	m_ui->lineEditMMY->setText( (*m_dimensionHistory)[index-1].yMM );
	m_ui->lineEditMMX->blockSignals(false);
	m_ui->lineEditMMY->blockSignals(false);
	updateDimensions();
}

} // namespace SCI



