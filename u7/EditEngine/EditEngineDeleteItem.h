#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class DeleteItemCommand : public EditCommand
	{
		DeleteItemCommand();		// deleted;
	public:
		DeleteItemCommand(
			EditSchemeView* videoFrameView,
			std::vector<std::shared_ptr<VFrame30::SchemeItem>> items,
			std::shared_ptr<VFrame30::SchemeLayer> layer,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* schemeView) override;
		virtual void unExecuteCommand(EditSchemeView* schemeView) override;

		// Data
		//
	private:
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> m_items;				// Items for delete operation
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> m_prevOrder;			// Item's order in the layer before delete
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> m_selectedItems;

		std::shared_ptr<VFrame30::SchemeLayer> m_layer;
	};

}

