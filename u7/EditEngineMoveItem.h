#ifndef EDITENGINEMOVEITEM_H
#define EDITENGINEMOVEITEM_H

#include "EditEngine.h"

namespace EditEngine
{

	class MoveItemCommand  : public EditCommand
	{
		MoveItemCommand();		// deleted;
	public:
		MoveItemCommand(
				EditVideoFrameView* videoFrameView,
				double xdiff,
				double ydiff,
				const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditVideoFrameView* videoFrameView) override;
		virtual void unExecuteCommand(EditVideoFrameView* videoFrameView) override;

		// Data
		//
	private:
		double m_xdiff;
		double m_ydiff;
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_items;
	};

}

#endif // EDITENGINEMOVEITEM_H
