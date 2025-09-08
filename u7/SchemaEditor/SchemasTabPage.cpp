#include "SchemasTabPage.h"
#include "GlobalMessanger.h"
#include "SchemaControlTabPage.h"
#include "EditSchemaTabPage.h"
#include <UiLib/TabWidgetEx.h>


//
//
// SchemasTabPage
//
//
SchemasTabPage::SchemasTabPage(DbController* dbc,
							   AppSignalSetProvider* signalSetProvider,
							   UiLib::ClickableLabel* statusBarLayerLabel,
							   UiLib::ClickableLabel* statusBarZoomLabel,
							   QWidget* parent) :
	MainTabPage{dbc, parent},
	m_statusBarLayerLabel{statusBarLayerLabel},
	m_statusBarZoomLabel{statusBarZoomLabel}
{
	Q_ASSERT(dbc);
	Q_ASSERT(signalSetProvider);
	Q_ASSERT(m_statusBarLayerLabel);
	Q_ASSERT(m_statusBarZoomLabel);

	m_tabWidget = new UiLib::TabWidgetEx{this};

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout();
	layout->setContentsMargins(0, 6, 0, 0);

	layout->addWidget(m_tabWidget);

	setLayout(layout);

	// --
	//
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectOpened, this, &SchemasTabPage::projectOpened);
	connect(&GlobalMessanger::instance(), &GlobalMessanger::projectClosed, this, &SchemasTabPage::projectClosed);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);

	// Add control page
	//
	m_controlTabPage = new SchemaControlTabPage(dbc, signalSetProvider);
	m_tabWidget->addTab(m_controlTabPage, tr("Schemas Control"));

	m_tabWidget->setTabToolTip(0, tr("Schemas Control\n"
									 "[CTRL + `]"));

	// Hide close button for control tab page
	//
	QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, m_tabWidget->tabBar());
	QWidget* closeButton = m_tabWidget->tabBar()->tabButton(0, closeSide);
	if (closeButton != nullptr)
	{
		closeButton->setVisible(false);
	}

	// Add shortcut for switching to control tab page
	//
	m_showControlTabAccelerator = new QAction{tr("Schemas Control"), this};
	m_showControlTabAccelerator->setShortcuts(QList<QKeySequence>{}
											  <<  QKeySequence{Qt::CTRL | Qt::Key_QuoteLeft}
											  <<  QKeySequence{Qt::CTRL | Qt::Key_AsciiTilde}
											  );
	m_showControlTabAccelerator->setShortcutContext(Qt::ApplicationShortcut);

	addAction(m_showControlTabAccelerator);

	connect(m_showControlTabAccelerator, &QAction::triggered,
			[this]()
			{
				for (int i = 0; i < m_tabWidget->count(); i++)
				{
					SchemaControlTabPage* w = dynamic_cast<SchemaControlTabPage*>(m_tabWidget->widget(i));

					if (w != nullptr)
					{
						if	(m_tabWidget->currentIndex() != i)
						{
							m_tabWidget->setCurrentIndex(i);
						}

						return;
					}
				}
			});

	connect(m_tabWidget->tabBar(), &QTabBar::tabCloseRequested, this, &SchemasTabPage::tabCloseRequested);
	connect(m_tabWidget->tabBar(), &QTabBar::currentChanged, this, &SchemasTabPage::currentTabChanged);

	// Start timer for updating StatusBar
	//
	startTimer(150);

	connect(m_statusBarLayerLabel, &UiLib::ClickableLabel::clicked, this, &SchemasTabPage::statusBarLayerClicked);
	connect(m_statusBarZoomLabel, &UiLib::ClickableLabel::clicked, this, &SchemasTabPage::statusBarZoomClicked);

	return;
}

SchemasTabPage::~SchemasTabPage()
{
}

bool SchemasTabPage::hasUnsavedSchemas() const
{
	Q_ASSERT(m_controlTabPage);
	return m_controlTabPage->hasUnsavedSchemas();
}

bool SchemasTabPage::saveUnsavedSchemas()
{
	Q_ASSERT(m_controlTabPage);
	return m_controlTabPage->saveUnsavedSchemas();
}

bool SchemasTabPage::resetModified()
{
	Q_ASSERT(m_controlTabPage);
	return m_controlTabPage->resetModified();
}


namespace
{
	struct SessionSchema
	{
		bool readOnly = false;
		int changesetId = 0;
		QString schemaId;
		int fileId = 0;
		double zoom = 100.0;

		QString saveSessionSchema() const;
		bool restoreSessionSchema(const QString& str);
	};

	QString SessionSchema::saveSessionSchema() const
	{
		return QString("1;%1;%2;%3;%4;%5")
				.arg(readOnly)
				.arg(changesetId)
				.arg(schemaId)
				.arg(fileId)
				.arg(zoom);
	}

	bool SessionSchema::restoreSessionSchema(const QString& str)
	{
		QStringList sl = str.split(';');

		if (sl.isEmpty() == true)
		{
			Q_ASSERT(sl.isEmpty() == false);
			return false;
		}

		int version = sl[0].toInt();

		if (version == 1 && sl.size() == 6)
		{
			readOnly = sl[1].toInt() ? true : false;
			changesetId = sl[2].toInt();
			schemaId = sl[3];
			fileId = sl[4].toInt();
			zoom = std::clamp(sl[5].toDouble(), 50.0, 250.0);
			return true;
		}

		// Unknow version? Just ignore it.
		//
		return false;
	}
} // anonymous namespace

void SchemasTabPage::saveSession() const
{
	if (m_requireRestoreSession == true)
	{
		// Session was not restored (no showEvent for widget)
		// return to avoid storing empty session
		//
		return;
	}

	// Save all opened schamas
	//
	if (db()->isProjectOpened() == false)
	{
		Q_ASSERT(db()->isProjectOpened());
		return;
	}

	// Save new session data, data saved to settings, for each project separately
	//
	QSettings settings;

	QString keyDir = QString("Session/%1-%2/SchemaEditor/")
					 .arg(db()->currentProject().projectName())
					 .arg(db()->currentUser().username());

	settings.setValue(keyDir + "Count", m_tabWidget->count() - 1);	// 1 is control tab page, it si not stored

	QString selectedSchema;

	for (int tabIndex = 0, schemaIndex = 0; tabIndex < m_tabWidget->count(); tabIndex++)
	{
		auto editWidget = dynamic_cast<const EditSchemaTabPage*>(m_tabWidget->widget(tabIndex));
		if (editWidget == nullptr)
		{
			continue;
		}

		const DbFileInfo& f = editWidget->fileInfo();

		SessionSchema ss{
			.readOnly = editWidget->readOnly(),
			.changesetId = f.changeset(),
			.schemaId = editWidget->schema()->schemaId(),
			.fileId = f.fileId(),
			.zoom = editWidget->zoom()
		};

		QString sessionString = ss.saveSessionSchema();
		settings.setValue(keyDir + QString("Schema_%1").arg(schemaIndex), sessionString);

		if (tabIndex == m_tabWidget->currentIndex())
		{
			selectedSchema = sessionString;
		}

		schemaIndex ++; // i is not index, as control widget is skipped
	}

	settings.setValue(keyDir + "Current", selectedSchema);

	return;
}

void SchemasTabPage::restoreSession()
{
	m_requireRestoreSession = false;

	// Restore all opened schemas
	//
	if (m_controlTabPage == nullptr || db()->isProjectOpened() == false)
	{
		Q_ASSERT(m_controlTabPage);
		Q_ASSERT(db()->isProjectOpened());
		return;
	}

	QSettings settings;
	QString keyDir = QString("Session/%1-%2/SchemaEditor/")
					 .arg(db()->currentProject().projectName())
					 .arg(db()->currentUser().username());
	
	int schemaCount = settings.value(keyDir + "Count", 0).toInt();
	QString currentSchemaRecord = settings.value(keyDir + "Current").toString();
	QStringList records;
	records.reserve(schemaCount);

	for (int i = 0; i < schemaCount; i++)
	{
		QString key = keyDir + QString("Schema_%1").arg(i);
		records.push_back(settings.value(key).toString());
	}

	// Clear settings records, in case of the crash the session will not be restored.
	//
	settings.remove(keyDir);

	// Load schemas
	//
	int currentSchemaIndex = 0;

	for (int i = 0; i < schemaCount; i++)
	{
		// Restore session, load schema, etc.
		//
		SessionSchema ss;

		if (bool ok = ss.restoreSessionSchema(records[i]);
			ok == false)
		{
			continue;
		}

		DbFileInfo f;
		if (bool ok = db()->getFileInfo(ss.fileId, &f, this);
			ok == false)
		{
			continue;
		}

		bool allowedToEdit = f.state() == E::VcsState::CheckedOut &&
							 (f.userId() == db()->currentUser().userId() || db()->currentUser().isAdministrator() == true);

		if (ss.readOnly == true)
		{
			m_controlTabPage->viewFile(f, ss.changesetId);
		}
		else if (allowedToEdit == false)
		{
			m_controlTabPage->viewFile(f, f.changeset()); // Latest changeset
		}
		else
		{
			m_controlTabPage->openFile(f);
		}

		auto editWidget = dynamic_cast<EditSchemaTabPage*>(m_tabWidget->widget(m_tabWidget->count() - 1));
		Q_ASSERT(editWidget);

		if (editWidget != nullptr)
		{
			editWidget->setZoom(ss.zoom, false);

			if (currentSchemaRecord == records[i])
			{
				currentSchemaIndex = m_tabWidget->count() - 1;
			}
		}
	}

	m_tabWidget->setCurrentIndex(currentSchemaIndex);

	return;
}

void SchemasTabPage::refreshControlTabPage()
{
	Q_ASSERT(m_controlTabPage);
	m_controlTabPage->refresh();

	return;
}

void SchemasTabPage::showEvent(QShowEvent* event)
{
	MainTabPage::showEvent(event);

	if (m_requireRestoreSession == true)
	{
		restoreSession();
	}

	return;
}

void SchemasTabPage::timerEvent(QTimerEvent* /*event*/)
{
	EditSchemaTabPage* w = dynamic_cast<EditSchemaTabPage*>(m_tabWidget->currentWidget());

	if (w != nullptr)
	{
		m_statusBarLayerLabel->setText(tr("Layer: %1").arg(w->activeLayer()));
		m_statusBarZoomLabel->setText(tr("Zoom: %1%").arg(static_cast<int>(w->zoom())));
	}
	else
	{
		m_statusBarLayerLabel->setText({});
		m_statusBarZoomLabel->setText({});
	}

	return;
}

void SchemasTabPage::projectOpened()
{
	this->setEnabled(true);

	m_requireRestoreSession = true;

	return;
}

void SchemasTabPage::projectClosed()
{
	GlobalMessanger::instance().clearBuildSchemaIssues();

	m_requireRestoreSession = false;

	this->setEnabled(false);
	return;
}

void SchemasTabPage::tabCloseRequested(int index)
{
	EditSchemaTabPage* w = dynamic_cast<EditSchemaTabPage*>(m_tabWidget->widget(index));
	if (w == nullptr)
	{
		return;
	}

	if (w->modified() == true && m_tabWidget->currentIndex() != index)
	{
		m_tabWidget->setCurrentIndex(index);
	}

	w->closeTab();

	return;
}

void SchemasTabPage::currentTabChanged(int index)
{
	// Show/hide close button for inactive tab bar.
	//
	QTabBar::ButtonPosition closeSide = (QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, m_tabWidget->tabBar());

	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		EditSchemaTabPage* w = dynamic_cast<EditSchemaTabPage*>(m_tabWidget->widget(i));

		if (w != nullptr)
		{
			if (i == index)
			{
				m_tabWidget->tabBar()->tabButton(i, closeSide)->show();
			}
			else
			{
				m_tabWidget->tabBar()->tabButton(i, closeSide)->hide();
			}
		}
	}

	return;
}

void SchemasTabPage::statusBarLayerClicked()
{
	EditSchemaTabPage* w = dynamic_cast<EditSchemaTabPage*>(m_tabWidget->currentWidget());
	if (w != nullptr && w->schema())
	{
		auto schema = w->schema();

		QMenu menu;
		QList<QAction*> actions;

		auto layers = schema->layers();
		for (auto layer : layers)
		{
			QAction* layerAction = new QAction{layer->name(), &menu};
			layerAction->setData(layer->name());
			layerAction->setCheckable(true);
			layerAction->setChecked(layer->name() == w->activeLayer());

			actions.push_back(layerAction);
		}

		QAction* actSeparator = new QAction{"--", &menu};
		actSeparator->setSeparator(true);
		QAction* actLayers = new QAction{tr("Layers..."), &menu};

		actions.push_back(actSeparator);
		actions.push_back(actLayers);

		// Show menu.
		//
		QAction* hitAchtion = menu.exec(actions, QCursor::pos());

		if (hitAchtion != nullptr)
		{
			if (hitAchtion == actLayers)
			{
				w->layersDialog();
				return;
			}

			if (hitAchtion->data().isValid() == true)
			{
				w->setActiveLayer(hitAchtion->data().toString());
				return;
			}
		}
	}

	return;
}

void SchemasTabPage::statusBarZoomClicked()
{
	EditSchemaTabPage* w = dynamic_cast<EditSchemaTabPage*>(m_tabWidget->currentWidget());
	if (w != nullptr)
	{
		int currentZoom = static_cast<int>(w->zoom());

		QMenu menu;
		QList<QAction*> actions;

		actions.push_back(new QAction{"100%", &menu});
		actions.back()->setShortcut(Qt::CTRL | Qt::Key_Asterisk);
		actions.back()->setData(100);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(actions.back()->data().toInt() == currentZoom);

		actions.push_back(new QAction{"Zoom to Fit", &menu});
		actions.back()->setData(0);

		actions.push_back(new QAction{"--", &menu});
		actions.back()->setSeparator(true);

		actions.push_back(new QAction{"50%", &menu});
		actions.back()->setData(50);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(actions.back()->data().toInt() == currentZoom);

		actions.push_back(new QAction{"75%", &menu});
		actions.back()->setData(75);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(actions.back()->data().toInt() == currentZoom);

		actions.push_back(new QAction{"100%", &menu});
		actions.back()->setData(100);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(actions.back()->data().toInt() == currentZoom);

		actions.push_back(new QAction{"125%", &menu});
		actions.back()->setData(125);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(actions.back()->data().toInt() == currentZoom);

		actions.push_back(new QAction{"150%", &menu});
		actions.back()->setData(150);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(actions.back()->data().toInt() == currentZoom);

		actions.push_back(new QAction{"200%", &menu});
		actions.back()->setData(200);
		actions.back()->setCheckable(true);
		actions.back()->setChecked(actions.back()->data().toInt() == currentZoom);

		// Show menu.
		//
		QAction* hitAction = menu.exec(actions, QCursor::pos());

		if (hitAction != nullptr && hitAction->data().isValid() == true)
		{
			w->setZoom(hitAction->data().toInt(), true);
		}
	}

	return;
}
