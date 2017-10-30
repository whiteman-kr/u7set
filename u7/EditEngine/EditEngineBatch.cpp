#include "EditEngineBatch.h"
#include "EditSchemaWidget.h"

namespace EditEngine
{
	BatchCommand::BatchCommand(EditSchemaView* schemaView, std::vector<std::shared_ptr<EditCommand>> batchCommands, QScrollBar* hScrollBar, QScrollBar* vScrollBar) :
		EditCommand(schemaView, hScrollBar, vScrollBar),
		m_batchCommands(batchCommands)
	{
		assert(schemaView != nullptr);
		return;
	}

	void BatchCommand::executeCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		for (auto it = m_batchCommands.begin(); it != m_batchCommands.end(); ++it)
		{
			std::vector<std::shared_ptr<VFrame30::SchemaItem>> commandSelection;

			std::shared_ptr<EditCommand> command = *it;
			command->executeCommand(&commandSelection);

			if (commandSelection.empty() == false)
			{
				itemsToSelect->insert(itemsToSelect->end(), commandSelection.begin(), commandSelection.end());
			}
		}

		return;
	}

	void BatchCommand::unExecuteCommand(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		for (auto it = m_batchCommands.rbegin(); it != m_batchCommands.rend(); ++it)
		{
			std::vector<std::shared_ptr<VFrame30::SchemaItem>> commandSelection;

			std::shared_ptr<EditCommand> command = *it;
			command->unExecuteCommand(&commandSelection);

			if (commandSelection.empty() == false)
			{
				itemsToSelect->insert(itemsToSelect->begin(), commandSelection.begin(), commandSelection.end());
			}
		}

		return;
	}
}
