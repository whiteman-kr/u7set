#include "EditEngineSetObject.h"
#include "EditSchemaWidget.h"
#include "../../VFrame30/SchemaItemAfb.h"
#include "../../VFrame30/SchemaItemBus.h"
#include <QMessageBox>

namespace EditEngine
{

	SetObjectCommand::SetObjectCommand(EditSchemaView* schemaView,
									   const QByteArray& oldState,
									   const QByteArray& newState,
									   const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
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

	void SetObjectCommand::executeCommand(EditSchemaView* schemaView)
	{
		assert(schemaView);

		std::list<std::shared_ptr<VFrame30::SchemaItem>> selection;

		for (const Record& r : m_items)
		{
			selection.push_back(r.item);

			bool ok = r.item->Load(r.newState);
			assert(ok);
		}

		schemaView->setSelectedItems(selection);
		return;
	}

	void SetObjectCommand::unExecuteCommand(EditSchemaView* schemaView)
	{
		assert(schemaView);

		std::list<std::shared_ptr<VFrame30::SchemaItem>> selection;

		for (const Record& r : m_items)
		{
			selection.push_back(r.item);

			bool ok = r.item->Load(r.oldState);
			assert(ok);
		}

		schemaView->setSelectedItems(selection);
		return;
	}

}
