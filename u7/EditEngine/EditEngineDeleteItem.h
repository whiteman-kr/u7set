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
			std::vector<std::shared_ptr<VFrame30::VideoItem>> items,
			std::shared_ptr<VFrame30::SchemeLayer> layer,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* videoFrameView) override;
		virtual void unExecuteCommand(EditSchemeView* videoFrameView) override;

		// Data
		//
	private:
		std::vector<std::shared_ptr<VFrame30::VideoItem>> m_items;				// Items for delete operation
		std::vector<std::shared_ptr<VFrame30::VideoItem>> m_prevOrder;			// Item's order in the layer before delete
		std::vector<std::shared_ptr<VFrame30::VideoItem>> m_selectedItems;

		std::shared_ptr<VFrame30::SchemeLayer> m_layer;
	};

}

