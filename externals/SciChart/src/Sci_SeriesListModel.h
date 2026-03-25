/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_SeriesListModelH
#define Sci_SeriesListModelH

#include <QAbstractListModel>


namespace SCI {

class AbstractLineSeriesModel;
class LineSeries;

/*! \brief The class SeriesListModel transforms series data from an AbstractLineSeriesModel into a list model
	suitable for combo boxes/list views.

	It has only one column, which shows the title of the series.
*/
class SeriesListModel : public QAbstractListModel {
	Q_OBJECT
public:
	/*! Default constructor for SeriesListModel. */
	SeriesListModel(AbstractLineSeriesModel* model, QObject *parent = 0);

	/*! Sets a new chart and resets the model.*/
	void setModel(AbstractLineSeriesModel* model);

	/*! Returns the internal chart model.*/
	AbstractLineSeriesModel* chartModel() const;

	/*! returns the row count of the list which is the number of series in the chart. */
	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	/*! Returns the series title for the row corresponding to a series. */
	QVariant data(const QModelIndex &index, int role) const;

	/*! Creates a index and sets the internal pointer to the selected series. */
	QModelIndex index( int row, int column, const QModelIndex & parent ) const;

	/*! Calls reset in order to update the model. */
//	void dataChanged();

public slots:
	void onSeriesAboutToBeRemoved(int start, int end );
	void onSeriesRemoved(int, int);
	void onSeriesAboutToBeInserted(int start, int end );
	void onSeriesInserted(int, int);

private:
	AbstractLineSeriesModel* m_model;				///< Internal chart model.

	friend class SeriesListDelegate;
};

/*! \file Sci_SeriesListModel.h
	\brief Contains the declaration of class SeriesListModel.
*/

} // end namespace SCI

#endif // Sci_SeriesListModelH
