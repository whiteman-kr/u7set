#include "EditEngineSetObject.h"
#include "SchemaEditor/EditSchemaView.h"
#include "../../VFrame30/SchemaItemAfb.h"
#include "../../VFrame30/SchemaItemBus.h"

namespace EditEngine
{

	SetObjectCommand::SetObjectCommand(EditSchemaView* schemaView,
									   const QByteArray& oldState,
									   const QByteArray& newState,
									   const std::vector<SchemaItemPtr>& items,
									   QScrollBar* hScrollBar,
									   QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar)
	{
		assert(oldState.isEmpty() == false);
		assert(newState.isEmpty() == false);
		assert(items.empty() == false);

		m_schema = schemaView->schema();
		assert(m_schema != nullptr);

		for (auto& item : items)
		{
			assert(item);

			Record r = {oldState, newState, item};
			m_items.push_back(r);
		}

		return;
	}

	void SetObjectCommand::executeCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		std::vector<SchemaItemPtr> selection;
		selection.reserve(m_items.size());

		for (const Record& r : m_items)
		{
			selection.push_back(r.item);

			bool ok = r.item->Load(r.newState);
			assert(ok);
			Q_UNUSED(ok);
		}

		*itemsToSelect = selection;
		return;
	}

	void SetObjectCommand::unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect)
	{
		std::vector<SchemaItemPtr> selection;
		selection.reserve(m_items.size());

		for (const Record& r : m_items)
		{
			selection.push_back(r.item);

			bool ok = r.item->Load(r.oldState);
			assert(ok);
			Q_UNUSED(ok);
		}

		*itemsToSelect = selection;
		return;
	}

}
