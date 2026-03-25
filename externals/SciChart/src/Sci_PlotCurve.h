/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PlotCurveH
#define Sci_PlotCurveH


#include <qwt_plot_curve.h>

namespace SCI {

class SeriesDataEmpty;

/*! \brief Wrapper class for QwtPlotCurve.
	Some functions of QwtPlotCurve are reimplemented to provide additional functionality, for example
	gaps in the data series due to NaN values.

	This class uses only rawData access. All other setData() functions of QwtPlotCurve are disabled.

	It implements addionally a locking mechanism,  \see lockData() and \see unlockData().
	This locks access to internal data structure in order to avoid access to unvalid pointers.

	\note This function never accesses the chart model data.
*/
class PlotCurve : public QwtPlotCurve {
public:

	/*! Standard constructor.*/
	PlotCurve();

	/*! Constructor that sets the plot title as QwtText.
		\param title
	*/
	explicit PlotCurve(const QwtText &title);

	/*! Constructor that sets the plot title as QString. */
	explicit PlotCurve(const QString &title);

	/*! Destructor, cleans up m_emptyData. */
	~PlotCurve() override;

	/*! \brief Initialize the data by pointing to memory blocks which are not managed by QwtPlotCurve.

		setRawSamples() is provided for efficiency. It is important to keep the pointers
		during the lifetime of the underlying QwtCPointerData class.

		\param x pointer to x data
		\param y pointer to y data
		\param size size of x and y

		\note Internally the data is stored in a QwtCPointerData object
	*/
	void setRawSamples(const double *x, const double *y, int size);

	/*! \brief Initialize the data by pointing to memory blocks which are not managed
		by QwtPlotCurve.

		setRawSamples() is provided for efficiency. It is important to keep the pointers
		during the lifetime of the underlying QwtCPointerData class.

		\param x pointer to x data
		\param y pointer to y data
		\param size size of x and y

		\note Internally the data is stored in a QwtCPointerData object

		This is essentially an overload of the function above, with additional error checking and
		an error string that is returned on error.
	*/
	bool setRawSamples(const double *x, const double *y, int size, std::string& errstr);

	/*! \return Returns the curve data (non-const version). If series is locked, a dummy data set is returned. */
	QwtSeriesData<QPointF>* data();

	/*! \return Returns the curve data (const version).  If series is locked, a dummy data set is returned. */
	const QwtSeriesData<QPointF>* data() const;

	/*! Return the size of the data arrays.
		\sa setData()
		\return Returns size of data, or 0 when curve is locked.
	*/
	size_t dataSize() const override;

	/*! \param i index
		\return x-value at position i, or 0 when curve is locked.
	*/
	double x(int i) const;

	/*! \param i index
		\return y-value at position i, or 0 when curve is locked.
	*/
	double y(int i) const;

	/*! Returns the bounding rectangle of the curve data. If there is
		no bounding rect, like for empty data the rectangle is invalid.
		\sa QwtData::boundingRect(), QRectF::isValid()
	*/
	QRectF boundingRect() const override;

	/*! Overloaded to implement support for gaps (NaN values) in data set. */
	void drawSeries( QPainter *,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		const QRectF &canvasRect, int from, int to ) const override;

	/*! Overloaded to implement support for gaps (NaN values) and marker_distance property. */
	void drawSymbols( QPainter *painter, const QwtSymbol &symbol,
		const QwtScaleMap &xMap, const QwtScaleMap &yMap,
		const QRectF &canvasRect, int from, int to ) const override;

	/*! Lock the data access.*/
	virtual void lockData();

	/*! Unlock the data access.*/
	virtual void unlockData();

	/*! Returns the internal lock counter.*/
	int lockCount() const { return m_lockCounter; }

	/*! Sets the NaN-check flag. Necessary if NaN values are possible (e.g. profile charts).*/
	void setNaNCheck(bool check) { m_nanCheck = check; }

protected:
	/*! Re-implemented from QwtPlotItem::trackerValueText(), provides item text in tracker with
		series title and value.
	*/
	QString trackerValueText(int, double value) const override;
	/*! Adjusts alignment of generated text. */
	QwtText trackerInfoAt( int attributes, const QPointF& pos ) const override;

private:
	/*! Set data function set to private.*/
	void setData(const double * /*xData*/, const double * /*yData*/, int /*size*/) {
		Q_ASSERT_X(false, "SCI::PlotCurve::setData", "Do not call this! Method disabled by private overload.");
	}
	/*! Set data function set to private.*/
	void setData(const QVector<double> &/*xData*/, const QVector<double> &/*yData*/) {
		Q_ASSERT_X(false, "SCI::PlotCurve::setData", "Do not call this! Method disabled by private overload.");
	}
#if QT_VERSION < 0x040000
	/*! Set data function set to private.*/
	void setData(const QVector<QPointF> &/*data*/) {
		Q_ASSERT_X(false, "SCI::PlotCurve::setData", "Do not call this! Method disabled by private overload.");
	}
#else
	/*! Set data function set to private.*/
	void setData(const QPolygonF &/*data*/) {
		Q_ASSERT_X(false, "SCI::PlotCurve::setData", "Do not call this! Method disabled by private overload.");
	}
#endif
	/*! Set data function set to private.*/
	void setData(const QwtSeriesData<QPointF>* /*data*/) {
		Q_ASSERT_X(false, "SCI::PlotCurve::setData", "Do not call this! Method disabled by private overload.");
	}

	/*! Counts the number currents locks.
		It will be increased from lockData and decreased from unlockData.
		\see lockCount.
	*/
	int	m_lockCounter;

	/*! Controls if a check for internal NaNs will be done.*/
	bool	m_nanCheck;

	SeriesDataEmpty*	m_emptyData;

};

} // namespace SCI

#endif // Sci_PlotCurveH
