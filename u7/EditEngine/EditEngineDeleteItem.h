#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class DeleteItemCommand : public EditCommand
	{
		DeleteItemCommand();		// deleted;
	public:
		DeleteItemCommand(EditSchemaView* schemaView,
			std::vector<SchemaItemPtr> items,
			std::shared_ptr<VFrame30::SchemaLayer> layer,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		std::vector<SchemaItemPtr> m_items;				// Items for delete operation
		std::vector<SchemaItemPtr> m_prevOrder;			// Item's order in the layer before delete
		std::vector<SchemaItemPtr> m_selectedItems;

		std::shared_ptr<VFrame30::SchemaLayer> m_layer;
	};

}

