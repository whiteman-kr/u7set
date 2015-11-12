#include "EditEngineSetProperty.h"
#include "EditSchemeWidget.h"
#include "../../VFrame30/SchemeItemAfb.h"

namespace EditEngine
{

	SetPropertyCommand::SetPropertyCommand(EditSchemeView* schemeView,
			QString propertyName,
			QVariant value,
			const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(schemeView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(items.empty() == false);
		assert(value.isValid() == true);

		m_scheme = schemeView->scheme();
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

	void SetPropertyCommand::executeCommand(EditSchemeView* schemeView)
	{
		std::list<std::shared_ptr<VFrame30::SchemeItem>> selection;

		for (Record& r : m_items)
		{
			selection.push_back(r.item);
			r.item->setPropertyValue(r.propertyName, r.newValue);

			auto property = r.item->propertyByCaption(r.propertyName);
			assert(property);

			if (property.get() != nullptr && property->dynamic() == true)
			{
				// Apparently it is FblParam, only VFrame30::SchemeItemAfb can have such kind of props
				//
				VFrame30::SchemeItemAfb* fblElement = dynamic_cast<VFrame30::SchemeItemAfb*>(r.item.get());

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

	void SetPropertyCommand::unExecuteCommand(EditSchemeView* schemeView)
	{
		std::list<std::shared_ptr<VFrame30::SchemeItem>> sel;

		for (Record& r : m_items)
		{
			sel.push_back(r.item);
			r.item->setPropertyValue(r.propertyName, r.oldValue);


			auto property = r.item->propertyByCaption(r.propertyName);
			assert(property);

			if (property.get() != nullptr && property->dynamic() == true)
			{
				// Apparently it is FblParam, only VFrame30::SchemeItemAfb can have such kind of props
				//
				VFrame30::SchemeItemAfb* fblElement = dynamic_cast<VFrame30::SchemeItemAfb*>(r.item.get());

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
