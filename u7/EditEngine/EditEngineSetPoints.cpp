#include "EditEngineSetPoints.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{

	SetPointsCommand::SetPointsCommand(EditSchemaView* schemeView,
			const std::vector<std::vector<VFrame30::SchemaPoint>>& points,
			const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemeView, hScrollBar, vScrollBar)
	{
		assert(schemeView != nullptr);
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

	void SetPointsCommand::executeCommand(EditSchemaView* schemeView)
	{
		if (m_items.size() != m_newPoints.size())
		{
			assert(m_items.size() == m_newPoints.size());
			return;
		}

		schemeView->setSelectedItems(m_items);

		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			m_items[i]->setPointList(m_newPoints[i]);
		}

		return;
	}

	void SetPointsCommand::unExecuteCommand(EditSchemaView* schemeView)
	{
		if (m_items.size() != m_newPoints.size())
		{
			assert(m_items.size() == m_newPoints.size());
			return;
		}

		schemeView->setSelectedItems(m_items);

		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			m_items[i]->setPointList(m_oldPoints[i]);
		}

		return;
	}

}
