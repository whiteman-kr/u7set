#include "EditEngineSetSchemaProperty.h"
#include "EditSchemaWidget.h"

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

		return;
	}

	void SetSchemaPropertyCommand::executeCommand(EditSchemaView* /*schemaView*/)
	{
		m_schema->setPropertyValue(m_propertyName, m_newValue);
		return;
	}

	void SetSchemaPropertyCommand::unExecuteCommand(EditSchemaView* /*schemaView*/)
	{
		m_schema->setPropertyValue(m_propertyName, m_oldValue);
		return;
	}

}
