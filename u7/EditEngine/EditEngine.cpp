#include "Stable.h"
#include "EditEngine.h"
#include "EditSchemaWidget.h"
#include "EditEngineAddItem.h"
#include "EditEngineBatch.h"
#include "EditEngineSetPoints.h"
#include "EditEngineDeleteItem.h"
#include "EditEngineMoveItem.h"
#include "EditEngineSetOrder.h"
#include "EditEngineSetProperty.h"
#include "EditEngineSetObject.h"
#include "EditEngineSetSchemaProperty.h"

namespace EditEngine
{

	EditEngine::EditEngine(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar, QObject* parent) :
		QObject(parent),
		m_schemaView(schemaView),
		m_hScrollBar(hScrollBar),
		m_vScrollBar(vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		m_commands.reserve(MaxCommandCount);

		return;
	}

	EditEngine::~EditEngine()
	{
	}

	void EditEngine::reset()
	{
		m_current = 0;
		m_commands.clear();

		resetModified();

		return;
	}

	bool EditEngine::addCommand(std::shared_ptr<EditCommand> command, bool runCommand)
	{
		assert(m_schemaView != nullptr);

		if (readOnly() == true)
		{
			QMessageBox mb(m_schemaView);
			mb.setText(tr("File was opened in readonly mode."));
			mb.exec();
			return false;
		}

		if (m_batch == true)
		{
			m_batchCommands.push_back(command);
		}
		else
		{
			// ќчистить commands до текущено уровн€ (current)
			//
			m_commands.erase(m_commands.begin() + m_current, m_commands.end());

			// ƒобавить новую команду в стек и выполить ее (runCommand)
			//
			m_commands.push_back(command);
			m_current++;

			if (runCommand == true)
			{
				execute(command);
				emit propertiesChanged();
			}

			if (m_commands.size() > MaxCommandCount)
			{
				m_commands.erase(m_commands.begin(), m_commands.begin() + (m_commands.size() - MaxCommandCount));
				m_current = static_cast<int>(m_commands.size());	// - 1
			}

			setModified();

			m_schemaView->update();

			emit stateChanged(canUndo(), canRedo());
		}

		return true;
	}

	bool EditEngine::startBatch()
	{
		assert(m_batch == false);
		assert(m_schemaView != nullptr);

		if (readOnly() == true)
		{
			QMessageBox mb(m_schemaView);
			mb.setText(tr("File was opened in readonly mode."));
			mb.exec();
			return false;
		}

		m_batchCommands.clear();
		m_batch = true;

		return true;
	}

	bool EditEngine::endBatch(bool runCommands /*= true*/)
	{
		if (m_batch == false)
		{
			assert(m_batch == true);

			m_batch = false;			// Try to mitigate issue
			m_batchCommands.clear();
			return false;
		}

		m_batch = false;

		if (m_batchCommands.empty() == true)
		{
			return false;
		}

		// Add batch command to m_commands
		//
		addCommand(std::make_shared<BatchCommand>(m_schemaView, m_batchCommands, m_hScrollBar, m_vScrollBar), runCommands);

		return true;
	}

	void EditEngine::redo()
	{
		assert(m_schemaView != nullptr);

		if (m_current < static_cast<int>(m_commands.size()))
		{
			execute(m_commands[m_current]);
			m_current++;
		}

		setModified();

		m_schemaView->update();

		emit stateChanged(canUndo(), canRedo());
		emit propertiesChanged();

		return;
	}

	void EditEngine::undo()
	{
		assert(m_schemaView != nullptr);

		if (m_current > 0)
		{
			m_current--;
			unExecute(m_commands[m_current]);
		}

		setModified();

		m_schemaView->update();

		emit stateChanged(canUndo(), canRedo());
		emit propertiesChanged();

		return;
	}

	bool EditEngine::canUndo() const
	{
		return m_current > 0;
	}

	bool EditEngine::canRedo() const
	{
		return m_current < static_cast<int>(m_commands.size());
	}

	bool EditEngine::readOnly() const
	{
		return m_readOnly;
	}

	void EditEngine::setReadOnly(bool value)
	{
		m_readOnly = value;
	}

	bool EditEngine::modified() const
	{
		return m_modified;
	}

	void EditEngine::setModified()
	{
		if (m_modified == false)
		{
			m_modified = true;
			emit modifiedChanged(m_modified);
		}
	}

	void EditEngine::resetModified()
	{
		if (m_modified == true)
		{
			m_modified = false;
			emit modifiedChanged(m_modified);
		}
	}

	void EditEngine::execute(std::shared_ptr<EditCommand> command)
	{
		assert(command);

		std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsToSelect;
		itemsToSelect.reserve(16);

		// Start
		//
		command->execute(&itemsToSelect);

		// Select changed items
		//
		m_schemaView->setSelectedItems(itemsToSelect);

		return;
	}

	void EditEngine::unExecute(std::shared_ptr<EditCommand> command)
	{
		assert(command);

		std::vector<std::shared_ptr<VFrame30::SchemaItem>> itemsToSelect;
		itemsToSelect.reserve(16);

		// Start
		//
		command->unExecute(&itemsToSelect);

		// Select changed items
		//
		m_schemaView->setSelectedItems(itemsToSelect);

		return;
	}

	void EditEngine::runAddItem(std::list<std::shared_ptr<VFrame30::SchemaItem>> items, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		addCommand(std::make_shared<AddItemCommand>(m_schemaView, items, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runAddItem(std::vector<std::shared_ptr<VFrame30::SchemaItem>> items, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		std::list<std::shared_ptr<VFrame30::SchemaItem>> l(items.begin(), items.end());
		addCommand(std::make_shared<AddItemCommand>(m_schemaView, l, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runAddItem(std::shared_ptr<VFrame30::SchemaItem> item, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		std::list<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.push_back(item);

		addCommand(std::make_shared<AddItemCommand>(m_schemaView, items, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runDeleteItem(const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		addCommand(std::make_shared<DeleteItemCommand>(m_schemaView, items, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runDeleteItem(std::shared_ptr<VFrame30::SchemaItem> item, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> v;
		v.push_back(item);

		return runDeleteItem(v, layer);
	}

	void EditEngine::runSetPoints(const std::vector<std::vector<VFrame30::SchemaPoint>>& points, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items, bool selectChangedItems)
	{
		addCommand(std::make_shared<SetPointsCommand>(m_schemaView, points, items, selectChangedItems, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runSetPoints(const std::vector<VFrame30::SchemaPoint>& points, const std::shared_ptr<VFrame30::SchemaItem>& item, bool selectChangedItems)
	{
		std::vector<VFrame30::SchemaPoint> ip(points.begin(), points.end());

		std::vector<std::vector<VFrame30::SchemaPoint>> allpoints;
		allpoints.push_back(ip);

		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.push_back(item);

		runSetPoints(allpoints, items, selectChangedItems);
		return;
	}

	void EditEngine::runMoveItem(double xdiff, double ydiff, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items, bool snapToGrid)
	{
		addCommand(std::make_shared<MoveItemCommand>(m_schemaView, xdiff, ydiff, items, snapToGrid, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runMoveItem(double xdiff, double ydiff, const std::shared_ptr<VFrame30::SchemaItem>& item, bool snapToGrid)
	{
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.push_back(item);

		runMoveItem(xdiff, ydiff, items, snapToGrid);
		return;
	}

	void EditEngine::runSetOrder(SetOrder setOrder, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		bool willThisChangeTheActualOrder = SetOrderCommand::checkIfCommandChangesOrder(setOrder, items, layer->Items);
		if (willThisChangeTheActualOrder == false)
		{
			return;
		}

		addCommand(std::make_shared<SetOrderCommand>(m_schemaView, setOrder, items, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runSetProperty(const QString& propertyName, QVariant value, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items)
	{
		addCommand(std::make_shared<SetPropertyCommand>(m_schemaView, propertyName, value, items, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runSetProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::SchemaItem>& item)
	{
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.push_back(item);

		return runSetProperty(propertyName, value, items);
	}

	void EditEngine::runSetObject(const QByteArray& currentState, const QByteArray& newState, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items)
	{
		addCommand(std::make_shared<SetObjectCommand>(m_schemaView, currentState, newState, items, m_hScrollBar, m_vScrollBar), true);
	}

	void EditEngine::runSetObject(const QByteArray& currentState, const QByteArray& newState, const std::shared_ptr<VFrame30::SchemaItem>& item)
	{
		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.push_back(item);

		return runSetObject(currentState, newState, items);
	}

	void EditEngine::runSetSchemaProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::Schema>& schema)
	{
		addCommand(std::make_shared<SetSchemaPropertyCommand>(m_schemaView, propertyName, value, schema, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	//
	//
	// EditCommand
	//
	//

	EditCommand::EditCommand(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar) :
		m_schemaView(schemaView),
		m_hScrollBar(hScrollBar),
		m_vScrollBar(vScrollBar),
		m_zoom(100.0)
	{
		assert(schemaView != nullptr);
		assert(schemaView->schema() != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		m_activeLayer = schemaView->activeLayer();

		saveViewPos();
	}

	void EditCommand::execute(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		assert(itemsToSelect);

		m_schemaView->setActiveLayer(m_activeLayer);
		restoreViewPos();

		executeCommand(itemsToSelect);

		return;
	}

	void EditCommand::unExecute(std::vector<std::shared_ptr<VFrame30::SchemaItem>>* itemsToSelect)
	{
		assert(itemsToSelect);

		m_schemaView->setActiveLayer(m_activeLayer);
		restoreViewPos();

		unExecuteCommand(itemsToSelect);

		return;
	}

	void EditCommand::saveViewPos()
	{
		assert(m_schemaView != nullptr);
		assert(m_hScrollBar != nullptr);
		assert(m_vScrollBar != nullptr);

		m_zoom = m_schemaView->zoom();

		m_hScrollBarCopy.setMaximum(m_hScrollBar->maximum());
		m_hScrollBarCopy.setValue(m_hScrollBar->value());

		m_vScrollBarCopy.setMaximum(m_vScrollBar->maximum());
		m_vScrollBarCopy.setValue(m_vScrollBar->value());

		return;
	}

	void EditCommand::restoreViewPos()
	{
		assert(m_schemaView != nullptr);
		assert(m_hScrollBar != nullptr);
		assert(m_vScrollBar != nullptr);

		m_schemaView->setZoom(m_zoom);			// ѕервым должен восстанавливатьс€ Zoom, т.к. он него завис€т скролы

		m_hScrollBar->setValue(m_hScrollBarCopy.value());
		m_vScrollBar->setValue(m_vScrollBarCopy.value());

		return;
	}

}
