#ifndef SCI_MARKER_H
#define SCI_MARKER_H

#include <QObject>
#include <QFont>
#include <QPen>
#include <QString>

namespace SCI {

/*! Stores all data for a plot marker (horizontal/vertical line or symbol). */
class Marker {
public:
	enum MarkerType {
		MT_Invisible,
		MT_HLine,
		MT_VLine,
		MT_Cross
	};

	MarkerType		m_markerType = MT_VLine;
	double			m_xPos = 0;
	double			m_yPos = 0;

	int				m_xAxisID = 2; // = QwtPlot::xBottom
	int				m_yAxisID = 0; // = QwtPlot::yLeft

	QString			m_label;
	QFont			m_labelFont;
	Qt::Alignment	m_labelAlignment = Qt::AlignLeft | Qt::AlignBottom;
	Qt::Orientation	m_labelOrientation = Qt::Vertical;

	int				m_spacing = 8;

	QPen			m_pen;

	/*! How far on top the marker should be plotted (higher values, higher placement) */
	int				m_zOrder = 90;
};

} // namespace SCI

// Declare as meta type so we can pass SCI::Marker through signal-slot connections
Q_DECLARE_METATYPE(SCI::Marker)

#endif // SCI_MARKER_H
