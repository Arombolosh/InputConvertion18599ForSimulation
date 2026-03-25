#include "QtExt_ValidatingLineEditUnitManager.h"

#include "QtExt_ValidatingLineEditUnit.h"

namespace QtExt {

void ValidatingLineEditUnitManager::registerEdit(ValidatingLineEditUnit *edit) {
	m_edits.insert(edit);
	connect(edit, &ValidatingLineEditUnit::unregister, this, &ValidatingLineEditUnitManager::unregisterEdit);
}

void ValidatingLineEditUnitManager::changeUnit(const IBK::Unit &unit) {
	try {
		for(auto edit : m_edits) {
			edit->changeUnit(unit);
		}
	}
	catch (...) {
		Q_ASSERT("Wrong length edit setting");
	}
}

void ValidatingLineEditUnitManager::unregisterEdit(ValidatingLineEditUnit *edit) {
	m_edits.remove(edit);
}

} // namespace QtExt
