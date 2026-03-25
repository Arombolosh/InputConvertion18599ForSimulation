/*	Authors: H. Fechner, A. Nicolai

	This file is part of the SciChart Library.
	All rights reserved.

	This software is copyrighted by the principle author(s).
	The right to reproduce the work (copy all or part of the source code),
	modify the source code or documentation, compile it to form object code,
	and the sole right to copy the object code thereby produced is hereby
	retained for the author(s) unless explicitely granted by the author(s).

*/

#include "Sci_StopPointDelegate.h"

#include <QLineEdit>
#include <QDoubleValidator>
#include <QColorDialog>

namespace SCI {

StopPointDelegate::StopPointDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{
}

QWidget *StopPointDelegate::createEditor(QWidget *parent,
	 const QStyleOptionViewItem &option,
	 const QModelIndex &index ) const
{
	if ( index.column() == 3 ) {

		QLineEdit* editor = new QLineEdit(parent);
		QDoubleValidator* val = new QDoubleValidator(editor);
		val->setNotation(QDoubleValidator::StandardNotation);
		editor->setValidator(val);
		return editor;
	}
	else if ( index.column() == 0 ) {
		QColorDialog* colorDialog = new QColorDialog();
		return colorDialog;
	}
	return QStyledItemDelegate::createEditor(parent, option, index);
}

void StopPointDelegate::setEditorData(QWidget *editor,
									 const QModelIndex &index) const
{
	if ( index.column() == 3 ) {
		double value = index.model()->data(index, Qt::EditRole).toDouble();

		QLineEdit* line = static_cast<QLineEdit*>(editor);
		line->setText(QString().setNum(value));
	}
	else if ( index.column() == 0 ) {
		QColorDialog* colorDialog = static_cast<QColorDialog*>( editor );
		QColor color = index.model()->data( index, Qt::BackgroundRole ).value<QColor>();
		colorDialog->setCurrentColor( color );
	}
}

void StopPointDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
									const QModelIndex &index) const
{
	if ( index.column() == 3 ) {
		QLineEdit* line = static_cast<QLineEdit*>(editor);
		QString value = line->text();
		model->setData(index,value);
	}
	else if ( index.column() == 0 ) {
		QColorDialog* colorDialog = static_cast<QColorDialog*>( editor );
		QColor color = colorDialog->currentColor();
		if ( color.isValid() )
			model->setData(index, color, Qt::BackgroundRole);
	}
}

void StopPointDelegate::updateEditorGeometry(QWidget *editor,
	 const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
	editor->setGeometry(option.rect);
}

QString StopPointDelegate::displayText(const QVariant &value, const QLocale &locale) const
{
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
	if (value.typeId() == QMetaType::Double) {
#else
	if (value.type() == QVariant::Double) {
#endif
		return locale.toString(value.toDouble(), 'f', 2);
	}
	else
		return QStyledItemDelegate::displayText(value, locale);
}

} //namespace SCI {

