#ifndef QTEXT_ReportPreviewPageWidgetH
#define QTEXT_ReportPreviewPageWidgetH

#include <QWidget>

#include "QtExt_global.h"

namespace QtExt {

namespace Ui {
class ReportPreviewPageWidget;
}

class ReportSettingsBase;
class ReportPreview;

class QtExt_EXPORT ReportPreviewPageWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ReportPreviewPageWidget(QWidget *parent = nullptr);
	~ReportPreviewPageWidget();

	/*! Set the report settings for additional properties.*/
	void setSettings(QtExt::ReportSettingsBase* settings);

signals:

	/*! Can be sent in case the visibility of a frame is changed.*/
	void visibilityChanged(int frameType, bool visible);

	/*! Must be emitted in case of a change of the report settings.*/
	void reportSettingsChanged();

	/*! Will be emitted in cas of changed report settings.
		This can only be done by calling setSettings.
		Derived classe can use this signal in order to update their own version of settings.
	*/
	void settingsChanged();

protected:
	QtExt::ReportSettingsBase*	m_settings;
	ReportPreview*				m_preview;

private:
	Ui::ReportPreviewPageWidget *ui;

};


} // namespace QtExt
#endif // QTEXT_ReportPreviewPageWidgetH
