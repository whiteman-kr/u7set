#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetPointsCommand  : public EditCommand
	{
		SetPointsCommand();		// deleted;
	public:
		SetPointsCommand(
				EditSchemaView* schemeView,
				const std::vector<std::vector<VFrame30::SchemaPoint>>& points,
				const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemaView* schemeView) override;
		virtual void unExecuteCommand(EditSchemaView* schemeView) override;

		// Data
		//
	private:
		std::vector<std::vector<VFrame30::SchemaPoint>> m_newPoints;
		std::vector<std::vector<VFrame30::SchemaPoint>> m_oldPoints;
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;
	};

}

