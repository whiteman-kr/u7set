#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class AddItemCommand : public EditCommand
	{
		AddItemCommand();		// deleted;
	public:
		AddItemCommand(EditSchemeView* videoFrameView, std::list<std::shared_ptr<VFrame30::CVideoItem>> items, std::shared_ptr<VFrame30::CVideoLayer> layer, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* videoFrameView) override;
		virtual void unExecuteCommand(EditSchemeView* videoFrameView) override;

		// Data
		//
	private:
		std::list<std::shared_ptr<VFrame30::CVideoItem>> m_items;
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_selectedItems;
		std::shared_ptr<VFrame30::CVideoLayer> m_layer;
	};

}

