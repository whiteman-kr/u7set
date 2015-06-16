#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class MoveItemCommand  : public EditCommand
	{
		MoveItemCommand();		// deleted;
	public:
		MoveItemCommand(EditSchemeView* schemeView,
				double xdiff,
				double ydiff,
				const std::vector<std::shared_ptr<VFrame30::VideoItem>>& items,
				bool snapToGrid,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* videoFrameView) override;
		virtual void unExecuteCommand(EditSchemeView* videoFrameView) override;

		// Data
		//
	private:
		double m_xdiff = 0.0;
		double m_ydiff = 0.0;
		std::vector<std::shared_ptr<VFrame30::VideoItem>> m_items;
		bool m_snapToGrid = true;
	};

}

