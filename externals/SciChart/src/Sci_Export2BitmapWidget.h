#ifndef Sci_Export2BitmapWidgetH
#define Sci_Export2BitmapWidgetH

#include <QWidget>

#include <qwt_plot_renderer.h>

namespace SCI {

namespace Ui {
	class Export2BitmapWidget;
}

class Chart;

/*! This widget handles everything related to bitmap export.
	In addition to the user interface stuff it provides
	convenience static functions export2Clipboard() and
	export2File() that exports the bitmap while providing
	all parameters are arguments.
	This function can be used without instantiating the widget.
*/
class Export2BitmapWidget : public QWidget {
	Q_OBJECT
public:
	explicit Export2BitmapWidget(QWidget *parent = 0);
	~Export2BitmapWidget();

	/*! Set the chart to export and update the size settings.*/
	void setChart(Chart * chart);

	/*! Sets dimensions in line edits. */
	void setDimension(int pxX, int pxY, int resolution);

	/*! Sets currently used discard flags. */
	void setDiscardFlags(QwtPlotRenderer::DiscardFlags discardFlags);

	/*! Called from ExportDialog to save the file, using the currently specified options from the export widget.
		\param fname Full path to export file (with valid extension).
		\return Returns true, if file could be written successfully. Otherwise the dialog pops up an error
			message.
	*/
	bool saveToFile(const QString & fname);

	/*! Copies the currently shown bitmap to the clipboard. */
	bool copyToClipboard();


signals:
	/*! Emitted when input has changed such that buttons shall be disabled/enabled. */
	void exportButtonsEnabled(bool enabled);

private slots:
	/*! Updates dimensions in input widgets and updates preview image. */
	void updateDimensions();

	void on_spinBox_resolution_valueChanged(int arg1);
	void on_lineEditPixelX_editingFinished();
	void on_lineEditPixelY_editingFinished();
	void on_lineEditMMX_editingFinished();
	void on_lineEditMMY_editingFinished();
	void on_radioButtonPixels_toggled(bool checked);

private:
	Ui::Export2BitmapWidget			*m_ui;
	SCI::Chart						*m_chart;

	/*! Discard flags to be used for preview and exported bitmap. */
	QwtPlotRenderer::DiscardFlags	m_discardFlags;

	/*! Cached aspect ratio, updated in updateDimensions() and initialized
		with the chart size.
	*/
	double							m_aspectRatio;

	friend class ExportDialog;
};


} // namespace SCI
#endif // Sci_Export2BitmapWidgetH
