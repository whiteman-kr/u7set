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
				const std::vector<SchemaItemPtr>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		//--
		//
		struct Record
		{
			QString propertyName;
			QVariant oldValue;
			QVariant newValue;
			SchemaItemPtr item;
		};

		// Data
		//
	private:
		std::vector<Record> m_items;
		std::shared_ptr<VFrame30::Schema> m_schema;
	};

}

