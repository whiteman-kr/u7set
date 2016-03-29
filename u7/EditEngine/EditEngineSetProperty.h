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
		SetPropertyCommand(EditSchemaView* schemaView,
				QString propertyName,
				QVariant value,
				const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(EditSchemaView* schemaView) override;
		virtual void unExecuteCommand(EditSchemaView* schemaView) override;

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
		std::shared_ptr<VFrame30::Schema> m_schema;
	};

}


#endif // EDITENGINESETPROPERTIY_H
