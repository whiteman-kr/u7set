#include "EditEngineSetProperty.h"
#include "VideoFrameWidget.h"
#include "EditSchemeWidget.h"

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

		std::for_each(m_items.begin(), m_items.end(),
			[this, &sel](Record& r)
			{
				sel.push_back(r.item);
				r.item->setProperty(r.propertyName.toStdString().c_str(), r.newValue);
			}
			);

		videoFrameView->setSelectedItems(sel);
		return;
	}

	void SetPropertyCommand::unExecuteCommand(EditSchemeView* videoFrameView)
	{
		std::list<std::shared_ptr<VFrame30::VideoItem>> sel;

		std::for_each(m_items.begin(), m_items.end(),
			[this, &sel](Record& r)
			{
				sel.push_back(r.item);
				r.item->setProperty(r.propertyName.toStdString().c_str(), r.oldValue);
			}
			);

		videoFrameView->setSelectedItems(sel);
		return;
	}

}
