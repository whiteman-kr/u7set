#include "Stable.h"
#include "EditEngine.h"
#include "EditSchemaWidget.h"
#include "EditEngineAddItem.h"
#include "EditEngineSetPoints.h"
#include "EditEngineDeleteItem.h"
#include "EditEngineMoveItem.h"
#include "EditEngineSetOrder.h"
#include "EditEngineSetProperty.h"
#include "EditEngineSetSchemaProperty.h"

namespace EditEngine
{

	EditEngine::EditEngine(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar, QObject* parent) :
		QObject(parent),
		m_schemaView(schemaView),
		m_hScrollBar(hScrollBar),
		m_vScrollBar(vScrollBar),
		m_current(0),
		m_readOnly(false),
		m_modified(false)
	{
		assert(schemaView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

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

		// ќчистить commands до текущено уровн€ (current)
		//
		m_commands.erase(m_commands.begin() + m_current, m_commands.end());

		// ƒобавить новую команду в стек и выполить ее (runCommand)
		//
		m_commands.push_back(command);
		m_current++;

		if (runCommand == true)
		{
			command->execute(m_schemaView, m_hScrollBar, m_vScrollBar);
		}

		if (m_commands.size() > MaxCommandCount)
		{
			m_commands.erase(m_commands.begin(), m_commands.begin() + (m_commands.size() - MaxCommandCount));
			m_current = static_cast<int>(m_commands.size());	// - 1
		}

		setModified();

		m_schemaView->update();

		emit stateChanged(canUndo(), canRedo());

		return true;
	}

	void EditEngine::redo(int levels)
	{
		assert(m_schemaView != nullptr);

		for (int i = 0; i < levels; i++)
		{
			if (m_current < static_cast<int>(m_commands.size()))// - 1)
			{
				m_commands[m_current]->execute(m_schemaView, m_hScrollBar, m_vScrollBar);
				m_current++;
			}
		}

		setModified();

		m_schemaView->update();

		emit stateChanged(canUndo(), canRedo());
		emit propertiesChanged();

		return;
	}

	void EditEngine::undo(int levels)
	{
		assert(m_schemaView != nullptr);

		for (int i = 0; i < levels; i++)
		{
			if (m_current > 0)
			{
				m_current--;
				m_commands[m_current]->unExecute(m_schemaView, m_hScrollBar, m_vScrollBar);
			}
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

	void EditEngine::runSetPoints(const std::vector<std::vector<VFrame30::SchemaPoint>>& points, const std::vector<std::shared_ptr<VFrame30::SchemaItem>>& items)
	{
		addCommand(std::make_shared<SetPointsCommand>(m_schemaView, points, items, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runSetPoints(const std::vector<VFrame30::SchemaPoint>& points, const std::shared_ptr<VFrame30::SchemaItem>& item)
	{
		std::vector<VFrame30::SchemaPoint> ip(points.begin(), points.end());

		std::vector<std::vector<VFrame30::SchemaPoint>> allpoints;
		allpoints.push_back(ip);

		std::vector<std::shared_ptr<VFrame30::SchemaItem>> items;
		items.push_back(item);

		runSetPoints(allpoints, items);
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
		m_zoom(100.0)
	{
		assert(schemaView != nullptr);
		assert(schemaView->schema() != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		m_activeLayer = schemaView->activeLayer();

		saveViewPos(schemaView, hScrollBar, vScrollBar);
	}

	void EditCommand::execute(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		schemaView->setActiveLayer(m_activeLayer);

		restoreViewPos(schemaView, hScrollBar, vScrollBar);

		executeCommand(schemaView);
	}

	void EditCommand::unExecute(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		schemaView->setActiveLayer(m_activeLayer);

		restoreViewPos(schemaView, hScrollBar, vScrollBar);

		unExecuteCommand(schemaView);
	}

	void EditCommand::saveViewPos(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		m_zoom = schemaView->zoom();

		m_hScrollBar.setMaximum(hScrollBar->maximum());
		m_hScrollBar.setValue(hScrollBar->value());

		m_vScrollBar.setMaximum(vScrollBar->maximum());
		m_vScrollBar.setValue(vScrollBar->value());

		return;
	}

	void EditCommand::restoreViewPos(EditSchemaView* schemaView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemaView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		schemaView->setZoom(m_zoom);			// ѕервым должен восстанавливатьс€ Zoom, т.к. он него завис€т скролы

		hScrollBar->setValue(m_hScrollBar.value());
		vScrollBar->setValue(m_vScrollBar.value());

		return;
	}

}
