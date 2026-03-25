#include "QtExt_ReportPreviewPageWidget.h"
#include "ui_QtExt_ReportPreviewPageWidget.h"

#include "QtExt_ReportPreview.h"

namespace QtExt {

ReportPreviewPageWidget::ReportPreviewPageWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ReportPreviewPageWidget)
{
	ui->setupUi(this);
	m_preview = dynamic_cast<QtExt::ReportPreview*>(parent);
	Q_ASSERT(m_preview != nullptr);
//	m_settings = m_preview->m_reportSettings;
}

ReportPreviewPageWidget::~ReportPreviewPageWidget()
{
	delete ui;
}

void ReportPreviewPageWidget::setSettings(ReportSettingsBase* settings) {
	m_settings = settings;
	emit settingsChanged();
}

} // namespace QtExt
