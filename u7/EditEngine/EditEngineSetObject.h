#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetObjectCommand : public EditCommand
	{
		SetObjectCommand();
	public:
		SetObjectCommand(EditSchemaView* schemaView,
				const QByteArray& oldState,
				const QByteArray& newState,
				const std::vector<SchemaItemPtr>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		//--
		//
		struct Record
		{
			QByteArray oldState;
			QByteArray newState;
			SchemaItemPtr item;
		};

		// Data
		//
	private:
		std::vector<Record> m_items;
		std::shared_ptr<VFrame30::Schema> m_schema;
	};

}

