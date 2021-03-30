#include "EditEngineDeleteItem.h"
#include "SchemaEditor/EditSchemaView.h"

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
		m_prevOrder.assign(layer->Items.begin(), layer->Items.end());

		m_selectedItems = schemaView->selectedItems();

		return;
	}

	void DeleteItemCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		std::for_each(m_items.begin(), m_items.end(),
			[this](SchemaItemPtr item)
			{
				m_layer->Items.erase(std::remove(m_layer->Items.begin(), m_layer->Items.end(), item), m_layer->Items.end());
			}
			);

		//schemaView->clearSelection();
		Q_UNUSED(itemsToSelect);

		return;
	}

	void DeleteItemCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		m_layer->Items.assign(m_prevOrder.begin(), m_prevOrder.end());

		*itemsToSelect = m_items;

		return;
	}
}
