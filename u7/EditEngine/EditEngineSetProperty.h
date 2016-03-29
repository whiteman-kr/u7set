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
				EditSchemaView* schemeView,
				QString propertyName,
				QVariant value,
				const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemaView* schemeView) override;
		virtual void unExecuteCommand(EditSchemaView* schemeView) override;

		//--
		//
		struct Record
		{
			QString propertyName;
			QVariant oldValue;
			QVariant newValue;
			std::shared_ptr<VFrame30::SchemaItem> item;
		};

		// Data
		//
	private:
		std::vector<Record> m_items;
		std::shared_ptr<VFrame30::Schema> m_scheme;
	};

}


#endif // EDITENGINESETPROPERTIY_H
