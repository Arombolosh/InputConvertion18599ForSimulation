#ifndef QtExt_ValidatingLineEditUnitManagerH
#define QtExt_ValidatingLineEditUnitManagerH

#include <QObject>
#include <QSet>

#include <IBK_Unit.h>

#include "QtExt_global.h"

namespace QtExt {

class ValidatingLineEditUnit;

class QtExt_EXPORT ValidatingLineEditUnitManager : public QObject
{
	Q_OBJECT
public:
	ValidatingLineEditUnitManager() = default;

	void registerEdit(ValidatingLineEditUnit* edit);

	void changeUnit(const IBK::Unit &unit);

private slots:
	void unregisterEdit(ValidatingLineEditUnit* edit);

private:
	QSet<ValidatingLineEditUnit*>	m_edits;
};

} // namespace QtExt

#endif // QtExt_ValidatingLineEditUnitManagerH
