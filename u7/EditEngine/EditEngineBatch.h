#pragma once

#include "EditEngine.h"

namespace EditEngine
{

	class BatchCommand : public EditCommand
	{
		BatchCommand();		// deleted;
	public:
		BatchCommand(EditSchemaView* schemaView, std::vector<std::shared_ptr<EditCommand>> batchCommands, QScrollBar* hScrollBar, QScrollBar* vScrollBar);

		bool addCommand();

	protected:
		virtual void executeCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;
		virtual void unExecuteCommand(std::vector<SchemaItemPtr>* itemsToSelect) override;

		// Data
		//
	private:
		std::vector<std::shared_ptr<EditCommand>> m_batchCommands;
	};

}

