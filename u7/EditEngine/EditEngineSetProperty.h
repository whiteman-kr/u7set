#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetPropertyCommand : public EditCommand
	{
	public:
		SetPropertyCommand() = delete;
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

	public:
		static void setProperty(const QString& propertyName,
								const QVariant& value,
								const SchemaItemPtr& item,
								std::shared_ptr<VFrame30::Schema> schema,
								EditSchemaView* schemaView);

		// Data
		//
	private:
		std::vector<Record> m_items;
		std::shared_ptr<VFrame30::Schema> m_schema;
	};

} // namespace EditEngine
