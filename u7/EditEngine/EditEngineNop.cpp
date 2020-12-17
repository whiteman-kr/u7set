#include "EditEngineNop.h"
#include "SchemaEditor/EditSchemaWidget.h"

namespace EditEngine
{

	NopItemCommand::NopItemCommand(EditSchemaView* schemaView,
			const std::vector<SchemaItemPtr>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar),
		m_items(items)
	{
		Q_ASSERT(schemaView != nullptr);
		Q_ASSERT(items.empty() == false);
		return;
	}

	void NopItemCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		*itemsToSelect = m_items;
		return;
	}

	void NopItemCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		*itemsToSelect = m_items;
		return;
	}

}
