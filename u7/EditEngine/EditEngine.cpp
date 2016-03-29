#include "Stable.h"
#include "EditEngine.h"
#include "EditSchemeWidget.h"
#include "EditEngineAddItem.h"
#include "EditEngineSetPoints.h"
#include "EditEngineDeleteItem.h"
#include "EditEngineMoveItem.h"
#include "EditEngineSetProperty.h"
#include "EditEngineSetSchemeProperty.h"

namespace EditEngine
{

	EditEngine::EditEngine(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar, QObject* parent) :
		QObject(parent),
		m_schemeView(schemeView),
		m_hScrollBar(hScrollBar),
		m_vScrollBar(vScrollBar),
		m_current(0),
		m_readOnly(false),
		m_modified(false)
	{
		assert(schemeView != nullptr);
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
		assert(m_schemeView != nullptr);

		if (readOnly() == true)
		{
			QMessageBox mb(m_schemeView);
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
			command->execute(m_schemeView, m_hScrollBar, m_vScrollBar);
		}

		if (m_commands.size() > MaxCommandCount)
		{
			m_commands.erase(m_commands.begin(), m_commands.begin() + (m_commands.size() - MaxCommandCount));
			m_current = static_cast<int>(m_commands.size());	// - 1
		}

		setModified();

		m_schemeView->update();

		emit stateChanged(canUndo(), canRedo());

		return true;
	}

	void EditEngine::redo(int levels)
	{
		assert(m_schemeView != nullptr);

		for (int i = 0; i < levels; i++)
		{
			if (m_current < static_cast<int>(m_commands.size()))// - 1)
			{
				m_commands[m_current]->execute(m_schemeView, m_hScrollBar, m_vScrollBar);
				m_current++;
			}
		}

		setModified();

		m_schemeView->update();

		emit stateChanged(canUndo(), canRedo());
		emit propertiesChanged();

		return;
	}

	void EditEngine::undo(int levels)
	{
		assert(m_schemeView != nullptr);

		for (int i = 0; i < levels; i++)
		{
			if (m_current > 0)
			{
				m_current--;
				m_commands[m_current]->unExecute(m_schemeView, m_hScrollBar, m_vScrollBar);
			}
		}

		setModified();

		m_schemeView->update();

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

	void EditEngine::runAddItem(std::list<std::shared_ptr<VFrame30::SchemeItem>> items, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		addCommand(std::make_shared<AddItemCommand>(m_schemeView, items, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runAddItem(std::vector<std::shared_ptr<VFrame30::SchemeItem>> items, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		std::list<std::shared_ptr<VFrame30::SchemeItem>> l(items.begin(), items.end());
		addCommand(std::make_shared<AddItemCommand>(m_schemeView, l, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runAddItem(std::shared_ptr<VFrame30::SchemeItem> item, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		std::list<std::shared_ptr<VFrame30::SchemeItem>> items;
		items.push_back(item);

		addCommand(std::make_shared<AddItemCommand>(m_schemeView, items, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runDeleteItem(const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		addCommand(std::make_shared<DeleteItemCommand>(m_schemeView, items, layer, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runDeleteItem(std::shared_ptr<VFrame30::SchemeItem> item, std::shared_ptr<VFrame30::SchemaLayer> layer)
	{
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> v;
		v.push_back(item);

		return runDeleteItem(v, layer);
	}

	void EditEngine::runSetPoints(const std::vector<std::vector<VFrame30::SchemePoint>>& points, const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items)
	{
		addCommand(std::make_shared<SetPointsCommand>(m_schemeView, points, items, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runSetPoints(const std::vector<VFrame30::SchemePoint>& points, const std::shared_ptr<VFrame30::SchemeItem>& item)
	{
		std::vector<VFrame30::SchemePoint> ip(points.begin(), points.end());

		std::vector<std::vector<VFrame30::SchemePoint>> allpoints;
		allpoints.push_back(ip);

		std::vector<std::shared_ptr<VFrame30::SchemeItem>> items;
		items.push_back(item);

		runSetPoints(allpoints, items);
		return;
	}

	void EditEngine::runMoveItem(double xdiff, double ydiff, const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items, bool snapToGrid)
	{
		addCommand(std::make_shared<MoveItemCommand>(m_schemeView, xdiff, ydiff, items, snapToGrid, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runMoveItem(double xdiff, double ydiff, const std::shared_ptr<VFrame30::SchemeItem>& item, bool snapToGrid)
	{
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> items;
		items.push_back(item);

		runMoveItem(xdiff, ydiff, items, snapToGrid);
		return;
	}

	void EditEngine::runSetProperty(const QString& propertyName, QVariant value, const std::vector<std::shared_ptr<VFrame30::SchemeItem>>& items)
	{
		addCommand(std::make_shared<SetPropertyCommand>(m_schemeView, propertyName, value, items, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	void EditEngine::runSetProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::SchemeItem>& item)
	{
		std::vector<std::shared_ptr<VFrame30::SchemeItem>> items;
		items.push_back(item);

		return runSetProperty(propertyName, value, items);
	}

	void EditEngine::runSetSchemeProperty(const QString& propertyName, QVariant value, const std::shared_ptr<VFrame30::Schema>& scheme)
	{
		addCommand(std::make_shared<SetSchemePropertyCommand>(m_schemeView, propertyName, value, scheme, m_hScrollBar, m_vScrollBar), true);
		return;
	}

	//
	//
	// EditCommand
	//
	//

	EditCommand::EditCommand(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar) :
		m_zoom(100.0)
	{
		assert(schemeView != nullptr);
		assert(schemeView->scheme() != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		m_activeLayer = schemeView->activeLayer();

		saveViewPos(schemeView, hScrollBar, vScrollBar);
	}

	void EditCommand::execute(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemeView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		schemeView->setActiveLayer(m_activeLayer);

		restoreViewPos(schemeView, hScrollBar, vScrollBar);

		executeCommand(schemeView);
	}

	void EditCommand::unExecute(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemeView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		schemeView->setActiveLayer(m_activeLayer);

		restoreViewPos(schemeView, hScrollBar, vScrollBar);

		unExecuteCommand(schemeView);
	}

	void EditCommand::saveViewPos(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemeView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		m_zoom = schemeView->zoom();

		m_hScrollBar.setMaximum(hScrollBar->maximum());
		m_hScrollBar.setValue(hScrollBar->value());

		m_vScrollBar.setMaximum(vScrollBar->maximum());
		m_vScrollBar.setValue(vScrollBar->value());

		return;
	}

	void EditCommand::restoreViewPos(EditSchemeView* schemeView, QScrollBar* hScrollBar, QScrollBar* vScrollBar)
	{
		assert(schemeView != nullptr);
		assert(hScrollBar != nullptr);
		assert(vScrollBar != nullptr);

		schemeView->setZoom(m_zoom);			// ѕервым должен восстанавливатьс€ Zoom, т.к. он него завис€т скролы

		hScrollBar->setValue(m_hScrollBar.value());
		vScrollBar->setValue(m_vScrollBar.value());

		return;
	}

}
