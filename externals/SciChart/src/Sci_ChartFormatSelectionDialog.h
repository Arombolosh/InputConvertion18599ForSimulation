#ifndef SCI_ChartFormatSelectionDialogH
#define SCI_ChartFormatSelectionDialogH

#include <QDialog>
#include <QSet>

namespace SCI {

namespace Ui {
	class ChartFormatSelectionDialog;
}

/*! Dialog that allows users to select different properties from another model
	to transfer to the target model.
*/
class ChartFormatSelectionDialog : public QDialog {
	Q_OBJECT

public:
	enum SettingsToTransfer {
		OptionGeneral,
		OptionLegend,
		OptionAxisX,
		OptionAxisYLeft,
		OptionAxisYRight,
		OptionLineProperties,
		OptionColorMapProperties
	};

	explicit ChartFormatSelectionDialog(QWidget *parent = 0);
	~ChartFormatSelectionDialog();

	/*! Configures the dialog to enable line or color map options. */
	void setLineFormatOptionEnabled(bool enabled);

	/*! Returns a set with all options that were checked by the user. */
	QSet<SettingsToTransfer> selectedOptions() const;

private slots:
	void on_checkBoxAll_toggled(bool checked);

	void onCheckboxToggled(bool);

private:
	Ui::ChartFormatSelectionDialog *m_ui;
};

} // namespace SCI

#endif // SCI_ChartFormatSelectionDialogH
