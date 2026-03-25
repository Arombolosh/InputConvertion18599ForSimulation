/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_AbstractChartModelH
#define Sci_AbstractChartModelH

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QStack>
#include <QFont>
#include <QColor>
#include <QPoint>
#include <QSizeF>
#include <QPen>

#include <IBK_UnitVector.h>

namespace DATAIO {
	class ConstructionLines2D;
}

#include "Sci_Globals.h"

class TiXmlElement;

namespace SCI {

/*! \brief Abstract base class for chart models used in SCI::Chart.
	All real chart models should be derived from this class.
	The derived classes should contain the data for the series and all other informatios necessary for drawing a chart.
	All insert, remove or change functions must guarantee that the pointer to the data of the other data sets
	keep valid. Therefore the functions beginInsertSeries(), beginRemoveSeries() and beginChangeSeries()
	need to be called before the model changes data. Once the model's internal state has updated, the
	corresponding endInsertSeries(), endRemoveSeries() or endChangeSeries() functions must be called.

	When chart-specific data has changed, the signal dataChanged() should be emitted with the corresponding
	role. This should be done in setData() functions of the respective subclass.
	When the complete data of the model changes, the signals beginResetModel() and endResetModel() should
	be used to signal views that a radical change of the model's data occurs.

	Chart-specific properties can be queried with data(int chartDataRole) and set with setData(int).
*/
class AbstractChartModel : public QObject {
	Q_OBJECT
	Q_DISABLE_COPY(AbstractChartModel)
public:

	/*! Roles for chart meta-data, accessible via data().
		Don't change role order. Always add new roles before NUM_ADR.
	*/
	enum ChartDataRole {
		TitleText = 0,			///< Chart title as QString, may contain placeholders. Empty title string means "no title visible".
		TitleFont,				///< Font of chart title.
		Title,					///< Chart title as QString (read-only), with replaced placeholders. Default implementation returns same as TitleText.
		LegendVisible,			///< Visibility of legend.
		/*! Position of the legend (int, of type LegendPositionType).
			\warning DO NOT MOVE LegendPosition within enumeration, since onChartChanged() code relies on
				all legend appearance properties being within LegendPosition+1 and LegendFont
			\sa LegendPositionType .
		*/
		LegendPosition,
		/*! Alignment of the embedded legend plot item (only for LegendPosition=LegendInChart) within in the chart
			(int,0=Left, 1=Top-Left, 2=Top,...7, clockwise for each outside alignment).
		*/
		LegendItemAlignment,
		/*! Has frame or not (bool). */
		LegendItemFrame,
		/*! Set style for legend item icon (int, of type LegendIconStyleType).
			\sa LegendIconStyleType
		*/
		LegendIconStyle,
		/*! Width of series/legend item icons (e.g. line example) (double). */
		LegendIconWidth,
		/*! Color of background (QColor). */
		LegendItemBackgroundColor,
		/*! Transparency of the background as factor (0 to 1) (double). */
		LegendItemBackgroundTransparency,
		/*! Distance/offset of legend to border in pixels, with respect to alignment/anchor point (QSize). */
		LegendItemOffset,
		/*! Maximum number of columns of identifiers in legend box (0 - automatically select, legend item will
			use always at least one column when LegendMaxColumns is 0).
		*/
		LegendMaxColumns,
		/*! Distance between legend icon and label (int, in pixel). */
		LegendSpacing,
		/*! Font of legend text (QFont).
			\warning DO NOT MOVE LegendFont within enumeration, since onChartChanged() code relies on
				all legend appearance properties being within LegendPosition+1 and LegendFont
		*/
		LegendFont,
		/*! If true, the floating tracker legend is enabled (and the picker is off). */
		TrackerLegendEnabled,
		/*! Number format ('f', 'g', 'e'). */
		TrackerLegendNumberFormat,
		/*! Number format ('f', 'g', 'e'). */
		TrackerLegendNumberPrecision,
		/*! When set (bool, false by default), y2 axis title is rotated (inverted) to be readable from right. */
		AxisY2TitleInverted,
		/*! When set (bool, false by default), axis scales are drawn inside the chart. */
		AxisScalesInside,
		ConstructionLineVisible,	///< Show construction/boundary lines.
		ConstructionLinePen,		///< Pen for construction/boundary lines (QPen).
		CanChangeAxisUnit,			///< Controls if the chart can change the unit of each axis (bool).
		/*! If true, and if the chart is a chart with a time x-axis, the chart will show a vertical marker at
			the current time point.
		*/
		ShowCurrentTimePosition,
		/*! The current time position in seconds.
			This is a hidden property, that cannot be adjusted in the property widget (within PostProc, this is
			aligned to the south-panel slider position).
		*/
		CurrentTimePosition,
		/*! Always last value. */
		NUM_CDR
	};

	/*! Roles for axis meta-data, accessible via axisData().
		\note AxisPosition is declared here, because we also define the necessary pure virtual
			access functions setAxisData() and axisData().
	*/
	enum AxisPosition {
		BottomAxis,
		LeftAxis,
		RightAxis
	};

	/*! Position type of the legend.
		  \li 0 Beside left axis.
		  \li 1 Beside right axis.
		  \li 2 Below bottom axis.
		  \li 3 Over top axis.
		  \li 4 Inside chart (aligned or arbitrary position).

	*/
	enum LegendPositionType {
		LegendAtLeft,
		LegendAtRight,
		LegendAtBottom,
		LegendAtTop,
		LegendInChart
	};

	/*! Style type of the legend item. */
	enum LegendIconStyleType {
		LegendIconRectangle,
		LegendIconLineWithSymbol
	};

	/*! Holds properties of the legend, title and sizes. */
	struct ChartInformation {
		ChartInformation() :
			m_titleText( tr("Title") ),
			m_legendVisible(true),
			m_legendPosition(LegendAtBottom),
			m_legendAlignment(3), // top-right
			m_legendIconStyle(LegendIconRectangle),
			m_legendIconWidth(20),
			m_legendMaxColumns(0), // zero - automatically determine suitable number of columns
			m_legendHasFrame(true),
			m_legendBackgroundTransparency(1),
			m_legendBackgroundColor(Qt::white),
			m_legendOffset(10,10),
			m_legendSpacing(8),
			m_axisY2TitleInverted(false),
			m_axisScalesInside(false),
			m_constructionLinesVisible(true),
			m_constructionPen(QPen(Qt::SolidLine)),
			m_canChangeAxisUnit(true),
			m_showCurrentTimePosition(false),
			m_currentTimePosition(0),
			m_trackerLegendVisible(false),  // by default turned off, only available for some models who enable this manually
			m_trackerLegendNumberFormat('f'),
			m_trackerLegendNumberPrecision(2)
		{}

		QString				m_titleText;
		QFont				m_titleFont;
		bool				m_legendVisible;
		LegendPositionType	m_legendPosition;
		int					m_legendAlignment;
		int					m_legendIconStyle;
		int					m_legendIconWidth;
		int					m_legendMaxColumns;
		bool				m_legendHasFrame;
		double				m_legendBackgroundTransparency;
		QColor				m_legendBackgroundColor;
		QSize				m_legendOffset;
		QFont				m_legendFont;
		int					m_legendSpacing;
		bool				m_axisY2TitleInverted;
		bool				m_axisScalesInside;
		bool				m_constructionLinesVisible;	///< Visibility of construction lines for profile charts.
		QPen				m_constructionPen;			///< Pen style for construction lines.
		bool				m_canChangeAxisUnit;
		bool				m_showCurrentTimePosition;
		double				m_currentTimePosition;
		bool				m_trackerLegendVisible;
		char				m_trackerLegendNumberFormat;
		int					m_trackerLegendNumberPrecision;
	};

	/*! If true all pamater will be saved in writeXML otherwise only the values there differs from default.*/
	bool		m_writeAll;

	/*! Standard constructor.*/
	explicit AbstractChartModel(QObject *parent = nullptr);

	/*! Standard destructor. Emits modelAboutToBeDeleted.*/
	virtual ~AbstractChartModel();


	// *** General chart data access functions ***

	/*! Returns the type of chart/series. */
	virtual ChartSeriesInfos::SeriesType chartType() const = 0;

	/*! Returns the chart meta data stored under the given role.
		\param chartDataRole Item data role for chart data, \see ChartDataRole
	*/
	virtual QVariant data(int chartDataRole) const;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		This function and data() must be reimplemented for editable models.
		Default implementation does nothing.
		\param value Value to be set.
		\param chartDataRole Item data role, see ChartDataRole
	*/
	virtual bool setData(const QVariant& value, int chartDataRole);



	// *** Series data access functions ***

	/*! Returns number of series.
		Must be re-implemented in derived models.
	*/
	virtual int seriesCount() const = 0;

	/*! Returns the data stored under the given role for the item referred to by the index.
		\note If you do not have a value to return, return an invalid QVariant instead of returning 0.
		\param index Series index.
		\param role Role selects the value kind, \see AbstractChartModel::SeriesDataRole.
	*/
	virtual QVariant seriesData(int seriesIndex, int seriesDataRole) const = 0;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		The base class implementation returns false. This function and data() must be reimplemented for editable models.
		\param index Series index.
		\param value Value to be set.
		\param role Role selects the value kind, \see AbstractChartModel::Role.
	*/
	virtual bool setSeriesData(const QVariant& value, int seriesIndex, int seriesDataRole) = 0;


	// *** Axis informationen access functions ***

	/*! Returns the chart meta data stored under the given role.
		Must be re-implemented in derived classes.
		Well-behaved models will at least return a suitable axis title.
		\param axisPosition Value for axis position, \see AxisPosition
		\param axisDataRole Item data role for axis data, see AxisDataRole
	*/
	virtual QVariant axisData(AxisPosition axisPosition, int axisDataRole) const = 0;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The seriesDataChanged() signal should be emitted if the data was successfully set.
		This function and data() must be reimplemented for editable models.
		Default implementation does nothing.
		\param value Value to be set.
		\param axisPosition Value for axis position
		\param axisDataRole Item data role for axis data, see AxisDataRole
	*/
	virtual bool setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) = 0;


	// *** Marker data access functions ***

	/*! Returns number of marker. */
	virtual int markerCount() const = 0;

	/*! Returns the data stored under the given role for the item referred to by the index.
		\note If you do not have a value to return, return an invalid QVariant instead of returning 0.
		\param markerIndex Marker index.
		\param markerDataRole Role selects the value kind, \see AbstractCarterisanSeriesModel::MarkerDataRole.
	*/
	virtual QVariant markerData(int markerIndex, int markerDataRole) const = 0;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The markerDataChanged() signal should be emitted if the data was successfully set.
		This function and data() must be reimplemented for editable models.
		\param value Value to be set.
		\param markerIndex Marker index.
		\param markerDataRole Role selects the value kind, \see AbstractCarterisanSeriesModel::MarkerDataRole.
	*/
	virtual bool setMarkerData(const QVariant& value, int markerIndex, int markerDataRole) = 0;



	// *** Other functions ***

	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	virtual void readXML(const TiXmlElement * element);

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	virtual void writeXML(TiXmlElement * parent) const;

	/*! Begins a model reset operation.
		Default implementation emits modelAboutToBeReset() signal.
	*/
	virtual void beginResetModel();

	/*! Finishes a model reset operation.
		Default implementation emits modelReset() signal.
	*/
	virtual void endResetModel();

	/*! Returns a pointer to a construction lines object. It is a nullptr in case no object exist.
		The unit is the same as the x-axis.
		Construction lines are only possible if all following conditions are fulfilled:
		- x-axis unit must be a length unit
		- a geometry file must exist (only D6 outputs)
		- all series should have the same geometry file
		The function must be rewritten in derived classes if construction lines objects exist.
	*/
	virtual const DATAIO::ConstructionLines2D*	constructionLines() const { return nullptr; }

signals:
	/*! This signal is emitted whenever the data of the chart changes.
		When reimplementing the setData() function, this signal must be emitted explicitly.
		\param chartDataRole Item role of the changed property.
	*/
	void chartChanged(int chartDataRole);

	/*! Reacts on chartChanged from model. This is emitted after changes of chart properties in model (AxisDataRole).
		\param axisPosition Position of the axis or axis type (bottom, left or right for cartesian axis).
		\param role Role of the changed item \see AxisDataRole.
	*/
	void axisChanged( int axisPosition, int role );

	/*! Emitted just before a model reset operation begins.
		Views should prepare for a complete rebuild.
	*/
	void modelAboutToBeReset();

	/*! Emitted just after a model reset operation completed.
		Chart is completely rebuild.
	*/
	void modelReset();

	/*! Emitted just before something in the model is changed.
		The user can/should emit this signal just before a series of model change operations
		to block the chart from doing updates.
		The view will not be updated until modelChanged() signal is emitted.
		\todo Make use of this signal.
	*/
	void modelAboutToBeChanged();

	/*! Emitted just after change operations are completed.
		The user must emit this signal after a series of model change operation with
		a modelAboutToBeChanged() signal emitted beforehand.
		After this model is received by the chart, it is replotted (once) with all updated information.
		In contrast to modelAboutToBeReset() and modelReset() the view is not completely reset,
		but only updated, i.e. no series are recreated from model. Thus, this is typically faster than
		using the reset feature.
		\todo Make use of this signal.
	*/
	void modelChanged();


	// *** Signals related to series ***

	/*! This signal is emitted whenever an individual series property was changed.
		When this signal is received by a view, it should update the layout of items to reflect this change.
		\param start Start series index.
		\param end End series index.
		\param role	Role of the changed item.
	*/
	void seriesViewChanged(int start, int end, int role);

	/*! This signal is emitted just before series are inserted into the model.
		The new items will be positioned between start and end inclusive.
		\param start Start series index.
		\param end End series index.
	*/
	void seriesAboutToBeInserted(int start, int end );

	/*! This signal is emitted just after series are inserted into the model.
		The new items are positioned between start and end inclusive.
		\param start Start series index.
		\param end End series index.
	*/
	void seriesInserted(int start, int end);

	/*! This signal is emitted just before series are removed from the model.
		The items that will be removed are those between start and end inclusive.
		\param start Start series index.
		\param end End series index.
	*/
	void seriesAboutToBeRemoved(int start, int end );

	/*! This signal is emitted just after series are removed from the model.
		The items that is removed are those between start and end inclusive.
		\param start Start series index.
		\param end End series index.
	*/
	void seriesRemoved(int start, int end);

	/*! This signal is emitted just before series data are changed in the model.
		The items that will be changed are those between start and end inclusive.
		\param start Start series index.
		\param end End series index.
	*/
	void seriesAboutToBeChanged(int start, int end );

	/*! This signal is emitted just after series data are generally changed (added/removed) in the model or
		if several properties are changed together.
		The items that are changed are those between start and end inclusive.
		\param start Start series index.
		\param end End series index.
	*/
	void seriesChanged(int start, int end);


	// *** Signals related to marker ***

	/*! This signal is emitted whenever an individual marker property was changed.
		When this signal is received by a view, it should update the layout of items to reflect this change.
		\param start Start series index.
		\param end End series index.
		\param role	Role of the changed item.
	*/
	void markerDataChanged(int start, int end, int role);

	/*! This signal is emitted just after markers are inserted into the model.
		The new items are positioned between start and end inclusive.
		\param start Start marker index.
		\param end End marker index.
	*/
	void markerInserted(int start, int end);

	/*! This signal is emitted just before markers are removed from the model.
		The items that will be removed are those between start and end inclusive.
		\param start Start marker index.
		\param end End marker index.
	*/
	void markerAboutToBeRemoved(int start, int end );

	/*! This signal is emitted just after marker data are changed.
		The items that are changed are those between start and end inclusive.
		\param start Start marker index.
		\param end End marker index.
	*/
	void markerChanged(int start, int end);



protected:
	/*! Begins a series insertion operation.
		When inserting new series into a model, you must call this function
		before inserting data into the model's underlying data store.
		\note This function emits the seriesAboutToBeInserted() signal which connected views (or proxies) must handle before the data is inserted.
		Otherwise, the views may end up in an invalid state.
		See also QAbstractItemModel::beginInsertRows()
		\param first Index at which first inserted series is to be placed
		\param last Index of series last inserted (after insertion).
	*/
	virtual void beginInsertSeries( int first, int last);

	/*! Ends a series insertion operation.
		When inserting new series into a model, you must call this function
		after inserting data into the model's underlying data store.
	*/
	virtual void endInsertSeries();

	/*! Begins a series removal operation.
		When removing series from the model, you must call this function
		before remove data from the model's underlying data store.
		\note This function emits the seriesAboutToBeRemoved() signal which connected views (or proxies) must handle before the data is removed.
		Otherwise, the views may end up in an invalid state.
	*/
	virtual void beginRemoveSeries ( int first, int last );

	/*! Ends a series removal operation.
		When removing series from the model, you must call this function
		after removing data from the model's underlying data store.
	*/
	virtual void endRemoveSeries();

	/*! Begins a series data change operation.
		You must call this function in a subclass function just before a series data change operation.
		\note This function emits the seriesAboutToBeChanged() signal which connected views (or proxies) must handle before the data is changed.
		Otherwise, the views may end up in an invalid state or error can occure (invalid data pointer).
	*/
	virtual void beginChangeSeries ( int first, int last );

	/*! Ends a series change operation.
		This function must be called in a subclass function just afetr the end of a change operation.
		You need always pairs of beginChangeSeries and endChangeSeries, otherwise the data acces in the chart is permanently locked.
	*/
	virtual void endChangeSeries();

	/*! Begins a model change operation.
		Default implementation emits modelAboutToBeChanged() signal.
	*/
	virtual void beginChangeModel();

	/*! Finishes a model change operation.
		Default implementation emits modelChanged() signal.
	*/
	virtual void endChangeModel();

protected:

	/*! Clears internally stored data.
		Use this function to clear the internal state of the AbstractChartModel
		whenever derived classes reset their content.
		The current implementation makes nothing. This function is for further use.
		A call to clearCachedData() should be place between emitted signals
		of type beginResetModel() and endResetModel().
	*/
	virtual void clearCachedData();

	/*! Internal storage for chart legend properties.*/
	ChartInformation	m_chartProperties;

	/*! Empty cut vector as return value for cutValues in case of no existing cut plane.*/
	IBK::UnitVector			m_emptyCutVector;

private:

	/*! Contains information about a change of series data that
		is used by the chart to update itself using a minimum of
		work and drawing effort.
	*/
	struct Change {
		Change() : first(-1), last(-1) {}
		Change(const Change &c) :
			first(c.first),
			last(c.last),
			needsAdjust(c.needsAdjust)
		{}
		Change & operator=(const Change &) = default;
		Change(int f, int l) :
			first(f),
			last(l),
			needsAdjust(false) {}
		int first, last;
		bool needsAdjust;
		bool isValid() {
			return first >= 0 && last >= 0;
		}
	};

	/*! Stores the changes in the model.
		Added from beginInsertSeries and beginRemoveSeries.
		Removed from endInsertSeries and endRemoveSeries.
		If empty no change process is running.
	*/
	QStack<Change>	m_changes;

}; // class AbstractChartModel

} // namespace SCI

#endif // Sci_AbstractChartModelH
