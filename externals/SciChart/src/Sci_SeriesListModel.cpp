/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QPixmap>
#include <QIcon>

#include "Sci_SeriesListModel.h"
#include "Sci_Chart.h"
#include "Sci_LineSeries.h"
#include "Sci_AbstractLineSeriesModel.h"

namespace SCI {

SeriesListModel::SeriesListModel(AbstractLineSeriesModel* model, QObject *parent) :
	QAbstractListModel(parent),
	m_model(nullptr)
{
	setModel(model);
}

AbstractLineSeriesModel* SeriesListModel::chartModel() const {
	return m_model;
}

void SeriesListModel::setModel(AbstractLineSeriesModel* model) {

	if (m_model != nullptr) {
		disconnect(m_model, SIGNAL(seriesAboutToBeRemoved(int,int)), this, SLOT(onSeriesAboutToBeRemoved(int,int)));
		disconnect(m_model, SIGNAL(seriesRemoved(int,int)), this, SLOT(onSeriesRemoved(int,int)));
		disconnect(m_model, SIGNAL(seriesAboutToBeInserted(int,int)), this, SLOT(onSeriesAboutToBeInserted(int,int)));
		disconnect(m_model, SIGNAL(seriesInserted(int,int)), this, SLOT(onSeriesInserted(int,int)));
	}

	beginResetModel();
	m_model = model;
	endResetModel();

	if (m_model != nullptr) {
		connect(m_model, SIGNAL(seriesAboutToBeRemoved(int,int)), this, SLOT(onSeriesAboutToBeRemoved(int,int)));
		connect(m_model, SIGNAL(seriesRemoved(int,int)), this, SLOT(onSeriesRemoved(int,int)));
		connect(m_model, SIGNAL(seriesAboutToBeInserted(int,int)), this, SLOT(onSeriesAboutToBeInserted(int,int)));
		connect(m_model, SIGNAL(seriesInserted(int,int)), this, SLOT(onSeriesInserted(int,int)));
	}

}

int SeriesListModel::rowCount(const QModelIndex &parent) const {
	if( !m_model || parent.isValid() || m_model->chartType() != ChartSeriesInfos::LineSeries) {
		return 0;
	}
	return m_model->seriesCount();
}

QVariant SeriesListModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	if( !m_model)
		return QVariant();

	if (role == Qt::ToolTipRole) {
		return m_model->seriesData(index.row(), AbstractLineSeriesModel::SeriesTitle);
	}

//	if (role == Qt::DecorationRole) {
//		QPixmap pix(24, 12);
//		QPainter painter(&pix);
//		QPen pen = ser->pen();
//		painter.setPen(pen);
//
//		painter.drawLine(0, 6, 23, 6);
//		QIcon icon;
//		icon.addPixmap(pix);
//		return icon;
//	}
	return QVariant();
}

QModelIndex SeriesListModel::index( int row, int column, const QModelIndex & parent ) const {
	if( !m_model)
		return QModelIndex();

	if (row < 0 ||row >= static_cast<int>(m_model->seriesCount()))
		return QModelIndex();

	if( !parent.isValid()) {
		return createIndex(row, column, (void*)(0));
	}
	return QModelIndex();
}

void SeriesListModel::onSeriesAboutToBeRemoved(int start, int end ) {
	beginRemoveRows(QModelIndex(), start, end);
}

void SeriesListModel::onSeriesRemoved(int, int) {
	endRemoveRows();
}

void SeriesListModel::onSeriesAboutToBeInserted(int start, int end) {
	beginInsertRows(QModelIndex(), start, end);
}

void SeriesListModel::onSeriesInserted(int, int) {
	endInsertRows();
}

} // end namespace SCI
