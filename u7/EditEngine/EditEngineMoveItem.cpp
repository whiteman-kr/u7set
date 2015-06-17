#include "EditEngineMoveItem.h"
#include "VideoFrameWidget.h"
#include "EditSchemeWidget.h"

namespace EditEngine
{

	MoveItemCommand::MoveItemCommand(EditSchemeView* schemeView,
			double xdiff,
			double ydiff,
			const std::vector<std::shared_ptr<VFrame30::VideoItem>>& items,
			bool snapToGrid,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemeView, hScrollBar, vScrollBar)
	{
		assert(schemeView != nullptr);
		assert(items.empty() == false);

		m_xdiff = xdiff;
		m_ydiff = ydiff;

		m_items = items;
		m_snapToGrid = snapToGrid;
		return;
	}

	void MoveItemCommand::executeCommand(EditSchemeView* videoFrameView)
	{
		videoFrameView->setSelectedItems(m_items);

		for (std::shared_ptr<VFrame30::VideoItem> item : m_items)
		{
			item->MoveItem(m_xdiff, m_ydiff);

			if (m_snapToGrid)
			{
				item->snapToGrid(videoFrameView->scheme()->gridSize());
			}
		}

		return;
	}

	void MoveItemCommand::unExecuteCommand(EditSchemeView* videoFrameView)
	{
		videoFrameView->setSelectedItems(m_items);

		for (std::shared_ptr<VFrame30::VideoItem> item : m_items)
		{
			item->MoveItem(-m_xdiff, -m_ydiff);

			if (m_snapToGrid)
			{
				item->snapToGrid(videoFrameView->scheme()->gridSize());
			}
		}

		return;
	}

}
