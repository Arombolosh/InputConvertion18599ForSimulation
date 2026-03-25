/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_BarSeries.h"
#include "Sci_AbstractBarSeriesModel.h"
#include "Sci_BarLabelScaleDraw.h"
#include "Sci_Chart.h"

namespace SCI {

BarSeries::BarSeries(Chart* chart, const QString& title) :
	ChartSeries(new QwtPlotMultiBarChart(title), chart),
	m_barCount(0)
{
	m_chart->setAxisScaleDraw(QwtPlot::xBottom, new BarLabelScaleDraw);
}


//bool BarSeries::setData(int size, const double* x, const double* y) {
//	QVector<QwtSetSample> s;
//	m_barCount = size;
//	m_barValueCount = 1;
//	for( int i=0; i<size; ++i) {
//		s += QwtSetSample( x[i], QVector<double>(1, y[i]) );
//	}
//	curve()->setSamples(s);
//	return true;
//}


//bool BarSeries::setData(const std::vector<std::pair<double,std::vector<double> > > & values) {
//	QVector<QwtSetSample> s;
//	m_barCount = (unsigned int)values.size();
//	m_barValueCount = (unsigned int)values[0].second.size();
//	for( unsigned int i=0; i<values.size(); ++i) {
//		s += QwtSetSample( values[i].first, QVector<double>::fromStdVector(values[i].second) );
//	}
//	curve()->setSamples(s);
//	return true;
//}


bool BarSeries::setData(AbstractBarSeriesModel* model) {
	if( model == NULL)
		return false;

	QVector<QwtSetSample> s;
	int size = model->seriesData(0, AbstractBarSeriesModel::SeriesSize).toInt();
	if( size == 0)
		return false;

	m_barCount = size;
	m_barValueCount = model->yValuesCount();
	const double* xvals = model->xValues();
	if(!xvals || model->yValuesCount() == 0)
		return false;

	for( int i=0; i<size; ++i) {
		const double* yvals = model->yValues(i);
		if(!yvals)
			return false;
#if QT_VERSION >= QT_VERSION_CHECK(5,14,0)
		QVector<double> ytvect(yvals, yvals + m_barValueCount);
#else
		QVector<double> ytvect = QVector<double>::fromStdVector(std::vector<double>(yvals, yvals + m_barValueCount));
#endif
		s += QwtSetSample( xvals[i], ytvect );
	}
	curve()->setSamples(s);
	BarLabelScaleDraw* scaleDraw = dynamic_cast<BarLabelScaleDraw*>(m_chart->axisScaleDraw(QwtPlot::xBottom));
	if(scaleDraw)
		scaleDraw->setLabels(model->xLabels());

	return true;
}


void BarSeries::setLayoutPolicy(QwtPlotAbstractBarChart::LayoutPolicy layoutPolicy) {
	curve()->setLayoutPolicy(layoutPolicy);
}


void BarSeries::setLayoutHint( double val) {
	curve()->setLayoutHint(val);
}


void BarSeries::setSpacing( int spacing) {
	curve()->setSpacing(spacing);
}


void BarSeries::setMargin( int margin) {
	curve()->setMargin(margin);
}


void BarSeries::setBaseline( double value) {
	curve()->setBaseline(value);
}


void BarSeries::setBarChartStyle(QwtPlotMultiBarChart::ChartStyle chartStyle) {
	curve()->setStyle(chartStyle);
}


void BarSeries::setBarTitles(const QStringList& titles) {
	QList<QwtText> titleList;
	foreach (QString title, titles) {
		titleList.push_back(title);
	}
	curve()->setBarTitles(titleList);
}


void BarSeries::setBarFrameStyle(unsigned int index, QwtColumnSymbol::FrameStyle frameStyle) {
	QwtColumnSymbol* value = new QwtColumnSymbol(QwtColumnSymbol::Box);
	value->setFrameStyle(frameStyle);

	// const cast necessary in order to call const version ofQwtPlotMultiBarChart::symbol()
	const QwtColumnSymbol* current = const_cast<const QwtPlotMultiBarChart*>(curve())->symbol(index);
	if(current != 0) {
		value->setLineWidth(current->lineWidth());
		value->setPalette(current->palette());
	}
	else {
		value->setLineWidth(1);
	}

	curve()->setSymbol(index, value);
}


void BarSeries::setBarLineWidth(unsigned int index, int width) {
	QwtColumnSymbol* value = new QwtColumnSymbol(QwtColumnSymbol::Box);
	value->setLineWidth(width);

	// const cast necessary in order to call const version ofQwtPlotMultiBarChart::symbol()
	const QwtColumnSymbol* current = const_cast<const QwtPlotMultiBarChart*>(curve())->symbol(index);
	if(current != 0) {
		value->setFrameStyle(current->frameStyle());
		value->setPalette(current->palette());
	}
	else {
		value->setFrameStyle(QwtColumnSymbol::Plain);
	}

	curve()->setSymbol(index, value);
}


void BarSeries::setBarPalette(unsigned int index, const QPalette& palette) {
	QwtColumnSymbol* value = new QwtColumnSymbol(QwtColumnSymbol::Box);
	value->setPalette(palette);

	// const cast necessary in order to call const version ofQwtPlotMultiBarChart::symbol()
	const QwtColumnSymbol* current = const_cast<const QwtPlotMultiBarChart*>(curve())->symbol(index);
	if(current != 0) {
		value->setFrameStyle(current->frameStyle());
		value->setLineWidth(current->lineWidth());
	}
	else {
		value->setFrameStyle(QwtColumnSymbol::Plain);
		value->setLineWidth(1);
	}

	curve()->setSymbol(index, value);
}


void BarSeries::setBarColor(unsigned int index, const QColor& color) {
	QwtColumnSymbol* value = new QwtColumnSymbol(QwtColumnSymbol::Box);
	QPalette palette;

	// const cast necessary in order to call const version ofQwtPlotMultiBarChart::symbol()
	const QwtColumnSymbol* current = const_cast<const QwtPlotMultiBarChart*>(curve())->symbol(index);
	if(current != 0) {
		value->setFrameStyle(current->frameStyle());
		value->setLineWidth(current->lineWidth());
		palette = current->palette();
	}
	else {
		value->setFrameStyle(QwtColumnSymbol::Plain);
		value->setLineWidth(1);
	}

	palette.setColor(QPalette::Window, color);
	palette.setBrush(QPalette::Dark, Qt::black);

	value->setPalette(palette);
	curve()->setSymbol(index, value);
}


void BarSeries::setOrientation(Qt::Orientation orientation) {
	curve()->setOrientation(orientation);
}

} // namespace SCI
