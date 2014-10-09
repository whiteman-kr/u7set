#include "EditEngineSetPoints.h"
#include "VideoFrameWidget.h"
#include "EditSchemeWidget.h"

namespace EditEngine
{

	SetPointsCommand::SetPointsCommand(
			EditSchemeView* videoFrameView,
			const std::vector<std::vector<VFrame30::VideoItemPoint>>& points,
			const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(videoFrameView, hScrollBar, vScrollBar)
	{
		assert(videoFrameView != nullptr);
		assert(items.empty() == false);
		assert(points.empty() == false);

		m_items = items;
		m_newPoints = points;

		for (auto it = items.begin(); it != items.end(); ++it)
		{
			m_oldPoints.push_back(it->get()->getPointList());
		}

		return;
	}

	void SetPointsCommand::executeCommand(EditSchemeView* videoFrameView)
	{
		if (m_items.size() != m_newPoints.size())
		{
			assert(m_items.size() == m_newPoints.size());
			return;
		}

		videoFrameView->setSelectedItems(m_items);

		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			m_items[i]->setPointList(m_newPoints[i]);
		}

		return;
	}

	void SetPointsCommand::unExecuteCommand(EditSchemeView* videoFrameView)
	{
		if (m_items.size() != m_newPoints.size())
		{
			assert(m_items.size() == m_newPoints.size());
			return;
		}

		videoFrameView->setSelectedItems(m_items);

		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			m_items[i]->setPointList(m_oldPoints[i]);
		}

		return;
	}

}
