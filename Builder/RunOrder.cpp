#include "RunOrder.h"

namespace Builder
{
	void RunOrder::clear()
	{
		m_runOrder.clear();
	}

	void RunOrder::setRunOrder(QString equipmentId, const std::unordered_map<QUuid, std::pair<int, int>>& data)
	{
		m_runOrder[equipmentId] = {equipmentId, data};
	}

	std::pair<int, int> RunOrder::schemaItemRunOrder(const QString& equipmentId, const QUuid& itemId) const
	{
		auto itd = m_runOrder.find(equipmentId);
		if (itd == m_runOrder.end())
		{
			return {-1, -1};
		}

		const RunOrderDevice& ro = itd->second;
		assert(ro.equipmentId == equipmentId);

		auto it = ro.schemaItemsRunOrder.find(itemId);
		if (it == ro.schemaItemsRunOrder.end())
		{
			return {-1, -1};
		}

		return it->second;
	}
}
