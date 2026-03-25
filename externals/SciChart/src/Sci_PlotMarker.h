#ifndef SCI_PLOTMARKER_H
#define SCI_PLOTMARKER_H

#include <QObject>
#include <qwt_plot_marker.h>

namespace SCI {

class Marker;

/*! Wrapper class around QwtPlotMarker. */
class PlotMarker : public QwtPlotMarker {
public:

	void setProperties(const Marker & m);
};

} // namespace SCI

#endif // SCI_PLOTMARKER_H
