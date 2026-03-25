/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_LegendH
#define Sci_LegendH

#include <qwt_legend.h>
#include <qwt_plot.h>

namespace SCI {

/*! \brief Legend for external legend widgets in a SCI::Chart (legends outside plot canvas).
	It is derived from QwtLegend.
	Key purpose of this class is the common handling of legend item properties.
	\note QwtLegend is a widget and many appearance properties are controlled by native member functions
		of QWidget and QFrame.
*/
class Legend : public QwtLegend {
	Q_OBJECT
public:
	/*! Default constructor.*/
	explicit Legend(QwtPlot *parent, QwtPlot::LegendPosition pos = QwtPlot::BottomLegend);

	/*! Set spacing (distance between marker and label) for all legend items.*/
	void setSpacing(int space);

	/*! Set the maximum number of items in a row.
		This function overloads the QwtLegend function to set the columns
		and call updateGeometry() in the widget afterwards. This in turn re-layouts
		the legend items.
	*/
	void setMaxColumns(unsigned int maxColumns);

protected:
	/*! Re-implemented from QwtLegend::createWidget() to construct new QwtLegendLabels
		(icon plus text) with correct spacing and appearance.
	*/
	virtual QWidget *createWidget( const QwtLegendData & ) const;

private slots:
	/*! Called after legend was updated, used to sort legend entries. */
	void sortLegend();

private:
	/*! Cached icon/text spacing, updated in setSpacing() and used in createWidget() to
		construct correctly spaced icon/text QwtLegendLabels.
	*/
	int	m_spacing;
};

/*!	\file Sci_Legend.h
	\brief Contains the declaration of class Legend derived from QwtLegend.
*/

} // namespace SCI

#endif // Sci_LegendH
