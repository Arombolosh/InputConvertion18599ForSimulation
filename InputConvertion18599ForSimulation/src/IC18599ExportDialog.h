#ifndef IC18599ExportDialogH
#define IC18599ExportDialogH

#include <QDialog>

#include <vector>

class QCheckBox;

/*! Dialog for selecting which profiles to include in the PDF report. */
class IC18599ExportDialog : public QDialog {
	Q_OBJECT
public:
	explicit IC18599ExportDialog(const QStringList &profileNames, QWidget *parent = nullptr);

	/*! Returns the list of profile names that were checked by the user. */
	QStringList selectedProfiles() const;

private slots:
	void onCheckAll();
	void onUncheckAll();

private:
	std::vector<QCheckBox*>		m_checkBoxes;
};

#endif // IC18599ExportDialogH
