#ifndef IC18599ReportFrameToCH
#define IC18599ReportFrameToCH

#include <QtExt_ReportFrameLongTable.h>

#include <QCoreApplication>
#include <QString>

#include <vector>

class IC18599ReportFrameToC : public QtExt::ReportFrameLongTable {
	Q_DECLARE_TR_FUNCTIONS(IC18599ReportFrameToC)
public:
	IC18599ReportFrameToC(QtExt::Report *report, QTextDocument *textDocument,
						  const std::vector<QString> &profileNames,
						  int nonResidentialStartIndex);

	void update(QPaintDevice *paintDevice, double width) override;
	void configureHeading() override;
	void configureTable() override;

private:
	std::vector<QString>	m_profileNames;
	int						m_nonResStartIdx;
	double					m_tableWidth = 0;
};

#endif // IC18599ReportFrameToCH
