#pragma once

#include "DeviceRoot.h"

namespace Hardware
{
	//
	//
	// EquipmentSet
	//
	//
	class EquipmentSet
	{
	public:
		EquipmentSet() = default;
		EquipmentSet(std::shared_ptr<DeviceRoot> root);
		~EquipmentSet();

	public:
		void set(std::shared_ptr<DeviceRoot> root);

		[[nodiscard]] std::shared_ptr<DeviceObject> deviceObject(const QString& equipmentId);
		[[nodiscard]] const std::shared_ptr<DeviceObject> deviceObject(const QString& equipmentId) const;

		[[nodiscard]] std::shared_ptr<DeviceRoot> root();
		[[nodiscard]] const std::shared_ptr<DeviceRoot> root() const;

		[[nodiscard]] std::vector<std::shared_ptr<DeviceObject>> devices();

		void dump(bool dumpProps, QDebug d) const;

	private:
		void addDeviceChildrenToHashTable(const std::shared_ptr<DeviceObject>& parent);

	private:
		std::shared_ptr<DeviceRoot> m_root;
		QHash<QString, std::shared_ptr<DeviceObject>> m_deviceTable;
	};
} // namespace Hardware