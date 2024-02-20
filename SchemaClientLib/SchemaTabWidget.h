#pragma once

#include "../VFrame30/ClientSchemaWidget.h"
#include "../VFrame30/Context.h"
#include "../VFrame30/MonitorSchema.h"
#include "../lib/Ui/TabWidgetEx.h"
#include "ClientSchemaManager.h"
#include <QFileDialog>


namespace SchemaClientLib
{
	//
	// Signal/slot implementation for SchemaTabWidget template class.
	//
	class SchemaTabWidgetSignalSlot : public TabWidgetEx
	{
		Q_OBJECT

	public:
		explicit SchemaTabWidgetSignalSlot(QWidget* parent);

		// Slots
		//
	public slots:
		virtual void slot_newSchemaTab(QString schemaId);
		virtual void slot_newTab();
		virtual void slot_closeCurrentTab();

		virtual void slot_zoomIn();
		virtual void slot_zoomOut();
		virtual void slot_zoom100();
		virtual void slot_zoomToFit();

		virtual void slot_historyBack();
		virtual void slot_historyForward();

		virtual void slot_selectSchemaForCurrentTab(QString schemaId);

		virtual void slot_signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu);
		virtual void slot_signalInfo(QString signalId);

		virtual void slot_export();

		//--
		//
		virtual void slot_tabCloseRequested(int index);
		virtual void slot_resetSchema();

		virtual void slot_newSameTab(VFrame30::ClientSchemaWidget* tabWidget);
		virtual void slot_closeTab(QWidget* tabWidget);

		virtual void slot_schemaChanged(VFrame30::ClientSchemaWidget* tabWidget, VFrame30::Schema* schema);

		virtual void slot_tabPageChanged(int index);

		// Signals
		//
	signals:
		void signal_tabPageChanged(bool schemaWidgetSelected); // Emitted to enable/disable QActions depend on current tab (schema/schemaList).
		void signal_actionCloseTabUpdated(bool allowed);
		void signal_schemaChanged(QString strId);
		void signal_historyChanged(bool enableBack, bool enableForward);
	};


	// ClientSchemaWidgetConcept is a type that is derived from VFrame30::ClientSchemaWidget
	// or is VFrame30::ClientSchemaWidget.
	//
	template<typename T>
	concept ClientSchemaWidgetConcept = std::is_base_of<VFrame30::ClientSchemaWidget, T>::value ||
										std::is_same<VFrame30::ClientSchemaWidget, T>::value;


	//
	// SchemaTabWidget is a tab container for ClientSchemaWidgetType (VFrame30::ClientSchemaWidget).
	// Usually used as a central widget for main window.
	//
	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	class SchemaTabWidget : public SchemaTabWidgetSignalSlot
	{
	public:
		using CreateSchemaWidgetFunc = std::function<ClientSchemaWidgetType*(std::shared_ptr<VFrame30::Schema>, QWidget*)>;

	public:
		SchemaTabWidget(ClientSchemaManager* schemaManager,
						CreateSchemaWidgetFunc createSchemaWidgetFunc,
						QWidget* parent);

	public:
		void setVisibleTabBar(bool visible);

		ClientSchemaWidgetType* currentTab()
		{
			return dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
		}

	protected:
		virtual void timerEvent(QTimerEvent* event) override;

	protected:
		int addSchemaTabPage(const QString& schemaId, const QVariantHash& variables);

		void applyZoomMode(VFrame30::ZoomMode zoomMode);

		// Slots
		//
	public:
		void slot_newSchemaTab(QString schemaId) override;
		void slot_newTab() override;
		void slot_closeCurrentTab() override;

		void slot_zoomIn() override;
		void slot_zoomOut() override;
		void slot_zoom100() override;
		void slot_zoomToFit() override;

		void slot_historyBack() override;
		void slot_historyForward() override;

		void slot_selectSchemaForCurrentTab(QString schemaId) override;

		void slot_signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu) override;
		void slot_signalInfo(QString signalId) override;

		void slot_export() override;

	protected:
		void slot_tabCloseRequested(int index) override;
		void slot_resetSchema() override;

		void slot_newSameTab(VFrame30::ClientSchemaWidget* tabWidget) override;
		void slot_closeTab(QWidget* tabWidget) override;

		void slot_schemaChanged(VFrame30::ClientSchemaWidget* tabWidget, VFrame30::Schema* schema) override;

		void slot_tabPageChanged(int index) override;

		// Properties
		//
	public:
		[[nodiscard]] const QString& startSchemaId() const { return m_startSchemaId; }
		void setStartSchemaId(const QString& schemaId) { m_startSchemaId = schemaId; }

		[[nodiscard]] VFrame30::ZoomMode zoomMode() const { return m_zoomMode; }
		void setZoomMode(VFrame30::ZoomMode zoomMode)
		{
			if (m_zoomMode != zoomMode)
			{
				applyZoomMode(zoomMode);
				m_zoomMode = zoomMode;
			}

			return;
		}

		// Data
		//
	private:
		ClientSchemaManager* m_schemaManager = nullptr;
		CreateSchemaWidgetFunc m_createSchemaWidgetFunc;

		QString m_startSchemaId;
		VFrame30::ZoomMode m_zoomMode = VFrame30::ZoomMode::Manual;

		int m_eventLoopTimerId = 0;      // We need to catch event loop. Start timer, as we enter event loop timerEvent comes.
		int m_eventLoopTimerCounter = 0; // We need to catch event loop. Start timer, as we enter event loop timerEvent comes.
	};


	//
	// SchemaTabWidget Implementation
	//
	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	SchemaTabWidget<ClientSchemaWidgetType>::SchemaTabWidget(ClientSchemaManager* schemaManager,
															 CreateSchemaWidgetFunc createSchemaWidgetFunc,
															 QWidget* parent) :
		SchemaTabWidgetSignalSlot(parent),
		m_schemaManager(schemaManager),
		m_createSchemaWidgetFunc(std::move(createSchemaWidgetFunc))
	{
		Q_ASSERT(m_schemaManager);

		// --
		//
		tabBar()->setExpanding(true);

		// At first we see just one tab, so it is not closable
		//
		setTabsClosable(false);
		setMovable(false);

		// --
		//
		connect(tabBar(), &QTabBar::tabCloseRequested, this, &SchemaTabWidgetSignalSlot::slot_tabCloseRequested);
		connect(this, &SchemaTabWidgetSignalSlot::currentChanged, this, &SchemaTabWidgetSignalSlot::slot_tabPageChanged);

		connect(m_schemaManager, &ClientSchemaManager::schemasWereReseted, this, &SchemaTabWidgetSignalSlot::slot_resetSchema);

		m_eventLoopTimerId = startTimer(1);

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::setVisibleTabBar(bool visible)
	{
		tabBar()->setVisible(visible);
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::timerEvent(QTimerEvent* event)
	{
		if (m_eventLoopTimerId != 0)
		{
			m_eventLoopTimerCounter++;

			if (event->timerId() == m_eventLoopTimerId && m_eventLoopTimerCounter > 10)
			{
				killTimer(m_eventLoopTimerId);
				m_eventLoopTimerId = 0;

				// Create first tab here
				// Problem - we set zoom to FitToScreen, for it we need to have window geometry
				// but in constructor it is not set correctly, and it changes when message loop starts/
				// So we have to process several messages in message loop and only after that
				// we can add new tab page
				//
				slot_newSchemaTab("EMPTY");
			}
		}

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	int SchemaTabWidget<ClientSchemaWidgetType>::addSchemaTabPage(const QString& schemaId, const QVariantHash& variables)
	{
		// Create a dummy context, later it will be changed to the normal one inside ClientSchemaWidgetType
		// constructor, as MonitorSchemaView is created inside MonitorSchemaView().
		//
		auto dummyContext = VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr, nullptr);

		std::shared_ptr<VFrame30::Schema> tabSchema;

		if (m_schemaManager->hasSchema(schemaId) == true)
		{
			tabSchema = m_schemaManager->schema(schemaId, dummyContext);
		}
		else
		{
			if (m_schemaManager->hasSchema(startSchemaId()) == true)
			{
				// If schema is not found try to set StartSchemaID
				//
				tabSchema = m_schemaManager->schema(startSchemaId(), dummyContext);
			}
		}

		// Schema still not found, create empty schema
		//
		if (tabSchema == nullptr)
		{
			// It does not matter what kind of schema was created.
			// It is just a dummy schema to show something.
			//
			tabSchema = std::make_shared<VFrame30::MonitorSchema>();
			tabSchema->setSchemaId("EMPTYSCHEMA");
			tabSchema->setCaption("Empty Schema");
		}

		ClientSchemaWidgetType* schemaWidget = m_createSchemaWidgetFunc(tabSchema, this);

		schemaWidget->setZoomMode(m_zoomMode, false);
		schemaWidget->clientSchemaView()->setVariables(variables);

		connect(schemaWidget, &ClientSchemaWidgetType::signal_schemaChanged, this, &SchemaTabWidgetSignalSlot::slot_schemaChanged);
		connect(schemaWidget, &ClientSchemaWidgetType::signal_historyChanged, this, &SchemaTabWidgetSignalSlot::signal_historyChanged);

		int index = addTab(schemaWidget, tabSchema->caption());

		if (count() > 1 && tabsClosable() == false)
		{
			setTabsClosable(true);
			setMovable(true);
		}

		// clientSchemaView->setVariables() (line 125) may override already created variables in scripts,
		// so run onShowScript here, after setting view variables(!). Also onShowScript triggers
		// running onConfigurationArrivedScript
		//
		schemaWidget->schema()->onShowEvent(schemaWidget->clientSchemaView()->jsEngine(),
											schemaWidget->clientSchemaView()->logFile());

		// --
		//
		emit signal_actionCloseTabUpdated(count() > 1);

		return index;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::applyZoomMode(VFrame30::ZoomMode zoomMode)
	{
		for (int tabIndex = 0; tabIndex < count(); tabIndex++)
		{
			auto tabWidget = dynamic_cast<ClientSchemaWidgetType*>(widget(tabIndex));

			if (tabWidget != nullptr)
			{
				tabWidget->setZoomMode(zoomMode, true);
			}
		}

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_newSchemaTab(QString schemaId)
	{
		int tabIndex = addSchemaTabPage(schemaId, {});

		// Switch to the new tab
		//
		if (tabIndex != -1)
		{
			setCurrentIndex(tabIndex);

			ClientSchemaWidgetType* newTab = currentTab();
			Q_ASSERT(newTab);

			if (m_zoomMode == VFrame30::ZoomMode::Manual)
			{
				// Initially set zoom to "fit to screen".
				//
				newTab->setZoom(0, false);
			}

			emit signal_schemaChanged(newTab->schemaId()); // Different schema could be set, it can happen if schema does not exist
			newTab->emitHistoryChanged();
		}

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_newTab()
	{
		ClientSchemaWidgetType* curTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());

		if (curTabWidget == nullptr)
		{
			// If Current tab not schema widget,
			// then create new empty or start schema widget
			//
			QString schemaId = m_startSchemaId;
			int tabIndex = addSchemaTabPage(schemaId, {});

			// Switch to the new tab
			//
			if (tabIndex != -1)
			{
				setCurrentIndex(tabIndex);
				emit signal_schemaChanged(schemaId);

				ClientSchemaWidgetType* newTab = currentTab();
				Q_ASSERT(newTab);

				newTab->emitHistoryChanged();
			}
		}
		else
		{
			// Duplicate tab
			//
			slot_newSameTab(curTabWidget);
		}

		// Set zoom for new tab
		//
		if (auto newTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
			newTabWidget != nullptr)
		{
			if (m_zoomMode == VFrame30::ZoomMode::Manual ||
				m_zoomMode == VFrame30::ZoomMode::FitToScreen)
			{
				newTabWidget->clientSchemaView()->setZoom(0);
			}

			if (m_zoomMode == VFrame30::ZoomMode::Always100Percent)
			{
				newTabWidget->clientSchemaView()->setZoom(100);
			}
		}

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_closeCurrentTab()
	{
		QWidget* curTabWidget = currentWidget();
		if (curTabWidget == nullptr)
		{
			Q_ASSERT(curTabWidget);
			return;
		}

		slot_closeTab(curTabWidget);
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_zoomIn()
	{
		ClientSchemaWidgetType* curTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
		if (curTabWidget == nullptr)
		{
			Q_ASSERT(curTabWidget);
			return;
		}

		curTabWidget->zoomIn();
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_zoomOut()
	{
		ClientSchemaWidgetType* curTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
		if (curTabWidget == nullptr)
		{
			Q_ASSERT(curTabWidget);
			return;
		}

		curTabWidget->zoomOut();
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_zoom100()
	{
		ClientSchemaWidgetType* curTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
		if (curTabWidget == nullptr)
		{
			Q_ASSERT(curTabWidget);
			return;
		}

		curTabWidget->zoom100();
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_zoomToFit()
	{
		ClientSchemaWidgetType* curTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
		if (curTabWidget == nullptr)
		{
			Q_ASSERT(curTabWidget);
			return;
		}

		curTabWidget->zoomToFit();
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_historyBack()
	{
		ClientSchemaWidgetType* curTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
		if (curTabWidget == nullptr || curTabWidget->canBackHistory() == false)
		{
			Q_ASSERT(curTabWidget && curTabWidget->canBackHistory() == true);
			return;
		}

		curTabWidget->historyBack();
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_historyForward()
	{
		ClientSchemaWidgetType* curTabWidget = dynamic_cast<ClientSchemaWidgetType*>(currentWidget());
		if (curTabWidget == nullptr || curTabWidget->canForwardHistory() == false)
		{
			Q_ASSERT(curTabWidget && curTabWidget->canForwardHistory() == true);
			return;
		}

		curTabWidget->historyForward();
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_selectSchemaForCurrentTab(QString schemaId)
	{
		ClientSchemaWidgetType* tab = currentTab();
		if (tab == nullptr)
		{
			Q_ASSERT(tab);
			return;
		}

		tab->setSchema(schemaId, QStringList{}, false);
		tab->emitHistoryChanged();
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu)
	{
		currentTab()->signalContextMenu(signalList, {}, {}, customMenu);
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_signalInfo(QString signalId)
	{
		currentTab()->signalInfo(signalId);
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_export()
	{
		auto schema = currentTab()->schema();
		if (schema == nullptr)
		{
			Q_ASSERT(schema);
			return;
		}

		static QString path{"."};
		QString fileName = QFileDialog::getSaveFileName(this,
														tr("Export Schema"),
														path + QDir::separator() + schema->schemaId() + ".pdf",
														tr("PDF Files (*.pdf);;PNG Files (*.png)"));
		if (fileName.isEmpty() == true)
		{
			return;
		}

		path = QFileInfo(fileName).path(); // store path for next time

		bool ok = false;

		if (fileName.endsWith(".pdf", Qt::CaseInsensitive) == true)
		{
			ok = currentTab()->clientSchemaView()->saveSchemaToPdf(fileName);
		}
		else
		{
			if (fileName.endsWith(".png", Qt::CaseInsensitive) == true)
			{
				ok = currentTab()->clientSchemaView()->saveSchemaToPng(fileName);
			}
			else
			{
				QMessageBox::critical(this, qAppName(), tr("Wrong file '%1' format, expected '.png' or '.pdf'!").arg(fileName));
				return;
			}
		}

		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to save file '%1'!").arg(fileName));
		}

		return;
	}


	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_tabCloseRequested(int index)
	{
		// Close Tab request
		//
		if (count() == 1)
		{
			// Don't close the last tab
			return;
		}

		QWidget* tabWidget = widget(index);
		if (tabWidget == nullptr)
		{
			return;
		}

		removeTab(index);
		delete tabWidget;

		if (count() <= 1)
		{
			// Hide close button to prevent blink
			//
			QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, tabBar());
			for (int i = 0; i < count(); i++)
			{
				QWidget* closeButton = tabBar()->tabButton(i, closeSide);
				if (closeButton != nullptr)
				{
					closeButton->setVisible(false);
				}
			}

			setTabsClosable(false);
			setMovable(false);
		}

		emit signal_actionCloseTabUpdated(count() > 1);
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_resetSchema()
	{
		// All schemas must be refreshed, apparently the new configuration has arrived
		// if there is no schema with prev SchemaID, load startSchemaId
		//
		for (int i = 0; i < count(); i++)
		{
			ClientSchemaWidgetType* tabPage = dynamic_cast<ClientSchemaWidgetType*>(widget(i));
			if (tabPage == nullptr)
			{
				// it can be schema list
				//
				continue;
			}

			QString schemaToLoad = tabPage->schemaId();
			if (m_schemaManager->hasSchema(tabPage->schemaId()) == true)
			{
				schemaToLoad = tabPage->schemaId();
			}
			else
			{
				if (m_schemaManager->hasSchema(m_startSchemaId) == true)
				{
					schemaToLoad = m_startSchemaId;
				}
				else
				{
					// schemaToLoad will stay tabPage->schemaId(); as during initialization,
					// in that case empty schema will be loaded
					//
				}
			}

			// Set schema for client widget. Force schema update, as we are reloading all schemas and onSomeEvent should be called.
			//
			tabPage->setSchema(schemaToLoad, tabPage->clientSchemaView()->highlightIds(), true);

			tabPage->clientSchemaView()->deleteControlWidgets(); // deleteControlWidgets after loading new schema, as it will delete old widgets and later they will be created
			tabPage->clientSchemaView()->updateControlWidgets(false);

			tabPage->resetHistory();

			if (i == currentIndex())
			{
				emit signal_schemaChanged(tabPage->schema()->schemaId());
			}
		}

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_newSameTab(VFrame30::ClientSchemaWidget* tabWidget)
	{
		if (tabWidget == nullptr)
		{
			Q_ASSERT(tabWidget);
			return;
		}

		QString schemaId = tabWidget->schema()->schemaId();
		QVariantHash variables = tabWidget->clientSchemaView()->variables();

		int tabIndex = addSchemaTabPage(schemaId, variables);

		// Switch to the new tab
		//
		if (tabIndex != -1)
		{
			setCurrentIndex(tabIndex);
			emit signal_schemaChanged(schemaId);

			ClientSchemaWidgetType* newTab = currentTab();
			Q_ASSERT(newTab);

			newTab->emitHistoryChanged();
		}

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_closeTab(QWidget* tabWidget)
	{
		if (tabWidget == nullptr)
		{
			Q_ASSERT(tabWidget);
			return;
		}

		int tabIndex = indexOf(tabWidget);
		if (tabIndex == -1)
		{
			Q_ASSERT(tabIndex != -1);
			return;
		}

		slot_tabCloseRequested(tabIndex);
		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_schemaChanged(VFrame30::ClientSchemaWidget* tabWidget, VFrame30::Schema* schema)
	{
		if (tabWidget == nullptr ||
			schema == nullptr)
		{
			Q_ASSERT(tabWidget);
			Q_ASSERT(schema);
			return;
		}

		int tabIndex = indexOf(tabWidget);
		if (tabIndex >= 0)
		{
			setTabText(tabIndex, schema->caption());
		}

		emit signal_schemaChanged(schema->schemaId());
		tabWidget->emitHistoryChanged();

		return;
	}

	template<ClientSchemaWidgetConcept ClientSchemaWidgetType>
	void SchemaTabWidget<ClientSchemaWidgetType>::slot_tabPageChanged(int index)
	{
		ClientSchemaWidgetType* schemaWidgetTab = currentTab();

		emit signal_tabPageChanged(schemaWidgetTab != nullptr); // This signal is to enable/disable QActions

		// Show/hide close button for inactive tab bar
		//
		QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, this->tabBar());

		for (int i = 0; i < count(); i++)
		{
			QWidget* w = tabBar()->tabButton(i, closeSide);
			if (w != nullptr)
			{
				w->setVisible(i == index);
			}
		}

		if (schemaWidgetTab != nullptr)
		{
			emit signal_schemaChanged(schemaWidgetTab->schemaId());
			schemaWidgetTab->emitHistoryChanged();
		}

		return;
	}

} // namespace SchemaClientLib
