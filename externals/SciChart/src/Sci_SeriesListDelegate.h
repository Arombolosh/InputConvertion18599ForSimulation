/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_SeriesListDelegateH
#define Sci_SeriesListDelegateH

#include <QItemDelegate>

namespace SCI {

class SeriesListModel;
class AbstractChartModel;

/*! \brief The class SeriesListDelegate is used for draw items in a ListView or similar widget.
*/
/*! It works together only with a model of type SCI::SeriesListModel.*/
class SeriesListDelegate : public QItemDelegate {
	Q_OBJECT
public:
	/*! Default constructor for SeriesListModel.
		\param parentModel Should the parent model.
		\param parent Parent is responsible for deleting.
	*/
	SeriesListDelegate(SeriesListModel * parentModel, QObject * parent);

	/*! Paints a line with the pen of the actual LineSeries/Marker and the Title. */
	virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;

	/*! The actual implementation for painting line/symbol and text. Called also from LineSeriesSelectionComboBox. */
	static void paintLineSymbolText(QPainter * painter, const QRect& rect, AbstractChartModel* serModel, int index);

private:
	SeriesListModel*		m_model;	///< Pointer to the model.

};

/*! @file Sci_SeriesListDelegate.h
	@brief Contains the declaration of class SeriesListDelegate.
*/

} // end namespace SCI

#endif // Sci_SeriesListDelegateH
