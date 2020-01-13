#include "EditEngineMoveItem.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{

	MoveItemCommand::MoveItemCommand(EditSchemaView* schemaView,
			double xdiff,
			double ydiff,
			const std::vector<SchemaItemPtr>& items,
			bool snapToGrid,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(items.empty() == false);

		m_xdiff = xdiff;
		m_ydiff = ydiff;

		m_items = items;
		m_snapToGrid = snapToGrid;
		return;
	}

	void MoveItemCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		*itemsToSelect = m_items;

		for (SchemaItemPtr item : m_items)
		{
			item->moveItem(m_xdiff, m_ydiff);

			if (m_snapToGrid)
			{
				item->snapToGrid(m_schemaView->schema()->gridSize());
			}
		}

		return;
	}

	void MoveItemCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		*itemsToSelect = m_items;

		for (SchemaItemPtr item : m_items)
		{
			item->moveItem(-m_xdiff, -m_ydiff);

			if (m_snapToGrid)
			{
				item->snapToGrid(m_schemaView->schema()->gridSize());
			}
		}

		return;
	}

}
