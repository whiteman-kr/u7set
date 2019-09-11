#include "EditEngineNop.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{

	NopItemCommand::NopItemCommand(EditSchemaView* schemaView,
			const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar),
		m_items(items)
	{
		Q_ASSERT(schemaView != nullptr);
		Q_ASSERT(items.empty() == false);
		return;
	}

	void NopItemCommand::executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		*itemsToSelect = m_items;
		return;
	}

	void NopItemCommand::unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		*itemsToSelect = m_items;
		return;
	}

}
