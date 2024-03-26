#include "./include/HardwareLib/ScriptEquipment.h"
#include "./include/HardwareLib/ScriptDeviceObject.h"
#include "./include/HardwareLib/DeviceRoot.h"

namespace Hardware
{
	ScriptEquipment::ScriptEquipment(QJSEngine& jsEngine, QObject* parent) :
		QObject{parent},
		m_jsEngine{jsEngine}
	{
		setRoot(std::make_shared<DeviceRoot>());

		m_deviceTable.reserve(4096);

		return;
	}

	ScriptEquipment::~ScriptEquipment() = default;

	QJSValue ScriptEquipment::root() const
	{
		return m_jsEngine.newQObject(new ScriptDeviceObject(m_root));
	}

	// setRoot
	//
	void ScriptEquipment::setRoot(std::shared_ptr<DeviceObject> root)
	{
		m_root = std::move(root);

		if (m_root == nullptr)
		{
			// m_root must not be empty.
			//
			m_root = std::make_shared<DeviceRoot>();
		}

		fillDeviceTable(m_root);

		qDebug() << "ScriptEquipment::setRoot(): Added " << m_deviceTable.size() << " device objects";

		return;
	}

	QJSValue ScriptEquipment::find(QString equipmentId) const
	{
		QJSValue result;

		auto it = m_deviceTable.find(equipmentId);
		if (it != m_deviceTable.end())
		{
			Q_ASSERT(it->second != nullptr);
			result = m_jsEngine.newQObject(new ScriptDeviceObject(it->second));
		}

		return result;
	}

	void ScriptEquipment::fillDeviceTable(const std::shared_ptr<DeviceObject>& parent, int recursionLevel /*= 0*/)
	{
		if (recursionLevel == 0)
		{
			m_deviceTable.clear();
		}

		if (recursionLevel >= 16)
		{
			Q_ASSERT(recursionLevel < 16);
			return;
		}

		m_deviceTable[parent->equipmentId()] = parent;

		for (int i = 0; i < parent->childrenCount(); i++)
		{
			const std::shared_ptr<DeviceObject>& child = parent->child(i);
			fillDeviceTable(child, recursionLevel + 1);
		}

		return;
	}

} // namespace Hardware