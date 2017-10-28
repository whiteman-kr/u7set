#include "EditEngineSetProperty.h"
#include "EditSchemaWidget.h"
#include "../../VFrame30/SchemaItemAfb.h"
#include "../../VFrame30/SchemaItemBus.h"
#include <QMessageBox>

namespace EditEngine
{

	SetPropertyCommand::SetPropertyCommand(EditSchemaView* schemaView,
			QString propertyName,
			QVariant value,
			const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(items.empty() == false);
		assert(value.isValid() == true);

		m_schema = schemaView->schema();
		assert(m_schema != nullptr);

		for (auto& i : items)
		{
			assert(i);

			Record r;

			r.propertyName = propertyName;
			r.oldValue = i->propertyValue(propertyName);
			r.newValue = value;
			r.item = i;

			assert(r.oldValue.isValid() == true);

			m_items.push_back(r);
		}

		return;
	}

	void SetPropertyCommand::executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> selection;
		selection.reserve(m_items.size());

		for (Record& r : m_items)
		{
			QVariant newValue = r.newValue;

			selection.push_back(r.item);
			r.item->setPropertyValue(r.propertyName, newValue);

			auto property = r.item->propertyByCaption(r.propertyName);
			assert(property);

			if (property.get() != nullptr && property->specific() == true)
			{
				// load the new value again from property, it could be corrected by checkLimits
				//
				newValue = r.item->propertyValue(r.propertyName);

				if (r.item->isSchemaItemAfb() == true)
				{
					VFrame30::SchemaItemAfb* fblElement = dynamic_cast<VFrame30::SchemaItemAfb*>(r.item.get());
					assert(fblElement != nullptr);

					QString errorMsg;
					bool ok = fblElement->setAfbParam(r.propertyName, newValue, m_schema, &errorMsg);

					if (ok == false)
					{
						QMessageBox::critical(m_schemaView, QObject::tr("Error"), errorMsg);
					}

					// setAfbParam executes script that can correct the value. If it was corrected, update the value
					//

					QVariant v = fblElement->getAfbParam(r.propertyName);
					if (v != newValue)
					{
						r.item->setPropertyValue(r.propertyName, v);
					}

					continue;
				}

				if (r.item->isType<VFrame30::SchemaItemBusExtractor>() == true)
				{
					VFrame30::SchemaItemBusExtractor* busExtractor = dynamic_cast<VFrame30::SchemaItemBusExtractor*>(r.item.get());
					assert(busExtractor != nullptr);

					// It will update output pins
					//
					busExtractor->specificPropertyCouldBeChanged(r.propertyName, newValue);

					continue;
				}

				assert(false);		// Specific proprties have only SchemaItemAfb and SchemaItemBusExtractor
			}
		}

		*itemsToSelect = selection;
		return;
	}

	void SetPropertyCommand::unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> sel;
		sel.reserve(m_items.size());

		for (Record& r : m_items)
		{
			sel.push_back(r.item);
			r.item->setPropertyValue(r.propertyName, r.oldValue);

			auto property = r.item->propertyByCaption(r.propertyName);
			assert(property);

			if (property.get() != nullptr && property->specific() == true)
			{
				if (r.item->isSchemaItemAfb() == true)
				{
					// Apparently it is FblParam, only VFrame30::SchemaItemAfb can have such kind of props
					//
					VFrame30::SchemaItemAfb* fblElement = dynamic_cast<VFrame30::SchemaItemAfb*>(r.item.get());
					assert(fblElement != nullptr);

					QString errorMsg;
					bool ok = fblElement->setAfbParam(r.propertyName, r.oldValue, m_schema, &errorMsg);

					if (ok == false)
					{
						QMessageBox::critical(m_schemaView, QObject::tr("Error"), errorMsg);
					}

					continue;
				}

				if (r.item->isType<VFrame30::SchemaItemBusExtractor>() == true)
				{
					VFrame30::SchemaItemBusExtractor* busExtractor = dynamic_cast<VFrame30::SchemaItemBusExtractor*>(r.item.get());
					assert(busExtractor != nullptr);

					// It will update output pins
					//
					busExtractor->specificPropertyCouldBeChanged(r.propertyName, r.oldValue);

					continue;
				}

				assert(false);		// Specific proprties have only SchemaItemAfb and SchemaItemBusExtractor
			}
		}

		*itemsToSelect = sel;
		return;
	}

}
