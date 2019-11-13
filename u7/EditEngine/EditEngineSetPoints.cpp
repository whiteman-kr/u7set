#include "EditEngineSetPoints.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{

	SetPointsCommand::SetPointsCommand(EditSchemaView* schemaView,
			const std::vector<std::vector<VFrame30::SchemaPoint>>& points,
			const std::vector<SchemaItemPtr>& items,
			bool selectChangedItems,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(items.empty() == false);
		assert(points.empty() == false);

		m_items = items;
		m_newPoints = points;
		m_selectChangedItems = selectChangedItems;

		for (auto it = items.begin(); it != items.end(); ++it)
		{
			m_oldPoints.push_back(it->get()->getPointList());
		}

		return;
	}

	void SetPointsCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		if (m_items.size() != m_newPoints.size())
		{
			assert(m_items.size() == m_newPoints.size());
			return;
		}

		if (m_selectChangedItems == true)
		{
			*itemsToSelect = m_items;
		}

		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			m_items[i]->setPointList(m_newPoints[i]);
		}

		return;
	}

	void SetPointsCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		if (m_items.size() != m_newPoints.size())
		{
			assert(m_items.size() == m_newPoints.size());
			return;
		}

		if (m_selectChangedItems == true)
		{
			*itemsToSelect = m_items;
		}

		for (unsigned int i = 0; i < m_items.size(); i++)
		{
			m_items[i]->setPointList(m_oldPoints[i]);
		}

		return;
	}

}
