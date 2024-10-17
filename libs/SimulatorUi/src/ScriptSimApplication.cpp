#include "ScriptSimApplication.h"
#include "SimSchemaView.h"

namespace SimUi
{
	ScriptSimApplication::ScriptSimApplication(const SimSchemaView* simSchemaView, QObject* parent) :
		QObject(parent),
		m_simSchemaView(simSchemaView)
	{
	}

	void ScriptSimApplication::showArchive(QStringList /*signalsList*/, QDateTime /*startTime*/, QDateTime /*endTime*/, int /*timeType*/) {}

	void ScriptSimApplication::showSnapshot(QStringList /*signalsList*/) {}

	void ScriptSimApplication::showSnapshotByMask(QStringList /*masks*/) {}

	void ScriptSimApplication::showSnapshotByTag(QStringList /*tags*/) {}

	void ScriptSimApplication::setVisibleTabBar(bool /*visible*/) {}

	void ScriptSimApplication::setVisibleSchemaTree(bool /*visible*/) {}

	void ScriptSimApplication::toggleSchemaTree() {}

	void ScriptSimApplication::setVisibleToolBar(bool /*visible*/) {}

	void ScriptSimApplication::setVisibleStatusBar(bool /*visible*/) {}

	void ScriptSimApplication::setVisibleMenu(bool /*visible*/) {}

	void ScriptSimApplication::setFullScreen(bool /*fullScreen*/) {}

	QString ScriptSimApplication::equipmentId() const
	{
		if (m_simSchemaView != nullptr)
		{
			return m_simSchemaView->monitorId();
		}
		return "";
	}
} // namespace SimUi