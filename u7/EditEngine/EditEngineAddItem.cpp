#include "EditEngineAddItem.h"
#include "SchemaEditor/EditSchemaView.h"

#include <VFrame30/SchemaLayer.h>

namespace EditEngine
{
	AddItemCommand::AddItemCommand(EditSchemaView* schemaView, std::list<SchemaItemPtr> items, std::shared_ptr<VFrame30::SchemaLayer> layer, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
		: EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(items.empty() == false);
		assert(layer != nullptr);

		m_items = items;
		m_layer = layer;

		m_selectedItems = schemaView->selectedItems();

		return;
	}

	void AddItemCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		m_layer->pushBackItems(m_items.begin(), m_items.end());
		itemsToSelect->assign(m_items.begin(), m_items.end());
		return;
	}

	void AddItemCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		for (const auto& item : m_items)
		{
			[[maybe_unused]] bool ok = m_layer->removeItem(item);
			assert(ok);
		}

		itemsToSelect->assign(m_selectedItems.begin(), m_selectedItems.end());

		return;
	}
}
