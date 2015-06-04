#include "EditEngineSetProperty.h"
#include "VideoFrameWidget.h"
#include "EditSchemeWidget.h"
#include "../../VFrame30/VideoItemFblElement.h"

namespace EditEngine
{

	SetPropertyCommand::SetPropertyCommand(
			EditSchemeView* videoFrameView,
			QString propertyName,
			QVariant value,
			const std::vector<std::shared_ptr<VFrame30::VideoItem>>& items,
			QScrollBar* hScrollBar,
			QScrollBar* vScrollBar) :
		EditCommand(videoFrameView, hScrollBar, vScrollBar)
	{
		assert(propertyName.isEmpty() == false);
		assert(items.empty() == false);
		assert(value.isValid() == true);

		m_scheme = videoFrameView->scheme();
		assert(m_scheme != nullptr);

		for (auto& i : items)
		{
			assert(i);

			Record r;

			r.propertyName = propertyName;
			r.oldValue = i->property(propertyName.toStdString().c_str());
			r.newValue = value;
			r.item = i;

			assert(r.oldValue.isValid() == true);

			m_items.push_back(r);
		}

		return;
	}

	void SetPropertyCommand::executeCommand(EditSchemeView* videoFrameView)
	{
		std::list<std::shared_ptr<VFrame30::VideoItem>> sel;

		for (Record& r : m_items)
		{
			QList<QByteArray> dynamicProperties = r.item->dynamicPropertyNames();

			bool isDynamic = false;
			for (QByteArray& ba : dynamicProperties)
			{
				QString strName(ba);

				if (r.propertyName == strName)
				{
					isDynamic = true;
					break;
				}
			}

			if (isDynamic == true)
			{
				// Apparently it is FblParam, only VFrame30::VideoItemFblElement can have such kind of props
				//
				VFrame30::VideoItemFblElement* fblElement = dynamic_cast<VFrame30::VideoItemFblElement*>(r.item.get());

				if (fblElement == nullptr)
				{
					assert(fblElement != nullptr);
					continue;
				}

				fblElement->setAfbParam(r.propertyName, r.newValue, m_scheme);
			}

			sel.push_back(r.item);
			r.item->setProperty(r.propertyName.toStdString().c_str(), r.newValue);
		}

		videoFrameView->setSelectedItems(sel);
		return;
	}

	void SetPropertyCommand::unExecuteCommand(EditSchemeView* videoFrameView)
	{
		std::list<std::shared_ptr<VFrame30::VideoItem>> sel;

		for (Record& r : m_items)
		{
			QList<QByteArray> dynamicProperties = r.item->dynamicPropertyNames();

			bool isDynamic = false;
			for (QByteArray& ba : dynamicProperties)
			{
				QString strName(ba);

				if (r.propertyName == strName)
				{
					isDynamic = true;
					break;
				}
			}

			if (isDynamic == true)
			{
				// Apparently it is FblParam, only VFrame30::VideoItemFblElement can have such kind of props
				//
				VFrame30::VideoItemFblElement* fblElement = dynamic_cast<VFrame30::VideoItemFblElement*>(r.item.get());

				if (fblElement == nullptr)
				{
					assert(fblElement != nullptr);
					continue;
				}

				fblElement->setAfbParam(r.propertyName, r.newValue, m_scheme);

			}

			// --
			//
			sel.push_back(r.item);
			r.item->setProperty(r.propertyName.toStdString().c_str(), r.oldValue);
		}

		videoFrameView->setSelectedItems(sel);
		return;
	}

}
