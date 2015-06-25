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
				const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemeView* schemeView) override;
		virtual void unExecuteCommand(EditSchemeView* schemeView) override;

		//--
		//
		struct Record
		{
			QString propertyName;
			QVariant oldValue;
			QVariant newValue;
			std::shared_ptr<VFrame30::SchemeItem> item;
		};

		// Data
		//
	private:
		std::vector<Record> m_items;
		std::shared_ptr<VFrame30::Scheme> m_scheme;
	};

}


#endif // EDITENGINESETPROPERTIY_H
