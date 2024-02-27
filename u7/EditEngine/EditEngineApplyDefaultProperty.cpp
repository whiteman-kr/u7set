#include "EditEngineApplyDefaultProperty.h"
#include "EditEngineSetProperty.h"
#include "SchemaEditor/EditSchemaView.h"


namespace EditEngine
{

	ApplyDefaultPropertyCommand::ApplyDefaultPropertyCommand(EditSchemaView* schemaView,
															 ProjectDefaults projectDefaults,
															 std::vector<SchemaItemPtr> items,
															 QScrollBar* hScrollBar,
															 QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar),
		m_projectDefaults(std::move(projectDefaults)),
		m_items(std::move(items)),
		m_schema(schemaView->schemaSharedPtr())
	{
		assert(m_items.empty() == false);
		assert(m_schema != nullptr);

		return;
	}

	void ApplyDefaultPropertyCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		*itemsToSelect = std::vector<SchemaItemPtr>{m_items};
		m_records.clear();
		m_records.reserve(m_items.size() * 8);

		for (SchemaItemPtr& item : m_items)
		{
			const auto& defaultValues = m_projectDefaults.values(item->type());

			// Apply defaults in the order of defaultValues.
			//
			for (const auto& [propName, defaultValue] : defaultValues)
			{
				if (item->propertyExists(propName, false) == false)
				{
					// Property might not exist. It often happens for SchemaItemAfb's.
					//
					continue;
				}

				// Save old value.
				//
				m_records.emplace_back(item, propName, item->propertyValue(propName), defaultValue);

				SetPropertyCommand::setProperty(propName, defaultValue, item, m_schema, m_schemaView);
			}
		}

		return;
	}

	void ApplyDefaultPropertyCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		*itemsToSelect = std::vector<SchemaItemPtr>{m_items};

		for (const auto& r : m_records)
		{
			if (r.item->propertyExists(r.propertyName, false) == false)
			{
				// Property might not exist. It can happen if default properties are applied to the item.
				//
				continue;
			}

			SetPropertyCommand::setProperty(r.propertyName, r.oldValue, r.item, m_schema, m_schemaView);
		}

		return;
	}

} // namespace EditEngine
