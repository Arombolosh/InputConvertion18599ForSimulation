/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_ChartSeriesH
#define Sci_ChartSeriesH

#include <QColor>
#include <QObject>
#include <QSharedPointer>

#include <IBK_UnitVector.h>

#include "Sci_Globals.h"

class QwtPlotItem;
class QwtLegend;

class QPen;
class QString;

namespace SCI {

class Chart;

/*! \brief The class SCI::ChartSeries contains all variables and functions for creating and
	mainpulating the common properties of chart series of SCI::Chart.

	This class is a helper class for SCI::Chart that takes model data and applies it to the QwtPlot and
	its entities. This class does not modify the chart model properties.

	This a abstract class. You have to derive concrete classes.
	\sa LineSeries
	\sa ColorGridSeries
	\sa BarSeries
*/
class ChartSeries : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(ChartSeries)
public:

	/*! Which object of a series is changed. For future use. */
	enum  ChangeSeriesKind { None,
							 Name,
							 NewData,
							 ChangeData,
							 XAxisTitle,
							 YAxisTitle,
							 LineStyle,
							 LineOrder,
							 DeleteLine,
							 UpdateLine,
							 SaveAs,
							 Header,
							 Selection };

	/*! What is the changing type (creation). For future use. */
	enum ChangeSeriesType {
		Unknown				= 0x000,
		NewSeries			= 0x001,
		NoChange			= 0x002,
		FromLineSeries		= 0x004,
		FromSurfaceSeries	= 0x008,
		FromContourSeries	= 0x010,
		FromColorGridSeries	= 0x020
	};

	/*! Kind of the cut if the series is a part of an other series. */
	enum CutKind { NoCut,
				   X3DCut,
				   Y3DCut,
				   X4DCut,
				   Y4DCut,
				   T4DCut,
				   XY4DCut,
				   XT4DCut,
				   YT4DCut,
				   X3DMin,
				   X3DMax,
				   X3DMean,
				   X3DMinMaxMean,
				   Y3DMinMaxMean
				 };

	/*! Destructor, removes series from plot before destructing (to avoid memory access from plot series). */
	virtual ~ChartSeries();

	/*! Returns the series type.
		\warning Up to now only LineSeries possible.
	*/
	virtual ChartSeriesInfos::SeriesType type() const;

	/*! Returns the title of the series. It is used in legend. */
	QString title() const;

	/*! Returns the number of values in the line. */
	unsigned int size() const;

	/*! Returns true if series is attached to the left axis otherwise it is attached to the right axis.*/
	bool isLeftAxisAttached() const;

public slots:
	/*! Lock the data access. */
	virtual void lockSeries() {}
	/*! Unlock the data access. */
	virtual void unlockSeries() {}

protected:
	/*! Default constructor creates a new series.
		It contains no data and is not connected with a chart (see ChartSeries::setData and ChartSeries::addToChart).
	*/
	ChartSeries(QwtPlotItem* internalSeries);
	/*! Constructor that creates an empty series which is connected to the given chart.
		\param internalSeries QSharedPointer of the internale series representation.
		Should only be used from derived classes.
		\param chart Parent chart.
	*/
	ChartSeries(QwtPlotItem* internalSeries, Chart* chart);

	QwtPlotItem*		m_ser;				///< Internal curve (e.g. line).
	bool				m_valid;			///< Is series valid (data);
	Chart*				m_chart;			///< Parent chart which contains the series.

private:
	/*! Add the series to a chart.
		A possible old attachment will be removed.
	*/
	void addToChart(Chart* chart);

	/*! Set a new title. */
	void setTitle(QString title);

	/*! Set visibility of legend item for this series.*/
	void setShowInLegend(bool visible);

	/*! Attach the series to the left axis and enables this axis if necessary.*/
	void attachToLeftAxis();

	/*! Attach the series to the right axis and enables this axis if necessary.*/
	void attachToRightAxis();

	QString				m_orgTitleText;		///< Text of series title original from setTitle (includes variables).

	friend class Chart;
};

/*! \brief Unary predicate struct that identifies ChartSeries object of type LineSeries.
*/
struct IsSeriesType {
	ChartSeriesInfos::SeriesType m_type;		///< Stored series type for searching.
	/*! Constructor.
		\param type Series type for searching.
	*/
	IsSeriesType(ChartSeriesInfos::SeriesType type) : m_type(type) {}
	/*! operator() used in find_if or similar algorithms.
		\param ser Current ChartSeries.
	*/
	bool operator()(ChartSeries* ser) const {
		return ser->type() == m_type;
	}
};

/*! \file Sci_ChartSeries.h
	\brief Contains the declaration of class ChartSeries, a helper class for SCI::Chart.
*/

}

#endif // Sci_ChartSeriesH
