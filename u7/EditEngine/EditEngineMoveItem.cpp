#include "EditEngineMoveItem.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{

	MoveItemCommand::MoveItemCommand(EditSchemaView* schemeView,
			double xdiff,
			double ydiff,
			const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
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

	void MoveItemCommand::executeCommand(EditSchemaView* schemeView)
	{
		schemeView->setSelectedItems(m_items);

		for (std::shared_ptr<VFrame30::SchemaItem> item : m_items)
		{
			item->MoveItem(m_xdiff, m_ydiff);

			if (m_snapToGrid)
			{
				item->snapToGrid(schemeView->schema()->gridSize());
			}
		}

		return;
	}

	void MoveItemCommand::unExecuteCommand(EditSchemaView* schemeView)
	{
		schemeView->setSelectedItems(m_items);

		for (std::shared_ptr<VFrame30::SchemaItem> item : m_items)
		{
			item->MoveItem(-m_xdiff, -m_ydiff);

			if (m_snapToGrid)
			{
				item->snapToGrid(schemeView->schema()->gridSize());
			}
		}

		return;
	}

}
