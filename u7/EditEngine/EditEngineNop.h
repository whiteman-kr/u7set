#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class NopItemCommand : public EditCommand
	{
	public:
		NopItemCommand() = delete;
		NopItemCommand(EditSchemaView* schemaView,
					   const std::vector<SchemaItemPtr>& items,
					   QScrollBar* hScrollBar,
					   QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		std::vector<SchemaItemPtr> m_items;
	};

}

