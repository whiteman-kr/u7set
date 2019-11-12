#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class MoveItemCommand  : public EditCommand
	{
		MoveItemCommand();		// deleted;
	public:
		MoveItemCommand(EditSchemaView* schemaView,
				double xdiff,
				double ydiff,
				const std::vector<SchemaItemPtr>& items,
				bool snapToGrid,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		double m_xdiff = 0.0;
		double m_ydiff = 0.0;
		std::vector<SchemaItemPtr> m_items;
		bool m_snapToGrid = true;
	};

}

