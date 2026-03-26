#ifndef IC18599ReportFrameTitlePageH
#define IC18599ReportFrameTitlePageH

#include <QtExt_ReportFrameBase.h>
#include <QtExt_TextFrame.h>
#include <QtExt_Table.h>

#include <QCoreApplication>
#include <QString>

#include <vector>

class IC18599ReportFrameTitlePage : public QtExt::ReportFrameBase {
	Q_DECLARE_TR_FUNCTIONS(IC18599ReportFrameTitlePage)
public:
	IC18599ReportFrameTitlePage(QtExt::Report *report, QTextDocument *textDocument,
								const std::vector<QString> &profileNames);

	void update(QPaintDevice *paintDevice, double width) override;

private:
	QtExt::TextFrame		m_title;
	QtExt::TextFrame		m_subtitle;
	QtExt::Table			m_infoTable;
	QtExt::TextFrame		m_tocHeading;
	QtExt::Table			m_tocTable;
	std::vector<QString>	m_profileNames;
};

#endif // IC18599ReportFrameTitlePageH
