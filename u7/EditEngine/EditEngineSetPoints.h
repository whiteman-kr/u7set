#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetPointsCommand  : public EditCommand
	{
		SetPointsCommand();		// deleted;
	public:
		SetPointsCommand(
				EditSchemaView* schemaView,
				const std::vector<std::vector<VFrame30::SchemaPoint>>& points,
				const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
				bool selectChangedItems,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;

		// Data
		//
	private:
		std::vector<std::vector<VFrame30::SchemaPoint>> m_newPoints;
		std::vector<std::vector<VFrame30::SchemaPoint>> m_oldPoints;
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;

		bool m_selectChangedItems = true;
	};

}

