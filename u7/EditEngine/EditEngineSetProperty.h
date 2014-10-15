#ifndef EDITENGINESETPROPERTIY_H
#define EDITENGINESETPROPERTIY_H

#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetPropertyCommand : public EditCommand
	{
		SetPropertyCommand();
	public:
		SetPropertyCommand(
				EditSchemeView* videoFrameView,
				QString propertyName,
				QVariant value,
				const std::vector<std::shared_ptr<VFrame30::VideoItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* videoFrameView) override;
		virtual void unExecuteCommand(EditSchemeView* videoFrameView) override;

		//--
		//
		struct Record
		{
			QString propertyName;
			QVariant oldValue;
			QVariant newValue;
			std::shared_ptr<VFrame30::VideoItem> item;
		};

		// Data
		//
	private:
		std::vector<Record> m_items;
	};

}


#endif // EDITENGINESETPROPERTIY_H
