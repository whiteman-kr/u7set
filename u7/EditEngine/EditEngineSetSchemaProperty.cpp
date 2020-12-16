#include "EditEngineSetSchemaProperty.h"
#include "SchemaEditor/EditSchemaView.h"

namespace EditEngine
{

	SetSchemaPropertyCommand::SetSchemaPropertyCommand(EditSchemaView* schemaView,
			QString propertyName,
			QVariant value,
			std::shared_ptr<VFrame30::Schema> schema,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(value.isValid() == true);
		assert(schema.get());

		m_schema = schema;

		m_propertyName = propertyName;
		m_oldValue = m_schema->propertyValue(m_propertyName);
		m_newValue = value;

		assert(m_oldValue.isValid() == true);

		m_selectedItems = m_schemaView->selectedItems();

		return;
	}

	void SetSchemaPropertyCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		m_schema->setPropertyValue(m_propertyName, m_newValue);

		itemsToSelect->assign(m_selectedItems.begin(), m_selectedItems.end());
		return;
	}

	void SetSchemaPropertyCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		m_schema->setPropertyValue(m_propertyName, m_oldValue);

		itemsToSelect->assign(m_selectedItems.begin(), m_selectedItems.end());
		return;
	}

}
