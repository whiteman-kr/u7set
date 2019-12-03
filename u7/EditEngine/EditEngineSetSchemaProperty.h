#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetSchemaPropertyCommand : public EditCommand
	{
		SetSchemaPropertyCommand();
	public:
		SetSchemaPropertyCommand(EditSchemaView* schemaView,
				QString propertyName,
				QVariant value,
				std::shared_ptr<VFrame30::Schema> schema,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		QString m_propertyName;
		QVariant m_oldValue;
		QVariant m_newValue;

		std::shared_ptr<VFrame30::Schema> m_schema;
		std::vector<SchemaItemPtr> m_selectedItems;
	};

}

