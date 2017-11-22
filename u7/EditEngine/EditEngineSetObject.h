#ifndef EDITENGINESETOBJECT_H
#define EDITENGINESETOBJECT_H

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
				const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
				QScrollBar* hScrollBar,
				QScrollBar* vScrollBar);

	protected:
		virtual void executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect) override;

		//--
		//
		struct Record
		{
			QByteArray oldState;
			QByteArray newState;
			std::shared_ptr<VFrame30::SchemaItem> item;
		};

		// Data
		//
	private:
		std::vector<Record> m_items;
		std::shared_ptr<VFrame30::Schema> m_schema;
	};

}


#endif // EDITENGINESETOBJECT_H
