/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#ifndef Sci_LineSeriesSelectionComboBoxH
#define Sci_LineSeriesSelectionComboBoxH

#include <QComboBox>

namespace SCI {

class AbstractLineSeriesModel;
class AbstractChartModel;
class SeriesListModel;

/*! \brief ComboBox that allows to select a line series from all line series in a chart.*/
class LineSeriesSelectionComboBox : public QComboBox {
	Q_OBJECT
public:
	/*! Default constructor.*/
	explicit LineSeriesSelectionComboBox(QWidget *parent = 0);

	/*! destructor */
	~LineSeriesSelectionComboBox();

	/*! Sets the chart model and the internal combobox model.*/
	void setModel(AbstractChartModel* model);

protected:
	/*! Overloaded paint event.
		Paints the line and description into the edit field.
	*/
	virtual void paintEvent ( QPaintEvent * event );

private:
	AbstractLineSeriesModel*	m_chartModel;			///< Internal chart model.
	SeriesListModel*			m_seriesListModel;		///< Model for list box.

};

/*! @file Sci_LineSeriesSelectionComboBox.h
	@brief Contains the declaration of class LineSeriesSelectionComboBox.
*/
} // namespace SCI

#endif // Sci_LineSeriesSelectionComboBoxH
