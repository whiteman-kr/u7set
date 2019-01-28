#pragma once

#include "../lib/Hash.h"

namespace Builder
{
	class RunOrder
	{
	public:
		RunOrder() = default;
		RunOrder(const RunOrder&) = default;
		RunOrder(RunOrder&&) = default;
		RunOrder& operator= (const RunOrder&) = default;
		RunOrder& operator= (RunOrder&&) = default;

	public:
		void clear();
		void setRunOrder(QString equipmentId, const std::unordered_map<QUuid, std::pair<int, int> >& data);
		std::pair<int, int> schemaItemRunOrder(const QString& equipmentId, const QUuid& itemId) const;

	private:
		struct RunOrderDevice	// Device or UFB
		{
			QString equipmentId;												// LM's EquipmnetId or UfbSchemaId
			std::unordered_map<QUuid, std::pair<int, int>> schemaItemsRunOrder;	// Key is item's guid, value is run order index
																				// It is pair, for UFBs:
																				// if first != second then it's a range, (runorder from - to)
		};

		std::map<QString, RunOrderDevice> m_runOrder;
	};

}

Q_DECLARE_METATYPE(Builder::RunOrder)
