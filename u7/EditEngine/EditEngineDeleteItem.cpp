#include "EditEngineDeleteItem.h"
#include "SchemaEditor/EditSchemaView.h"
#include <VFrame30/SchemaLayer.h>

namespace EditEngine
{
	DeleteItemCommand::DeleteItemCommand(EditSchemaView* schemaView,
		std::vector<SchemaItemPtr> items,
		std::shared_ptr<VFrame30::SchemaLayer> layer,
		QScrollBar* hScrollBar,
		QScrollBar* vScrollBar)
		: EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(items.empty() == false);
		assert(layer != nullptr);

		m_layer = layer;

		m_items.assign(items.begin(), items.end());
		m_prevOrder.assign(layer->items().begin(), layer->items().end());

		m_selectedItems = schemaView->selectedItems();

		return;
	}

	void DeleteItemCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		Q_UNUSED(itemsToSelect);

		std::for_each(m_items.begin(), m_items.end(),
			[this](const SchemaItemPtr& item)
			{
				m_layer->removeItem(item);
			});

		return;
	}

	void DeleteItemCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		m_layer->setItems(m_prevOrder.begin(), m_prevOrder.end());

		*itemsToSelect = m_items;

		return;
	}
}
