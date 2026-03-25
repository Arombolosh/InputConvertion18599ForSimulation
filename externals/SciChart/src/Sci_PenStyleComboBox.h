/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_PenStyleComboBoxH
#define Sci_PenStyleComboBoxH

#include <QComboBox>

class QPaintEvent;

namespace SCI {

/*! \brief ComboBox that allows to select a pen style.*/
class PenStyleComboBox : public QComboBox {
	Q_OBJECT
public:
	/*! Default constructor.*/
	explicit PenStyleComboBox(QWidget *parent = 0);
	/*! Returns the current style.*/
	Qt::PenStyle currentStyle() const;
	/*! Sets the index depending on given style.*/
	void setCurrentStyle(Qt::PenStyle);

signals:
	/*! Is emitted if index is changed.*/
	void currentIndexChanged(Qt::PenStyle);

protected:
	/*! Overloaded paint event.
		Paints the line into the edit field.
	*/
	virtual void paintEvent ( QPaintEvent * event );

private slots:
	/*! Catches a currentIndexChanged(int) signal and sends a corresponding currentIndexChanged(Qt::PenStyle) signal.*/
	void translateCurrentIndex(int);

};

/*! @file Sci_PenStyleCombobox.h
	@brief Contains the declaration of class PenStyleComboBox.
*/

} // namespace SCI

#endif // Sci_PenStyleComboBoxH
