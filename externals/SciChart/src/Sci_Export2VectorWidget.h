#ifndef Sci_Export2VectorWidgetH
#define Sci_Export2VectorWidgetH

#include <QWidget>
#include <QDateTime>

#include <qwt_plot_renderer.h>

namespace SCI {

namespace Ui {
	class Export2VectorWidget;
}

class Chart;

/*! The widget that holds all export options related to vector export.
	In addition to the user interface stuff it provides
	convenience static functions export2Clipboard() and
	export2File() that exports the vector image while providing
	all parameters are arguments.
	This function can be used without instantiating the widget.
*/
class Export2VectorWidget : public QWidget {
	Q_OBJECT
public:

	struct DimensionHistory {
		QString xMM;
		QString yMM;
		QDateTime lastUsed;
	};

	explicit Export2VectorWidget(QWidget *parent = 0);
	~Export2VectorWidget();

	/*! Set the chart to export and update the size settings.*/
	void setChart(Chart * chart);

	/*! Sets dimensions in line edits. */
	void setDimension(double mmX, double mmY);

	/*! Pointer to persistent string list is taken, but no ownership transfer!. */
	void setDimensionHistory(QList<DimensionHistory> * dimensionHistory);

	/*! Sets currently used discard flags. */
	void setDiscardFlags(QwtPlotRenderer::DiscardFlags discardFlags);

	/*! Takes current mm input and appends that to the dimension history. */
	void updateDimensionHistory();

	/*! Called from ExportDialog to save the file, using the currently specified options from the export widget.
		\param fname Full path to export file (with valid extension).
		\return Returns true, if file could be written successfully. Otherwise the dialog pops up an error
			message.
	*/
	bool saveToFile(const QString & fname);

	/*! Copies the currently diagram to the clipboard (EMF on Windows, SVG on Linux/Mac). */
	bool copyToClipboard();

signals:
	/*! Emitted when input has changed such that buttons shall be disabled/enabled. */
	void exportButtonsEnabled(bool enabled);

private slots:
	void on_lineEditMMX_editingFinishedSuccessfully();

	void on_lineEditMMY_editingFinishedSuccessfully();

	void on_checkBoxDataReduction_toggled(bool checked);

	void on_comboBoxLastPlotDimensions_currentIndexChanged(int index);

private:
	/*! Updates dimensions in input widgets and updates preview image. */
	void updateDimensions();

	Ui::Export2VectorWidget			*m_ui;
	SCI::Chart						*m_chart;

	/*! Discard flags to be used for preview and exported bitmap. */
	QwtPlotRenderer::DiscardFlags	m_discardFlags;

	/*! Cached aspect ratio, updated in updateDimensions() and initialized
		with the chart size.
	*/
	double							m_aspectRatio;

	QList<DimensionHistory>			*m_dimensionHistory;

	friend class ExportDialog;
};


} // namespace SCI
#endif // Sci_Export2VectorWidgetH
