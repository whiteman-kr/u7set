#include "EditEngineSetProperty.h"
#include "../../VFrame30/SchemaItemAfb.h"
#include "../../VFrame30/SchemaItemBus.h"
#include "../../VFrame30/SchemaItemUfb.h"
#include "SchemaEditor/EditSchemaView.h"

namespace EditEngine
{

	SetPropertyCommand::SetPropertyCommand(EditSchemaView* schemaView,
										   QString propertyName,
										   QVariant value,
										   const std::vector<SchemaItemPtr>& items,
										   QScrollBar* hScrollBar,
										   QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(items.empty() == false);
		assert(value.isValid() == true);

		m_schema = schemaView->schemaSharedPtr();
		assert(m_schema != nullptr);

		for (auto& i : items)
		{
			assert(i);

			Record r;

			r.propertyName = propertyName;
			r.newValue = value;
			r.item = i;

			m_items.push_back(r);
		}

		return;
	}

	void SetPropertyCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		std::vector<SchemaItemPtr> selection;
		selection.reserve(m_items.size());

		for (Record& r : m_items)
		{
			if (r.item->propertyExists(r.propertyName, false) == false)
			{
				// Property might not exist. It can happen if default properties are applied to the item.
				//
				continue;
			}

			// Save the old value now, as in the constructor of SetPropertyCommand it might not ne existing (example
			// setColumnCount for SchemaItemSignal must be called in a batch run).
			//
			r.oldValue = r.item->propertyValue(r.propertyName);
			assert(r.oldValue.isValid() == true);
		}

		for (Record& r : m_items)
		{
			if (r.item->propertyExists(r.propertyName, false) == false)
			{
				// Property might not exist. It can happen if default properties are applied to the item.
				//
				continue;
			}

			selection.push_back(r.item);

			SetPropertyCommand::setProperty(r.propertyName, r.newValue, r.item, m_schema, m_schemaView);
		}

		*itemsToSelect = selection;
		return;
	}

	void SetPropertyCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		std::vector<SchemaItemPtr> sel;
		sel.reserve(m_items.size());

		for (Record& r : m_items)
		{
			if (r.item->propertyExists(r.propertyName, false) == false)
			{
				// Property might not exist. It can happen if default properties are applied to the item.
				//
				continue;
			}

			sel.push_back(r.item);

			SetPropertyCommand::setProperty(r.propertyName, r.oldValue, r.item, m_schema, m_schemaView);
		}

		*itemsToSelect = sel;
		return;
	}

	void SetPropertyCommand::setProperty(const QString& propertyName,
										 const QVariant& value,
										 const SchemaItemPtr& item,
										 std::shared_ptr<VFrame30::Schema> schema,
										 EditSchemaView* schemaView)
	{
		assert(item);

		item->setPropertyValue(propertyName, value);

		auto property = item->propertyByCaption(propertyName);
		assert(property);

		if (property == nullptr || property->specific() == false)
		{
			return;
		}

		// load the new value again from property, it could be corrected by checkLimits
		//
		QVariant newValue = item->propertyValue(propertyName);

		if (item->isSchemaItemAfb() == true)
		{
			VFrame30::SchemaItemAfb* fblElement = dynamic_cast<VFrame30::SchemaItemAfb*>(item.get());
			assert(fblElement != nullptr);

			QString errorMsg;
			bool ok = fblElement->setAfbParam(propertyName, newValue, schema, &errorMsg);

			if (ok == false)
			{
				QMessageBox::critical(schemaView, QObject::tr("Error"), errorMsg);
			}

			// setAfbParam executes script that can correct the value. If it was corrected, update the value
			//
			if (QVariant v = fblElement->getAfbParam(propertyName); v != newValue)
			{
				item->setPropertyValue(propertyName, v);
			}

			return;
		}

		if (VFrame30::SchemaItemUfb* ufbItem = dynamic_cast<VFrame30::SchemaItemUfb*>(item.get()); ufbItem != nullptr)
		{
			// Actually no action here
			//
			return;
		}

		if (VFrame30::SchemaItemBusExtractor* busExtractor =
				dynamic_cast<VFrame30::SchemaItemBusExtractor*>(item.get());
			busExtractor != nullptr)
		{
			// It will update output pins
			//
			busExtractor->specificPropertyCouldBeChanged(propertyName, newValue);

			return;
		}

		assert(false); // Specific proprties have only SchemaItemAfb, SchemaItemUfb, SchemaItemBusExtractor
		return;
	}

} // namespace EditEngine
