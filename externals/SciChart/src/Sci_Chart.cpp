/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <algorithm>
#include <memory>

#include <QMouseEvent>
#include <QScrollBar>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QDebug>
#include <QVector>
#include <QPair>
#include <QRect>
#include <QFileDialog>
#include <QPainter>
#include <QPointer>
#include <QLayout>
#include <QTimer>
#include <QMessageBox>

#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_legend.h>
#include <qwt_color_map.h>
#include <qwt_plot_canvas.h>
#include <qwt_text_label.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_renderer.h>
#include <qwt_legend_label.h>
#include <qwt_plot_canvas.h>

#ifdef SCICHART_HAS_EMFENGINE
#include <EmfEngine.h>
#endif // SCICHART_HAS_EMFENGINE

#include <IBK_messages.h>
#include <IBK_FormatString.h>
#include <IBK_Exception.h>
#include <IBK_UnitList.h>

#include <tinyxml.h>

#include <DATAIO_ConstructionLines2D.h>

#include "Sci_Chart.h"
#include "Sci_LineSeries.h"
#include "Sci_ColorGridSeries.h"
#include "Sci_BarSeries.h"
#include "Sci_VectorFieldSeries.h"
#include "Sci_Legend.h"
#include "Sci_LegendItem.h"
#include "Sci_AbstractChartModel.h"
#include "Sci_AbstractLineSeriesModel.h"
#include "Sci_AbstractColorGridSeriesModel.h"
#include "Sci_AbstractBarSeriesModel.h"
#include "Sci_AbstractVectorFieldSeriesModel.h"
#include "Sci_PlotPanner.h"
#include "Sci_PlotZoomer.h"
#include "Sci_PlotRescaler.h"
#include "Sci_ColorMapCreator.h"
#include "Sci_ExportDialog.h"
#include "Sci_LegendItemMover.h"
#include "Sci_PlotLayout.h"
#include "Sci_PlotScaleItem.h"
#include "Sci_ChartFormatSelectionDialog.h"
#include "Sci_CurveTracker.h"
#include "Sci_PlotMarker.h"

namespace SCI {

// *** Implementation for SCI::Chart ***

Chart::Chart(QWidget * parent) :
	QwtPlot(parent),
	m_model(nullptr),
	m_dataLock(false),
	m_noUpdateCounter(0),
	m_leftAxis(this, QwtPlot::yLeft),
	m_rightAxis(this, QwtPlot::yRight),
	m_topAxis(this, QwtPlot::xTop),
	m_bottomAxis(this, QwtPlot::xBottom),
	m_internalBottomAxis(nullptr),
	m_internalTopAxis(nullptr),
	m_internalLeftAxis(nullptr),
	m_internalRightAxis(nullptr),
	m_grid(new QwtPlotGrid),
	m_legend(nullptr),
	m_legendItem(nullptr),
	m_colorLegend(nullptr),
	m_startDate(2000,0.),
	m_currentTimeIdx((unsigned int)-1),
	m_markerCurrentTimePosition(nullptr),
	m_rescaler(nullptr),
	m_formatSelectionDialog(nullptr)
{
	// disable auto replot
	setAutoReplot(false);

	// *** set all properties that we do not have a ChartDataRole for

	// no frame around plot widget
	setFrameStyle(QFrame::NoFrame);
	setContentsMargins(0,0,0,0);
	setAutoFillBackground(true);

	// configure canvas
	setCanvasBackground(Qt::white);

	// use our own plot layout that avoids the off-by-one-pixel problem
	PlotLayout * plotlay = new PlotLayout;
	setPlotLayout(plotlay);

	// apply chart style selected in preferences
	configureChartFormat();


	// grid - currently no AxisDataRole for grid pen properties
	QPen p(Qt::gray, 1, Qt::SolidLine);
	m_grid->setMajorPen(p);
	QPen pe(Qt::lightGray, 1, Qt::DotLine);
	m_grid->setMinorPen(pe);
	m_grid->attach(this);
	m_grid->setZ(0);

	// set axis pen width to 1
	axisScaleDraw(QwtPlot::xBottom)->setPenWidthF(1);
	axisScaleDraw(QwtPlot::yLeft)->setPenWidthF(1);
	axisScaleDraw(QwtPlot::yRight)->setPenWidthF(1);

	// only used for ColorSeries -> see addColorGridSeries()
	m_colorLegend = axisWidget(QwtPlot::yRight);
	m_colorLegend->setMargin( 2 );
	m_colorLegend->setColorBarEnabled( false );
	m_colorLegend->setVisible(false);

	m_zoomer = new SCI::PlotZoomer( QwtPlot::xBottom, QwtPlot::yLeft, canvas(), &m_bottomAxis);

	m_panner = new SCI::PlotPanner(canvas());
	m_panner->setMouseButton(Qt::MiddleButton);

	// for now permanently attaching curve tracker to chart
	m_curveTracker = new SCI::CurveTracker(canvas(), &m_bottomAxis);

	// LegendItemMover is constructed using the QwtPlot as parent, and thus is descructed
	// when the plot dies. Inside the constructor it registers itself to the eventFilter list.
	new LegendItemMover(this, m_zoomer);

	// create rescaler, disabled by default
	m_rescaler = new SCI::PlotRescaler(this);
	m_rescaler->setEnabled(false); // removes eventFilter

	// create internal scale axis objects

	// Mind: scale position means "scale is drawn ... relative to position"
	m_internalBottomAxis = new SCI::PlotScaleItem(QwtScaleDraw::TopScale);
	m_internalTopAxis = new SCI::PlotScaleItem(QwtScaleDraw::BottomScale);
	m_internalLeftAxis = new SCI::PlotScaleItem(QwtScaleDraw::RightScale);
	m_internalRightAxis = new SCI::PlotScaleItem(QwtScaleDraw::LeftScale);

	// attach
	m_internalBottomAxis->attach(this);
	m_internalTopAxis->attach(this);
	m_internalLeftAxis->attach(this);
	m_internalRightAxis->attach(this);

	// Visibility is adjusted when setting the "Internal scales" chart property
}


Chart::~Chart() {
	foreach(ChartSeries* series, m_series)
		delete series;

	// DO NOT delete objects we store pointers to, QwtPlot takes care of this (as QObject parent)
	m_grid = nullptr;
	m_legend = nullptr;
	m_legendItem = nullptr;

	// do not delete model, since model may be shared among several views
	m_model = nullptr;
}


void Chart::clear() {
	Q_ASSERT_X(false, "[Chart::clear]","Do not ever ever call this function!");
	// QwtPlot::clear();
}



const ChartSeries * Chart::seriesAt(unsigned int index) const {
	if( index >= static_cast<unsigned int>(m_series.size()))
		return nullptr;
	return m_series[(int)index];
}


void Chart::configureChartFormat() {
	QwtPlotCanvas * scicanvas = (QwtPlotCanvas *)canvas();
	PlotLayout * plotlay = dynamic_cast<PlotLayout *>(plotLayout());

	// black thin frame around canvas
	scicanvas->setLineWidth(1);
	scicanvas->setFrameStyle(QFrame::Box);
	QPalette pal;
	pal.setColor(QPalette::WindowText, Qt::black); // used for scale widgets
	pal.setColor(QPalette::Dark, Qt::black); // used for frame widget
//	pal.setColor(QPalette::Window, QColor(Qt::gray).lighter()); // used for scale widgets
	setPalette(pal);

	plotlay->setAlignCanvasToScales( true );
	plotlay->setCanvasMargin(0, QwtPlot::xBottom);
	plotlay->setCanvasMargin(0, QwtPlot::xTop);
	plotlay->setCanvasMargin(0, QwtPlot::yLeft);
	plotlay->setCanvasMargin(0, QwtPlot::yRight);
	// adjust scale widgets

	// customize backbone/tick gap
	for (int axisId=0; axisId<3; ++axisId) {
		if (axisWidget(axisId)->isColorBarEnabled()) {
			// color bar needs backbone of scale and gap between color bar and canvas frame
			axisScaleDraw(axisId)->enableComponent(QwtAbstractScaleDraw::Backbone, true);
			axisWidget(axisId)->setMargin(4);
		}
		else {
			axisScaleDraw(axisId)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
			axisWidget(axisId)->setMargin(0);
		}
		axisScaleDraw(axisId)->setTickLength(QwtScaleDiv::MinorTick,2);
		axisScaleDraw(axisId)->setTickLength(QwtScaleDiv::MediumTick,4);
		axisScaleDraw(axisId)->setTickLength(QwtScaleDiv::MajorTick,6);
	}

}


bool Chart::saveChartStyle(const QString & styleFilePath) const {
	// create style file and store only chart appearance options

	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "PostProcStyle" );
	doc.LinkEndChild(root);

	root->SetAttribute("xmlns", "http://www.bauklimatik-dresden.de");
	root->SetAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance");
	root->SetAttribute("xmlns:IBK", "http://www.bauklimatik-dresden.de/IBK");
	root->SetAttribute("xsi:schemaLocation", "http://www.bauklimatik-dresden.de PostProc.xsd");
	root->SetAttribute("fileVersion", "1.0");

	const SCI::AbstractLineSeriesModel * lineModel = dynamic_cast<const SCI::AbstractLineSeriesModel *>(model());
	if (lineModel != nullptr ) {

		TiXmlElement * es = new TiXmlElement("ChartModel");
		root->LinkEndChild(es);
		es->SetAttribute("chartType", "2D" );
		lineModel->SCI::AbstractLineSeriesModel::writeXML(es);
		std::string filePath = styleFilePath.toStdString();
		return doc.SaveFile( filePath.c_str() ); // UTF8-filename handling done inside TiCPP library
	}

	const SCI::AbstractColorGridSeriesModel * colorModel = dynamic_cast<const SCI::AbstractColorGridSeriesModel *>(model());
	if (colorModel != nullptr) {
		TiXmlElement * es = new TiXmlElement("ChartModel");
		root->LinkEndChild(es);
		es->SetAttribute("chartType", "3D" );
		colorModel->SCI::AbstractColorGridSeriesModel::writeXML(es);
		std::string filePath = styleFilePath.toStdString();
		return doc.SaveFile( filePath.c_str() ); // UTF8-filename handling done inside TiCPP library
	}

	return true;
}


// Dummy class that serves only as data container when reading in saved chart styles
class ColorGridSeriesDummyModel : public AbstractColorGridSeriesModel {
public:
	virtual int seriesCount() const override { return 1; }
	virtual void calculateGlobalMinMax() override {}
};


// Dummy class that serves only as data container when reading in saved chart styles
class LineSeriesDummyModel : public AbstractLineSeriesModel {
public:
	virtual int seriesCount() const override { return lineInformation().size(); }
	virtual const double*	xValues(unsigned int ) const override { return nullptr; }
	virtual const double*	yValues(unsigned int ) const override { return nullptr; }
};


void Chart::applyChartStyle(const QString & styleFilePath) {
	const char * const FUNC_ID = "[Project::readXML]";

	try {
		std::string filename = styleFilePath.toStdString();
		TiXmlDocument doc( filename.c_str() );
		if (!doc.LoadFile()) {
			throw IBK::Exception(IBK::FormatString("Error in line %1 of project file '%2':\n%3")
					.arg(doc.ErrorRow())
					.arg(filename)
					.arg(doc.ErrorDesc()), FUNC_ID);
		}

		// now we parse the different sections of the project file

		// we use a handle so that nullptr pointer checks are done during the query functions
		TiXmlHandle xmlHandleDoc(&doc);

		TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
		if (!xmlElem)
			return; // empty project, this means we are using only defaults
		std::string rootnode = xmlElem->Value();
		if (rootnode != "PostProcStyle")
			throw IBK::Exception("Expected PostProcStyle as root node in XML file.", FUNC_ID);

		// we read our subsections from this handle
		TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

		xmlElem = xmlRoot.FirstChild( "ChartModel" ).Element();
		if (xmlElem) {
			// detect type of model to instantiate
			const TiXmlAttribute * idAttrib = TiXmlAttribute::attributeByName(xmlElem, "chartType");
			if (!idAttrib)
				throw IBK::Exception(IBK::FormatString("Expected 'chartType' attribute in ChartModel tag."), FUNC_ID);

			// instantiate appropriate content model
			std::unique_ptr<SCI::AbstractChartModel> mo;
			if (idAttrib->ValueStr() == "2D") {
				mo.reset(new SCI::LineSeriesDummyModel);
				mo->readXML(xmlElem);
			}
			else {
				mo.reset(new SCI::ColorGridSeriesDummyModel);
				mo->readXML(xmlElem);
			}

			// Now fill the dialog to ask user about the properties to apply to the current chart
			if (m_formatSelectionDialog == nullptr)
				m_formatSelectionDialog = new ChartFormatSelectionDialog(this);
			m_formatSelectionDialog->setLineFormatOptionEnabled(idAttrib->ValueStr() == "2D");
			if (m_formatSelectionDialog->exec() == QDialog::Accepted) {
				QSet<ChartFormatSelectionDialog::SettingsToTransfer> selectedOptions = m_formatSelectionDialog->selectedOptions();
				prohibitUpdate();

				// process all options and copy relevant properties from temporary model to current model

				// *** General options
				if (selectedOptions.contains(ChartFormatSelectionDialog::OptionGeneral)) {
					std::vector<int> rolesToCopy;
					rolesToCopy.push_back(SCI::AbstractChartModel::TitleFont);
					rolesToCopy.push_back(SCI::AbstractChartModel::AxisY2TitleInverted);
					rolesToCopy.push_back(SCI::AbstractChartModel::AxisScalesInside);
					for (unsigned int i=0; i<rolesToCopy.size(); ++i)
						m_model->setData( mo->data(rolesToCopy[i]), rolesToCopy[i]);
				}

				// *** Legend
				if (selectedOptions.contains(ChartFormatSelectionDialog::OptionLegend)) {
					std::vector<int> rolesToCopy;
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendVisible);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendPosition);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendItemAlignment);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendItemFrame);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendIconStyle);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendIconWidth);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendItemBackgroundColor);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendItemBackgroundTransparency);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendItemOffset);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendMaxColumns);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendSpacing);
					rolesToCopy.push_back(SCI::AbstractChartModel::LegendFont);
					for (unsigned int i=0; i<rolesToCopy.size(); ++i)
						m_model->setData( mo->data(rolesToCopy[i]), rolesToCopy[i]);
				}

				// *** Bottom Axis
				if (selectedOptions.contains(ChartFormatSelectionDialog::OptionAxisX)) {
					std::set<int> rolesToCopy;
					for (int i=0; i<SCI::AbstractCartesianChartModel::NUM_ADR; ++i)
						rolesToCopy.insert(i);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisTitleText);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisTitle);

					// only copy date-time flag, if current axis is also a time series
					unsigned int axisUnitID = mo->axisData(SCI::AbstractChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit).toUInt();
					bool timeAxis = false;
					try {
						timeAxis = (IBK::Unit(axisUnitID).base_id() == IBK::Unit("s").base_id());
					}
					catch (...) {}
					if (!timeAxis)
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisDateTime);

					// apply the unit only, if the current unit is convertable to this unit
					bool applyUnit = false;
					try {
						IBK::Unit currentXUnit(m_model->axisData(SCI::AbstractChartModel::BottomAxis, SCI::AbstractCartesianChartModel::AxisUnit).toUInt());
						applyUnit = (IBK::Unit(axisUnitID).base_id() == currentXUnit.base_id());
					}
					catch (...) {}
					if (!applyUnit)
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisUnit);

					for (std::set<int>::iterator it = rolesToCopy.begin(); it != rolesToCopy.end(); ++it)
						m_model->setAxisData(mo->axisData(SCI::AbstractChartModel::BottomAxis, *it), SCI::AbstractChartModel::BottomAxis, *it);
				}

				// *** Y-Left Axis
				if (selectedOptions.contains(ChartFormatSelectionDialog::OptionAxisYLeft)) {
					std::set<int> rolesToCopy;
					for (int i=0; i<SCI::AbstractCartesianChartModel::NUM_ADR; ++i)
						rolesToCopy.insert(i);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisTitleText);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisTitle);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisDateTime);

					// apply the unit only, if the current unit is convertable to this unit
					bool applyUnit = false;
					try {
						unsigned int axisUnitID = mo->axisData(SCI::AbstractChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit).toUInt();
						IBK::Unit currentUnit(m_model->axisData(SCI::AbstractChartModel::LeftAxis, SCI::AbstractCartesianChartModel::AxisUnit).toUInt());
						applyUnit = (IBK::Unit(axisUnitID).base_id() == currentUnit.base_id());
					}
					catch (...) {}
					if (!applyUnit)
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisUnit);

					for (std::set<int>::iterator it = rolesToCopy.begin(); it != rolesToCopy.end(); ++it)
						m_model->setAxisData(mo->axisData(SCI::AbstractChartModel::LeftAxis, *it), SCI::AbstractChartModel::LeftAxis, *it);
				}

				// *** Y-Right Axis
				if (selectedOptions.contains(ChartFormatSelectionDialog::OptionAxisYRight)) {
					std::set<int> rolesToCopy;
					for (int i=0; i<SCI::AbstractCartesianChartModel::NUM_ADR; ++i)
						rolesToCopy.insert(i);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisTitleText);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisTitle);
					rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisDateTime);

					// apply the unit only, if the current unit is convertable to this unit
					bool applyUnit = false;
					try {
						unsigned int axisUnitID = mo->axisData(SCI::AbstractChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisUnit).toUInt();
						IBK::Unit currentUnit(m_model->axisData(SCI::AbstractChartModel::RightAxis, SCI::AbstractCartesianChartModel::AxisUnit).toUInt());
						applyUnit = (IBK::Unit(axisUnitID).base_id() == currentUnit.base_id());
					}
					catch (...) {}
					if (!applyUnit) {
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisUnit);
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisMaximum);
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisMinimum);
					}
					// if target model is a color map plot, disable all properties that do not related to
					// a color axis
					if (dynamic_cast<AbstractColorGridSeriesModel *>(m_model) == nullptr) {
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisScalesInside);
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisDateTime);
						rolesToCopy.erase(SCI::AbstractCartesianChartModel::AxisLogarithmic);
					}

					for (std::set<int>::iterator it = rolesToCopy.begin(); it != rolesToCopy.end(); ++it)
						m_model->setAxisData(mo->axisData(SCI::AbstractChartModel::RightAxis, *it), SCI::AbstractChartModel::RightAxis, *it);
				}

				// *** Line Information
				if (selectedOptions.contains(ChartFormatSelectionDialog::OptionLineProperties)) {
					AbstractLineSeriesModel * srcModel = dynamic_cast<AbstractLineSeriesModel *>(mo.get());
					AbstractLineSeriesModel * destModel = dynamic_cast<AbstractLineSeriesModel *>(m_model);
					// it is possible that our current chart is not a line chart, in this case skip the application of line properties
					IBK_ASSERT(srcModel != nullptr);
					if (destModel != nullptr) {
						// determine max line series properties to copy
						int max = std::min(srcModel->seriesCount(), destModel->seriesCount());
						std::vector<int> rolesToCopy;
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesPen);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesLineStyle);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesMarkerStyle);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesInverted);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesInLegend);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesMarkerSize);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesMarkerColor);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesMarkerFilled);
						rolesToCopy.push_back(SCI::AbstractLineSeriesModel::SeriesBrush);
						for (int i=0; i<max; ++i) {
							for (unsigned int r=0; r<rolesToCopy.size(); ++r)
								destModel->setSeriesData( srcModel->seriesData( i, rolesToCopy[r] ), i, rolesToCopy[r]);
						}
					}
				}
				if (selectedOptions.contains(ChartFormatSelectionDialog::OptionColorMapProperties)) {
					AbstractColorGridSeriesModel * srcModel = dynamic_cast<AbstractColorGridSeriesModel *>(mo.get());
					AbstractColorGridSeriesModel * destModel = dynamic_cast<AbstractColorGridSeriesModel *>(m_model);
					// it is possible that our current chart is not a color map chart, in this case skip the application of color map properties
					IBK_ASSERT(srcModel != nullptr);
					if (destModel != nullptr) {
						std::set<int> rolesToCopy;
						for (int i=SCI::AbstractColorGridSeriesModel::ColorMap; i<SCI::AbstractColorGridSeriesModel::NUM_SDR; ++i)
							rolesToCopy.insert(i);
						rolesToCopy.erase(SCI::AbstractColorGridSeriesModel::DataMaximumZ);
						rolesToCopy.erase(SCI::AbstractColorGridSeriesModel::DataMinimumZ);
						rolesToCopy.erase(SCI::AbstractColorGridSeriesModel::ZValueRange);

						for (std::set<int>::iterator it = rolesToCopy.begin(); it != rolesToCopy.end(); ++it)
							destModel->setSeriesData( srcModel->seriesData( 0, *it ), 0, *it);
					}
				}
				allowUpdate();
				replot();
			}

		}
	}
	catch ( IBK::Exception &ex ) {
		ex.writeMsgStackToError();
		QMessageBox::critical(this, QString(), tr("Error applying chart style from file"));
	}
}

void Chart::render(QPainter * p, const QPointF& pos, bool horizontal, QwtPlotRenderer::LayoutFlag layoutFlag,
				   const QFlags<QwtPlotRenderer::DiscardFlag>& discardFlags) {
	prepareForVectorExport(true, false);
	QwtPlotRenderer renderer;
	renderer.setDiscardFlags(discardFlags);
	renderer.setLayoutFlag(layoutFlag);
	QSizeF chartSize = geometry().size();
	QRectF chartrect = QRectF(pos, chartSize);
	p->save();
	if(!horizontal) {
		p->translate(pos);
		chartrect.setRect(-chartrect.height(), pos.x(),
						  chartrect.height(), chartrect.width());
		p->rotate(-90);
	}
	renderer.render(this, p, chartrect);
	p->restore();
	resetAfterVectorExport();
}

const IBK::Time Chart::startDate() const {
	return m_startDate;
}


const IBK::Time Chart::endDate() const {
	IBK::Time eDate = m_startDate;
	eDate += m_timePoints.back();
	return eDate;
}


IBK::Time Chart::currentDate() const {
	IBK::Time curDate = m_startDate;
	Q_ASSERT(m_currentTimeIdx < m_timePoints.size());
	curDate += m_timePoints[m_currentTimeIdx];
	return curDate;
}


unsigned int Chart::nTimePoints() const {
	return (unsigned int)m_timePoints.size();
}


void Chart::setTimeFrame(const std::vector<double> & timePoints, const IBK::Time& startDateTime) {
	m_timePoints = timePoints;
	if (m_timePoints.size() >= 1)
		m_currentTimeIdx = (unsigned int)m_timePoints.size() - 1;
	m_startDate = startDateTime;
}


unsigned int Chart::currentTimeIndex() const {
	return m_currentTimeIdx;
}


void Chart::setCurrentTimeIndex(unsigned int tIdx) {
	m_currentTimeIdx = tIdx;
}


ChartAxis* Chart::axis(ChartAxis::AxisType type) {
	switch(type) {
		case ChartAxis::UnknownAxis: return nullptr;
		case ChartAxis::LeftAxis: return &m_leftAxis;
		case ChartAxis::RightAxis: return &m_rightAxis;
		case ChartAxis::BottomAxis: return &m_bottomAxis;
		case ChartAxis::TopAxis: return &m_topAxis;
		case ChartAxis::DepthAxis: return nullptr;
		case ChartAxis::CustomAxis: return nullptr;
	}
	return nullptr;
}


ChartAxis const * Chart::axis(ChartAxis::AxisType type) const {
	switch(type) {
		case ChartAxis::UnknownAxis: return nullptr;
		case ChartAxis::LeftAxis: return &m_leftAxis;
		case ChartAxis::RightAxis: return &m_rightAxis;
		case ChartAxis::BottomAxis: return &m_bottomAxis;
		case ChartAxis::TopAxis: return &m_topAxis;
		case ChartAxis::DepthAxis: return nullptr;
		case ChartAxis::CustomAxis: return nullptr;
	}
	return nullptr;
}

#ifdef SCICHART_HAS_EMFENGINE
void Chart::copyToClipboardAsEMF() {
	QwtPlotRenderer renderer;
	renderer.setDiscardFlags(QwtPlotRenderer::DiscardBackground | QwtPlotRenderer::DiscardCanvasBackground);
	renderer.setLayoutFlag(QwtPlotRenderer::FrameWithScales);
	int w = geometry().width();
	int h = geometry().height();
	double pdpi = physicalDpiX();
	double ldpi = logicalDpiX();
	double fact = pdpi / ldpi;
	EmfPaintDevice emf(QSize(w * fact, h * fact));

	// adjust chart properties to improve vector export
	prepareForVectorExport(true, true);

	// render chart
	renderer.renderTo(this, emf);

	// reset chart appearance
	resetAfterVectorExport();
}
#endif // SCICHART_HAS_EMFENGINE


void Chart::copyToClipboardAsPixmap() {
#if QT_VERSION >= 0x050000
	QApplication::clipboard()->setPixmap(grab());
#else
	QApplication::clipboard()->setPixmap(QPixmap::grabWidget(this));
#endif // QT_VERSION >= 0x050000

	QMessageBox::information(this, QString(), tr("Image has been copied to the clipboard."));
}


void Chart::prepareForVectorExport(bool legendVisible, bool useDataReduction) {
	// adjust pen width of grid to "thin lines"
	QPen majorPen = m_grid->majorPen();
	QPen minorPen = m_grid->minorPen();
	majorPen.setWidthF(0.5);
	m_grid->setMajorPen(majorPen);
	minorPen.setWidthF(0.5);
	m_grid->setMinorPen(minorPen);

	QPen black05;
	black05.setWidthF(0.5);
	if (m_legendItem != nullptr) {
		m_legendItem->setVisible(m_model->data(AbstractCartesianChartModel::LegendVisible).toBool() && legendVisible);
		// adjust legend border width if border is visible
		if (m_model->data(AbstractCartesianChartModel::LegendItemFrame).toBool())
			m_legendItem->setBorderPen(black05);
	}

	// adjust the axis scale widgets to use thin lines
//	axisWidget(QwtPlot::yLeft)->scaleDraw()->enableComponent(QwtAbstractScaleDraw::Backbone, true);
	double penWidthScales = 0.5;
	axisWidget(QwtPlot::yLeft)->scaleDraw()->setPenWidthF(penWidthScales);
	axisWidget(QwtPlot::yRight)->scaleDraw()->setPenWidthF(penWidthScales);
	axisWidget(QwtPlot::xBottom)->scaleDraw()->setPenWidthF(penWidthScales);

	m_internalBottomAxis->scaleDraw()->setPenWidthF(0.5);
	m_internalTopAxis->scaleDraw()->setPenWidthF(0.5);
	m_internalLeftAxis->scaleDraw()->setPenWidthF(0.5);
	m_internalRightAxis->scaleDraw()->setPenWidthF(0.5);

	// for line series charts, tell all plot curves to use data reduction
	AbstractLineSeriesModel * seriesModel = qobject_cast<AbstractLineSeriesModel*>(model());
	if (seriesModel != nullptr) {
		for (int i=0; i<m_series.count(); ++i) {
			LineSeries * series = qobject_cast<LineSeries*>(m_series[i]);
			if (series != nullptr) {
				PlotCurve * plotCurve = series->curve();
				plotCurve->setCurveAttribute(QwtPlotCurve::Fitted, useDataReduction);
			}
		}
	}

	QPalette pal = palette();
	// make background white
	pal.setColor(QPalette::Window, Qt::white);
	setPalette(pal);
}


void Chart::prepareForBitmapExport(bool legendVisible) {
	if (m_legendItem != nullptr) {
		m_legendItem->setVisible(m_model->data(AbstractCartesianChartModel::LegendVisible).toBool() && legendVisible);
	}
}


void Chart::resetAfterVectorExport() {
	// restore settings to default (should correspond to model settings
	// if supported)
	QPen majorPen = m_grid->majorPen();
	QPen minorPen = m_grid->minorPen();
	majorPen.setWidth( int(m_model->axisData(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisGridPenWidth).toInt()*0.5) );
	m_grid->setMajorPen(majorPen);
	minorPen.setWidth( int(m_model->axisData(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisMinorGridPenWidth).toInt()*0.5) );
	m_grid->setMinorPen(minorPen);

	if (m_legendItem != nullptr) {
		m_legendItem->setVisible(m_model->data(AbstractCartesianChartModel::LegendVisible).toBool() );
		if (m_model->data(AbstractCartesianChartModel::LegendItemFrame).toBool())
			m_legendItem->setBorderPen(QPen(Qt::black)); // reset to cosmetic pen, as we do not allow configuring border pen, yet
	}
	axisWidget(QwtPlot::yLeft)->scaleDraw()->setPenWidthF(1);
	axisWidget(QwtPlot::yRight)->scaleDraw()->setPenWidthF(1);
	axisWidget(QwtPlot::xBottom)->scaleDraw()->setPenWidthF(1);

	m_internalBottomAxis->scaleDraw()->setPenWidthF(1);
	m_internalTopAxis->scaleDraw()->setPenWidthF(1);
	m_internalLeftAxis->scaleDraw()->setPenWidthF(1);
	m_internalRightAxis->scaleDraw()->setPenWidthF(1);

	// for line series charts, tell all plot curves to use data reduction
	AbstractLineSeriesModel * seriesModel = qobject_cast<AbstractLineSeriesModel*>(model());
	if (seriesModel != nullptr) {
		for (int i=0; i<m_series.count(); ++i) {
			LineSeries * series = qobject_cast<LineSeries*>(m_series[i]);
			if (series != nullptr) {
				PlotCurve * plotCurve = series->curve();
				plotCurve->setCurveAttribute(QwtPlotCurve::Fitted, false);
			}
		}
	}
	QPalette pal = palette();
	// make background white
	pal.setColor(QPalette::Window, QPalette().color(QPalette::Window));
	setPalette(pal);
}


void Chart::resetAfterBitmapExport() {
	if (m_legendItem != nullptr) {
		m_legendItem->setVisible(m_model->data(AbstractCartesianChartModel::LegendVisible).toBool() );
	}
}


void Chart::setYAxisWidth( int width ) {
	axisScaleDraw( QwtPlot::yLeft )->setMinimumExtent( width );
	updateLayout();
}


void Chart::setXAxisHeight( int height ) {
	axisScaleDraw( QwtPlot::xBottom )->setMinimumExtent( height );
	updateLayout();
}


void Chart::replot() {
	if( m_noUpdateCounter == 0)
		QwtPlot::replot();
}


bool Chart::insertLineSeriesFromModel(int index) {
	Q_ASSERT(index >= 0 && index <= m_series.count()); // no gaps allowed when inserting series

	AbstractLineSeriesModel* lineModel = dynamic_cast<AbstractLineSeriesModel*>(m_model);
	if (lineModel == nullptr)
		return false;

	try {

		bool leftAxis = lineModel->seriesData(index, AbstractLineSeriesModel::SeriesLeftYAxis).toBool();
		QString title = lineModel->seriesData(index, AbstractLineSeriesModel::SeriesTitle).toString();
		QPen pen = lineModel->seriesData(index, AbstractLineSeriesModel::SeriesPen).value<QPen>();
		IBK::Unit xunit = IBK::Unit(lineModel->axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toUInt());
		AbstractChartModel::AxisPosition axisPosition = leftAxis ? AbstractCartesianChartModel::LeftAxis : AbstractLineSeriesModel::RightAxis;
		IBK::Unit yunit = IBK::Unit(lineModel->axisData(axisPosition, AbstractCartesianChartModel::AxisUnit).toUInt());

		if( xunit == IBK::Unit() || yunit == IBK::Unit()) {
			qDebug() << "Invalid xunit or yunit returned from LineSeriesModel.";
			return false;
		}

		// add data with actual unit to axis
		bool nanCheck = xunit.base_id() == IBK::Unit("m").base_id(); // enable checks for NaN values for profile charts
		int lineSeriesSize = lineModel->seriesData(index, AbstractLineSeriesModel::SeriesSize).toInt();
#if (__cplusplus > 199711L)
		std::unique_ptr<LineSeries> ser(new LineSeries(this, lineSeriesSize,
													 lineModel->xValues(index),
													 lineModel->yValues(index),
													 title, nanCheck, pen.color()) );
#else
		std::auto_ptr<LineSeries> ser(new LineSeries(this, lineSeriesSize,
													 lineModel->xValues(index),
													 lineModel->yValues(index),
													 title, nanCheck, pen.color()) );
#endif

		// Initialize LineSeries object with data from model

		ser->setPen(pen);
		ser->setBrush(lineModel->seriesData(index, AbstractLineSeriesModel::SeriesBrush).value<QBrush>());
		int legendIconStyle = lineModel->data( AbstractChartModel::LegendIconStyle ).toInt();
		ser->setLegendIconStyle( legendIconStyle );
		if (legendIconStyle == SCI::AbstractChartModel::LegendIconLineWithSymbol) {
			QSize sizeLegendIcon;
			sizeLegendIcon.setHeight( 9 ); // in case of symbols, we need some space to draw them
			sizeLegendIcon.setWidth( lineModel->data( AbstractChartModel::LegendIconWidth ).toInt() );
			ser->setLegendIconSize( sizeLegendIcon );
		}
		else {
			ser->setLegendIconSize( QSize(9,9) );
		}
		bool inLegend = lineModel->seriesData(index, AbstractLineSeriesModel::SeriesInLegend).toBool();
		ser->setLegendIconVisible(inLegend);

		// marker properties
		ser->setMarkerStyle( lineModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerStyle).toInt() );
		ser->setMarkerFilled(lineModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerFilled).toBool());
		ser->setMarkerColor( lineModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerColor).value<QColor>() );
		ser->setMarkerSize( lineModel->seriesData(index, AbstractLineSeriesModel::SeriesMarkerSize).toInt() );


		if( leftAxis) {
			ser->attachToLeftAxis();
		}
		else {
			ser->attachToRightAxis();
		}

		m_zoomer->m_xUnit = QString::fromStdString( xunit.name() );
		m_curveTracker->m_xUnit = QString::fromStdString( xunit.name() );
		if ( leftAxis ) {
			m_zoomer->m_y1Unit = QString::fromStdString( yunit.name() );
			m_curveTracker->m_y1Unit = QString::fromStdString( yunit.name() );
		}
		else {
			m_zoomer->m_y2Unit = QString::fromStdString( yunit.name() );
			m_curveTracker->m_y2Unit = QString::fromStdString( yunit.name() );
		}

		// inserting with index = m_series.count() is the same as adding the series
		m_series.insert(index, ser.release());
	}
	catch(...) {
		qDebug() << "Exception caught while creating new line series.";
		return false;
	}

	return true;
}


bool Chart::insertColorGridSeriesFromModel() {
	const char * const FUNC_ID = "[Chart::insertColorGridSeriesFromModel]";
	Q_ASSERT(m_series.empty());  // make sure existing series are deleted!

	// check that this is the correct model, otherwise we must not call this function!
	AbstractColorGridSeriesModel* colorModel = dynamic_cast<AbstractColorGridSeriesModel*>(m_model);
	Q_ASSERT(colorModel != nullptr);

	IBK::Unit xunit(colorModel->axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toUInt());
	IBK::Unit yunit(colorModel->axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toUInt());
	IBK::Unit valueunit(colorModel->axisData(AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisUnit).toUInt());

	if ( xunit == IBK::Unit() || yunit == IBK::Unit()) {
		IBK::IBK_Message("Invalid x-unit or y-unit returned from ColorGridSeriesModel.", IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	if ( valueunit == IBK::Unit()) {
		IBK::IBK_Message("Invalid value unit returned from ColorGridSeriesModel.", IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	// can only update if we have a valid model and series
	if( !m_model || m_model->seriesCount() != 1)
		return false;

	ColorGridSeries* ser = new ColorGridSeries(this);

	unsigned int xGridSize = colorModel->seriesData(0, AbstractColorGridSeriesModel::XGridArraySize).toUInt();
	const double* xGrid = static_cast<const double*>(colorModel->seriesData(0, AbstractColorGridSeriesModel::XGridArray).value<void*>());
	unsigned int yGridSize = colorModel->seriesData(0, AbstractColorGridSeriesModel::YGridArraySize).toUInt();
	const double* yGrid = static_cast<const double*>(colorModel->seriesData(0, AbstractColorGridSeriesModel::YGridArray).value<void*>());
	unsigned int xValueSize = colorModel->seriesData(0, AbstractColorGridSeriesModel::XValueSize).toUInt();
	unsigned int yValueSize = colorModel->seriesData(0, AbstractColorGridSeriesModel::YValueSize).toUInt();
	const double* values = static_cast<const double*>(colorModel->seriesData(0, AbstractColorGridSeriesModel::ValueMatrix).value<void*>());
	try {
		ser->setData(xGridSize, xGrid, yGridSize, yGrid, xValueSize, yValueSize, values);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
	}
	catch (std::exception & ex) {
		IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
	}

	/// \note Most of the properties below are set through the model-view framework, so
	///       the code below should be removed at some point

	// enable right axis, also for editing common properties in property widget
	m_model->setAxisData(true, AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
	// enable color legend (always on, not managed by model)
	m_colorLegend->setColorBarEnabled( true );

	m_zoomer->m_xUnit = QString::fromStdString( xunit.name() );
	m_zoomer->m_y1Unit = QString::fromStdString( yunit.name() );
	m_zoomer->m_valueUnit = QString::fromStdString( valueunit.name() );

	m_series.append(ser);

	return true;
}


bool Chart::insertBarSeriesFromModel() {
	const char * const FUNC_ID = "[Chart::insertBarSeriesFromModel]";
	Q_ASSERT(m_series.empty());  // make sure existing series are deleted!

	// check that this is the correct model, otherwise we must not call this function!
	AbstractBarSeriesModel* barModel = dynamic_cast<AbstractBarSeriesModel*>(m_model);
	Q_ASSERT(barModel != nullptr);

	QString title = barModel->seriesData(0, AbstractBarSeriesModel::SeriesTitle).toString();

	IBK::Unit xunit(barModel->axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toInt());
	IBK::Unit yunit(barModel->axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toInt());
	int layoutPolicy = barModel->seriesData(0, AbstractBarSeriesModel::LayoutPolicy).toInt();
	double layoutHint = barModel->seriesData(0, AbstractBarSeriesModel::LayoutHint).toDouble();
	int spacing = barModel->seriesData(0, AbstractBarSeriesModel::Spacing).toInt();
	int margin = barModel->seriesData(0, AbstractBarSeriesModel::Margin).toInt();
	double baseline = barModel->seriesData(0, AbstractBarSeriesModel::BaseLine).toDouble();
	int style = barModel->seriesData(0, AbstractBarSeriesModel::BarChartStyle).toInt();

	if ( yunit == IBK::Unit()) {
		IBK::IBK_Message("Invalid y-unit returned from BarSeriesModel.",
						 IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	// can only update if we have a valid model and series
	if( !m_model || m_model->seriesCount() != 1)
		return false;

	BarSeries* ser = new BarSeries(this, title);

	try {
		ser->setData(barModel);
	}
	catch (std::exception & ex) {
		IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
	}

	ser->setLayoutPolicy(static_cast<QwtPlotAbstractBarChart::LayoutPolicy>(layoutPolicy));
	ser->setLayoutHint(layoutHint);
	ser->setSpacing(spacing);
	ser->setMargin(margin);
	ser->setBaseline(baseline);
	ser->setBarChartStyle(static_cast<QwtPlotMultiBarChart::ChartStyle>(style));
	unsigned int valueSize = barModel->yValuesCount();
	for(unsigned int i=0; i<valueSize; ++i) {
		ser->setBarPalette(i, barModel->valuePalette(i));
		ser->setBarFrameStyle(i, barModel->frameStyle(i));
		ser->setBarLineWidth(i, barModel->frameLineWidth(i));
	}

	m_rightAxis.setVisible( false );
	m_colorLegend->setColorBarEnabled( false );
	plotLayout()->setAlignCanvasToScales(true);

	if (xunit != IBK::Unit()) {
		m_zoomer->m_xUnit = QString::fromStdString( xunit.name() );
	}
	m_zoomer->m_y1Unit = QString::fromStdString( yunit.name() );

	m_series.append(ser);

	return true;
}


bool Chart::insertVectorFieldSeriesFromModel() {
	const char * const FUNC_ID = "[Chart::insertVectorFieldSeriesFromModel]";
	Q_ASSERT(m_series.empty());  // make sure existing series are deleted!

	// check that this is the correct model, otherwise we must not call this function!
	AbstractVectorFieldSeriesModel* vectorFieldModel = dynamic_cast<AbstractVectorFieldSeriesModel*>(m_model);
	Q_ASSERT(vectorFieldModel != nullptr);

	IBK::Unit xunit(vectorFieldModel->axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toUInt());
	IBK::Unit yunit(vectorFieldModel->axisData(AbstractCartesianChartModel::LeftAxis, AbstractCartesianChartModel::AxisUnit).toUInt());
	IBK::Unit valueunit(vectorFieldModel->axisData(AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisUnit).toUInt());

	if ( xunit == IBK::Unit() || yunit == IBK::Unit()) {
		IBK::IBK_Message("Invalid x-unit or y-unit returned from AbstractVectorFieldSeriesModel.", IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	if ( valueunit == IBK::Unit()) {
		IBK::IBK_Message("Invalid value unit returned from AbstractVectorFieldSeriesModel.", IBK::MSG_ERROR, FUNC_ID);
		return false;
	}

	// can only update if we have a valid model and series
	if( !m_model || m_model->seriesCount() != 1)
		return false;

	VectorFieldSeries* ser = new VectorFieldSeries(this);

	unsigned int vectorSampleSize = vectorFieldModel->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleCount).toUInt();
	const double* x = static_cast<const double*>(vectorFieldModel->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleXArray).value<void*>());
	const double* y = static_cast<const double*>(vectorFieldModel->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleYArray).value<void*>());
	const double* vx = static_cast<const double*>(vectorFieldModel->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleVXArray).value<void*>());
	const double* vy = static_cast<const double*>(vectorFieldModel->seriesData(0, AbstractVectorFieldSeriesModel::VectorSampleVYArray).value<void*>());
	try {
		ser->setData(vectorSampleSize, x, y, vx, vy);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
	}
	catch (std::exception & ex) {
		IBK::IBK_Message(ex.what(), IBK::MSG_ERROR, FUNC_ID);
	}

	// enable right axis, also for editing common properties in property widget
	m_model->setAxisData(true, AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
	// enable color legend (always on, not managed by model)
	m_colorLegend->setColorBarEnabled( true );

	m_zoomer->m_xUnit = QString::fromStdString( xunit.name() );
	m_zoomer->m_y1Unit = QString::fromStdString( yunit.name() );
	m_zoomer->m_valueUnit = QString::fromStdString( valueunit.name() );

	m_series.append(ser);

	return true;
}


bool Chart::removeSeries(unsigned int index) {
	Q_ASSERT(static_cast<int>(index) < m_series.size()); // series must exist

	ChartSeries* ser = m_series[(int)index];
	m_series.erase(m_series.begin() + index);
	delete ser;

	return true;
}


bool Chart::insertMarkerFromModel(unsigned int index) {
	Q_ASSERT(index <= (unsigned int)m_markers.count()); // no gaps allowed when inserting markers

	AbstractCartesianChartModel* model = dynamic_cast<AbstractCartesianChartModel*>(m_model);
	if (model == nullptr)
		return false;

#if (__cplusplus > 199711L)
	std::unique_ptr<PlotMarker> marker(new PlotMarker);
#else
	std::auto_ptr<PlotMarker> marker(new PlotMarker);
#endif
	marker->setProperties(model->markerData((int)index, AbstractCartesianChartModel::MarkerData).value<Marker>() );
	marker->attach(this);
	m_markers.insert((int)index, marker.release());

	return true;
}


bool Chart::removeMarker(unsigned int index) {
	Q_ASSERT(index < (unsigned int)m_markers.size());

	PlotMarker* m = m_markers[(int)index];
	m_markers.erase(m_markers.begin() + index);
	delete m;

	return true;
}


void Chart::addVerticalConstructionLine(double position, const QPen& pen) {
	QwtPlotMarker* marker = new QwtPlotMarker;
	marker->setLineStyle(QwtPlotMarker::VLine);
	marker->setZ(5);
	marker->setXValue(position);
	marker->setItemAttribute( QwtPlotItem::AutoScale, true );
	marker->setLinePen(pen);
	m_verticalConstructionLines.push_back(marker);
	marker->attach(this);
}


void Chart::removeVerticalConstructionLines() {
	for(int i=0; i<m_verticalConstructionLines.size(); ++i) {
		m_verticalConstructionLines[i]->detach();
		delete m_verticalConstructionLines[i];
	}
	m_verticalConstructionLines.clear();
}


void Chart::updateConstructionLines() {
	removeVerticalConstructionLines();
	// no series, nothing to do
	if (m_series.empty())
		return;
	ChartSeriesInfos::SeriesType chartSeriesType = (ChartSeriesInfos::SeriesType)m_model->chartType();
	bool visible = m_model->data(AbstractChartModel::ConstructionLineVisible).toBool();
	const DATAIO::ConstructionLines2D* clines= m_model->constructionLines();
	if (chartSeriesType == ChartSeriesInfos::LineSeries) {

		if ( visible && clines != nullptr && !clines->emptyVertical()) {
			QVariant value = m_model->data(AbstractChartModel::ConstructionLinePen);
			Q_ASSERT(value.canConvert<QPen>());

			// first inner construction lines
			QPen pen = value.value<QPen>();
			for(size_t i=0; i<clines->m_verticalConstructionLines.size(); ++i) {
				double pos = clines->m_verticalConstructionLines[i].m_pos;
				addVerticalConstructionLine(pos, pen);
			}

			// now boundary lines
			int width = pen.width();
			++width;
			++width;
			pen.setWidth(width);
			for(size_t i=0; i<clines->m_verticalBoundaryLines.size(); ++i) {
				double pos = clines->m_verticalBoundaryLines[i].m_pos;
				addVerticalConstructionLine(pos, pen);
			}
		}
	}
	else if( chartSeriesType == ChartSeriesInfos::ColorGridSeries) {
		ColorGridSeries* colorGridSeries = dynamic_cast<ColorGridSeries*>( m_series[0]);
		Q_ASSERT(colorGridSeries);
		if ( visible  && clines != nullptr && !clines->empty()) {
			colorGridSeries->spectrogram()->setConstructionLines(*clines);
			QVariant value = m_model->data(AbstractChartModel::ConstructionLinePen);
			if( value.canConvert<QPen>()) {
				colorGridSeries->spectrogram()->setConstructionLinePen( value.value<QPen>() );
			}
		}
		else {
			colorGridSeries->spectrogram()->setConstructionLinePen( QPen( Qt::NoPen ) );
			colorGridSeries->spectrogram()->setConstructionLines(DATAIO::ConstructionLines2D());
		}
	}
	else if( chartSeriesType == ChartSeriesInfos::VectorFieldSeries) {
		VectorFieldSeries* series = dynamic_cast<VectorFieldSeries*>( m_series[0]);
		Q_ASSERT(series);
		if ( visible  && clines != nullptr && !clines->empty()) {
			series->vectorField()->setConstructionLines(*clines);
			QVariant value = m_model->data(AbstractChartModel::ConstructionLinePen);
			if( value.canConvert<QPen>()) {
				series->vectorField()->setConstructionLinePen( value.value<QPen>() );
			}
		}
		else {
			series->vectorField()->setConstructionLinePen( QPen( Qt::NoPen ) );
			series->vectorField()->setConstructionLines(DATAIO::ConstructionLines2D());
		}
	}
}


void Chart::updateInternalAxisConfiguration() {
	bool enableInternalScales = m_model->data(AbstractChartModel::AxisScalesInside).toBool();


	// make internal scales visible
	m_internalBottomAxis->setVisible(enableInternalScales);
	m_internalTopAxis->setVisible(enableInternalScales);
	m_internalRightAxis->setVisible(enableInternalScales);
	m_internalLeftAxis->setVisible(enableInternalScales);

	// disable ticks and backbone of outside axes
	axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, !enableInternalScales);
	axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, !enableInternalScales);
	axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, !enableInternalScales);

	// set appropriate axis to right axis
	ChartSeriesInfos::SeriesType chartSeriesType = static_cast<ChartSeriesInfos::SeriesType>(m_model->chartType());
	if (m_model->axisData(AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled).toBool() &&
			chartSeriesType == ChartSeriesInfos::LineSeries)
	{
		m_internalRightAxis->setAxes(QwtPlot::xBottom, QwtPlot::yRight);
	}
	else {
		m_internalRightAxis->setAxes(QwtPlot::xBottom, QwtPlot::yLeft);
	}
	updateLayout();
}


void Chart::resetView() {
	const char * const FUNC_ID = "[Chart::resetView]";
	// freeze chart, allow no update to be triggered by the various setter function calls
	prohibitUpdate();

	IBK::IBK_Message("Chart view reset.\n", IBK::MSG_PROGRESS, FUNC_ID, IBK::VL_DEVELOPER);

//	m_model->setAxisData(false, AbstractCartesianChartModel::RightAxis, AbstractCartesianChartModel::AxisEnabled);
	m_colorLegend->setColorBarEnabled( false );

	if (m_model->chartType() == ChartSeriesInfos::LineSeries) {

		int serCount = m_model->seriesCount();
		for (int i=0; i<serCount; ++i) {
			bool result = insertLineSeriesFromModel(i);
			if (!result) {
				// FIXME: when line series has failed to insert line, we get an inconsistent model
				IBK::IBK_Message( "Error creating line series from model, probably incomplete model implementation.",
								  IBK::MSG_ERROR, FUNC_ID);
				allowUpdate();
				return;
			}
		}
	}
	else if( m_model->chartType() == ChartSeriesInfos::ColorGridSeries) {
		int serCount = m_model->seriesCount();
		if (serCount > 0) {
			bool result = insertColorGridSeriesFromModel();
			if (!result) {
				// FIXME: when line series has failed to insert line, we get an inconsistent model
				IBK::IBK_Message( "Failed to add ColorGridSeries to chart, probably due to missing/inconsistent data in ColorGridSeriesModel.",
								  IBK::MSG_ERROR, FUNC_ID);
				allowUpdate();
				return;
			}
		}
	}
	else if( m_model->chartType() == ChartSeriesInfos::BarSeries) {
		int serCount = m_model->seriesCount();
		if (serCount > 0) {
			bool result = insertBarSeriesFromModel();
			if (!result) {
				// FIXME: when line series has failed to insert line, we get an inconsistent model
				IBK::IBK_Message( "Error creating bar series from model, probably incomplete model implementation.",
								  IBK::MSG_ERROR, FUNC_ID);
				allowUpdate();
				return;
			}
		}
	}
	else if( m_model->chartType() == ChartSeriesInfos::VectorFieldSeries) {
		int serCount = m_model->seriesCount();
		if (serCount > 0) {
			bool result = insertVectorFieldSeriesFromModel();
			if (!result) {
				// FIXME: when line series has failed to insert line, we get an inconsistent model
				IBK::IBK_Message( "Error creating vector field series from model, probably incomplete model implementation.",
								  IBK::MSG_ERROR, FUNC_ID);
				allowUpdate();
				return;
			}
		}
	}

	// for all 3 axis positions
	for (int j=0; j<3; ++j) {
		for (int i=AbstractCartesianChartModel::AxisEnabled; i<AbstractCartesianChartModel::NUM_ADR; ++i)
			onAxisChanged(j,i);
	}

	if (m_model->chartType() == ChartSeriesInfos::LineSeries) {
		for (int i=AbstractLineSeriesModel::SeriesTitle; i<AbstractLineSeriesModel::NUM_SDR; ++i) {
			onSeriesViewChanged(0,m_series.size()-1,i);
		}
	}
	else if (m_model->chartType() == ChartSeriesInfos::ColorGridSeries) {
		for (int i=AbstractColorGridSeriesModel::XGridArray; i<AbstractColorGridSeriesModel::NUM_SDR; ++i) {
			onSeriesViewChanged(0,m_series.size()-1,i);
		}
	}
	else if (m_model->chartType() == ChartSeriesInfos::BarSeries) {
		for (int i=AbstractBarSeriesModel::SeriesTitle; i<AbstractBarSeriesModel::NUM_SDR; ++i) {
			onSeriesViewChanged(0,m_series.size()-1,i);
		}
	}
	else if (m_model->chartType() == ChartSeriesInfos::VectorFieldSeries) {
		for (int i=AbstractVectorFieldSeriesModel::VectorSampleCount; i<AbstractVectorFieldSeriesModel::NUM_SDR; ++i) {
			onSeriesViewChanged(0,m_series.size()-1,i);
		}
	}

	for (int i=AbstractChartModel::TitleText; i<AbstractChartModel::NUM_CDR; ++i)
		onChartChanged(i);

	// markers
	int markerCount = m_model->markerCount();
	for (int i=0; i<markerCount; ++i) {
		bool result = insertMarkerFromModel((unsigned int)i);
		if (!result) {
			// FIXME: when line series has failed to insert line, we get an inconsistent model
			IBK::IBK_Message( "Error inserting marker from model, probably incomplete model implementation.", IBK::MSG_ERROR, FUNC_ID);
			allowUpdate();
			return;
		}
	}


	configureChartFormat();
	updateInternalAxisConfiguration();
	allowUpdate();
	replot();

	double x1 = m_bottomAxis.minimum();
	double w = m_bottomAxis.maximum() - x1;
	double y1 = m_leftAxis.minimum();
	double h = m_leftAxis.maximum() - y1;
	QRectF zoomRect( x1, y1, w, h);
	m_zoomer->setZoomBase( zoomRect );
}


void Chart::setModel(AbstractChartModel* model) {
	if( m_model) {
		disconnect(m_model, &AbstractChartModel::chartChanged,					this, &Chart::onChartChanged);
		disconnect(m_model, &AbstractChartModel::axisChanged,					this, &Chart::onAxisChanged);

		disconnect(m_model, &AbstractChartModel::seriesViewChanged,				this, &Chart::onSeriesViewChanged);
		disconnect(m_model, &AbstractChartModel::seriesChanged,					this, &Chart::onSeriesChanged);
		disconnect(m_model, &AbstractChartModel::seriesAboutToBeRemoved,		this, &Chart::onSeriesAboutToBeRemoved);
		disconnect(m_model, &AbstractChartModel::seriesInserted,				this, &Chart::onSeriesInserted);

		disconnect(m_model, &AbstractCartesianChartModel::markerDataChanged,	this, &Chart::onMarkerDataChanged);
		disconnect(m_model, &AbstractCartesianChartModel::markerInserted,		this, &Chart::onMarkerInserted);
		disconnect(m_model, &AbstractCartesianChartModel::markerAboutToBeRemoved, this, &Chart::onMarkerAboutToBeRemoved);
		disconnect(m_model, &AbstractCartesianChartModel::markerChanged,		this, &Chart::onMarkerChanged);

		disconnect(m_model, &AbstractChartModel::modelAboutToBeReset,			this, &Chart::onModelAboutToBeReset);
		disconnect(m_model, &AbstractChartModel::modelReset,					this, &Chart::onSeriesModelReset);
		disconnect(m_model, &AbstractChartModel::modelAboutToBeChanged,			this, &Chart::prohibitUpdate);
		disconnect(m_model, &AbstractChartModel::modelChanged,					this, &Chart::allowUpdateAndReplot);

		disconnect(m_model, &AbstractChartModel::destroyed,						this, &Chart::onModelAboutToBeDeleted);
	}

	if( !m_series.empty()) {
		qDeleteAll(m_series);
		m_series.clear();
	}

	if (!m_markers.empty()) {
		qDeleteAll(m_markers);
		m_markers.clear();
	}

	m_model = model;

	m_zoomer->m_xUnit.clear();
	m_zoomer->m_y1Unit.clear();
	m_zoomer->m_y2Unit.clear();
	m_zoomer->m_valueUnit.clear();

	m_rescaler->setEnabled(false);

	if( !model) {
		replot();
		return;
	}

	connect(m_model, &AbstractChartModel::chartChanged,					this, &Chart::onChartChanged);
	connect(m_model, &AbstractChartModel::axisChanged,					this, &Chart::onAxisChanged);

	connect(m_model, &AbstractChartModel::seriesViewChanged,			this, &Chart::onSeriesViewChanged);
	connect(m_model, &AbstractChartModel::seriesChanged,				this, &Chart::onSeriesChanged);
	connect(m_model, &AbstractChartModel::seriesAboutToBeRemoved,		this, &Chart::onSeriesAboutToBeRemoved);
	connect(m_model, &AbstractChartModel::seriesInserted,				this, &Chart::onSeriesInserted);

	connect(m_model, &AbstractCartesianChartModel::markerDataChanged,	this, &Chart::onMarkerDataChanged);
	connect(m_model, &AbstractCartesianChartModel::markerInserted,		this, &Chart::onMarkerInserted);
	connect(m_model, &AbstractCartesianChartModel::markerAboutToBeRemoved, this, &Chart::onMarkerAboutToBeRemoved);
	connect(m_model, &AbstractCartesianChartModel::markerChanged,		this, &Chart::onMarkerChanged);

	connect(m_model, &AbstractChartModel::modelAboutToBeReset,			this, &Chart::onModelAboutToBeReset);
	connect(m_model, &AbstractChartModel::modelReset,					this, &Chart::onSeriesModelReset);
	connect(m_model, &AbstractChartModel::modelAboutToBeChanged,		this, &Chart::prohibitUpdate);
	connect(m_model, &AbstractChartModel::modelChanged,					this, &Chart::allowUpdateAndReplot);

	connect(m_model, &AbstractChartModel::destroyed,					this, &Chart::onModelAboutToBeDeleted);

	resetView();
}


void Chart::prohibitUpdate() {
	++m_noUpdateCounter;
}


void Chart::allowUpdate() {
	Q_ASSERT(m_noUpdateCounter > 0);
	--m_noUpdateCounter;
}


void Chart::allowUpdateAndReplot() {
	allowUpdate();
	replot();
}


void Chart::onSeriesViewChanged(int start, int end, int role) {
//	const char * const FUNC_ID = "[Chart::onSeriesViewChanged]";

	// no series? maybe a chart template
	if(m_series.empty())
		return;

	ChartSeriesInfos::SeriesType chartSeriesType = (ChartSeriesInfos::SeriesType)m_model->chartType();
	// handle change events from LineSeries model

	switch (chartSeriesType) {
		case ChartSeriesInfos::LineSeries : {
			for( int index=start; index<=end; ++index) {
				ChartSeries* series = m_series[index];
				LineSeries* lineSeries = dynamic_cast<LineSeries*>(series);
				switch (role) {
					case AbstractLineSeriesModel::SeriesSize : break; // do nothing but initiate a replot
					case AbstractLineSeriesModel::SeriesTitle :
					case AbstractLineSeriesModel::SeriesTitleText :
						series->setTitle(m_model->seriesData(index, AbstractLineSeriesModel::SeriesTitle).toString());
					break;
					case AbstractLineSeriesModel::SeriesPen :
						lineSeries->setPen(m_model->seriesData(index, role).value<QPen>());
					break;
					case AbstractLineSeriesModel::SeriesColor :
						lineSeries->setColor(m_model->seriesData(index, role).value<QColor>());
					break;
					case AbstractLineSeriesModel::SeriesWidth:
						lineSeries->setLineWidth(m_model->seriesData(index, role).toInt());
					break;
					case AbstractLineSeriesModel::SeriesMarkerSize:
						lineSeries->setMarkerSize(m_model->seriesData(index, role).toUInt());
					break;
					case AbstractLineSeriesModel::SeriesMarkerColor :
						lineSeries->setMarkerColor(m_model->seriesData(index, AbstractLineSeriesModel::SeriesMarkerColor).value<QColor>());
					break;
					case AbstractLineSeriesModel::SeriesMarkerFilled :
						lineSeries->setMarkerFilled(m_model->seriesData(index, AbstractLineSeriesModel::SeriesMarkerFilled).toBool());
					break;
					case AbstractLineSeriesModel::SeriesLineStyle: {
						int styleIndex = m_model->seriesData(index, role).toInt();
						QwtPlotCurve::CurveStyle style = static_cast<QwtPlotCurve::CurveStyle>(styleIndex);
						lineSeries->setType(style);
					} break;
					case AbstractLineSeriesModel::SeriesMarkerStyle:
						lineSeries->setMarkerStyle(m_model->seriesData(index, role).toInt());
					break;
					case AbstractLineSeriesModel::SeriesInverted:
						lineSeries->setInverted(m_model->seriesData(index, role).toBool());
					break;
					case AbstractLineSeriesModel::SeriesFitted:
						lineSeries->setFitted(m_model->seriesData(index, role).toBool());
					break;
					case AbstractLineSeriesModel::SeriesLeftYAxis: {
						bool leftAxis = m_model->seriesData(index, role).toBool();
						if (leftAxis)
							lineSeries->attachToLeftAxis();
						else
							lineSeries->attachToRightAxis();
					} break;
					case AbstractLineSeriesModel::SeriesInLegend:
						lineSeries->setLegendIconVisible(m_model->seriesData(index, role).toBool());
					break;
					case AbstractLineSeriesModel::SeriesBrush :
						lineSeries->setBrush(m_model->seriesData(index, role).value<QBrush>());
						break;
					default: return;
				}
			}
			replot();
		} break; // ChartSeriesInfos::LineSeries


		case ChartSeriesInfos::ColorGridSeries : {
			Q_ASSERT(start == 0 && end == 0);
			// get access to the color grid series (QwtPlot interface)
			ColorGridSeries* colorGridSeries = dynamic_cast<ColorGridSeries*>( m_series[0]);
			Q_ASSERT(colorGridSeries);
			// get access to the model that provides all data for the spectrogram series
	//		AbstractColorGridSeriesModel* colorGridSeriesModel = dynamic_cast<AbstractColorGridSeriesModel*>( m_model);
	//		Q_ASSERT(colorGridSeriesModel);
			switch( role) {
				case AbstractColorGridSeriesModel::DisplayMode : {
					QVariant value = m_model->seriesData(0, AbstractColorGridSeriesModel::DisplayMode);
					colorGridSeries->setDisplayMode( value.toInt() );

					onAxisChanged(AbstractCartesianChartModel::LeftAxis,AbstractCartesianChartModel::AxisGridVisible);
					onAxisChanged(AbstractCartesianChartModel::RightAxis,AbstractCartesianChartModel::AxisGridVisible);
				}
				break;

				case AbstractColorGridSeriesModel::ContourPen : {
					QVariant value = m_model->seriesData(0, AbstractColorGridSeriesModel::ContourPen);
					if (value.canConvert<QPen>()) {
						colorGridSeries->setContourPen( value.value<QPen>() );
					}
				}
				break;

				case AbstractColorGridSeriesModel::ContourLabelPlacement : {
					QVariant value = m_model->seriesData(0, AbstractColorGridSeriesModel::ContourLabelPlacement);
					colorGridSeries->setLabelPlacement( value.toUInt() );
				}
				break;

				case AbstractColorGridSeriesModel::ContourInterval : {
					QVariant value = m_model->seriesData(0, AbstractColorGridSeriesModel::ContourInterval);
					colorGridSeries->setContourInterval( value.toDouble() );
				}
				break;

				case AbstractColorGridSeriesModel::HasContourIntervalColor : {
					QVariant value = m_model->seriesData(0, AbstractColorGridSeriesModel::HasContourIntervalColor);
					colorGridSeries->setContourIntervalColor( value.toBool() );
				}
				break;

				case AbstractColorGridSeriesModel::ColorBarVisible : {
					bool colorBar = m_model->seriesData(0, AbstractColorGridSeriesModel::ColorBarVisible).toBool();
					enableAxis(QwtPlot::yRight, colorBar);
				} break;

				case AbstractColorGridSeriesModel::InterpolationMode : {
					QVariant value = m_model->seriesData(0, AbstractColorGridSeriesModel::InterpolationMode);
					int interpolationMode = value.toInt();
					// 0 - raw data: interpolation turned off, all others: interpolation on
					colorGridSeries->setInterpolation( interpolationMode != 0 );
				} break;

				// here are all series data roles that require a new color map to be created
				case AbstractColorGridSeriesModel::ZValueRange :
				case AbstractColorGridSeriesModel::AutoScale :
				case AbstractColorGridSeriesModel::MinimumZ :
				case AbstractColorGridSeriesModel::MaximumZ :
				case AbstractColorGridSeriesModel::ColorMap :
					break; // nothing to do here, update of color map happens below
			} // switch

			// *** Color Map Update ***

			// some data roles require re-setting of the color map, because in QwtPlot this is combined
			switch (role) {
				case AbstractColorGridSeriesModel::ZValueRange :
				case AbstractColorGridSeriesModel::AutoScale :
				case AbstractColorGridSeriesModel::MinimumZ :
				case AbstractColorGridSeriesModel::MaximumZ :
				case AbstractColorGridSeriesModel::InterpolationMode :
				case AbstractColorGridSeriesModel::ColorMap : {
					// get the color map (contains a description of the color map, but this is not a QwtColorMap object!)
					QVariant value = m_model->seriesData(0, AbstractColorGridSeriesModel::ColorMap);
					Q_ASSERT( value.canConvert<SCI::ColorMap>() );
					ColorMap colorMap = value.value<ColorMap>();

					// get value range
					value = m_model->seriesData(0, AbstractColorGridSeriesModel::ZValueRange);
					Q_ASSERT( value.canConvert<Range>() );
					Range zValueRange = value.value<Range>();

					// adjust axis
					setAxisScale(QwtPlot::yRight, zValueRange.first, zValueRange.second );

					// banded colors or not?

					value = m_model->seriesData(0, AbstractColorGridSeriesModel::InterpolationMode);
					int interpolationMode = value.toInt();
					// 0 - raw data: banded off
					// 1 - banded: banded on
					// 2 - continue: banded off
					bool bandedColors = (interpolationMode == 1);

					// set new color map in color legend (legend takes ownership)
					QwtInterval interval(zValueRange.first, zValueRange.second);
					m_colorLegend->setColorMap(interval, ColorMapCreator::createColorMap(colorMap, bandedColors) );

					// finally set the color map in the spectrogram chart (chart takes ownership)
					colorGridSeries->spectrogram()->setColorMap(ColorMapCreator::createColorMap(colorMap, bandedColors) );

					// needed for color map and contour intervals
					colorGridSeries->setZValueRange(interval);

					// also update contour lines (by default they are aligned to color bands in color map)
					value = m_model->seriesData(0, AbstractColorGridSeriesModel::ContourInterval);
					colorGridSeries->setContourInterval( value.toDouble() );

					/// \todo check functionality of border code below
					int border1, border2;
					m_colorLegend->getBorderDistHint(border1, border2);
					m_colorLegend->setBorderDist(border1, border2);
					break;
				}
			}

			replot();
		} break; // ChartSeriesInfos::ColorGridSeries


		case ChartSeriesInfos::BarSeries :  {
			Q_ASSERT(start == 0 && end == 0);
			// get access to the bar series (QwtPlot interface)
			BarSeries* barSeries = dynamic_cast<BarSeries*>( m_series[0]);
			Q_ASSERT(barSeries);
			// get access to the model that provides all data for the spectrogram series
			AbstractBarSeriesModel* barSeriesModel = dynamic_cast<AbstractBarSeriesModel*>( m_model);
			Q_ASSERT(barSeriesModel);
			switch( role) {
				case AbstractBarSeriesModel::SeriesTitle : {
					barSeries->setTitle(m_model->seriesData(0, AbstractBarSeriesModel::SeriesTitle).toString());
					break;
				}
				case AbstractBarSeriesModel::LayoutPolicy : {
					QwtPlotAbstractBarChart::LayoutPolicy layoutPolicy =
							static_cast<QwtPlotAbstractBarChart::LayoutPolicy>(barSeriesModel->seriesData(0, AbstractBarSeriesModel::LayoutPolicy).toInt());
					barSeries->setLayoutPolicy(layoutPolicy);
					break;
				}
				case AbstractBarSeriesModel::LayoutHint : {
					double layoutHint = barSeriesModel->seriesData(0, AbstractBarSeriesModel::LayoutHint).toDouble();
					barSeries->setLayoutHint(layoutHint);
					break;
				}
				case AbstractBarSeriesModel::Spacing : {
					int spacing = barSeriesModel->seriesData(0, AbstractBarSeriesModel::Spacing).toInt();
					barSeries->setSpacing(spacing);
					break;
				}
				case AbstractBarSeriesModel::Margin : {
					int margin = barSeriesModel->seriesData(0, AbstractBarSeriesModel::Margin).toInt();
					barSeries->setMargin(margin);
					break;
				}
				case AbstractBarSeriesModel::BaseLine : {
					double baseLine = barSeriesModel->seriesData(0, AbstractBarSeriesModel::BaseLine).toDouble();
					barSeries->setBaseline(baseLine);
					break;
				}
				case AbstractBarSeriesModel::BarChartStyle : {
					QwtPlotMultiBarChart::ChartStyle chartStyle =
							static_cast<QwtPlotMultiBarChart::ChartStyle>(barSeriesModel->seriesData(0, AbstractBarSeriesModel::BarChartStyle).toInt());
					barSeries->setBarChartStyle(chartStyle);
					break;
				}
				case AbstractBarSeriesModel::Orientation : {
					Qt::Orientation orientation =
							static_cast<Qt::Orientation>(barSeriesModel->seriesData(0, AbstractBarSeriesModel::Orientation).toInt());
					barSeries->setOrientation(orientation);
					break;
				}
				case AbstractBarSeriesModel::Information : {
					unsigned int valueSize = barSeriesModel->yValuesCount();
					for( unsigned int i=0; i<valueSize; ++i) {
						barSeries->setBarPalette(i, barSeriesModel->valuePalette(i));
						barSeries->setBarLineWidth(i, barSeriesModel->frameLineWidth(i));
						barSeries->setBarFrameStyle(i, barSeriesModel->frameStyle(i));
					}
					break;
				}
			}

			replot();
		} break; // ChartSeriesInfos::BarSeries


		case ChartSeriesInfos::VectorFieldSeries : {
			Q_ASSERT(start == 0 && end == 0);
			// get access to the series (QwtPlot interface)
			VectorFieldSeries* vectorSeries = dynamic_cast<VectorFieldSeries*>( m_series[0]);
			Q_ASSERT(vectorSeries);
			switch( role) {

				case AbstractVectorFieldSeriesModel::ColorBarVisible : {
					bool colorBarVisible = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ColorBarVisible).toBool();
					bool colorArrowsEnabled = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ColoredArrowsEnabled).toBool();
					enableAxis(QwtPlot::yRight, colorArrowsEnabled && colorBarVisible);
				} break;

				case AbstractVectorFieldSeriesModel::RasterEnabled : {
					bool rasterEnabled = m_model->seriesData(0, AbstractVectorFieldSeriesModel::RasterEnabled).toBool();
					vectorSeries->vectorField()->setRasterEnabled(rasterEnabled);
				} break;

				case AbstractVectorFieldSeriesModel::RasterSize : {
					unsigned int rasterSize = m_model->seriesData(0, AbstractVectorFieldSeriesModel::RasterSize).toUInt();
					vectorSeries->vectorField()->setRasterSize(QSizeF(rasterSize, rasterSize));
				} break;

				case AbstractVectorFieldSeriesModel::UniformArrowColor : {
					QColor c = m_model->seriesData(0, AbstractVectorFieldSeriesModel::UniformArrowColor).value<QColor>();
					vectorSeries->vectorField()->setArrowColor(c);
				} break;

				case AbstractVectorFieldSeriesModel::IndicatorOrigin : {
					int indicatorOrigin = m_model->seriesData(0, AbstractVectorFieldSeriesModel::IndicatorOrigin).toInt();
					vectorSeries->vectorField()->setIndicatorOrigin( (QwtPlotVectorField::IndicatorOrigin)indicatorOrigin );
				} break;

				case AbstractVectorFieldSeriesModel::ThinArrow :
				case AbstractVectorFieldSeriesModel::MaxArrowLength :
				case AbstractVectorFieldSeriesModel::MinArrowLength :
				case AbstractVectorFieldSeriesModel::ArrowScaleFactor :
				{
					bool thinArrow = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ThinArrow).toBool();
					double maxLength = m_model->seriesData(0, AbstractVectorFieldSeriesModel::MaxArrowLength).toDouble();
					double minLength = m_model->seriesData(0, AbstractVectorFieldSeriesModel::MinArrowLength).toDouble();
					double scaleFactor = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ArrowScaleFactor).toDouble();
					vectorSeries->vectorField()->setArrowAttributes(thinArrow, maxLength, minLength, scaleFactor);
				} break;

			} // switch

			// *** Color Map Update ***

			// some data roles require re-setting of the color map, because in QwtPlot this is combined
			switch (role) {
				case AbstractVectorFieldSeriesModel::ZValueRange :
				case AbstractVectorFieldSeriesModel::AutoScale :
				case AbstractVectorFieldSeriesModel::MinimumZ :
				case AbstractVectorFieldSeriesModel::MaximumZ :
				case AbstractVectorFieldSeriesModel::ColorMap :
				case AbstractVectorFieldSeriesModel::ColoredArrowsEnabled : {
					bool colorArrowsEnabled = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ColoredArrowsEnabled).toBool();
					vectorSeries->vectorField()->setMagnitudeMode( QwtPlotVectorField::MagnitudeAsColor, colorArrowsEnabled);
					// adjust right axis visibility
					bool colorBarVisible = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ColorBarVisible).toBool();
					enableAxis(QwtPlot::yRight, colorArrowsEnabled && colorBarVisible);

					if (!colorArrowsEnabled)
						break;

					// get the color map (contains a description of the color map, but this is not a QwtColorMap object!)
					QVariant value = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ColorMap);
					Q_ASSERT( value.canConvert<SCI::ColorMap>() );
					ColorMap colorMap = value.value<ColorMap>();

					// get value range
					value = m_model->seriesData(0, AbstractVectorFieldSeriesModel::ZValueRange);
					Q_ASSERT( value.canConvert<Range>() );
					Range zValueRange = value.value<Range>();

					// adjust axis
					setAxisScale(QwtPlot::yRight, zValueRange.first, zValueRange.second );

					// set new color map in color legend (legend takes ownership)
					QwtInterval interval(zValueRange.first, zValueRange.second);
					m_colorLegend->setColorMap(interval, ColorMapCreator::createColorMap(colorMap, false) );

					// finally set the color map in the spectrogram chart (chart takes ownership)
					vectorSeries->vectorField()->setColorMap(ColorMapCreator::createColorMap(colorMap, false) );

					// needed for color map and contour intervals
					vectorSeries->setZValueRange(interval);

					/// \todo check functionality of border code below
					int border1, border2;
					m_colorLegend->getBorderDistHint(border1, border2);
					m_colorLegend->setBorderDist(border1, border2);
				} break;
				default: ; // all other roles do not require a color map update
			} // switch - color map update

			replot();

		} break; // ChartSeriesInfos::VectorFieldSeries

		default:
			Q_ASSERT(false); // nothing to do - should we throw an exception here?
	} // switch type
}


void Chart::onSeriesInserted(int start, int end) {
	ChartSeriesInfos::SeriesType chartSeriesType = static_cast<ChartSeriesInfos::SeriesType>(m_model->chartType());
	if (chartSeriesType == ChartSeriesInfos::LineSeries) {
		for( int i=start; i<=end; ++i) {
			bool result = insertLineSeriesFromModel(i);
			Q_ASSERT_X(result, "Chart::onSeriesInserted", "Error in SeriesModel. Series cannot be inserted");
		}

		replot();
	}
	if (chartSeriesType == ChartSeriesInfos::BarSeries) {
		bool result = insertBarSeriesFromModel();
		Q_ASSERT_X(result, "Chart::onSeriesInserted", "Error in SeriesModel. Series cannot be inserted");

		replot();
	}
}


void Chart::onSeriesAboutToBeRemoved(int start, int end ) {
	ChartSeriesInfos::SeriesType chartSeriesType = static_cast<ChartSeriesInfos::SeriesType>(m_model->chartType());
	if (chartSeriesType == ChartSeriesInfos::LineSeries || chartSeriesType == ChartSeriesInfos::BarSeries) {
		for( int i=end; i>=start; --i) {
			bool result = removeSeries( (unsigned int)i);
			Q_ASSERT_X(result, "Chart::onSeriesAboutToBeRemoved", "Error in SeriesModel. Series cannot be removed");
		}

		replot();
	}
}


void Chart::onSeriesAboutToBeChanged(int start, int end ) {
	for( int i=start; i<=end; ++i) {
		m_series[i]->lockSeries();
	}
}


void Chart::onSeriesChanged(int start, int end) {
	if(end < m_series.size()) {
		for( int i=start; i<=end; ++i) {
			m_series[i]->unlockSeries();
			// Check if our data has changed, since we use "raw pointer" series,
			// we must check if the raw data has changed
			// knowledge about this has only the model, so we request it to give
			// us size and pointers of the data
			ChartSeriesInfos::SeriesType chartSeriesType = static_cast<ChartSeriesInfos::SeriesType>(m_model->chartType());
			if (chartSeriesType == ChartSeriesInfos::LineSeries) {
				AbstractLineSeriesModel* lineModel = dynamic_cast<AbstractLineSeriesModel*>(m_model);
				if(lineModel != nullptr) {
					// let line series model give us new raw pointers
					int lineSeriesSize = lineModel->seriesData(i, AbstractLineSeriesModel::SeriesSize).toInt();
					const double * xPtr = lineModel->xValues(i);
					const double * yPtr = lineModel->yValues(i);
					LineSeries * lineSeries = dynamic_cast<LineSeries *>(m_series[i]);
					lineSeries->setData(lineSeriesSize, xPtr, yPtr);
				}
			}
			else if (chartSeriesType == ChartSeriesInfos::BarSeries) {
				AbstractBarSeriesModel* barModel = dynamic_cast<AbstractBarSeriesModel*>(m_model);
				if(barModel != nullptr) {
					// let line series model give us new raw pointers
					BarSeries * barSeries = dynamic_cast<BarSeries *>(m_series[i]);
					Q_ASSERT(barSeries != nullptr);
					barSeries->setData(barModel);
				}
			}

		}
	}
	replot();
}


// *** Marker code ***

void Chart::onMarkerDataChanged(unsigned int start, unsigned int end, int role) {

	// no markers? maybe a chart template
	if (m_markers.empty())
		return;

	IBK_ASSERT(start <= end && end < (unsigned int)m_markers.count());

	for (int index=(int)start; index<=(int)end; ++index) {
		PlotMarker* marker = m_markers[index];
		switch ( (AbstractCartesianChartModel::MarkerDataRole)role) {
			case AbstractCartesianChartModel::MarkerType:
				marker->setLineStyle( (QwtPlotMarker::LineStyle) m_model->markerData(index, role).toInt() ); break;

			case AbstractCartesianChartModel::MarkerXPosDateTime:
			case AbstractCartesianChartModel::MarkerXPos:
				marker->setXValue( m_model->markerData(index, role).toDouble() ); break;

			case AbstractCartesianChartModel::MarkerYPos:
				marker->setYValue( m_model->markerData(index, role).toDouble() ); break;

			case AbstractCartesianChartModel::MarkerXAxisID:
				marker->setXAxis( m_model->markerData(index, role).toInt() ); break;

			case AbstractCartesianChartModel::MarkerYAxisID:
				marker->setYAxis( m_model->markerData(index, role).toInt() ); break;

			case SCI::AbstractCartesianChartModel::MarkerLabel:
				marker->setLabel( m_model->markerData(index, role).toString() ); break;

			case SCI::AbstractCartesianChartModel::MarkerLabelFont: {
				QwtText t = marker->label();
				t.setFont( m_model->markerData(index, role).value<QFont>() );
				marker->setLabel(t);
			} break;

			case SCI::AbstractCartesianChartModel::MarkerLabelVAligment:
			case SCI::AbstractCartesianChartModel::MarkerLabelAligment:
				// NOTE: both roles deliver the same alignment value, as Qt::Alignment combines both vertical and horizontal alignments
				marker->setLabelAlignment( (Qt::Alignment) m_model->markerData(index, role).toInt() ); break;

			case SCI::AbstractCartesianChartModel::MarkerLabelOrientation:
				marker->setLabelOrientation( (Qt::Orientation) m_model->markerData(index, role).toInt() ); break;

			case SCI::AbstractCartesianChartModel::MarkerSpacing:
				marker->setSpacing( m_model->markerData(index, role).toInt() ); break;

			case SCI::AbstractCartesianChartModel::MarkerPen:
				marker->setLinePen(m_model->markerData(index, role).value<QPen>()); break;

			case SCI::AbstractCartesianChartModel::MarkerPenWidth: {
				QPen p = marker->linePen();
				p.setWidth(m_model->markerData(index, role).toInt());
				marker->setLinePen(p);
			} break;

			case SCI::AbstractCartesianChartModel::MarkerPenColor: {
				QPen p = marker->linePen();
				p.setColor(m_model->markerData(index, role).value<QColor>());
				marker->setLinePen(p);
			} break;

			case SCI::AbstractCartesianChartModel::MarkerZOrder:
				marker->setZ( m_model->markerData(index, role).toInt() ); break;

			// just to make compiler happy
			case SCI::AbstractCartesianChartModel::MarkerData:
			case SCI::AbstractCartesianChartModel::NUM_MDR:
				break;
		}
	} // for
	replot();
}


void Chart::onMarkerInserted(unsigned int start, unsigned int end) {
	IBK_ASSERT(start <= end && end <= (unsigned int)m_markers.count());
	for (unsigned int i=start; i<=end; ++i) {
		bool result = insertMarkerFromModel(i);
		Q_ASSERT_X(result, "Chart::onMarkerInserted", "Error in CartesianModel. Marker cannot be inserted");
	}

	replot();
}


void Chart::onMarkerAboutToBeRemoved(unsigned int start, unsigned int end ) {
	IBK_ASSERT(start <= end && end < (unsigned int)m_markers.count());
	// CAUTION: index must be an int as we run below 0 in this loop!
	for (int i=(int)end; i>=(int)start; --i) {
		bool result = removeMarker((unsigned int)i);
		Q_ASSERT_X(result, "Chart::onMarkerAboutToBeRemoved", "Error in CartesianModel. Marker cannot be removed");
	}

	replot();
}


void Chart::onMarkerChanged(unsigned int start, unsigned int end) {
	IBK_ASSERT(start <= end && end < (unsigned int)m_markers.count());
	AbstractCartesianChartModel * model = dynamic_cast<AbstractCartesianChartModel *>(m_model);
	if (model != nullptr) {
		for (int i=(int)start; i<=(int)end; ++i)
			m_markers[i]->setProperties( m_model->markerData(i, AbstractCartesianChartModel::MarkerData).value<Marker>() );
	}
	replot();
}



void Chart::onModelAboutToBeReset( ) {
	foreach(ChartSeries* series, m_series) {
		series->lockSeries();
	}
}


void Chart::onSeriesModelReset() {
	// get rid of existing series, this will never trigger an update
	qDeleteAll(m_series);
	m_series.clear();

	qDeleteAll(m_markers);
	m_markers.clear();
	// now reset the view, i.e. rebuild view from model
	resetView();
}


void Chart::onModelAboutToBeDeleted( ) {
	IBK::IBK_Message("Chart model about to be deleted, setting m_model pointer to nullptr.\n", IBK::MSG_PROGRESS,
		"[Chart::onModelAboutToBeDeleted]", IBK::VL_DEVELOPER);
	m_model = nullptr;
}


void Chart::onAxisChanged( int axisPos, int role ) {
	const char * const FUNC_ID = "[Chart::onAxisChanged]";
	AbstractChartModel::AxisPosition axisPosition = (AbstractChartModel::AxisPosition)axisPos;
	ChartSeriesInfos::SeriesType chartSeriesType = static_cast<ChartSeriesInfos::SeriesType>(m_model->chartType());
	switch( role) {

		case AbstractCartesianChartModel::AxisUnit: {
			unsigned int unitId = m_model->axisData(axisPosition, role).toUInt();
			// change of unit must be communicated to picker and zoomer
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:
					m_zoomer->m_xUnit = QString::fromStdString(IBK::Unit(unitId).name());
					break;
				case AbstractCartesianChartModel::LeftAxis:
					m_zoomer->m_y1Unit = QString::fromStdString(IBK::Unit(unitId).name());
					break;
				case AbstractCartesianChartModel::RightAxis : {
					// special handling for right axis - if not visible, or color map plot, picker get's empty
					// unit which means "do not show value for this unit"
					if (!m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisEnabled).toBool() ||
						chartSeriesType != ChartSeriesInfos::LineSeries)
					{
						m_zoomer->m_y2Unit.clear();
					}
					else {
						m_zoomer->m_y2Unit = QString::fromStdString(IBK::Unit(unitId).name());
					}
				}
				// update left scale based on bottom scale, but this must be made *after* the chart has been updated
				if (m_rescaler->isEnabled()) {
					updateAxes();
					m_rescaler->rescale();
				}
				break;
			}
		} // fall-through

		Q_FALLTHROUGH();  // fall-through, unit has impact on title text due to placeholders

		case AbstractCartesianChartModel::AxisTitleText:
		case AbstractCartesianChartModel::AxisTitle: {
			QString title = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisTitle).toString();
			int spacing = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisTitleSpacing).toInt();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:
					m_bottomAxis.setTitle(title);
					m_bottomAxis.setTitleSpacing(spacing); break;
				case AbstractCartesianChartModel::LeftAxis:
					m_leftAxis.setTitle(title);
					m_leftAxis.setTitleSpacing(spacing); break;
				case AbstractCartesianChartModel::RightAxis:
					m_rightAxis.setTitle(title);
					m_rightAxis.setTitleSpacing(spacing); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisTitleFont: {
			QFont font = m_model->axisData(axisPosition, role).value<QFont>();
			int spacing = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisTitleSpacing).toInt();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:
					m_bottomAxis.setTitleFont(font);
					m_bottomAxis.setTitleSpacing(spacing); break;
				case AbstractCartesianChartModel::LeftAxis:
					m_leftAxis.setTitleFont(font);
					m_leftAxis.setTitleSpacing(spacing); break;
				case AbstractCartesianChartModel::RightAxis:
					m_rightAxis.setTitleFont(font);
					m_rightAxis.setTitleSpacing(spacing); break;
			}
			break;
		}

		case AbstractCartesianChartModel::AxisAutoScale:
		case AbstractCartesianChartModel::AxisMaximum:
		case AbstractCartesianChartModel::AxisMinimum: {
			bool autoScale = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisAutoScale).toBool();
			if (autoScale) {
				switch (axisPosition) {
					case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setAutoScaleEnabled(true); break;
					case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setAutoScaleEnabled(true); break;
					case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setAutoScaleEnabled(true); break;
				}
			}
			else {
				double minimum = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisMinimum).toDouble();
				double maximum = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisMaximum).toDouble();
				switch (axisPosition) {
					case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setMinMax(minimum, maximum); break;
					case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setMinMax(minimum, maximum); break;
					case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setMinMax(minimum, maximum); break;
				}
			}
			// any change of axis requires an update of the axis scales - also because
			// of the proportional flag, which can be disabled; in this case the previously apply axis
			// scalings have to be applied accordingly
			updateAxes();
			// update left scale based on bottom scale, but this must be made *after* the chart has been updated
			if (m_rescaler->isEnabled()) {
				double minimum = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisMinimum).toDouble();
				bool proportionalEnabled = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisProportional).toBool();
				if (proportionalEnabled)
					m_rescaler->m_minValueOffset = minimum;
				m_rescaler->rescale();
			}
			break;
		}

		case AbstractCartesianChartModel::AxisProportional : {
			// this property is generally ignored for non-colorgrid plots and right axis
			if (axisPosition == AbstractChartModel::RightAxis)
				break;
			SCI::AbstractColorGridSeriesModel * colorGridModel = qobject_cast<SCI::AbstractColorGridSeriesModel*>(m_model);
			if (colorGridModel == nullptr)
				break;

			// there is only one scaler per chart, so we need to treat this property for both axis in
			// a centralized way
			bool proportionalBottomAxisEnabled = m_model->axisData(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisProportional).toBool();
			bool proportionalLeftAxisEnabled = m_model->axisData(AbstractChartModel::LeftAxis, AbstractCartesianChartModel::AxisProportional).toBool();
			// if both are turned off, we turn the scaler off
			if (!proportionalBottomAxisEnabled && !proportionalLeftAxisEnabled) {
				if (m_rescaler->isEnabled()) { // additional check added, because during chart setup this function is called several times
					// deactivate rescaler
					m_rescaler->setEnabled(false);
					// restore axis properties, this will trigger updateAxes()
					if (axisPosition == AbstractChartModel::LeftAxis)
						onAxisChanged(AbstractChartModel::LeftAxis, AbstractCartesianChartModel::AxisAutoScale);
					else
						onAxisChanged(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisAutoScale);
				}
				break;
			}


			// *** proportional enabled ***

			// first toggle other axis proportional setting if both are enabled
			if (proportionalBottomAxisEnabled && proportionalLeftAxisEnabled) {
				if (axisPosition == AbstractChartModel::LeftAxis) {
					m_model->setAxisData(false, AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisProportional);
					proportionalBottomAxisEnabled = false;
				}
				else {
					m_model->setAxisData(false, AbstractChartModel::LeftAxis, AbstractCartesianChartModel::AxisProportional);
					proportionalLeftAxisEnabled = false;
				}
			}
			// configure rescaler
			if (proportionalLeftAxisEnabled)
				m_rescaler->setAdjustingLeftAxis(true);
			else
				m_rescaler->setAdjustingLeftAxis(false);
			double minimum = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisMinimum).toDouble();
			m_rescaler->m_minValueOffset = minimum;
			m_rescaler->setEnabled(true);
			m_rescaler->rescale();
			break;
		}

		// enabling/disabling date time is only possible for bottom axis
		case AbstractCartesianChartModel::AxisDateTime: {
			bool isDateTime = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisDateTime).toBool();
			// datetime can only be *enabled* for bottom axis, but during regular
			// model setup, the datetime flag will be false for all axis
			if (axisPosition == AbstractCartesianChartModel::BottomAxis)
				m_bottomAxis.setDateTime(isDateTime);
		} break;

		case AbstractCartesianChartModel::AxisLogarithmic: {
			bool isDateTime = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisDateTime).toBool();
			// only go on if axis is not a datetime axis
			if (!isDateTime) {
				bool logarithmic = m_model->axisData(axisPosition, role).toBool();
				if (chartSeriesType == ChartSeriesInfos::LineSeries) {
					switch (axisPosition) {
						case AbstractCartesianChartModel::BottomAxis:
							if( logarithmic)	m_bottomAxis.setLogarithmic();
							else				m_bottomAxis.setLinear();
							break;
						case AbstractCartesianChartModel::LeftAxis:
							if( logarithmic)	m_leftAxis.setLogarithmic();
							else				m_leftAxis.setLinear();
							break;
						case AbstractCartesianChartModel::RightAxis:
							if( logarithmic)	m_rightAxis.setLogarithmic();
							else				m_rightAxis.setLinear();
							break;
					}
				}
				else {
					if (logarithmic)
						// this can only happen if someone hacks the project file... but still worth an error message
						IBK::IBK_Message("Cannot set logarithmic axis for colormap plot.", IBK::MSG_ERROR, FUNC_ID);
				}
			}
		} break;

		case AbstractCartesianChartModel::AxisDateTimeFormats: {
			// Warning: implementation assumes that during a model reset, the DateTime property was set first!
			QStringList formats = m_model->axisData(axisPosition, role).toStringList();
			bool isDateTime = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisDateTime).toBool();
			if (isDateTime) {
				IBK_ASSERT(axisPosition == AbstractCartesianChartModel::BottomAxis);
				m_bottomAxis.setDateTimeFormats(formats);
			}

			break;
		}
		case AbstractCartesianChartModel::AxisEnabled: {
			bool enabled = m_model->axisData(axisPosition, role).toBool();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setVisible(enabled); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setVisible(enabled); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setVisible(enabled); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisLabelFont: {
			QFont font = m_model->axisData(axisPosition, role).value<QFont>();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setLabelFont(font); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setLabelFont(font); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setLabelFont(font); break;
			}
			break;
		}

		case AbstractCartesianChartModel::AxisLabelHAlignment:
		case AbstractCartesianChartModel::AxisLabelVAlignment: {
			int Halignment = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisLabelHAlignment).toInt();
			int Valignment = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisLabelVAlignment).toInt();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setAxisLabelAlignment(Halignment, Valignment); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setAxisLabelAlignment(Halignment, Valignment); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setAxisLabelAlignment(Halignment, Valignment); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisLabelRotation: {
			double rotation = m_model->axisData(axisPosition, role).toDouble();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setAxisLabelRotation(rotation); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setAxisLabelRotation(rotation); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setAxisLabelRotation(rotation); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisTitleSpacing: {
			int spacing = m_model->axisData(axisPosition, role).toInt();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setTitleSpacing(spacing); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setTitleSpacing(spacing); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setTitleSpacing(spacing); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisLabelSpacing: {
			int spacing = m_model->axisData(axisPosition, role).toInt();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setLabelSpacing(spacing); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setLabelSpacing(spacing); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setLabelSpacing(spacing); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisMaxMinorScale: {
			int maxMinor = m_model->axisData(axisPosition, role).toInt();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setAxisMaxMinor( maxMinor ); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setAxisMaxMinor( maxMinor ); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setAxisMaxMinor( maxMinor ); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisMaxMajorScale: {
			int maxMajor = m_model->axisData(axisPosition, role).toInt();
			switch (axisPosition) {
				case AbstractCartesianChartModel::BottomAxis:	m_bottomAxis.setAxisMaxMajor( maxMajor ); break;
				case AbstractCartesianChartModel::LeftAxis:		m_leftAxis.setAxisMaxMajor( maxMajor ); break;
				case AbstractCartesianChartModel::RightAxis:	m_rightAxis.setAxisMaxMajor( maxMajor ); break;
			}
			break;
		}
		case AbstractCartesianChartModel::AxisGridVisible:
		case AbstractCartesianChartModel::AxisMinorGridVisible: {
			bool visible = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisGridVisible).toBool();
			bool visibleMinor = m_model->axisData(axisPosition, AbstractCartesianChartModel::AxisMinorGridVisible).toBool();
			if( axisPosition == AbstractCartesianChartModel::BottomAxis) {
				m_grid->enableX( visible );
				m_grid->enableXMin( visibleMinor );
			}
			else if( axisPosition == AbstractCartesianChartModel::LeftAxis) {
				m_grid->enableY( visible );
				m_grid->enableYMin( visibleMinor );
			}
			break;
		}

		case AbstractCartesianChartModel::AxisGridPen:
		case AbstractCartesianChartModel::AxisGridPenColor:
		case AbstractCartesianChartModel::AxisGridPenWidth: {
			// all properties are stored in pen property, so we can implement them all at once
			QPen pen = m_model->axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisGridPen).value<QPen>();
			m_grid->setMajorPen( pen );
		} break;

		case AbstractCartesianChartModel::AxisMinorGridPen:
		case AbstractCartesianChartModel::AxisMinorGridPenColor:
		case AbstractCartesianChartModel::AxisMinorGridPenWidth: {
			// all properties are stored in pen property, so we can implement them all at once
			QPen pen = m_model->axisData(AbstractCartesianChartModel::BottomAxis, AbstractCartesianChartModel::AxisMinorGridPen).value<QPen>();
			m_grid->setMinorPen( pen );
		} break;

	}
	replot();
}


void Chart::onChartChanged( int role ) {

	switch( role) {
		case AbstractChartModel::TitleText: {
			QString title = m_model->data( AbstractChartModel::Title ).toString(); // take title text with replaced placeholders
			QwtPlot::setTitle(title);
			break;
		}
		case AbstractChartModel::TitleFont: {
			QFont font = m_model->data(role).value<QFont>();
			QwtPlot::titleLabel()->setFont(font);
			break;
		}
		case AbstractChartModel::LegendVisible: {
			bool visible = m_model->data(role).toBool();
			// not visible, remove both external legend widget and legend item if any of those exist
			if (!visible) {
				if (m_legend != nullptr) {
					insertLegend(nullptr); // qwtplot deletes old legend
					m_legend = nullptr;
				}
				if (m_legendItem != nullptr) {
					delete m_legendItem;
					m_legendItem = nullptr;
				}
			}
			else {
				// this will recreate the legend if needed and populate it
				// with all properties
				onChartChanged(AbstractChartModel::LegendPosition);
			}
			break;
		}
		case AbstractChartModel::LegendPosition: {
			if ( !m_model->data(AbstractChartModel::LegendVisible).toBool() ) return;

			AbstractChartModel::LegendPositionType type = static_cast<AbstractChartModel::LegendPositionType>(m_model->data(role).toInt());
			// get position of legend widget from layout - mind that this position will be also returned when
			// there is no legend item and instead a legend item (embedded legend) is shown
			AbstractChartModel::LegendPositionType oldPos = static_cast<AbstractChartModel::LegendPositionType>(this->plotLayout()->legendPosition());
			if (m_legendItem != nullptr)
				oldPos = AbstractChartModel::LegendInChart; // set to correct legend position type
			prohibitUpdate();
			if ( type != oldPos || (m_legend == nullptr && m_legendItem == nullptr)) {
				// distinguish between legend widget and embedded legend item
				if (type == AbstractChartModel::LegendInChart) {
					// delete external legend
					insertLegend(nullptr);
					m_legend = nullptr;

					// create embedded legend item
					m_legendItem = new LegendItem;
					m_legendItem->attach(this);
				}
				else {
					// delete legend item, if exists
					delete m_legendItem;
					m_legendItem = nullptr;

					// old legend will be deleted internally
					// constructor of SCI::Legend call Chart::insertLegend and with this the legend will be connected to the chart
					m_legend = new SCI::Legend(this, (QwtPlot::LegendPosition)type);
				}

			}
			// re-use onChartChanged() to set all other legend properties
			for( int i=AbstractChartModel::LegendPosition+1; i<=AbstractChartModel::LegendFont; ++i) {
				// this will now update the properties in the legend one-by-one, but
				// since replots are disabled, this won't cost much performance
				onChartChanged(i);
			}
			allowUpdate(); // re-allow updates, at end of function replot() is called and all changes take effect

			break;
		}
		case AbstractChartModel::LegendItemAlignment: {
			if ( !m_model->data(AbstractChartModel::LegendVisible).toBool() ) return;

			SCI::AbstractChartModel::LegendPositionType positionType = static_cast<SCI::AbstractChartModel::LegendPositionType>(
						m_model->data(AbstractChartModel::LegendPosition).toInt());
			// if not an external legend, do nothing
			if (positionType != SCI::AbstractChartModel::LegendInChart)
				return;

			Q_ASSERT(m_legendItem != nullptr);
			int alignment = m_model->data(role).toInt(); // this is an index matching left, topleft, top, topright, ...
			Qt::Alignment a;
			switch (alignment) {
				case 0 : a = Qt::AlignLeft   | Qt::AlignVCenter; break;
				case 1 : a = Qt::AlignLeft   | Qt::AlignTop; break;
				case 2 : a = Qt::AlignHCenter | Qt::AlignTop; break;
				case 3 : a = Qt::AlignTop    | Qt::AlignRight; break;
				case 4 : a = Qt::AlignRight  | Qt::AlignVCenter; break;
				case 5 : a = Qt::AlignRight  | Qt::AlignBottom; break;
				case 6 : a = Qt::AlignHCenter | Qt::AlignBottom; break;
				case 7 : a = Qt::AlignLeft | Qt::AlignBottom; break;
			}
			m_legendItem->setAlignmentInCanvas(a);

			break;
		}
		case AbstractChartModel::LegendIconStyle: {
			if (m_model->chartType() != ChartSeriesInfos::LineSeries )
				break;

			int iconStyle = m_model->data(role).toInt();
			for ( int i = 0, endI = m_series.size(); i < endI; ++i ) {
				static_cast<LineSeries *>(m_series[i])->setLegendIconStyle( iconStyle );
				// width of icon is only set when drawing lines
				if (iconStyle == AbstractChartModel::LegendIconLineWithSymbol) {
					int width = m_model->data(AbstractChartModel::LegendIconWidth).toInt();
					QSize size;
					size.setHeight(0);
					size.setWidth( width );
					static_cast<LineSeries *>(m_series[i])->setLegendIconSize( size );
				}
				else {
					// rectangles always get square size
					static_cast<LineSeries *>(m_series[i])->setLegendIconSize( QSize(9,9) );
				}
			}
			break;
		}
		case AbstractChartModel::LegendIconWidth: {
			if (m_model->chartType() != ChartSeriesInfos::LineSeries )
				break;

			int iconStyle = m_model->data(AbstractChartModel::LegendIconStyle).toInt();
			if (iconStyle != AbstractChartModel::LegendIconLineWithSymbol )
				break;

			int width = m_model->data(role).toInt();

			QSize size;
			size.setHeight(0);
			size.setWidth( width );
			for ( int i = 0, endI = m_series.size(); i < endI; ++i ){
				static_cast<LineSeries *>(m_series[i])->setLegendIconSize( size );
			}
			break;
		}
		case AbstractChartModel::LegendMaxColumns: {
			unsigned int maxCols = m_model->data(role).toUInt();
			if (m_legend)
				m_legend->setMaxColumns(maxCols);
			if (m_legendItem)
				m_legendItem->setMaxColumns(maxCols);
			break;
		}
		case AbstractChartModel::LegendItemFrame: {
			if (!m_legendItem) break;
			bool frame = m_model->data(role).toBool();
//			QFrame::Shape shape = frame ? QFrame::Box : QFrame::NoFrame;
			if (frame)
				m_legendItem->setBorderPen(QPen(Qt::black));
			else
				m_legendItem->setBorderPen(Qt::NoPen);
			break;
		}
		case AbstractChartModel::LegendItemBackgroundColor:
		case AbstractChartModel::LegendItemBackgroundTransparency: {
			if (!m_legendItem)	break;
			double transp = m_model->data(AbstractChartModel::LegendItemBackgroundTransparency).toDouble();
			QColor col = m_model->data(AbstractChartModel::LegendItemBackgroundColor).value<QColor>();
			col.setAlpha(int(transp * 255));
			m_legendItem->setBackgroundBrush(col);
			break;
		}
		case AbstractChartModel::LegendItemOffset: {
			if (!m_legendItem)	break;
			QSize offset = m_model->data(role).toSize();
			m_legendItem->setOffsetInCanvas(Qt::Horizontal, offset.width());
			m_legendItem->setOffsetInCanvas(Qt::Vertical, offset.height());
			break;
		}
		case AbstractChartModel::LegendSpacing: {
			int space = m_model->data(role).toInt();
			if (m_legend)
				m_legend->setSpacing(space);
			if (m_legendItem)
				m_legendItem->setItemSpacing(space);
			break;
		}
		case AbstractChartModel::LegendFont: {
			QFont font = m_model->data(role).value<QFont>();
			// update external legend, if available
			if (m_legend) {
				m_legend->setFont(font);
				// we need to set the font in ALL plot item titles
				QwtPlotItemList& itmList = const_cast<QwtPlotItemList&>(itemList());
				for (QwtPlotItem * item : itmList) {
					QwtText titleText = item->title();
					titleText.setFont(font);
					item->setTitle(titleText);
				}
				updateLegend();
			}
			// update internal legend item if available
			if (m_legendItem)
				m_legendItem->setFont(font);
		} break;
		case AbstractChartModel::TrackerLegendEnabled: {
			bool enabled = m_model->data(role).toBool();
			if (enabled) {
				// hide the picker and show the tracker instead
				m_zoomer->m_trackerTextVisible = false;
				m_curveTracker->setVisible(true);
			}
			else {
				// show the picker and hide the curve tracker
				m_zoomer->m_trackerTextVisible = true;
				m_curveTracker->setVisible(false);
			}
		}
		break;

		case AbstractChartModel::TrackerLegendNumberFormat:
			m_curveTracker->setNumberFormat(m_model->data(role).toChar().toLatin1());
			break;

		case AbstractChartModel::TrackerLegendNumberPrecision:
			m_curveTracker->setNumberPrecision(m_model->data(role).toInt());
			break;

		case AbstractChartModel::AxisY2TitleInverted: {
			bool inverted = m_model->data(role).toBool();
			m_rightAxis.setTitleInverted(inverted);
		} break;

		case AbstractChartModel::AxisScalesInside: {
			updateInternalAxisConfiguration();
		} break;

		case AbstractChartModel::ConstructionLineVisible :
		case AbstractChartModel::ConstructionLinePen : {
			updateConstructionLines();
		} break;

		case AbstractChartModel::ShowCurrentTimePosition : {
			bool enabled = m_model->data(AbstractChartModel::ShowCurrentTimePosition).toBool();
			IBK::Unit timeUnit(m_model->axisData(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toInt());
			bool xAxisIsTime = (timeUnit.base_id() == IBK_UNIT_ID_SECONDS);
			if (!xAxisIsTime)
				enabled = false;	// no time axis? no marker!
			if (!enabled && m_markerCurrentTimePosition != nullptr) {
				delete m_markerCurrentTimePosition;
				m_markerCurrentTimePosition = nullptr;
				break;
			}
			if (enabled) {
				if (m_markerCurrentTimePosition == nullptr) {
					m_markerCurrentTimePosition = new QwtPlotMarker;
					m_markerCurrentTimePosition->setLineStyle(QwtPlotMarker::VLine);
					m_markerCurrentTimePosition->setLinePen(QPen(Qt::black, 2));
					m_markerCurrentTimePosition->attach(this);
				}
			}
		} // fall through

		case AbstractChartModel::CurrentTimePosition : {
			bool enabled = m_model->data(AbstractChartModel::ShowCurrentTimePosition).toBool();
			IBK::Unit timeUnit(m_model->axisData(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisUnit).toInt());
			bool xAxisIsTime = (timeUnit.base_id() == IBK_UNIT_ID_SECONDS);
			if (!xAxisIsTime)
				enabled = false;	// no time axis? no marker!
			if (enabled) {
				double time_in_seconds = m_model->data(AbstractChartModel::CurrentTimePosition).toDouble();
				// convert to axis unit
				IBK::UnitList::instance().convert(IBK::Unit(IBK_UNIT_ID_SECONDS), timeUnit, time_in_seconds);
				bool timeAxis = m_model->axisData(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisDateTime).toBool();
				if (timeAxis) {
					QDateTime t = m_model->axisData(AbstractChartModel::BottomAxis, AbstractCartesianChartModel::AxisDateTimeZero).toDateTime();
					time_in_seconds += t.toMSecsSinceEpoch();
				}
				// now set value in marker
				if (m_markerCurrentTimePosition != nullptr)
					m_markerCurrentTimePosition->setValue(time_in_seconds, 0);
			}
		} break;

	}
	replot();
}


AbstractChartModel* Chart::model() {
	return m_model;
}


const AbstractChartModel* Chart::model() const {
	return m_model;
}


} // namespace SCI
