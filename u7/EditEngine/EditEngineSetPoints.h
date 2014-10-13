#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetPointsCommand  : public EditCommand
	{
		SetPointsCommand();		// deleted;
	public:
		SetPointsCommand(
				EditSchemeView* videoFrameView,
				const std::vector<std::vector<VFrame30::VideoItemPoint>>& points,
				const std::vector<std::shared_ptr<VFrame30::VideoItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* videoFrameView) override;
		virtual void unExecuteCommand(EditSchemeView* videoFrameView) override;

		// Data
		//
	private:
		std::vector<std::vector<VFrame30::VideoItemPoint>> m_newPoints;
		std::vector<std::vector<VFrame30::VideoItemPoint>> m_oldPoints;
		std::vector<std::shared_ptr<VFrame30::VideoItem>> m_items;
	};

}

