/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ChartAxisH
#define Sci_ChartAxisH

#include <QObject>

#include <qwt_text.h>

#include <IBK_Unit.h>

#include "Sci_Globals.h"

class QString;
class QFont;

namespace SCI {

class Chart;

/*! \brief The class SCI::ChartAxis contains all variables and functions for creating and
	mainpulating of an axis of SCI::Chart.

	This class is a helper class for SCI::Chart that takes model data and applies it to the QwtPlotAxis.
	This class does not modify the chart model properties.
*/
class ChartAxis : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(ChartAxis)
public:
	/*! Enum for axis type. */
	enum AxisType { UnknownAxis, LeftAxis, RightAxis, BottomAxis, TopAxis, DepthAxis, CustomAxis };

	/*! Returns true if the axis is visible. */
	bool isVisible() const;

	/*! Returns the axis title as QString, may contain the $unit placeholder. */
	QString title() const;

	/*! returns true if axis scaling is logarithmic. */
	bool isLogarithmic() const;

	/*! Returns the maximum axis value (scale). */
	double maximum() const;

	/*! Returns the minimum axis value (scale). */
	double minimum() const;

	/*! Returns the maximum number of major ticks. */
	int axisMaxMajor() const;

	/*! Returns the maximum number of minor ticks. */
	int axisMaxMinor() const ;

	/*! Returns true if the the axis have an automatic scaling. */
	bool isAutoScaleEnabled() const;

	/*! Returns the chart that contains this axis. */
	Chart* chart() const { return m_chart; }

	/*! Returns true if a series is attached to this axis.*/
	bool hasSeriesAttached() const;

	/*! Returns the font used for axis title.*/
	QFont titleFont() const;

	/*! Returns the font used for axis lables.*/
	QFont labelFont() const;

	/*! Return the distance between labels and title.*/
	int titleSpacing() const;

	/*! Return the distance between labels and axis.*/
	int labelSpacing() const;

	/*! Returns the label for the given value created from used scale draw.
		Can be used for tracker or zoom text.
		\param value Value
		\param formatBack Number of levels lower than normal, use 0 for same accuracy as axis,
			higher numbers mean more detail.
	*/
	QwtText label(double value, int formatBack = 0) const;

	/*! Returns true if axis has DateTime scaling.*/
	bool isDateTime() const;

private:
	/*! Constructor that initializes the ChartAxis instance.
		\param chart Chart that contains this axis.
		\param axisId Type of the axis (see QwtPlot::Axis).
	*/
	ChartAxis(Chart* chart, int axisId);

	/*! Sets the visibility of the axis. */
	void setVisible(bool visible);

	/*! Sets a new axis title.
		\param str Title string.
		Note: there will be no placeholder substitution!
	*/
	void setTitle(QString str);

	/*! Specify the distance between chart border and backbone of axis scales.
		\note Only has effect if there is a title - if space shall be adjusted even without title, either use
			margin or set a space as title. If spacing is smaller than necessary space for title and scale labels,
			the spacing is increased accordingly.
		\param spacing Spacing
		\sa titleSpacing()
	*/
	void setTitleSpacing(int spacing);

	/*! Sets y2 axis title inverted. */
	void setTitleInverted(bool inverted);

	/*! Specify the distance between labels and axis.
		\param spacing Spacing
		\sa labelSpacing()
	*/
	void setLabelSpacing(int spacing);

	/*! Set a logarithmic axis scaling. */
	void setLogarithmic();

	/*! Set a linear axis scaling. */
	void setLinear();

	/*! Sets a new maximum scaling value. */
	void setMaximum( double);

	/*! Sets a new minimum scaling value. */
	void setMinimum( double);

	/*! Sets a new maximum and minimum.*/
	void setMinMax(double min, double max);

	/*! Sets the maximum number of major ticks. */
	void setAxisMaxMajor( int maxMajor ) ;

	/*! Sets the maximum number of minor ticks. */
	void setAxisMaxMinor( int maxMinor );

	/*! Sets the axis scaling to automatic.
	  This is switched of if setMinimum or setMaximum is called.
	*/
	void setAutoScaleEnabled(bool enabled);

	/*! Sets a font for axis title.*/
	void setTitleFont(const QFont&);

	/*! Sets a font for axis lables.*/
	void setLabelFont(const QFont&);

	/*! Sets the axis label alignment. */
	void setAxisLabelAlignment(int Halign, int Valign);

	/*! Sets the axis label rotaion. */
	void setAxisLabelRotation(double rotation);

	/*! Switch between normal and DateTime scaling.*/
	void setDateTime(bool dateTime);

	/*! Set time specification in case of a DateTime axis. Otherwise do nothing.*/
	void setDateTimeSpec(Qt::TimeSpec timeSpec);

	/*! Set format strings for time inetrvals in case of a DateTime axis. Otherwise do nothing.*/
	void setDateTimeFormats(const QStringList& formats);

	Chart*					m_chart;		///< Parent chart.
	bool					m_globMaxMin;	///< For future use in profile charts.
	int						m_axisId;		///< Type of the axis (of Qwt plot).
	bool					m_sqrtTime;		///< For future use. For axis with square root scaling (time).
	bool					m_verticalAxis;	///< Is true if the axis is vertical (left or right).
	int						m_axisMaxMinor; ///< Maximum number of minor ticks;
	int						m_axisMaxMajor; ///< Maximum number of major ticks;
	bool					m_dateTime;		///< If true axis has DateTime format.


	/*! Chart needs access to constructor. */
	friend class Chart;
};

/*! \file Sci_ChartAxis.h
	\brief Contains the declaration of class ChartAxis, a helper class for SCI::Chart.
*/

} // end namespace SCI

#endif // Sci_ChartAxisH
