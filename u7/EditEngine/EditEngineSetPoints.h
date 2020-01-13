#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetPointsCommand  : public EditCommand
	{
		SetPointsCommand();		// deleted;
	public:
		SetPointsCommand(EditSchemaView* schemaView,
				const std::vector<std::vector<VFrame30::SchemaPoint>>& points,
				const std::vector<SchemaItemPtr>& items,
				bool selectChangedItems,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		std::vector<std::vector<VFrame30::SchemaPoint>> m_newPoints;
		std::vector<std::vector<VFrame30::SchemaPoint>> m_oldPoints;
		std::vector<SchemaItemPtr> m_items;

		bool m_selectChangedItems = true;
	};

}

