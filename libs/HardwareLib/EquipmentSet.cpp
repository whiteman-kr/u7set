#include <HardwareLib/EquipmentSet.h>

namespace Hardware
{
	//
	//
	// EquipmentSet
	//
	//
	EquipmentSet::EquipmentSet(std::shared_ptr<DeviceRoot> root)
	{
		set(root);
	}

	EquipmentSet::~EquipmentSet()
	{
		// Release m_root in separate thread, if it is possible
		//
		m_deviceTable.clear();	// Clear it first because it also holds m_root

		if (m_root.use_count() == 1)
		{
			std::shared_ptr<Hardware::DeviceObject> equipmentSharedPointer = m_root;
			m_root.reset();

			QFuture<void> f = QtConcurrent::run(
				[](std::shared_ptr<Hardware::DeviceObject> deviceObject)
				{
					deviceObject.reset();
				},
				equipmentSharedPointer);
			Q_UNUSED(f);
		}

		return;
	}

	void EquipmentSet::set(std::shared_ptr<DeviceRoot> root)
	{
		m_deviceTable.clear();

		if (root == nullptr)
		{
			Q_ASSERT(root);
			return;
		}

		m_root = root;

		// fill map for fast access
		//
		m_deviceTable.insert({m_root->equipmentIdTemplate(), m_root});
		addDeviceChildrenToHashTable(m_root);

		return;
	}

	std::shared_ptr<DeviceObject> EquipmentSet::deviceObject(const QString& equipmentId)
	{
		auto it = m_deviceTable.find(equipmentId);

		if (it != m_deviceTable.end())
		{
			return it->second;
		}
		else
		{
			return nullptr;
		}
	}

	const std::shared_ptr<DeviceObject> EquipmentSet::deviceObject(const QString& equipmentId) const
	{
		auto it = m_deviceTable.find(equipmentId);

		if (it != m_deviceTable.end())
		{
			return it->second;
		}
		else
		{
			return nullptr;
		}
	}

	std::shared_ptr<DeviceRoot> EquipmentSet::root()
	{
		return m_root;
	}

	const std::shared_ptr<DeviceRoot> EquipmentSet::root() const
	{
		return m_root;
	}

	[[nodiscard]] std::vector<std::shared_ptr<DeviceObject>> EquipmentSet::devices()
	{
		std::vector<std::shared_ptr<DeviceObject>> result;
		result.reserve(m_deviceTable.size());

		for (auto& d : m_deviceTable)
		{
			result.push_back(d.second);
		}

		return result;
	}

	void EquipmentSet::dump(bool dumpProps, QDebug d) const
	{
		if (m_root)
		{
			m_root->dump(dumpProps, true);
		}
		else
		{
			d << "EquipmentSet::root is empty";
		}

		return;
	}

	void EquipmentSet::addDeviceChildrenToHashTable(const std::shared_ptr<DeviceObject>& parent)
	{
		for (int i = 0; i < parent->childrenCount(); i++)
		{
			const std::shared_ptr<DeviceObject>& child = parent->child(i);
			m_deviceTable.insert({child->equipmentIdTemplate(), child});

			addDeviceChildrenToHashTable(child);
		}

		return;
	}
} // namespace Hardware