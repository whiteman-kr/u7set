#include "EditEngineAddItem.h"
#include "EditSchemeWidget.h"

namespace EditEngine
{
	AddItemCommand::AddItemCommand(EditSchemeView* videoFrameView, std::list<std::shared_ptr<VFrame30::SchemeItem>> items, std::shared_ptr<VFrame30::SchemeLayer> layer, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
		: EditCommand(videoFrameView, hScrollBar, vScrollBar)
	{
		assert(videoFrameView != nullptr);
		assert(items.empty() == false);
		assert(layer != nullptr);

		m_items = items;
		m_layer = layer;

		m_selectedItems = videoFrameView->selectedItems();

		return;
	}

	void AddItemCommand::executeCommand(EditSchemeView* schemeView)
	{
		m_layer->Items.insert(m_layer->Items.end(), m_items.begin(), m_items.end());

		schemeView->setSelectedItems(m_items);
	}

	void AddItemCommand::unExecuteCommand(EditSchemeView* schemeView)
	{
		for (auto si = m_items.begin(); si != m_items.end(); ++si)
		{
			auto findResult = std::find_if(m_layer->Items.begin(), m_layer->Items.end(),
				[&si](std::shared_ptr<VFrame30::SchemeItem> item)
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

		schemeView->setSelectedItems(m_selectedItems);
	}
}
