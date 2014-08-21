#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetPointsCommand  : public EditCommand
	{
		SetPointsCommand();		// deleted;
	public:
		SetPointsCommand(
				EditVideoFrameView* videoFrameView,
				const std::vector<std::vector<VFrame30::VideoItemPoint>>& points,
				const std::vector<std::shared_ptr<VFrame30::CVideoItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditVideoFrameView* videoFrameView) override;
		virtual void unExecuteCommand(EditVideoFrameView* videoFrameView) override;

		// Data
		//
	private:
		std::vector<std::vector<VFrame30::VideoItemPoint>> m_newPoints;
		std::vector<std::vector<VFrame30::VideoItemPoint>> m_oldPoints;
		std::vector<std::shared_ptr<VFrame30::CVideoItem>> m_items;
	};

}

