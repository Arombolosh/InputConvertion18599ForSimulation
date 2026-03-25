#include "QtExt_QuestionBoxIndividual.h"

#include <QAbstractButton>
#include <QPushButton>
#include <QMessageBox>

namespace QtExt {

bool QuestionBoxIndividual::askYesNo(QWidget *parent, const QString &title, const QString &text, const QString &yesText, const QString &noText, bool yesIsDefault) {
	QMessageBox msgBox(QMessageBox::Question, title, text,
					   QMessageBox::Yes | QMessageBox::No, parent);

	msgBox.button(QMessageBox::Yes)->setText(yesText);
	msgBox.button(QMessageBox::No)->setText(noText);
	msgBox.setDefaultButton(yesIsDefault ? QMessageBox::Yes : QMessageBox::No);

	return msgBox.exec() == QMessageBox::Yes;
}


bool QuestionBoxIndividual::askYesNo(QWidget *parent, const QString &title, const QString &text, const QString &detailedText, const QString &yesText, const QString &noText, bool yesIsDefault) {
	QMessageBox msgBox(QMessageBox::Question, title, text,
					   QMessageBox::Yes | QMessageBox::No, parent);

	msgBox.button(QMessageBox::Yes)->setText(yesText);
	msgBox.button(QMessageBox::No)->setText(noText);
	msgBox.setDefaultButton(yesIsDefault ? QMessageBox::Yes : QMessageBox::No);

	msgBox.setDetailedText(detailedText);

	return msgBox.exec() == QMessageBox::Yes;
}

}
