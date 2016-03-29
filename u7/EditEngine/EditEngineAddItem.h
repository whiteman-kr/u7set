#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class AddItemCommand : public EditCommand
	{
		AddItemCommand();		// deleted;
	public:
		AddItemCommand(EditSchemeView* schemeView, std::list<std::shared_ptr<VFrame30::SchemaItem>> items, std::shared_ptr<VFrame30::SchemaLayer> layer, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* schemeView) override;
		virtual void unExecuteCommand(EditSchemeView* schemeView) override;

		// Data
		//
	private:
		std::list<std::shared_ptr<VFrame30::SchemaItem>> m_items;
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_selectedItems;
		std::shared_ptr<VFrame30::SchemaLayer> m_layer;
	};

}

