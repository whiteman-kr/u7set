#ifndef EDITENGINEDELETEITEM_H
#define EDITENGINEDELETEITEM_H

#include "EditEngine.h"

namespace EditEngine
{

	class DeleteItemCommand : public EditCommand
	{
		DeleteItemCommand();		// deleted;
	public:
		DeleteItemCommand(
			EditVideoFrameView* videoFrameView,
			std::vector<std::shared_ptr<VFrame30::CVideoItem>> items,
			std::shared_ptr<VFrame30::CVideoLayer> layer,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditVideoFrameView* videoFrameView) override;
		virtual void unExecuteCommand(EditVideoFrameView* videoFrameView) override;

		// Data
		//
	private:
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_items;				// Items for delete operation
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_prevOrder;			// Item's order in the layer before delete
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_selectedItems;

		std::shared_ptr<VFrame30::CVideoLayer> m_layer;
	};

}

#endif // EDITENGINEDELETEITEM_H
