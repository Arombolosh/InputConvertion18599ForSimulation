#include "Sci_Export2BitmapWidget.h"
#include "ui_Sci_Export2BitmapWidget.h"


#include <QClipboard>
#include <QFileDialog>
#include <QImage>
#include <QMessageBox>
#include <QPainter>

#include <qwt_plot_renderer.h>

#include "Sci_Chart.h"
#include "Sci_ExportDialog.h"

namespace SCI {

Export2BitmapWidget::Export2BitmapWidget(QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::Export2BitmapWidget),
	m_chart(NULL)
{
	m_ui->setupUi(this);

	m_ui->tab->layout()->setContentsMargins(0,0,0,0);
	m_ui->tab_2->layout()->setContentsMargins(0,0,0,0);
	m_ui->scrollAreaWidgetContents->layout()->setContentsMargins(0,0,0,0);

	m_ui->lineEditPixelX->setup(0, std::numeric_limits<double>::max(), tr("A positive number is required."));
	m_ui->lineEditPixelY->setup(0, std::numeric_limits<double>::max(), tr("A positive number is required."));
	m_ui->lineEditPixelX->setAcceptOnlyInteger(true);
	m_ui->lineEditPixelY->setAcceptOnlyInteger(true);
	m_ui->lineEditMMX->setup(0, std::numeric_limits<double>::max(), tr("A positive number is required."));
	m_ui->lineEditMMY->setup(0, std::numeric_limits<double>::max(), tr("A positive number is required."));

	connect(m_ui->checkBoxApplyDPI, &QCheckBox::toggled, this, &Export2BitmapWidget::updateDimensions);
}


Export2BitmapWidget::~Export2BitmapWidget() {
	delete m_ui;
}


void Export2BitmapWidget::setChart(SCI::Chart * chart) {
	if (m_chart == NULL) {
		// preset pixel size with current chart size, though this is likely to
		// be modified by the user
		m_ui->radioButtonPixels->setChecked(true);
		on_radioButtonPixels_toggled(m_ui->radioButtonPixels->isChecked());
		m_ui->lineEditPixelX->setText(QString("%1").arg(chart->size().width()));
		m_ui->lineEditPixelY->setText(QString("%1").arg(chart->size().height()));
	}
	m_chart = chart;
	updateDimensions();
}

void Export2BitmapWidget::setDimension(int pxX, int pxY, int resolution)
{
	m_ui->lineEditPixelX->setText( QString("%L1").arg(pxX) );
	m_ui->lineEditPixelY->setText( QString("%L1").arg(pxY) );
	m_ui->spinBox_resolution->setValue(resolution);
}


void Export2BitmapWidget::setDiscardFlags(QwtPlotRenderer::DiscardFlags discardFlags) {
	m_discardFlags = discardFlags;
	updateDimensions(); // updates the preview
}


bool Export2BitmapWidget::saveToFile(const QString & fname) {
	// save to file
	const QPixmap & pm = m_ui->labelPreviewUnscaled->pixmap(Qt::ReturnByValue);
	if (pm.save(fname))
		return true;
	else {
		QMessageBox::critical(this, tr("Export error"), tr("Error writing file '%1.").arg(fname));
		return false;
	}
}


bool Export2BitmapWidget::copyToClipboard() {
	QApplication::clipboard()->setPixmap(m_ui->labelPreviewUnscaled->pixmap(Qt::ReturnByValue));
	QMessageBox::information(this, QString(), tr("Image has been copied to the clipboard."));
	return true;
}


void Export2BitmapWidget::updateDimensions() {
	unsigned int dpi = m_ui->spinBox_resolution->value();
	// conversion factor pix/mm
	double pix_mm = dpi/25.4;
	if (m_ui->radioButtonPixels->isChecked()) {
		bool ok1, ok2;
		unsigned int ptx = m_ui->lineEditPixelX->text().toInt(&ok1);
		if (ok1)
			m_ui->lineEditMMX->setText( QString("%L1").arg(ptx/pix_mm) );
		unsigned int pty = m_ui->lineEditPixelY->text().toInt(&ok2);
		if (ok2)
			m_ui->lineEditMMY->setText( QString("%L1").arg(pty/pix_mm) );
		if (ok1 && ok2 && pty != 0)
			m_aspectRatio = (double)ptx/(double)pty;
	}
	else {
		bool ok1 = m_ui->lineEditMMX->isValid();
		bool ok2 = m_ui->lineEditMMY->isValid();
		double mmx = m_ui->lineEditMMX->value();
		if (ok1)
			m_ui->lineEditPixelX->setText( QString("%1").arg((int)(mmx*pix_mm)) );
		double mmy = m_ui->lineEditMMY->value();
		if (ok2)
			m_ui->lineEditPixelY->setText( QString("%1").arg((int)(mmy*pix_mm)) );
		if (ok1 && ok2 && mmy != 0)
			m_aspectRatio = (double)mmx/(double)mmy;
	}

	// update preview pixmap
	emit exportButtonsEnabled(false);

	unsigned int ptx, pty;
	if (m_ui->lineEditPixelX->isValid() && m_ui->lineEditPixelY->isValid() && m_chart != NULL) {
		ptx = (unsigned int)m_ui->lineEditPixelX->value();
		pty = (unsigned int)m_ui->lineEditPixelY->value();
		unsigned long maxNum = (unsigned long)1200*1920*1080;
		if ((unsigned long)ptx*(unsigned long)pty > maxNum) { // full HD, 1200 DPI
			m_ui->labelPreviewScaled->setPixmap(QPixmap());
			m_ui->labelPreviewScaled->setText(tr("Bitmap too large to preview."));
			m_ui->labelPreviewUnscaled->setPixmap(QPixmap());
			m_ui->labelPreviewUnscaled->setText(tr("Bitmap too large to preview."));
			return;
		}
		m_ui->labelPreviewScaled->setText("");
		m_ui->labelPreviewUnscaled->setText("");

		// create pixmap with desired size
		QwtPlotRenderer renderer; // the renderer
		renderer.setDiscardFlags(m_discardFlags);
		// TODO : make this option style-dependent
		renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
		QRect imageRect( 0.0, 0.0, ptx, pty ); // target size in pixels
		QImage image( imageRect.size(), QImage::Format_ARGB32 );
		image.fill( Qt::white ); // fill with uniform background color, usually white
		if (m_ui->checkBoxApplyDPI->isChecked()) {
			const double mmToInch = 1.0 / 25.4;
			const int dotsPerMeter = qRound( dpi * mmToInch * 1000.0 );
			image.setDotsPerMeterX( dotsPerMeter );
			image.setDotsPerMeterY( dotsPerMeter );
		}
		QPainter painter( &image );
		m_chart->prepareForBitmapExport(!(m_discardFlags & QwtPlotRenderer::DiscardLegend));
		renderer.render( m_chart, &painter, imageRect );
		m_chart->resetAfterBitmapExport();
		painter.end();

		m_ui->labelPreviewScaled->setPixmap( QPixmap::fromImage(image));
		m_ui->labelPreviewUnscaled->setPixmap( QPixmap::fromImage(image));
		emit exportButtonsEnabled(true);
	}
	else {
		m_ui->labelPreviewScaled->setPixmap(QPixmap());
		m_ui->labelPreviewUnscaled->setPixmap(QPixmap());
	}
}


void Export2BitmapWidget::on_spinBox_resolution_valueChanged(int) {
	updateDimensions();
}


void Export2BitmapWidget::on_lineEditPixelX_editingFinished() {
	// if aspect ratio is set, fix also y pixels
	if (m_ui->checkBoxAspectRatio->isChecked()) {
		m_ui->lineEditPixelY->blockSignals(true);
		bool ok1;
		unsigned int ptx = m_ui->lineEditPixelX->text().toInt(&ok1);
		if (ok1 && m_aspectRatio != 0) {
			unsigned int pty = ptx/m_aspectRatio;
			m_ui->lineEditPixelY->setText( QString("%1").arg(pty ) );
		}
		m_ui->lineEditPixelY->blockSignals(false);
	}
	updateDimensions();
}


void Export2BitmapWidget::on_lineEditPixelY_editingFinished() {
	// if aspect ratio is set, fix also y pixels
	if (m_ui->checkBoxAspectRatio->isChecked()) {
		m_ui->lineEditPixelX->blockSignals(true);
		bool ok1;
		unsigned int pty = m_ui->lineEditPixelY->text().toInt(&ok1);
		if (ok1) {
			unsigned int ptx = pty*m_aspectRatio;
			m_ui->lineEditPixelX->setText( QString("%1").arg(ptx ) );
		}
		m_ui->lineEditPixelX->blockSignals(false);
	}
	updateDimensions();
}


void Export2BitmapWidget::on_lineEditMMX_editingFinished() {
	// if aspect ratio is set, fix also y mm
	if (m_ui->checkBoxAspectRatio->isChecked()) {
		m_ui->lineEditMMY->blockSignals(true);
		bool ok1 = m_ui->lineEditMMX->isValid();
		double mmx = m_ui->lineEditMMX->value();
		if (ok1 && m_aspectRatio != 0) {
			double mmy = mmx/m_aspectRatio;
			m_ui->lineEditMMY->setText( QString("%L1").arg(mmy ) );
		}
		m_ui->lineEditMMY->blockSignals(false);
	}
	updateDimensions();
}


void Export2BitmapWidget::on_lineEditMMY_editingFinished() {
	// if aspect ratio is set, fix also x mm
	if (m_ui->checkBoxAspectRatio->isChecked()) {
		m_ui->lineEditMMX->blockSignals(true);
		bool ok1 = m_ui->lineEditMMY->isValid();
		double mmy = m_ui->lineEditMMY->value();
		if (ok1) {
			double mmx = mmy*m_aspectRatio;
			m_ui->lineEditMMX->setText( QString("%L1").arg(mmx ) );
		}
		m_ui->lineEditMMX->blockSignals(false);
	}
	updateDimensions();
}


void Export2BitmapWidget::on_radioButtonPixels_toggled(bool checked) {
	m_ui->lineEditPixelX->setReadOnly(!checked);
	m_ui->lineEditPixelY->setReadOnly(!checked);
	m_ui->lineEditMMX->setReadOnly(checked);
	m_ui->lineEditMMY->setReadOnly(checked);
}



} // namespace SCI


