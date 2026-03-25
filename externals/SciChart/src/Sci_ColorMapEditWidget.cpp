/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_ColorMapEditWidget.h"
#include "ui_Sci_ColorMapEditWidget.h"

#include <QStandardItemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QDirIterator>
#include <QPainter>
#include <QTextStream>

#include <IBK_Exception.h>
#include <IBK_InputOutput.h>
#include <IBK_StringUtils.h>
#include <IBK_messages.h>

#include <tinyxml.h>

#include "Sci_AbstractColorGridSeriesModel.h"
#include "Sci_AbstractVectorFieldSeriesModel.h"
#include "Sci_StopPointDelegate.h"


namespace SCI {

ColorMapEditWidget::ColorMapEditWidget(QWidget *parent, const QString & colorMapDirectory) :
	QFrame(parent),
	m_ui(new Ui::ColorMapEditWidget),
	m_colorMapDirectory(colorMapDirectory)
{
	m_ui->setupUi(this);

	StopPointDelegate* stopPointDelegate = new StopPointDelegate(this);
	m_ui->stopPointTable->setItemDelegate(stopPointDelegate);

	// built-in presets
	m_presets.append(qMakePair(m_ui->toolButtonPreset1, QString("built-in:1") ) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset2, QString("built-in:2") ) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset3, QString("built-in:3") ) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset4, QString("built-in:4") ) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset5, QString("built-in:5")) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset6, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset7, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset8, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset9, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset10, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset11, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset12, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset13, QString()) );
	m_presets.append(qMakePair(m_ui->toolButtonPreset14, QString()) );

	// read built-in presets from ressources and also user-defined presets from file
	readColorMapsFromResource();

	for (int i=0; i<m_presets.count(); ++i) {
		connect(m_presets[i].first, &QToolButton::clicked, this, &ColorMapEditWidget::onToolButtonPresetClicked);
		// generate preset image for this preset

		// disable unused presets
		if (!m_presets[i].second.isEmpty()) {

			// generate color map for this preset
			ColorMap cmap = colorMapFromPreset(i,32);

			QPixmap icon(32,32);
			// we generate a line for each of the 32 color stops
			QPainter p(&icon);

			Q_ASSERT(!cmap.m_linearColorStops.isEmpty());
			for (int r=0; r<32; ++r) {
				QColor c = cmap.m_linearColorStops[std::min<int>(r, cmap.m_linearColorStops.count())].m_color;
				p.setPen(c);
				p.drawLine(0,r,31,r);
			}

			m_presets[i].first->setIcon(icon);
		}
	}
}


ColorMapEditWidget::~ColorMapEditWidget() {
	delete m_ui;
}


static ColorMap readColorMap(const std::string& mapstr) {
	// now we parse the different sections of the color map file
	ColorMap cmap;
	TiXmlDocument doc;
	if (!doc.Parse(mapstr.c_str(), nullptr, TIXML_ENCODING_UTF8)) {
		return cmap;
	}
	try {
		// we use a handle so that NULL pointer checks are done during the query functions
		TiXmlHandle xmlHandleDoc(&doc);

		TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
		if (!xmlElem)
			return cmap;
		std::string rootnode = xmlElem->Value();
		if (rootnode != "PostProcColorMap")
			return cmap;

		TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

		xmlElem = xmlRoot.FirstChild( "ColorMap" ).Element();
		if (!xmlElem)
			return cmap;

		cmap.readXML(xmlElem);
		return cmap;
	}
	catch (IBK::Exception & ex) {
		return cmap;
	}
}


void ColorMapEditWidget::readColorMapsFromResource() {
	QDirIterator it(":/colormaps/", QDirIterator::Subdirectories);
	QStringList builtInColorMaps;
	QStringList userColorMaps;
	while (it.hasNext()) {
		QString ent = it.next();
		if (ent.contains(".p2colormap"))
			builtInColorMaps<< ent;
	}
	// now parse files in m_colorMapDirectory
	QDirIterator userIt(m_colorMapDirectory);
	while (userIt.hasNext()) {
		QString ent = userIt.next();
		if (ent.contains(".p2colormap"))
			userColorMaps << ent;
	}

	if (builtInColorMaps.empty() && userColorMaps.empty())
		return;

	builtInColorMaps.sort();
	userColorMaps.sort();

	builtInColorMaps.append(userColorMaps);

	m_colorMaps.clear();
	// determine first free preset
	int presetIndex = 0;
	for (;presetIndex < m_presets.count(); ++presetIndex)
		if (m_presets[presetIndex].second.isEmpty())
			break;

	for (const QString & fname : asConst(builtInColorMaps)) {
		QFile f(fname);
		bool res = f.open(QFile::ReadOnly);
		if (res) {
			QTextStream strm(&f);
#if QT_VERSION > QT_VERSION_CHECK(6,0,0)
			strm.setEncoding(QStringConverter::Utf8);
#else
			strm.setCodec("UTF-8");
#endif
			QString content = strm.readAll();
			f.close();
			ColorMap cmap = readColorMap(content.toStdString());
			if (!cmap.m_linearColorStops.empty()) {
				QFileInfo finfo(fname);
				m_colorMaps[finfo.filePath()] = cmap;
				// still free space in presets?
				if (presetIndex+1 < m_presets.count()) {
					m_presets[presetIndex++].second = finfo.filePath();
				}
			}
		}
	}
}


ColorMap ColorMapEditWidget::colorMapFromPreset(int idx, int stepCount) const {
	Q_ASSERT(idx >= 0 && idx < m_presets.count());
	QString fpath = m_presets[idx].second;

	int builtinIdx = 0;
	if (fpath.startsWith("built-in:"))
		builtinIdx = fpath.mid(9).toInt();


	QColor col1;
	QColor col2;
	switch (builtinIdx) {
		case 1:
			col1 = QColor("#ff1900");
			col2 = QColor("#4c00e5");
		break;
		case 2:
			col1 = QColor("#000030");
			col2 = QColor("#cfedff");
		break;
		case 3:
			col1 = QColor("#ff0004");
			col2 = QColor("#00007e");
		break;
		case 4:
			col1 = Qt::black;
			col2 = Qt::white;
		break;
		case 5:
			col1 = QColor("#00007e");
			col2 = QColor("#63d6ff");
		break;
	}
	if (builtinIdx != 0) {
		// create new color map
		ColorMap cmap;
		cmap.setLinearMap(col2, col1, stepCount);
		return cmap;
	}

	// not a built-in, generate colormap
	std::map<QString,ColorMap>::const_iterator it = m_colorMaps.find(m_presets[idx].second);
	if (it == m_colorMaps.end())
		return ColorMap();
	else {
		ColorMap cmap = it->second;
		if (!cmap.m_originalLinearColorStops.isEmpty())
			cmap.interpolateColorMap(stepCount);
		return cmap;
	}
}


void ColorMapEditWidget::setModel(AbstractChartModel * model) {

	if ( m_model ) {
		disconnect(m_model, &SCI::AbstractChartModel::modelReset, this, &ColorMapEditWidget::setProperties);
		disconnect(m_model, &SCI::AbstractChartModel::modelChanged, this, &ColorMapEditWidget::setProperties);
		disconnect(m_model, &SCI::AbstractChartModel::seriesViewChanged, this, &ColorMapEditWidget::onSeriesViewChanged);
	}

	if (m_model != model) {
		m_model = model;
	}

	if ( m_model ) {
		m_chartType = m_model->chartType();
		switch (m_chartType) {
			case ChartSeriesInfos::ColorGridSeries :
				m_colorMapRole = AbstractColorGridSeriesModel::ColorMap;
				m_autoScaleRole = AbstractColorGridSeriesModel::AutoScale;
				m_zValueRangeRole = AbstractColorGridSeriesModel::ZValueRange;
			break;
			case ChartSeriesInfos::VectorFieldSeries :
				m_colorMapRole = AbstractVectorFieldSeriesModel::ColorMap;
				m_autoScaleRole = AbstractVectorFieldSeriesModel::AutoScale;
				m_zValueRangeRole = AbstractVectorFieldSeriesModel::ZValueRange;
			break;
			default:
				Q_ASSERT(false); // unsupported model type!
		}

		connect(m_model, &SCI::AbstractChartModel::modelReset, this, &ColorMapEditWidget::setProperties);
		connect(m_model, &SCI::AbstractChartModel::modelChanged, this, &ColorMapEditWidget::setProperties);
		connect(m_model, &SCI::AbstractChartModel::seriesViewChanged, this, &ColorMapEditWidget::onSeriesViewChanged);

		setProperties();
	}
}


void ColorMapEditWidget::onSeriesViewChanged(int,int,int dataRole) {
	if (m_chartType == ChartSeriesInfos::ColorGridSeries) {
		// ignore all not-color-map related properties
		switch (dataRole) {
			case SCI::AbstractColorGridSeriesModel::XGridArray		:
			case SCI::AbstractColorGridSeriesModel::XGridArraySize	:
			case SCI::AbstractColorGridSeriesModel::YGridArray		:
			case SCI::AbstractColorGridSeriesModel::YGridArraySize	:
			case SCI::AbstractColorGridSeriesModel::XValueSize		:
			case SCI::AbstractColorGridSeriesModel::YValueSize		:
			case SCI::AbstractColorGridSeriesModel::ValueMatrix		:
				return;
		};
	}
	else if (m_chartType == ChartSeriesInfos::VectorFieldSeries) {
		// ignore all not-color-map related properties
		switch (dataRole) {
			case SCI::AbstractVectorFieldSeriesModel::VectorSampleCount :
			case SCI::AbstractVectorFieldSeriesModel::VectorSampleXArray :
			case SCI::AbstractVectorFieldSeriesModel::VectorSampleYArray :
			case SCI::AbstractVectorFieldSeriesModel::VectorSampleVXArray :
			case SCI::AbstractVectorFieldSeriesModel::VectorSampleVYArray :
			case SCI::AbstractVectorFieldSeriesModel::RasterEnabled :
			case SCI::AbstractVectorFieldSeriesModel::RasterSize :
			case SCI::AbstractVectorFieldSeriesModel::ColoredArrowsEnabled :
			case SCI::AbstractVectorFieldSeriesModel::UniformArrowColor :
			case SCI::AbstractVectorFieldSeriesModel::IndicatorOrigin :
				return;
		};
	}
	setProperties();
}


void ColorMapEditWidget::setProperties() {

	blockSignals(true);
	m_ui->stopPointTable->blockSignals(true);
	m_ui->spinBoxCount->blockSignals(true);

	m_ui->stopPointTable->clear();

	// retrieve current color map
	QVariant value = m_model->seriesData(0, m_colorMapRole);
	SCI::ColorMap colorMap;
	if (value.canConvert<SCI::ColorMap>())
		colorMap = value.value<SCI::ColorMap>();

	// retrieve z-value range
	value = m_model->seriesData(0, m_zValueRangeRole);
	Range interval(0,1);
	if (value.canConvert<Range>())
		interval = value.value<Range>();

	int colStops = (int)colorMap.m_linearColorStops.size();
	m_ui->spinBoxCount->setValue( colStops-1 );

	m_ui->stopPointTable->setRowCount( colStops );
	int col_width = m_ui->stopPointTable->verticalHeader()->defaultSectionSize();
	m_ui->stopPointTable->setColumnWidth( 0, col_width );

	double difference = interval.second - interval.first;
	for ( int i=0; i<colStops; ++i ) {
		double zValue = colorMap.m_linearColorStops[i].m_pos;
		QColor col = colorMap.m_linearColorStops[i].m_color;

		QTableWidgetItem * item = new QTableWidgetItem();
		item->setData(Qt::BackgroundRole,col);
		m_ui->stopPointTable->setItem(colStops-i-1,0,item);

		item = new QTableWidgetItem();
		item->setFlags( Qt::NoItemFlags );
		item->setData(Qt::DisplayRole,QString("%L1").arg(zValue*difference + interval.first));
		item->setData(Qt::UserRole, zValue*difference + interval.first);
		// build up table bottom-to-top
		m_ui->stopPointTable->setItem(colStops-i-1,1,item);

		m_ui->stopPointTable->resizeColumnsToContents();
	}

	blockSignals(false);
	m_ui->stopPointTable->blockSignals(false);
	m_ui->spinBoxCount->blockSignals(false);
}


void ColorMapEditWidget::on_buttonCalculate_clicked() {
	// ask series model to generate min/max values, then set autoscale off
	switch (m_chartType) {
		case ChartSeriesInfos::ColorGridSeries :
			dynamic_cast<AbstractColorGridSeriesModel*>(m_model)->calculateGlobalMinMax(); break;
		case ChartSeriesInfos::VectorFieldSeries :
			dynamic_cast<AbstractVectorFieldSeriesModel*>(m_model)->calculateGlobalMinMax(); break;
		default:;
	}
	m_model->setSeriesData(false, 0, m_autoScaleRole);
}


void ColorMapEditWidget::on_spinBoxCount_valueChanged(int count) {
	// minimum 2 intervals
	if ( count < 2 )
		return;

	// we now distinguish between having a master color map or having an auto-generated color map
	QVariant value = m_model->seriesData(0, m_colorMapRole);
	SCI::ColorMap colorMap;
	if (value.canConvert<SCI::ColorMap>())
		colorMap = value.value<SCI::ColorMap>();
	if (colorMap.m_originalLinearColorStops.isEmpty()) {

		// take first and last color from table
		// Mind: table is sorted top-2-bottom, that means last color stop comes first
		QColor col1 = m_ui->stopPointTable->item(m_ui->stopPointTable->rowCount()-1,0)->data(Qt::BackgroundRole).value<QColor>();
		QColor col2 = m_ui->stopPointTable->item(0,0)->data(Qt::BackgroundRole).value<QColor>();

		// create new color map
		colorMap.setLinearMap(col1, col2, count);
	}
	else {
		colorMap.interpolateColorMap(count);
	}

	// set new color map in model
	QVariant res;
	res.setValue(colorMap);
	m_model->setSeriesData(res, 0, m_colorMapRole);
}


void ColorMapEditWidget::on_stopPointTable_cellChanged(int row, int ) {
	QVariant map = m_model->seriesData(0, m_colorMapRole);
	Q_ASSERT(map.canConvert<SCI::ColorMap>());

	SCI::ColorMap colorMap = map.value<SCI::ColorMap>();

	// determine which color has changed
	QTableWidgetItem * item = m_ui->stopPointTable->item(row, 0);
	QColor c = item->data(Qt::BackgroundRole).value<QColor>();

	Q_ASSERT(row < colorMap.m_linearColorStops.size());
	// mind inverted display of color map in table
	unsigned int idx = colorMap.m_linearColorStops.size() - row-1;

	// set modified temperature in color map
	colorMap.m_linearColorStops[idx].m_color = c;

	// set updated color map in model
	QVariant res;
	res.setValue(colorMap);
	m_model->setSeriesData(res, 0, m_colorMapRole);
}


void ColorMapEditWidget::on_pushButtonInvertColorMap_clicked() {
	// retrieve current color map
	QVariant value = m_model->seriesData(0, m_colorMapRole);
	SCI::ColorMap colorMap;
	if (value.canConvert<SCI::ColorMap>())
		colorMap = value.value<SCI::ColorMap>();

	// now invert color map
	std::reverse(colorMap.m_linearColorStops.begin(), colorMap.m_linearColorStops.end());
	for (int i=0; i<colorMap.m_linearColorStops.size(); ++i)
		colorMap.m_linearColorStops[i].m_pos = 1 - colorMap.m_linearColorStops[i].m_pos;
	std::reverse(colorMap.m_originalLinearColorStops.begin(), colorMap.m_originalLinearColorStops.end());
	for (int i=0; i<colorMap.m_originalLinearColorStops.size(); ++i)
		colorMap.m_originalLinearColorStops[i].m_pos = 1 - colorMap.m_originalLinearColorStops[i].m_pos;

	// set new color map in model
	QVariant res;
	res.setValue(colorMap);
	m_model->setSeriesData(res, 0, m_colorMapRole);
}


void ColorMapEditWidget::onToolButtonPresetClicked() {

	// find preset index
	int presetIndex = 0;
	for (;presetIndex < m_presets.count(); ++ presetIndex) {
		if (m_presets[presetIndex].first == sender())
			break;
	}
	if (presetIndex == m_presets.count())
		return; // shouldn't happen, but just in case

	if (m_presets[presetIndex].second.isEmpty()) {
		QMessageBox::critical(this, QString(), tr("This preset is currently unused. You may save a color map in directory %1 to fill this slot.").arg(m_colorMapDirectory));
		return;
	}

	// set new color map in model
	QVariant res;
	res.setValue(colorMapFromPreset(presetIndex, m_ui->spinBoxCount->value()));
	m_model->setSeriesData(res, 0, m_colorMapRole);
}


void ColorMapEditWidget::on_buttonLoadMap_clicked() {
	FUNCID("ColorMapEditWidget::on_buttonLoadMap_clicked");
	QString filename = QFileDialog::getOpenFileName(
							this,
							tr("Select Color Map File"),
							m_lastOpenFileLocation,
							tr("Color map files (*.p2colormap);;All files (*.*)"), nullptr,
							QFileDialog::DontUseNativeDialog
						);

	if (filename.isEmpty()) return;

	// store open file location
	QFileInfo finfo(filename);
	m_lastOpenFileLocation = finfo.absoluteDir().absolutePath();

	TiXmlDocument doc( filename.toStdString() );
	if (!doc.LoadFile()) {
		throw IBK::Exception(IBK::FormatString("Error in line %1 of file '%2':\n%3")
				.arg(doc.ErrorRow())
				.arg(filename.toStdString())
				.arg(doc.ErrorDesc()), FUNC_ID);
	}

	// now we parse the different sections of the color map file
	ColorMap cmap;
	try {
		// we use a handle so that NULL pointer checks are done during the query functions
		TiXmlHandle xmlHandleDoc(&doc);

		TiXmlElement * xmlElem = xmlHandleDoc.FirstChildElement().Element();
		if (!xmlElem)
			return; // empty project, this means we are using only defaults
		std::string rootnode = xmlElem->Value();
		if (rootnode != "PostProcColorMap")
			throw IBK::Exception("Expected PostProcColorMap as root node in XML file.", FUNC_ID);

		TiXmlHandle xmlRoot = TiXmlHandle(xmlElem);

		xmlElem = xmlRoot.FirstChild( "ColorMap" ).Element();
		if (!xmlElem)
			throw IBK::Exception(IBK::FormatString("Expected top-level 'ColorMap' element."), FUNC_ID);

		cmap.readXML(xmlElem);
	}
	catch (IBK::Exception & ex) {
		ex.writeMsgStackToError();
		QMessageBox::critical(this, tr("File error"), tr("Error reading color map from file. See logfile '%1' for details."));
		return;
	}
	// set new color map
	QVariant res;
	res.setValue(cmap);
	// remember current step count as the call to setSeriesData() will reset the count to the number of steps in
	// the newly read color map
	int currentStepCount = m_ui->spinBoxCount->value();
	m_model->setSeriesData(res, 0, m_colorMapRole);
	// and finally re-apply the previous step count
	on_spinBoxCount_valueChanged(currentStepCount);
}


void ColorMapEditWidget::on_buttonSaveMap_clicked() {
	QString filename = QFileDialog::getSaveFileName( this,
			tr("Select Color Map File"), m_lastOpenFileLocation, tr("Color map files (*.p2colormap);;All files (*.*)") );

	if (filename.isEmpty()) return;

	// store open file location
	QFileInfo finfo(filename);
	m_lastOpenFileLocation = finfo.absoluteDir().absolutePath();
	if (finfo.suffix() != "p2colormap")
		filename += ".p2colormap";

	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration( "1.0", "UTF-8", "" );
	doc.LinkEndChild( decl );

	TiXmlElement * root = new TiXmlElement( "PostProcColorMap" );
	doc.LinkEndChild(root);

	root->SetAttribute("fileVersion", "1.0");

	QVariant map = m_model->seriesData(0, m_colorMapRole);
	Q_ASSERT(map.canConvert<SCI::ColorMap>());

	SCI::ColorMap colorMap = map.value<SCI::ColorMap>();
	TiXmlElement * e = new TiXmlElement("ColorMap");
	root->LinkEndChild(e);
	colorMap.writeXML(e);

	doc.SaveFile( filename.toStdString() );
}


void ColorMapEditWidget::on_pushButtonGenerateHSVMap_clicked() {
	// take first and last color from table
	// Mind: table is sorted top-2-bottom, that means last color stop comes first
	QColor col1 = m_ui->stopPointTable->item(m_ui->stopPointTable->rowCount()-1,0)->data(Qt::BackgroundRole).value<QColor>();
	QColor col2 = m_ui->stopPointTable->item(0,0)->data(Qt::BackgroundRole).value<QColor>();

	int count = m_ui->spinBoxCount->value();
	// create new color map
	ColorMap cmap;
	cmap.setLinearMap(col1, col2, count);
	// set new color map
	QVariant res;
	res.setValue(cmap);
	m_model->setSeriesData(res, 0, m_colorMapRole);
}


} // namespace SCI




