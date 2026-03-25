#ifndef IC18599ReportSettingsH
#define IC18599ReportSettingsH

#include <QtExt_ReportSettingsBase.h>
#include <QtExt_ReportUtilities.h>

// Convenience macros for formatted titles
#define TITLE_H1(x) QtExt::htmlCaptionFormat(x, QtExt::FT_Header1)
#define TITLE_H2(x) QtExt::htmlCaptionFormat(x, QtExt::FT_Header2)
#define TITLE_H3(x) QtExt::htmlCaptionFormat(x, QtExt::FT_Header3)

#define BOLD(x)  "<b>" + QString(x) + "</b>"

// Spacing factors for title distances
#define UPPERFACTOR 1.0
#define LOWERFACTOR 0.5

#define TOP_DIST_H1 UPPERFACTOR * m_report->m_textProperties.H1().ascent
#define BOTTOM_DIST_H1 LOWERFACTOR * m_report->m_textProperties.H1().ascent
#define TOP_DIST_H3 UPPERFACTOR * m_report->m_textProperties.H3().ascent
#define BOTTOM_DIST_H3 LOWERFACTOR * m_report->m_textProperties.H3().ascent


class IC18599ReportSettings : public QtExt::ReportSettingsBase {
public:
	enum ReportFrameType {
		FrameHeader = 0,
		FrameFooter = 1,
		FrameTitlePage = 2,
		FrameContent = 3,
		FrameCount
	};

	IC18599ReportSettings() {
		// Enable all frames by default
		for (int i = m_frameIdStart; i < FrameCount; ++i)
			m_frames.insert(i);
	}
};

#endif // IC18599ReportSettingsH
