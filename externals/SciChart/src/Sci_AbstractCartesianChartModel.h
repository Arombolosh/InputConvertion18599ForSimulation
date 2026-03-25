/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_AbstractCartesianChartModelH
#define Sci_AbstractCartesianChartModelH

#include <QDateTime>
#include <QPair>
#include <QPen>
#include <QTimeZone>

#include <IBK_Unit.h>

#include "Sci_AbstractChartModel.h"
#include "Sci_DateScaleDraw.h"
#include "Sci_Marker.h"

class TiXmlElement;

namespace SCI {

/*! Base class for all data models providing data for cartesian charts.
	\todo Refactor to use only properties AxisGridPen and AxisMinorGridPen (merge width and color into one property)
	as with BorderPen.
*/
class AbstractCartesianChartModel : public AbstractChartModel {
	Q_OBJECT
	Q_DISABLE_COPY(AbstractCartesianChartModel)
public:

	/*! Roles for axis meta-data, accessible via axisData().
		Don't change role order. Always add new roles before NUM_ADR.
	*/
	enum AxisDataRole {
		AxisEnabled = 0,			///< Disabled axis are not visible.
		AxisTitle,					///< Text of the axis title (read-only, QString, where placeholders have been replaced. Base implementation returns same as AxisTitleText).
		AxisTitleText,				///< Text of the axis title (QString, may contain placeholders, used for property widgets, if empty axis title is not shown).
		AxisMaximum,				///< Maximum value for axis scaling.
		AxisMinimum,				///< Minimum value for axis scaling.
		AxisLogarithmic,			///< Logarithmic scaling if true or linear scaling if false.
		AxisProportional,			///< If true, this axis is to be scaled proportionally to the other axis (not applicable for right y axis)
		AxisAutoScale,				///< If true maximum and minimum scaling will be set automatically.
		AxisTitleFont,				///< Font of the axis title (QFont).
		AxisLabelFont,				///< Font of the axis labels (QFont).
		AxisLabelHAlignment,		///< Horizontal alignment of the axis labels (0 - left, 1 - right, 2 - center)
		AxisLabelVAlignment,		///< Vertical alignment of the axis labels (0 - top, 1 - bottom, 2 - center).
		AxisLabelRotation,			///< Lable rotation angle in degrees (double).
		AxisMaxMinorScale,			///< Maximum number of minor scale intervals.
		AxisMaxMajorScale,			///< Maximum number of major scale intervals.
		AxisUnit,					///< Unit (as ID) for axis and all corresponding series.
		AxisGridVisible,			///< Visibility of axis grid lines
		AxisGridPen,				///< The pen for drawing the major grid axis lines (QPen), combines the properties PenStyle, width and color.
		AxisGridPenWidth,			///< The pen half-width for drawing the major grid axis lines (int)
		AxisGridPenColor,			///< The pen color for drawing the major grid axis lines (QColor)
		AxisMinorGridVisible,		///< Visibility of axis minor grid lines
		AxisMinorGridPen,			///< The pen for drawing the minor grid axis lines (QPen), combines the properties PenStyle, width and color.
		AxisMinorGridPenWidth,		///< The pen half-width for drawing the minor grid axis lines (int)
		AxisMinorGridPenColor,		///< The pen color for drawing the minor grid axis lines (QColor)
		AxisTitleSpacing,			///< Distance between bar, scale and title
		AxisLabelSpacing,			///< Distance between label and axis
		AxisDateTime,				///< If true a date time scaling will be used.
		AxisDateTimeZero,			///< QDateTime value (always in Qt::UTC) that represents the origin for the date data (i.e. the date that corresponds to 0 s).
		/*! QStringList (size 8) with list of formats representing DateTime labels for different intervals.
			The index of the list represents different intervals (must be exactly 8 strings in the QStringList).
				\li 0 - Millisecond	"hh:mm:ss:zzz\nddd dd MMM yyyy"
				\li 1 - Second		"hh:mm:ss\nddd dd MMM yyyy"
				\li 2 - Minute		"hh:mm\nddd dd MMM yyyy"
				\li 3 - Hour		"hh:mm\nddd dd MMM yyyy"
				\li 4 - Day			"ddd dd MMM yyyy"
				\li 5 - Week		"Www yyyy"
				\li 6 - Month		"MMM yyyy"
				\li 7 - Year		"yyyy"
		*/
		AxisDateTimeFormats,

		/*! Always last value.
			\note All data roles from AxisEnabled up to NUM_ADR are applied during a chart reset.
		*/
		NUM_ADR
	};

	/*! Data roles for controlling properties of marker. */
	enum MarkerDataRole {
		/*! Returns full data set as Marker object. */
		MarkerData,
		/*! Type of marker. \sa QwtPlotMarker::LineStyle */
		MarkerType,
		MarkerXPos,
		/*! Only for marker property widget. Value is converted to x-coordinate when axis is switched. */
		MarkerXPosDateTime,
		MarkerYPos,
		MarkerXAxisID,
		MarkerYAxisID,
		/*! Label text of marker. */
		MarkerLabel,
		MarkerLabelFont,
		MarkerLabelAligment,
		/*! Virtual role, data storage is joined with MarkerLabelAligment. Only used in property widget. */
		MarkerLabelVAligment,
		MarkerLabelOrientation,
		MarkerSpacing,
		MarkerPen,
		MarkerPenWidth,
		MarkerPenColor,
		MarkerZOrder,
		NUM_MDR
	};

	/*! Holds all information stored for a single axis.
		\note Must be a public type to be usable as QVariant type.
		\todo Check if this is still needed!
	*/
	struct AxisInformation {
		AxisInformation() :
			m_axisEnabled(true),
			m_axisMaximum(0.0),
			m_axisMinimum(0.0),
			m_logarithmicAxis(false),
			m_axisProportional(false),
			m_axisAutoScale(true),
			m_axisLabelHAlignment(0),  // left
			m_axisLabelVAlignment(2),  // center
			m_axisLabelRotation(0.0),
			m_axisMaxMinorScale(5),
			m_axisMaxMajorScale(8),
			m_gridVisible(true),
			m_gridPen(QPen(Qt::gray)),
			m_gridMinorVisible(true),
			m_gridMinorPen(QPen(Qt::lightGray)),
			m_titleSpacing(35), // this is ok for standard font sizes, maybe this should be OS dependent
			m_labelSpacing(5),
			m_dateTime(false),
			m_dateTimeZero(QDateTime()), // initialize with invalid DateTime
			m_dateTimeSpec(Qt::UTC),
			m_dateTimeFormats(DateScaleDraw::defaultFormats())
		{
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
			m_dateTimeZero.setTimeZone(QTimeZone("UTC"));
#else
			m_dateTimeZero.setTimeSpec(Qt::UTC);
#endif
			// Note: penwidth = 1 by default to have 1 pixel wide grid lines on screen
			//		 during vector export, the grid lines are automatically scaled by 0.5 to get thin grid lines
//			m_gridPen.setWidth(1);
			m_gridPen.setStyle(Qt::DashLine);
//			m_gridMinorPen.setWidth(1);
			m_gridMinorPen.setStyle(Qt::DotLine);
		}

		bool			m_axisEnabled;
		double			m_axisMaximum;
		double			m_axisMinimum;
		bool			m_logarithmicAxis;
		bool			m_axisProportional;
		bool			m_axisAutoScale;
		QFont			m_axisTitleFont;
		QString			m_axisTitleText;
		QFont			m_axisLabelFont;
		int				m_axisLabelHAlignment;
		int				m_axisLabelVAlignment;
		double			m_axisLabelRotation;
		int				m_axisMaxMinorScale;
		int				m_axisMaxMajorScale;
		IBK::Unit		m_axisUnit;
		bool			m_gridVisible;
		QPen			m_gridPen;
		bool			m_gridMinorVisible;
		QPen			m_gridMinorPen;
		int				m_titleSpacing;
		int				m_labelSpacing;
		bool			m_dateTime;
		QDateTime		m_dateTimeZero;
		Qt::TimeSpec	m_dateTimeSpec;
		QStringList		m_dateTimeFormats;
	};

	/*! Constructor, initializes axis data with defaults. */
	explicit AbstractCartesianChartModel(QObject *parent = nullptr);

	/*! Returns the axis meta data stored under the given role.
		Can be re-implemented in derived classes.
		Well-behaved models will at least return a suitable axis title.
		\param axisPosition Value for axis position, \see AxisPosition
		\param axisDataRole Item data role for axis data, see AxisDataRole
	*/
	virtual QVariant axisData(AxisPosition axisPosition, int axisDataRole) const override;

	/*! Returns the axis meta data stored under the given role.
		Can be re-implemented in derived classes.
		\param axisPosition Value for axis position, \see AxisPosition
	*/
	virtual AxisInformation axisInformation(int axisPosition) const;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The dataChanged() signal should be emitted if the data was successfully set.
		This function and data() can be reimplemented for editable models.
		\param value Value to be set.
		\param axisPosition Value for axis position, \see AxisPosition
		\param axisDataRole Item data role for axis data, see AxisDataRole
		\return It returns false in case of wrong data or errors. True will be returned if set value is set correctly or no set is necessary.
	*/
	virtual bool setAxisData(const QVariant& value, AxisPosition axisPosition, int axisDataRole) override;

	/*! Set all axis data in once. Can be used for initialisation.*/
	virtual bool setAxisData(AxisPosition axisPosition, const AxisInformation& axisinfo);


	// *** Marker data access functions ***

	/*! Returns number of marker. */
	virtual int markerCount() const override { return m_markerProperties.count(); }

	/*! Inserts a new marker.
		\param markerIndex Index to insert the marker before. Pass markerCount() to append a marker.
		\param marker The data to insert.
	*/
	virtual void insertMarker(int markerIndex, const Marker & marker);

	/*! Removes a marker. */
	virtual void removeMarker(int markerIndex);

	/*! Returns the data stored under the given role for the item referred to by the index.
		\note If you do not have a value to return, return an invalid QVariant instead of returning 0.
		\param markerIndex Marker index.
		\param markerDataRole Role selects the value kind, \see AbstractCarterisanSeriesModel::MarkerDataRole.
	*/
	virtual QVariant markerData(int markerIndex, int markerDataRole) const override;

	/*! Sets the role data for the item at index to value.
		Returns true if successful; otherwise returns false.
		The markerDataChanged() signal is emitted if the data was successfully set.
		\param value Value to be set.
		\param markerIndex Marker index.
		\param markerDataRole Role selects the value kind, \see AbstractCarterisanSeriesModel::MarkerDataRole.
	*/
	virtual bool setMarkerData(const QVariant& value, int markerIndex, int markerDataRole) override;

	/*! Sets the complete data for a series at once.
		The markerChanged() signal is emitted if the data was successfully set.
		\param markerIndex Marker index.
		\param value The line series data.
	*/
	virtual void setMarkerData(int markerIndex, const Marker& value);

	/*! Swaps the marker for the given indexes.
		Indexes must be valid (assert is used).
	*/
	virtual void swapMarkerData(int first, int second);



	/*! Reads the data from the xml element.
		Throws an IBK::Exception if a syntax error occurs.
	*/
	virtual void readXML(const TiXmlElement * element) override;

	/*! Appends the element to the parent xml element.
		Throws an IBK::Exception in case of invalid data.
	*/
	virtual void writeXML(TiXmlElement * parent) const override;

protected:

	/*! Clears internally stored data.
		Use this function to clear the internal state of the AbstractCartesianChartModel
		whenever derived classes reset their content.
		A call to clearCachedData() should be place between emitted signals
		of type beginResetModel() and endResetModel().
	*/
	virtual void clearCachedData() override;

private:

	/*! Storage member for all three axis, mapping see AxisPosition. */
	AxisInformation m_axisInformation[3];

	/*! Storage member for markers. */
	QList<Marker>	m_markerProperties;

}; // class AbstractCartesianChartModel

} // namespace SCI

Q_DECLARE_METATYPE(SCI::AbstractCartesianChartModel::AxisInformation);

#endif // Sci_AbstractCartesianChartModelH
