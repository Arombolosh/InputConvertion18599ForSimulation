/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PenStyleDelegateH
#define Sci_PenStyleDelegateH

#include <QItemDelegate>

namespace SCI {

/*! \brief The class SeriesListDelegate is used for draw items in a ListView or similar widget.
*/
class PenStyleDelegate : public QItemDelegate {
	Q_OBJECT
public:
	/*! Default constructor for PenStyleDelegate.
		\param parent Parent is responsible for deleting.
	*/
	explicit PenStyleDelegate(QObject * parent);

	/*! Paints a line with the pen style dependend on row. */
	virtual void paint( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

} // namespace SCI

/*! @file Sci_PenStyleDelegate.h
	@brief Contains the declaration of class PenStyleDelegate.
*/

#endif // Sci_PenStyleDelegateH
