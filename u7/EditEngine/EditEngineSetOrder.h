#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class SetOrderCommand  : public EditCommand
	{
		SetOrderCommand();		// deleted;
	public:
		SetOrderCommand(EditSchemaView* schemaView,
						SetOrder setOrder,
						const std::vector<SchemaItemPtr>& items,
						std::shared_ptr<VFrame30::SchemaLayer> layer,
						QScrollBar* hScrollBar,
						QScrollBar* vScrollBar);

	public:
		// Check if the setOrder command will really change items order in the layer
		// it is used to void the unnecessary operation
		//
		static bool checkIfCommandChangesOrder(SetOrder setOrder,
				const std::vector<SchemaItemPtr>& items,
				const std::list<SchemaItemPtr>& layerItems);

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		SetOrder m_setOrder = SetOrder::BringForward;
		std::vector<SchemaItemPtr> m_items;
		std::shared_ptr<VFrame30::SchemaLayer> m_layer;

		std::list<SchemaItemPtr> m_oldOrder;
	};

}


