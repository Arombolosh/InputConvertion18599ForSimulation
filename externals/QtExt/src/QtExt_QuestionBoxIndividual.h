#ifndef QtExt_QuestionBoxIndividual_H
#define QtExt_QuestionBoxIndividual_H

#include <QString>
#include <QWidget>

#include "QtExt_global.h"

namespace QtExt {

/*! A wrapper class around QMessageBox::question() that allows customizing button texts in one call. */
class QtExt_EXPORT QuestionBoxIndividual {
public:
	/*! Variant without detailed text. */
	static bool askYesNo(QWidget *parent, const QString &title, const QString &text,
						 const QString &yesText, const QString &noText,
						 bool yesIsDefault = true);

	/*! Variant with detailed text. */
	static bool askYesNo(QWidget *parent, const QString &title, const QString &text, const QString &detailedText,
						 const QString &yesText, const QString &noText,
						 bool yesIsDefault = true);
};

}

#endif // QtExt_QuestionBoxIndividual_H
