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
						const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
						std::shared_ptr<VFrame30::SchemaLayer> layer,
						QScrollBar* hScrollBar,
						QScrollBar* vScrollBar);

	public:
		// Check if the setOrder command will really change items order in the layer
		// it is used to void the unnecessary operation
		//
		static bool checkIfCommandChangesOrder(
				SetOrder setOrder,
				const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items,
				const std::list<std::shared_ptr<VFrame30::SchemaItem>>& layerItems);

	protected:
		virtual void executeCommand(EditSchemaView* schemaView) override;
		virtual void unExecuteCommand(EditSchemaView* schemaView) override;

		// Data
		//
	private:
		SetOrder m_setOrder = SetOrder::BringForward;
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> m_items;
		std::shared_ptr<VFrame30::SchemaLayer> m_layer;

		std::list<std::shared_ptr<VFrame30::SchemaItem>> m_oldOrder;
	};

}


