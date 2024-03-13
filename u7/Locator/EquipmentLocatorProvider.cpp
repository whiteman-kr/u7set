#include "EquipmentLocatorProvider.h"
#include "../EquipmentEditor/EquipmentTabPage.h"

#include <HardwareLib/DeviceObject.h>

namespace Locator
{
	QString EquipmentLocatorProvider::name() const
	{
		return QStringLiteral("Equipment");
	}

	void EquipmentLocatorProvider::locateFor(const QString& text)
	{
		if (m_equipmentTabPage == nullptr)
		{
			emit resultReady(text, this, {});
			return;
		}

		std::vector<std::shared_ptr<Hardware::DeviceObject>> objects;
		std::vector<std::shared_ptr<Hardware::DeviceObject>> objectsWithChildren;

		objects = m_equipmentTabPage->deviceObjects(text);

		for (auto& foundObject : objects)
		{
			objectsWithChildren.push_back(foundObject);
		}

		// Show childer only if found 1 object
		//
		if (objects.size() == 1)
		{
			for (const auto& child : objects[0]->children())
			{
				objectsWithChildren.push_back(child);
			}
		}

		std::vector<LocatedItem> result;
		result.reserve(objectsWithChildren.size());

		for (const auto& obj : objectsWithChildren)
		{
			result.emplace_back(obj->equipmentId(), QStringLiteral("Device") + obj->deviceTypeName() + ": " + obj->caption());
		}

		emit resultReady(text, this, result);
		return;
	}

	void EquipmentLocatorProvider::stopSearching()
	{
		// This is blocking provider, stop searching is not applicable in this case.
		//
		return;
	}

	const EquipmentTabPage* EquipmentLocatorProvider::equipmentTabPage() const
	{
		return m_equipmentTabPage;
	}

	void EquipmentLocatorProvider::setEquipmentTabPage(const EquipmentTabPage* value)
	{
		m_equipmentTabPage = value;
	}
}

