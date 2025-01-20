#include "ScriptTestSuiteApplication.h"

ScriptTestSuiteApplication::ScriptTestSuiteApplication(const QString& equipmentId)
	: QObject{},
	m_equipmentId{equipmentId}
{
}

QString ScriptTestSuiteApplication::equipmentId() const
{
	return m_equipmentId;
}

