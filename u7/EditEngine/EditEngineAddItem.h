#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class AddItemCommand : public EditCommand
	{
		AddItemCommand();		// deleted;
	public:
		AddItemCommand(EditSchemeView* videoFrameView, std::list<std::shared_ptr<VFrame30::SchemeItem>> items, std::shared_ptr<VFrame30::SchemeLayer> layer, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* schemeView) override;
		virtual void unExecuteCommand(EditSchemeView* schemeView) override;

		// Data
		//
	private:
		std::list<std::shared_ptr<VFrame30::SchemeItem>> m_items;
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> m_selectedItems;
		std::shared_ptr<VFrame30::SchemeLayer> m_layer;
	};

}

