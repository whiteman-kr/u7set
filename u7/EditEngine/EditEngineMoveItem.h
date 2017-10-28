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
				const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
				bool snapToGrid,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;

		// Data
		//
	private:
		double m_xdiff = 0.0;
		double m_ydiff = 0.0;
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;
		bool m_snapToGrid = true;
	};

}

