#include "EditEngineSetProperty.h"
#include "EditSchemaWidget.h"
#include "../../VFrame30/SchemaItemAfb.h"

namespace EditEngine
{

	SetPropertyCommand::SetPropertyCommand(EditSchemaView* schemeView,
			QString propertyName,
			QVariant value,
			const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemeView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(items.empty() == false);
		assert(value.isValid() == true);

		m_scheme = schemeView->schema();
		assert(m_scheme != nullptr);

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

	void SetPropertyCommand::executeCommand(EditSchemaView* schemeView)
	{
		std::list<std::shared_ptr<VFrame30::SchemaItem>> selection;

		for (Record& r : m_items)
		{
			selection.push_back(r.item);
			r.item->setPropertyValue(r.propertyName, r.newValue);

			auto property = r.item->propertyByCaption(r.propertyName);
			assert(property);

			if (property.get() != nullptr && property->specific() == true)
			{
				// Apparently it is FblParam, only VFrame30::SchemeItemAfb can have such kind of props
				//
				VFrame30::SchemaItemAfb* fblElement = dynamic_cast<VFrame30::SchemaItemAfb*>(r.item.get());

				if (fblElement == nullptr)
				{
					assert(fblElement != nullptr);
					continue;
				}

				fblElement->setAfbParam(r.propertyName, r.newValue, m_scheme);
			}
		}

		schemeView->setSelectedItems(selection);
		return;
	}

	void SetPropertyCommand::unExecuteCommand(EditSchemaView* schemeView)
	{
		std::list<std::shared_ptr<VFrame30::SchemaItem>> sel;

		for (Record& r : m_items)
		{
			sel.push_back(r.item);
			r.item->setPropertyValue(r.propertyName, r.oldValue);


			auto property = r.item->propertyByCaption(r.propertyName);
			assert(property);

			if (property.get() != nullptr && property->specific() == true)
			{
				// Apparently it is FblParam, only VFrame30::SchemeItemAfb can have such kind of props
				//
				VFrame30::SchemaItemAfb* fblElement = dynamic_cast<VFrame30::SchemaItemAfb*>(r.item.get());

				if (fblElement == nullptr)
				{
					assert(fblElement != nullptr);
					continue;
				}

				fblElement->setAfbParam(r.propertyName, r.oldValue, m_scheme);
			}
		}

		schemeView->setSelectedItems(sel);
		return;
	}

}
