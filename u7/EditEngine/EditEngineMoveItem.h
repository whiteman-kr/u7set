#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class MoveItemCommand  : public EditCommand
	{
		MoveItemCommand();		// deleted;
	public:
		MoveItemCommand(
				EditSchemeView* videoFrameView,
				double xdiff,
				double ydiff,
				const std::vector<std::shared_ptr<VFrame30::VideoItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* videoFrameView) override;
		virtual void unExecuteCommand(EditSchemeView* videoFrameView) override;

		// Data
		//
	private:
		double m_xdiff;
		double m_ydiff;
		std::vector<std::shared_ptr<VFrame30::VideoItem>> m_items;
	};

}

