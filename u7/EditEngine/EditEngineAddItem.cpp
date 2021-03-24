#include "EditEngineAddItem.h"
#include "SchemaEditor/EditSchemaView.h"

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
		m_layer->Items.insert(m_layer->Items.end(), m_items.begin(), m_items.end());

		itemsToSelect->assign(m_items.begin(), m_items.end());

		return;
	}

	void AddItemCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		for (auto si = m_items.begin(); si != m_items.end(); ++si)
		{
			auto findResult = std::find_if(m_layer->Items.begin(), m_layer->Items.end(),
				[&si](SchemaItemPtr item)
				{
					return item.get() == si->get();
				});

			if(findResult != m_layer->Items.end())
			{
				m_layer->Items.erase(findResult);
			}
			else
			{
				assert(findResult != m_layer->Items.end());
			}
		}

		itemsToSelect->assign(m_selectedItems.begin(), m_selectedItems.end());

		return;
	}
}
