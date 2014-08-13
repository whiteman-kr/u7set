#include "EditEngineMoveItem.h"
#include "VideoFrameWidget.h"
#include "EditVideoFrameWidget.h"

namespace EditEngine
{

	MoveItemCommand::MoveItemCommand(
			EditVideoFrameView* videoFrameView,
			double xdiff,
			double ydiff,
			const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(videoFrameView, hScrollBar, vScrollBar)
	{
		assert(videoFrameView != nullptr);
		assert(items.empty() == false);

		m_xdiff = xdiff;
		m_ydiff = ydiff;

		m_items = items;
		return;
	}

	void MoveItemCommand::executeCommand(EditVideoFrameView* videoFrameView)
	{
		videoFrameView->setSelectedItems(m_items);

		std::for_each(m_items.begin(), m_items.end(),
			[this](const std::shared_ptr<VFrame30::CVideoItem>& item)
			{
				item->MoveItem(m_xdiff, m_ydiff);
			}
			);

		return;
	}

	void MoveItemCommand::unExecuteCommand(EditVideoFrameView* videoFrameView)
	{
		videoFrameView->setSelectedItems(m_items);

		std::for_each(m_items.begin(), m_items.end(),
			[this](const std::shared_ptr<VFrame30::CVideoItem>& item)
			{
				item->MoveItem(-m_xdiff, -m_ydiff);
			}
			);

		return;
	}

}
