#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class DeleteItemCommand : public EditCommand
	{
		DeleteItemCommand();		// deleted;
	public:
		DeleteItemCommand(EditSchemaView* schemaView,
			std::vector<std::shared_ptr<VFrame30::SchemaItem>> items,
			std::shared_ptr<VFrame30::SchemaLayer> layer,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;

		// Data
		//
	private:
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;				// Items for delete operation
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_prevOrder;			// Item's order in the layer before delete
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_selectedItems;

		std::shared_ptr<VFrame30::SchemaLayer> m_layer;
	};

}

