#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class NopItemCommand : public EditCommand
	{
	public:
		NopItemCommand() = delete;
		NopItemCommand(EditSchemaView* schemaView,
					   const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
					   QScrollBar* hScrollBar,
					   QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;

		// Data
		//
	private:
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;
	};

}

