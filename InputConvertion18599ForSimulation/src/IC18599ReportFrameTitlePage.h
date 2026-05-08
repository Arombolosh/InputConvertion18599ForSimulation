#ifndef IC18599ReportFrameTitlePageH
#define IC18599ReportFrameTitlePageH

#include <QtExt_ReportFrameBase.h>
#include <QtExt_TextFrame.h>

#include <QCoreApplication>

class IC18599ReportFrameTitlePage : public QtExt::ReportFrameBase {
	Q_DECLARE_TR_FUNCTIONS(IC18599ReportFrameTitlePage)
public:
	IC18599ReportFrameTitlePage(QtExt::Report *report, QTextDocument *textDocument);

	void update(QPaintDevice *paintDevice, double width) override;

private:
	QtExt::TextFrame		m_title;
	QtExt::TextFrame		m_subtitle;
};

#endif // IC18599ReportFrameTitlePageH
