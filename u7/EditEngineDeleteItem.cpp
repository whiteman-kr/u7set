#include "EditEngineDeleteItem.h"
#include "VideoFrameWidget.h"
#include "EditVideoFrameWidget.h"

namespace EditEngine
{
	DeleteItemCommand::DeleteItemCommand(
		EditVideoFrameView* videoFrameView,
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> items,
		std::shared_ptr<VFrame30::CVideoLayer> layer,
		QScrollBar* hScrollBar,
		QScrollBar* vScrollBar)
		: EditCommand(videoFrameView, hScrollBar, vScrollBar)
	{
		assert(videoFrameView != nullptr);
		assert(items.empty() == false);
		assert(layer != nullptr);

		m_layer = layer;

		m_items.assign(items.begin(), items.end());
		m_prevOrder.assign(layer->Items.begin(), layer->Items.end());

		m_selectedItems = videoFrameView->selectedItems();

		return;
	}

	void DeleteItemCommand::executeCommand(EditVideoFrameView* videoFrameView)
	{
		std::for_each(m_items.begin(), m_items.end(),
			[this](std::shared_ptr<VFrame30::CVideoItem> item)
			{
				m_layer->Items.erase(std::remove(m_layer->Items.begin(), m_layer->Items.end(), item), m_layer->Items.end());
			}
			);

		videoFrameView->clearSelection();
		return;
	}

	void DeleteItemCommand::unExecuteCommand(EditVideoFrameView* videoFrameView)
	{
		m_layer->Items.assign(m_prevOrder.begin(), m_prevOrder.end());

		videoFrameView->setSelectedItems(m_items);
		return;
	}
}
