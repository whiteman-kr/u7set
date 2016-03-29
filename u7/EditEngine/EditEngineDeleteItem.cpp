#include "EditEngineDeleteItem.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{
	DeleteItemCommand::DeleteItemCommand(EditSchemaView* schemaView,
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items,
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
		m_prevOrder.assign(layer->Items.begin(), layer->Items.end());

		m_selectedItems = schemaView->selectedItems();

		return;
	}

	void DeleteItemCommand::executeCommand(EditSchemaView* schemaView)
	{
		std::for_each(m_items.begin(), m_items.end(),
			[this](std::shared_ptr<VFrame30::SchemaItem> item)
			{
				m_layer->Items.erase(std::remove(m_layer->Items.begin(), m_layer->Items.end(), item), m_layer->Items.end());
			}
			);

		schemaView->clearSelection();
		return;
	}

	void DeleteItemCommand::unExecuteCommand(EditSchemaView* schemaView)
	{
		m_layer->Items.assign(m_prevOrder.begin(), m_prevOrder.end());

		schemaView->setSelectedItems(m_items);
		return;
	}
}
