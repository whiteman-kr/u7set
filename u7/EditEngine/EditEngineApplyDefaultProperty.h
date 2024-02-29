#pragma once

#include "../ProjectDefaults.h"
#include "EditEngine.h"

namespace EditEngine
{
	class ApplyDefaultPropertyCommand : public EditCommand
	{
	public:
		ApplyDefaultPropertyCommand() = delete;
		ApplyDefaultPropertyCommand(EditSchemaView* schemaView,
									ProjectDefaults projectDefaults,
									std::vector<SchemaItemPtr> items,
									QScrollBar* hScrollBar,
									QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		ProjectDefaults m_projectDefaults;
		std::vector<SchemaItemPtr> m_items;
		std::shared_ptr<VFrame30::Schema> m_schema;

		struct Record
		{
			SchemaItemPtr item;
			QString propertyName;
			QVariant oldValue;
			QVariant newValue;
		};

		std::vector<Record> m_records;
	};

} // namespace EditEngine
