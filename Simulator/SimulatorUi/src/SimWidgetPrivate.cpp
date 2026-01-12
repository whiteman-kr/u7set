#include "SimWidgetPrivate.h"
#include "NotificationPanel.h"
#include "SimCodePage.h"
#include "SimConnectionPage.h"
#include "SimLogicModulePage.h"
#include "SimOutputWidget.h"
#include "SimOverridePane.h"
#include "SimProjectWidget.h"
#include "SimSchemaPage.h"
#include "SimSelectSchemaPage.h"
#include "SimSignalInfo.h"
#include "SimSignalSnapshot.h"
#include "SimTrends.h"

#include <SchemaClientLib/DialogSignalSearch.h>
#include <SimulatorLib/SimControl.h>
#include <SimulatorLib/SimLogicModule.h>
#include <SimulatorLib/SimService.h>
#include <SimulatorLib/SimSoftware.h>
#include <SimulatorUi/SimWidget.h>
#include <UiLib/TabWidgetEx.h>
#include <VFrame30/Context.h>
#include <VFrame30/Schema.h>

#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QDir>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QLineEdit>
#include <QMimeData>
#include <QVBoxLayout>
#include <QXmlStreamReader>


namespace SimUi
{
	std::vector<QComboBox*> SimWidgetPrivate::s_speedComboBoxes;
	std::vector<QComboBox*> SimWidgetPrivate::s_profilesComboBoxes;


	SimWidgetPrivate::SimWidgetPrivate(std::shared_ptr<ILogFile> ideLogFile,
									   std::shared_ptr<SimIdeSimulator> simulator,
									   std::function<QString(QWidget*)> getProjectPathFunc,
									   ISimPropertyStorage& propertyStorage,
									   DbProjectStateNotifier* dbProjectStateNotifier,
									   QWidget* parent /*= nullptr*/,
									   Qt::WindowType windowType /*= Qt::Window*/,
									   bool slaveWindow /*= false*/,
									   SimWidgetPrivate* masterWindow /*= nullptr*/) :
		QMainWindow{parent},
		m_slaveWindow{slaveWindow},
		m_masterWindow{slaveWindow ? masterWindow : this},
		m_getProjectPathFunc{getProjectPathFunc},
		m_propertyStorage{propertyStorage},
		m_dbProjectStateNotifier{dbProjectStateNotifier},
		m_ideLogFile(ideLogFile),
		m_simulator{simulator ? simulator : std::make_shared<SimIdeSimulator>(m_ideLogFile.get(), true, nullptr)},
		m_schemaManager{m_simulator.get()}
	{
		Q_ASSERT(ideLogFile);
		Q_ASSERT((slaveWindow == false && masterWindow == nullptr) || (slaveWindow == true && masterWindow != nullptr));

		// --
		//
		m_appSignalController = new VFrame30::AppSignalController{m_simulator->appSignalManager(), this};

		// --
		//
		setWindowFlags(windowType);
		setDockOptions(AnimatedDocks | AllowTabbedDocks | GroupedDragging);

		m_tabWidget = new UiLib::TabWidgetEx{this};
		m_tabWidget->setDocumentMode(false);
		m_tabWidget->tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);

		setCentralWidget(m_tabWidget);
		centralWidget()->setAutoFillBackground(true);

		QVBoxLayout* layout = new QVBoxLayout;
		centralWidget()->setLayout(layout);

		auto margins = layout->contentsMargins();
		margins.setTop(0);
		layout->setContentsMargins(margins);

		createToolBar();
		createDocks();

		updateActions();

		// --
		//
		if (m_dbProjectStateNotifier != nullptr)
		{
			// If notifier is present then windows will be enabled when DbProjectStateNotifier::projectOpened happens.
			//
			setEnabled(m_simulator->isLoaded());

			// --
			//
			connect(m_dbProjectStateNotifier,
					&DbProjectStateNotifier::projectOpened,
					[this]()
					{
						setEnabled(true);
						closeBuild();
						projectOpened();
					});

			connect(m_dbProjectStateNotifier,
					&DbProjectStateNotifier::projectClosed,
					[this]()
					{
						closeBuild();
						setEnabled(false);
						projectClosed();
					});
		}

		connect(m_simulator.get(), &SimIdeSimulator::projectUpdated, this, &SimWidgetPrivate::updateActions);
		connect(m_simulator.get(),
				&SimIdeSimulator::projectUpdated,
				this,
				[this]()
				{
					if (m_notificationPanel != nullptr)
					{
						m_notificationPanel->hide();
					}
				});

		connect(&(m_simulator->control()), &Sim::Control::stateChanged, this, &SimWidgetPrivate::controlStateChanged);
		connect(&(m_simulator->control()), &Sim::Control::statusUpdate, this, &SimWidgetPrivate::updateTimeIndicator);

		connect(m_projectWidget, &SimProjectWidget::signal_openLogicModuleTabPage, this, &SimWidgetPrivate::openLogicModuleTabPage);
		connect(m_projectWidget, &SimProjectWidget::signal_openCodeTabPage, this, &SimWidgetPrivate::openCodeTabPage);
		connect(m_projectWidget, &SimProjectWidget::signal_openConnectionTabPage, this, &SimWidgetPrivate::openConnectionTabPage);
		connect(m_projectWidget, &SimProjectWidget::signal_openAppSchemasTabPage, this, &SimWidgetPrivate::openAppSchemasTabPage);

		connect(m_tabWidget, &QTabWidget::tabCloseRequested, this, &SimWidgetPrivate::tabCloseRequest);
		connect(m_tabWidget, &QTabWidget::currentChanged, this, &SimWidgetPrivate::tabCurrentChanged);
		connect(m_tabWidget->tabBar(), &QTabWidget::customContextMenuRequested, this, &SimWidgetPrivate::tabBarContextMenuRequest);

		connect(this, &SimWidgetPrivate::needUpdateActions, this, &SimWidgetPrivate::updateActions);

		if (m_slaveWindow == false)
		{
			connect(qApp, &QCoreApplication::aboutToQuit, this, &SimWidgetPrivate::aboutToQuit);
		}

		// Add shortcut for switching to control tab page
		//
		m_showControlTabAccelerator = new QAction{tr("Schemas Control"), this};
		m_showControlTabAccelerator->setShortcuts(QList<QKeySequence>{} << QKeySequence{Qt::CTRL | Qt::Key_QuoteLeft}
																		<< QKeySequence{Qt::CTRL | Qt::Key_AsciiTilde});
		m_showControlTabAccelerator->setShortcutContext(Qt::ApplicationShortcut);

		addAction(m_showControlTabAccelerator);

		connect(m_showControlTabAccelerator, &QAction::triggered, this, &SimWidgetPrivate::openAppSchemasTabPage);

		startTimer(2000);

		return;
	}

	SimWidgetPrivate::~SimWidgetPrivate()
	{
		std::erase(s_speedComboBoxes, m_speedComboBox);
		std::erase(s_profilesComboBoxes, m_profilesComboBox);
	}

	void SimWidgetPrivate::startTrends(const std::vector<AppSignalParam>& appSignals)
	{
		SimTrends::startTrendApp(m_simulator, appSignals, this);
	}

	void SimWidgetPrivate::signalContextMenu(const QStringList signalList, const QList<QMenu*>& customMenu)
	{
		// Compose menu
		//
		QMenu menu(this);

		for (const QString& s : signalList)
		{
			auto signal = m_appSignalController->signalParam(s);
			QString signalId = signal.has_value() ? QString("%1 %2").arg(signal->customSignalId()).arg(signal->caption()) : s;

			auto f = [this, s]() -> void
			{
				signalInfo(s);
			};

			menu.addAction(signalId, f);
		}

		if (customMenu.empty() == false)
		{
			menu.addSeparator();

			for (auto cm : customMenu)
			{
				menu.addActions(cm->actions());
			}
		}

		menu.exec(QCursor::pos());
	}

	void SimWidgetPrivate::signalInfo(QString appSignalId)
	{
		SimSignalInfo::showDialog(appSignalId, m_simulator.get(), this);
		return;
	}

	void SimWidgetPrivate::openSchemaTabPage(QString schemaId, QStringList highlightIds)
	{
		// Look for already opened schema, and activate it
		//
		{
			for (int i = 0; i < m_tabWidget->count(); i++)
			{
				SimSchemaPage* sp = dynamic_cast<SimSchemaPage*>(m_tabWidget->widget(i));

				if (sp != nullptr && sp->schemaId() == schemaId)
				{
					m_tabWidget->setCurrentIndex(i);
					sp->setHighlightIds(highlightIds);
					return;
				}
			}
		}

		// There is no such schema, load it and create a widget for it
		//

		// Create a fake context, later it will be changed in SimSchemaWidget::SimSchemaWidget(...)
		//
		auto fakeContext = VFrame30::Context::create(nullptr, nullptr, nullptr, nullptr, nullptr);

		std::shared_ptr<VFrame30::Schema> schema = m_schemaManager.schema(schemaId, std::move(fakeContext));
		if (schema == nullptr)
		{
			QMessageBox::critical(this, qAppName(), tr("Cannot open file %1").arg(schemaId));
			return;
		}

		SimSchemaPage* page = new SimSchemaPage{schema, m_simulator.get(), &m_schemaManager, m_appSignalController, m_tabWidget};

		int tabIndex = m_tabWidget->addTab(page, schema->schemaId());
		m_tabWidget->setCurrentIndex(tabIndex);

		page->simSchemaWidget()->setZoom(0, false);
		page->setHighlightIds(highlightIds);

		return;
	}

	void SimWidgetPrivate::createToolBar()
	{
		m_toolBar = new SimToolBar{"ToolBar"};
		addToolBar(m_toolBar);

		m_openProjectAction = new QAction{QIcon(":/SimulatorUi/Images/SimOpen.svg"), tr("Open Build"), this};
		m_openProjectAction->setShortcut(QKeySequence::Open);
		connect(m_openProjectAction, &QAction::triggered, this, &SimWidgetPrivate::openBuild);

		m_closeProjectAction = new QAction{QIcon(":/SimulatorUi/Images/SimClose.svg"), tr("Close"), this};
		m_closeProjectAction->setShortcut(QKeySequence::Close);
		connect(m_closeProjectAction, &QAction::triggered, this, &SimWidgetPrivate::closeBuild);

		m_refreshProjectAction = new QAction{QIcon(":/SimulatorUi/Images/SimRefresh.svg"), tr("Refresh"), this};
		m_refreshProjectAction->setShortcut(QKeySequence::Refresh);
		connect(m_refreshProjectAction, &QAction::triggered, this, &SimWidgetPrivate::refreshBuild);

		m_addWindowAction = new QAction{QIcon(":/SimulatorUi/Images/SimAddWindow.svg"), tr("Add Window"), this};
		m_addWindowAction->setShortcut(QKeySequence::New);
		connect(m_addWindowAction, &QAction::triggered, this, &SimWidgetPrivate::addNewWindow);
		m_toolBar->addAction(m_addWindowAction);

		// --
		//
		m_takeSnapshotAction = new QAction{QIcon(":/SimulatorUi/Images/SimTakeSnapshot.svg"), tr("Take Snapshot"), this};
		connect(m_takeSnapshotAction, &QAction::triggered, this, &SimWidgetPrivate::takeSnapshot);

		m_applySnapshotAction = new QAction{QIcon(":/SimulatorUi/Images/SimApplySnapshot.svg"), tr("Apply Snapshot"), this};
		connect(m_applySnapshotAction, &QAction::triggered, this, &SimWidgetPrivate::applySnapshot);

		// --
		//
		m_simulationTimeEdit = new QLineEdit{this};
		m_simulationTimeEdit->setPlaceholderText("Infinite");
		m_simulationTimeEdit->setClearButtonEnabled(false);
		m_simulationTimeEdit->setToolTip("Simulation time in seconds.\n\"0\" - at least one workcycle.\nClear the field for an infinite "
										 "simulation (till Stop or Pause).\nExamples: \"0.500\" - 500ms, \"60\" - 1min, \"3600\" - 1hour.");
		m_simulationTimeEdit->setSizePolicy(QSizePolicy::Policy::Minimum, m_simulationTimeEdit->sizePolicy().verticalPolicy());
		m_simulationTimeEdit->setMaxLength(18);

		QFontMetrics fm(m_simulationTimeEdit->font());
		int pixelWidth = fm.horizontalAdvance("0000000000.000");
		m_simulationTimeEdit->setMaximumWidth(pixelWidth);

		m_simulationTimeLocale.setNumberOptions(m_simulationTimeLocale.numberOptions() & ~(QLocale::OmitGroupSeparator));
		m_simulationTimeEditValidator.setLocale(m_simulationTimeLocale);
		m_simulationTimeEditValidator.setNotation(QDoubleValidator::Notation::StandardNotation);
		m_simulationTimeEditValidator.setBottom(0.001);
		m_simulationTimeEditValidator.setDecimals(3);

		m_simulationTimeEdit->setValidator(&m_simulationTimeEditValidator);

		if (m_slaveWindow == true)
		{
			Q_ASSERT(m_masterWindow != nullptr);
			Q_ASSERT(m_masterWindow->m_simulationTimeEdit != nullptr);

			m_simulationTimeEdit->setText(m_masterWindow->m_simulationTimeEdit->text());
		}

		// --
		//
		m_speedComboBox = new QComboBox{this};
		m_speedComboBox->setToolTip(
			"Simulation speed factor.\nFF - Fast Forward.\nNote: Simulation speed depends on hardware and project complexity.");
		m_speedComboBox->addItem("x0.1", QVariant{0.1});
		m_speedComboBox->addItem("x0.25", QVariant{0.25});
		m_speedComboBox->addItem("x0.5", QVariant{0.5});
		m_speedComboBox->addItem("x1", QVariant{1.0});
		m_speedComboBox->addItem("x2", QVariant{2.0});
		m_speedComboBox->addItem("x4", QVariant{4.0});
		m_speedComboBox->addItem("FF", QVariant{256.0});
		m_speedComboBox->setCurrentIndex(3); // x1

		auto speedChangedFunc = [this](int)
		{
			bool ok;
			double d = m_speedComboBox->currentData().toDouble(&ok);
			m_simulator->control().setSpeedFactor(ok ? d : 1.0);
		};

		if (m_slaveWindow == true)
		{
			Q_ASSERT(m_masterWindow != nullptr);
			Q_ASSERT(m_masterWindow->m_speedComboBox != nullptr);

			for (auto cb : s_speedComboBoxes)
			{
				Q_ASSERT(cb);

				connect(cb,
						&QComboBox::currentIndexChanged,
						[this](int index)
						{
							m_speedComboBox->blockSignals(true);
							m_speedComboBox->setCurrentIndex(index);
							m_speedComboBox->blockSignals(false);
						});

				connect(m_speedComboBox,
						&QComboBox::currentIndexChanged,
						[cb](int index)
						{
							cb->blockSignals(true);
							cb->setCurrentIndex(index);
							cb->blockSignals(false);
						});
			}
		}

		s_speedComboBoxes.push_back(m_speedComboBox);

		connect(m_speedComboBox, &QComboBox::currentIndexChanged, speedChangedFunc);

		if (m_slaveWindow == false)
		{
			// This is a master window, so we need to init simulator
			//
			speedChangedFunc(3); // Call first time to init m_simulator, x1
		}
		else
		{
			Q_ASSERT(m_masterWindow != nullptr);
			Q_ASSERT(m_masterWindow->m_speedComboBox != nullptr);

			m_speedComboBox->blockSignals(true);
			m_speedComboBox->setCurrentIndex(m_masterWindow->m_speedComboBox->currentIndex());
			m_speedComboBox->blockSignals(false);
		}

		// --
		//
		m_runAction = new QAction{QIcon(":/SimulatorUi/Images/SimRun.svg"), tr("Run simulation for complete project"), this};
		QList<QKeySequence> runsKeys;
		runsKeys << QKeySequence{Qt::CTRL | Qt::Key_R};
		runsKeys << QKeySequence{Qt::CTRL | Qt::Key_F5};
		m_runAction->setShortcuts(runsKeys);
		connect(m_runAction, &QAction::triggered, this, &SimWidgetPrivate::runSimulation);

		m_pauseAction = new QAction{QIcon(":/SimulatorUi/Images/SimPause.svg"), tr("Pause current simulation"), this};
		connect(m_pauseAction, &QAction::triggered, this, &SimWidgetPrivate::pauseSimulation);

		m_stopAction = new QAction{QIcon(":/SimulatorUi/Images/SimStop.svg"), tr("Stop current simulation"), this};
		m_stopAction->setShortcut(Qt::SHIFT | Qt::Key_F5);
		connect(m_stopAction, &QAction::triggered, this, &SimWidgetPrivate::stopSimulation);

		if (m_slaveWindow == false)
		{
			m_allowLanComm = new QAction{QIcon(":/SimulatorUi/Images/SimAllowRegData.svg"),
										 tr("Allow LogicModules' Application Data transmitting to AppDataSrv"),
										 this};
			m_allowLanComm->setCheckable(true);
			m_allowLanComm->setChecked(m_simulator->software().enabled() && m_simulator->service().enabled());
			connect(m_allowLanComm, &QAction::toggled, this, &SimWidgetPrivate::allowLanCommToggled);
		}
		else
		{
			Q_ASSERT(m_masterWindow);
			Q_ASSERT(m_masterWindow->m_allowLanComm);

			m_allowLanComm = m_masterWindow->m_allowLanComm;
		}

		m_profilesComboBox = new QComboBox{};
		m_profilesComboBox->setMinimumContentsLength(15);

		if (m_slaveWindow == true)
		{
			Q_ASSERT(m_masterWindow != nullptr);
			Q_ASSERT(m_masterWindow->m_profilesComboBox != nullptr);

			for (auto cb : s_profilesComboBoxes)
			{
				Q_ASSERT(cb);

				connect(cb,
						&QComboBox::currentIndexChanged,
						[this](int index)
						{
							m_profilesComboBox->blockSignals(true);
							m_profilesComboBox->setCurrentIndex(index);
							m_profilesComboBox->blockSignals(false);
						});

				connect(m_profilesComboBox,
						&QComboBox::currentIndexChanged,
						[cb](int index)
						{
							cb->blockSignals(true);
							cb->setCurrentIndex(index);
							cb->blockSignals(false);
						});
			}
		}

		s_profilesComboBoxes.push_back(m_profilesComboBox);

		connect(m_profilesComboBox, &QComboBox::currentTextChanged, this, &SimWidgetPrivate::profileComboTextChanged);

		m_trendsAction = new QAction{QIcon(":/SimulatorUi/Images/SimTrends.svg"), tr("Trends"), this};
		m_trendsAction->setEnabled(true);
		m_trendsAction->setData(
			QVariant("IAmIndependentTrend")); // This is required to find this action in MonitorToolBar for drag and drop
		connect(m_trendsAction, &QAction::triggered, this, &SimWidgetPrivate::showTrends);

		m_findSignalAction = new QAction{QIcon(":/SimulatorUi/Images/SimFindSignal.svg"), tr("Find Signal"), this};
		m_findSignalAction->setEnabled(true);
		m_findSignalAction->setShortcut(QKeySequence::Find);
		connect(m_findSignalAction, &QAction::triggered, this, &SimWidgetPrivate::showFindSignal);


		m_schemaListAction = new QAction{QIcon(":/SimulatorUi/Images/SchemaList.svg"), tr("Show All Schemas"), this};
		m_schemaListAction->setEnabled(true);
		connect(m_schemaListAction, &QAction::triggered, this, &SimWidgetPrivate::openAppSchemasTabPage);

		m_snapshotAction = new QAction{QIcon(":/SimulatorUi/Images/SimSnapshot.svg"), tr("Signals Snapshot"), this};
		m_snapshotAction->setEnabled(true);
		connect(m_snapshotAction, &QAction::triggered, this, &SimWidgetPrivate::showSnapshot);

		// --
		//
		m_timeIndicator = new QLabel;

#if defined(Q_OS_WIN)
		QFont f = QFont("Consolas");
#else
		QFont f = QFont("Courier");
#endif
		m_timeIndicator->setFont(f);

		if (m_slaveWindow == false)
		{
			updateTimeIndicator(Sim::ControlStatus{});
		}
		else
		{
			Q_ASSERT(m_masterWindow);
			Q_ASSERT(m_masterWindow->m_timeIndicator);

			m_timeIndicator->setText(m_masterWindow->m_timeIndicator->text());
		}

		// --
		//
		m_toolBar->addAction(m_openProjectAction);
		m_toolBar->addAction(m_closeProjectAction);
		m_toolBar->addAction(m_refreshProjectAction);
		m_toolBar->addAction(m_addWindowAction);

		m_toolBar->addSeparator();
		m_toolBar->addAction(m_takeSnapshotAction);
		m_toolBar->addAction(m_applySnapshotAction);

		m_toolBar->addSeparator();
		m_toolBar->addWidget(m_simulationTimeEdit);
		m_toolBar->addWidget(m_speedComboBox);
		m_toolBar->addAction(m_runAction);
		m_toolBar->addAction(m_pauseAction);
		m_toolBar->addAction(m_stopAction);

		m_toolBar->addSeparator();
		m_toolBar->addWidget(m_timeIndicator);

		m_toolBar->addSeparator();
		m_toolBar->addAction(m_allowLanComm);
		m_toolBar->addWidget(m_profilesComboBox);

		m_toolBar->addSeparator();
		m_toolBar->addAction(m_schemaListAction);
		m_toolBar->addAction(m_snapshotAction);
		m_toolBar->addAction(m_findSignalAction);

		m_toolBar->addSeparator();
		m_toolBar->addAction(m_trendsAction);

		// --
		//
		QWidget* trendsActionWidget = m_toolBar->widgetForAction(m_trendsAction);
		assert(trendsActionWidget);

		trendsActionWidget->setAcceptDrops(true);

		return;
	}

	void SimWidgetPrivate::createDocks()
	{
		setCorner(Qt::Corner::BottomLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);
		setCorner(Qt::Corner::BottomRightCorner, Qt::DockWidgetArea::BottomDockWidgetArea);
		setCorner(Qt::Corner::TopRightCorner, Qt::DockWidgetArea::RightDockWidgetArea);
		setCorner(Qt::Corner::TopLeftCorner, Qt::DockWidgetArea::LeftDockWidgetArea);

		// Project dock
		//
		QDockWidget* projectDock = new QDockWidget{"SimProjectBuild", this};
		projectDock->setObjectName(projectDock->windowTitle());
		projectDock->setFeatures(QDockWidget::NoDockWidgetFeatures);
		projectDock->setTitleBarWidget(new QWidget{}); // Hides title bar

		m_projectWidget = new SimProjectWidget{m_simulator.get()};
		projectDock->setWidget(m_projectWidget);

		addDockWidget(Qt::LeftDockWidgetArea, projectDock);

		// Overriden Signals dock
		//
		m_overridePaneDock = new QDockWidget{"Overrides", this};
		m_overridePaneDock->setObjectName("SimOverridenSignals");
		m_overridePaneDock->setWidget(new SimOverridePane{*m_simulator->simulator(), m_propertyStorage, m_overridePaneDock});
		m_overridePaneDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

		addDockWidget(Qt::BottomDockWidgetArea, m_overridePaneDock);

		// OutputLog dock
		//
		if (m_slaveWindow == false)
		{
			m_outputPaneDock = new QDockWidget{"Output", this};
			m_outputPaneDock->setObjectName("SimOutputWidget");

			m_outputWidget = new SimOutputWidget{m_outputPaneDock};

			m_outputPaneDock->setWidget(m_outputWidget);
			m_outputPaneDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);

			addDockWidget(Qt::BottomDockWidgetArea, m_outputPaneDock);
		}

		// Notification dock widget
		//
		if (m_slaveWindow == false)
		{
			m_notificationPanel = new NotificationPanel{this};
			addDockWidget(Qt::TopDockWidgetArea, m_notificationPanel);
		}

		return;
	}

	void SimWidgetPrivate::showEvent(QShowEvent* e)
	{
		QMainWindow::showEvent(e);
		e->ignore();

		if (m_showEventFired == false)
		{
			// Restore docks states only after show event, otherwise the _floated_ docks will be behind main window
			//
			if (m_slaveWindow == false)
			{
				QVariant v = QSettings().value("SimWidget/state");
				if (v.isValid() == true)
				{
					restoreState(v.toByteArray());

					QList<QDockWidget*> dockWidgets = findChildren<QDockWidget*>();
					for (QDockWidget* dw : dockWidgets)
					{
						restoreDockWidget(dw);
						dw->setVisible(true);
					}
				}

				m_toolBar->setVisible(true);

				Q_ASSERT(m_notificationPanel);
				m_notificationPanel->hide();
			}
		}

		m_showEventFired = true;

		return;
	}

	void SimWidgetPrivate::timerEvent([[maybe_unused]] QTimerEvent* event)
	{
		// Put any other timer events here, if needed, before checking for project updates.
		// Checking project uses `return` for control flow, so it should be the last in this function.
		// `return` instead of throw is used to avoid outputting error message in debug output.
		//
		if (m_slaveWindow == false && m_simulator->isLoaded() == true)
		{
			Q_ASSERT(m_notificationPanel);

			// Check if the build was updated and rise notification if it was.
			//
			QString path = m_simulator->buildPath();

			QString buildXmlPath = m_simulator->buildPath() + File::SLASH_BUILD_XML;
			QFile buildXmlFile(buildXmlPath);

			try
			{
				if (buildXmlFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
				{
					throw 1;
				}

				QXmlStreamReader xml{buildXmlFile.readAll()};

				/*
				<Build>
					<BuildInfo Project="cdu" ID="258" Date="23.01.2025 16:28:30" Changeset="0" User="Administrator"
				Workstation="SERHIY-TP17P"/> <Files Count="112"> <File Name="/Common/AppSignals.asgs" ID="APP_SIGNAL_SET" Tag=""
				Compressed="Yes" Size="8716" MD5="057327e32266fb553fb1b341526622de"/>
						...
					</Files>
					<BuildResult Errors="0" Warnings="4"/>
				</Build>
				*/

				// Read BuildInfo, ID.
				//
				int buildNo = 0;

				while (!xml.atEnd() && !xml.hasError())
				{
					QXmlStreamReader::TokenType token = xml.readNext();

					if (token == QXmlStreamReader::StartElement)
					{
						if (xml.name() == XmlElement::BUILD_INFO)
						{
							QString buildInfoID = xml.attributes().value("ID").toString();
							if (buildInfoID.isEmpty())
							{
								throw 1;
							}

							buildNo = buildInfoID.toInt();
							if (buildNo == m_simulator->buildNo())
							{
								// Build has not been changed.
								//
								// throw 0; - commented out to avoid outputting error message in debug output.
								return;
							}
						}
						else
						{
							if (xml.name() == XmlElement::BUILD_RESULT)
							{
								QString buildResultErrors = xml.attributes().value(XmlAttribute::ERRORS).toString();
								if (buildResultErrors.isEmpty() || buildResultErrors.toInt() != 0)
								{
									throw 2;
								}
							}
						}
					}
				}

				if (xml.hasError())
				{
					throw 3;
				}

				// Build is updated, show notification.
				//
				std::function<void(QString)> reloadProject = [this](QString link)
				{
					m_notificationPanel->hide();

					if (link == "reload_project")
					{
						refreshBuild();
					}
				};

				m_notificationPanel->showNotification(
					tr("The current project build has been updated to #%1. <a href='reload_project'>Reload Build</a>").arg(buildNo),
					m_toolBar->height(),
					std::move(reloadProject));
			}
			catch (...)
			{
			}
		}

		return;
	}

	void SimWidgetPrivate::projectOpened()
	{
		emit needCloseChildWindows();
		emit needUpdateActions();
	}

	void SimWidgetPrivate::projectClosed()
	{
		emit needCloseChildWindows();
	}

	void SimWidgetPrivate::openBuild()
	{
		m_simulator->control().stop();

		// This is an external function, it should be implemented in the main application.
		//
		if (!m_getProjectPathFunc)
		{
			Q_ASSERT(m_getProjectPathFunc);
			return;
		}

		QString path = m_getProjectPathFunc(this);

		if (path.isEmpty() == false)
		{
			bool loadOk = loadBuild(path);

			if (loadOk == true)
			{
				m_propertyStorage.saveProperty("SimulatorWidget/ProjectLastPath/" + m_simulator->projectName().toLower(), path);
			}
		}

		emit needUpdateActions();
		return;
	}

	void SimWidgetPrivate::closeBuild()
	{
		m_simulator->control().stop();

		m_simulator->clear();
		emit needUpdateActions();

		SimBasePage::deleteAllPages();

		if (m_outputWidget != nullptr)
		{
			// m_outputWidget exist only for the main simulator tab page.
			//
			m_outputWidget->clear();
		}

		return;
	}

	void SimWidgetPrivate::refreshBuild()
	{
		m_simulator->control().stop();

		if (m_outputWidget != nullptr) // Detached window does not have OutputWidget
		{
			m_outputWidget->clear();
		}

		QString buildPath = m_simulator->buildPath();
		if (buildPath.isEmpty() == true)
		{
			QString project = m_simulator->projectName().toLower();
			buildPath = m_propertyStorage.loadProperty("SimulatorWidget/ProjectLastPath/" + project);
		}

		if (buildPath.isEmpty() == false)
		{
			loadBuild(buildPath);
		}

		emit needUpdateActions();
		return;
	}

	void SimWidgetPrivate::aboutToQuit()
	{
		if (m_slaveWindow == false)
		{
			stopSimulation(true);
		}

		if (m_slaveWindow == false && m_showEventFired == true)
		{
			QSettings().setValue("SimWidget/state", saveState());
			qDebug() << "SimWidgetPrivate::aboutToQuit(): saveState()";
		}

		return;
	}

	void SimWidgetPrivate::controlStateChanged(Sim::SimControlState /*state*/)
	{
		updateActions();
	}

	void SimWidgetPrivate::updateTimeIndicator(Sim::ControlStatus state)
	{
		using namespace std::chrono;

		Q_ASSERT(m_timeIndicator);

		milliseconds durration = duration_cast<milliseconds>(state.m_duration);

		qint64 days = durration.count() / 1_day;
		qint64 hours = (durration.count() % 1_day) / 1_hour;
		qint64 minutes = (durration.count() % 1_hour) / 1_min;
		qint64 seconds = (durration.count() % 1_min) / 1_sec;
		qint64 millisecond = durration.count() % 1_sec;

		auto ms = duration_cast<milliseconds>(state.m_currentTime);
		QDateTime utcOffset = QDateTime::currentDateTime();
		TimeStamp plantTime{ms.count() + utcOffset.offsetFromUtc() * 1000};

		QDateTime currentTime = plantTime.toDateTime();

		if (currentTime.date().year() == 1970)
		{
			currentTime = QDateTime::currentDateTime();
		}

#if 1
		// IF UNCOMMENTING THIS CODE
		// and if you want to show milliseconds,
		// THEN do not forget to send message more frequently
		// in Sim::Control::processRun emit statusUpdate(ControlStatus{cd});
		//
		//        0d 00:20:03.580
		// 05/17/2020 15:18:59.335

		QString dateText = QString("%6 %7")
							   .arg(DateTimeToString::date(currentTime.date()))
							   .arg(DateTimeToString::timeMs(currentTime.time()));

		QString text = tr("%1d %2:%3:%4.%5\n%6")
						   .arg(days, static_cast<int>(dateText.size() - 14), 10, QChar(' '))
						   .arg(hours, 2, 10, QChar('0'))
						   .arg(minutes, 2, 10, QChar('0'))
						   .arg(seconds, 2, 10, QChar('0'))
						   .arg(millisecond, 3, 10, QChar('0'))
						   .arg(dateText);
#else
		//        0d 00:20:03
		// 05/17/2020 15:18:59

		QString dateText = QString("%6 %7")
							   .arg(DateTimeToString::date(currentTime.date()))
							   .arg(DateTimeToString::timeSec(currentTime.time()));

		QString text = tr("%1d %2:%3:%4\n%6")
						   .arg(days, static_cast<int>(dateText.size()) - 10, 10, QChar(' '))
						   .arg(hours, 2, 10, QChar('0'))
						   .arg(minutes, 2, 10, QChar('0'))
						   .arg(seconds, 2, 10, QChar('0'))
						   .arg(dateText);
#endif

		m_timeIndicator->setText(text);

		return;
	}

	void SimWidgetPrivate::updateActions()
	{
		bool projectIsLoaded = m_simulator->isLoaded();

		{
			m_openProjectAction->setEnabled(true);
			m_closeProjectAction->setEnabled(projectIsLoaded);

			QString project = m_simulator->projectName().toLower();
			QString lastPath = m_propertyStorage.loadProperty("SimulatorWidget/ProjectLastPath/" + project);
			bool lastPathExists = QDir(lastPath).exists() == true && lastPath.isEmpty() == false;

			m_refreshProjectAction->setEnabled(lastPathExists);
			m_addWindowAction->setEnabled(projectIsLoaded);
		}

		// Snapshot
		//
		{
			m_takeSnapshotAction->setEnabled(m_simulator->isPaused() || m_simulator->isRunning());
			m_applySnapshotAction->setEnabled(m_simulator->isLoaded() && m_simulator->isStopped());
		}

		// Run, Pause, Stop
		//
		{
			m_simulationTimeEdit->setEnabled((m_simulator->isStopped() == true || m_simulator->isPaused()) && projectIsLoaded == true);
			m_speedComboBox->setEnabled(projectIsLoaded);

			m_runAction->setEnabled((m_simulator->isStopped() == true || m_simulator->isPaused()) && projectIsLoaded == true);
			m_pauseAction->setEnabled(m_simulator->isRunning() == true && projectIsLoaded == true);
			m_stopAction->setEnabled(m_simulator->isStopped() == false && projectIsLoaded == true);

			m_timeIndicator->setEnabled(m_simulator->isStopped() == false && projectIsLoaded == true);
		}

		// Update profile combo box
		//
		{
			m_allowLanComm->setEnabled(projectIsLoaded);
			m_profilesComboBox->setEnabled(m_simulator->isStopped() == true && projectIsLoaded == true);

			bool hasLastSelected = false;
			QString lastSelectedProfile;

			if (projectIsLoaded == true)
			{
				lastSelectedProfile =
					QSettings().value(QString("SimWidget/lastSelectedProfile_%1").arg(m_simulator->projectName())).toString();
			}

			m_profilesComboBox->blockSignals(true);
			m_profilesComboBox->clear();

			if (projectIsLoaded == true)
			{
				QStringList profiles;

				profiles += m_simulator->profiles().profiles();

				for (QString p : profiles)
				{
					m_profilesComboBox->addItem(p, p);

					if (p.compare(lastSelectedProfile, Qt::CaseInsensitive) == 0)
					{
						hasLastSelected = true;
					}
				}
			}

			m_profilesComboBox->blockSignals(false);

			if (projectIsLoaded == true && hasLastSelected == true)
			{
				m_profilesComboBox->setCurrentText(lastSelectedProfile);
			}
		}

		return;
	}

	void SimWidgetPrivate::runSimulation()
	{
		if (m_simulator->isLoaded() == false)
		{
			qDebug() << "SimWidgetPrivate::runSimulation(): Project is not loaded";
			m_simulator->log()->writeError("Cannot start simulation, project is not loaded.");
			return;
		}

		if (m_simulator->isRunning() == true)
		{
			qDebug() << "SimWidgetPrivate::runSimulation(): Simulation is already running";
			return;
		}

		// Trim all trends only if it is the start of the new simulation.
		//
		if (m_simulator->isStopped() == true)
		{
			TimeStamp currentTime{QDateTime::currentDateTime()};
			auto trimFunc = [currentTime](SimTrendsWidget* simTrendsWidget)
			{
// Choose what to do, trim or clear.
//
#if 1
				simTrendsWidget->trimTrendData(currentTime);
				simTrendsWidget->addNonValidPoints();
#else
				simTrendsWidget->clear();
#endif
			};

			SimTrends::applyForAll(trimFunc);
		}

		// Get simulation time
		//
		std::chrono::microseconds duration{-1};

		if (QString simTimeText = m_simulationTimeEdit->text(); simTimeText.isEmpty() == false)
		{
			QStringList splitted = simTimeText.split(m_simulationTimeLocale.decimalPoint());
			QString secondsText;
			QString millisecondsText;

			if (splitted.size() >= 1)
			{
				secondsText = splitted[0].remove(m_simulationTimeLocale.groupSeparator());
			}

			if (splitted.size() == 2)
			{
				millisecondsText = splitted[1];
			}

			if (secondsText.isEmpty() == false)
			{
				bool ok;
				auto s = secondsText.toULongLong(&ok, 10);

				if (ok == true)
				{
					duration = std::chrono::seconds{s};
				}
			}

			if (millisecondsText.isEmpty() == false)
			{
				uint64_t order = 100;
				uint64_t ms = 0;

				for (qsizetype i = 0; i < millisecondsText.size(); i++)
				{
					QChar ch = millisecondsText[i];
					Q_ASSERT(ch.isDigit());

					ms += ch.digitValue() * order;
					order /= 10;
				}

				if (duration.count() < 0)
				{
					duration = std::chrono::microseconds{0};
				}

				duration += std::chrono::milliseconds{ms};
			}
		}

		if (duration.count() == 0)
		{
			duration = std::chrono::microseconds{1}; // It will run one work cycle
		}

		// Set profile to simulator
		//
		Q_ASSERT(m_profilesComboBox);

		QString selectedProfile = m_profilesComboBox->currentText();

		if (bool ok = m_simulator->setCurrentProfile(selectedProfile); ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Profile %1 not found").arg(selectedProfile));
			return;
		}

		// --
		//
		Sim::Control& mutableControl = m_simulator->control();

		if (m_simulator->isPaused() == true)
		{
			mutableControl.startSimulation(duration);
		}
		else
		{
			m_simulator->appSignalManager().resetRam(); // It prevents from short show of previous run results

			// Star simulation for all project
			//
			mutableControl.reset();

			// Get all modules to simulation
			//
			QStringList equipmentIds;
			auto lms = m_simulator->logicModules();

			for (const auto& lm : lms)
			{
				equipmentIds << lm.equipmentId();
			}

			if (equipmentIds.isEmpty() == true)
			{
				m_simulator->log()->writeWarning(tr("Nothing to simulate, no LogicModules are found."));
				// Nothing to simulate
				//
				return;
			}

			// Start simulation
			//
			mutableControl.setRunList(equipmentIds);
			mutableControl.startSimulation(duration);
		}

		return;
	}

	void SimWidgetPrivate::pauseSimulation()
	{
		qDebug() << "SimWidgetPrivate::pauseSimulation()";

		if (m_simulator->isLoaded() == false)
		{
			return;
		}

		if (m_simulator->isRunning() == false)
		{
			return;
		}

		Sim::Control& control = m_simulator->control();
		control.pause();

		return;
	}

	void SimWidgetPrivate::stopSimulation(bool stopSimulationThread)
	{
		qDebug() << "SimWidgetPrivate::stopSimulation()";

		if (m_simulator->isLoaded() == false)
		{
			return;
		}

		if (m_simulator->isRunning() == false && m_simulator->isPaused() == false)
		{
			return;
		}

		Sim::Control& control = m_simulator->control();
		control.stop();

		if (stopSimulationThread == true)
		{
			control.stopThread();
		}

		return;
	}

	void SimWidgetPrivate::allowLanCommToggled(bool state)
	{
		m_simulator->software().setEnabled(state);
		m_simulator->service().setEnabled(state);
		return;
	}

	void SimWidgetPrivate::profileComboTextChanged(QString text)
	{
		if (m_simulator->isLoaded() == true && text.isEmpty() == false)
		{
			QSettings{}.setValue(QString("SimWidget/lastSelectedProfile_%1").arg(m_simulator->projectName()), text);
		}

		return;
	}

	void SimWidgetPrivate::showSnapshot()
	{
		// 1. Use this->m_appSignalController for getting signal list and state
		// 2. this->m_simulator (Sim::Simulator) has signal 'projectUpdated' and function 'isLoaded()' use these to
		//	  update signal list
		// 3. You can pass and store 'this->m_appSignalController' and 'this->m_simulator.get()' to your function
		//    it is guarantee will not be deleted
		//

		SimDialogSignalSnapshot::showDialog(m_simulator.get(), m_appSignalController, QString(), this);

		return;
	}

	void SimWidgetPrivate::showFindSignal()
	{
		// 1. Use this->m_appSignalController for getting signal list and state
		// 2. this->m_simulator (Sim::Simulator) has signal 'projectUpdated' and function 'isLoaded()' use these to
		//	  update signal list
		// 3. You can pass and store 'this->m_appSignalController' and 'this->m_simulator.get()'  to your function
		//    it is guarantee will not be deleted
		//
		auto dsi = new SchemaClientLib::DialogSignalSearch(this, &m_appSignalController->appSignalManager());

		connect(m_simulator.get(), &SimIdeSimulator::projectUpdated, dsi, &SchemaClientLib::DialogSignalSearch::signalsUpdated);

		connect(dsi, &SchemaClientLib::DialogSignalSearch::signalContextMenu, this, &SimWidgetPrivate::signalContextMenu);
		connect(dsi, &SchemaClientLib::DialogSignalSearch::signalInfo, this, &SimWidgetPrivate::signalInfo);

		connect(this, &SimWidgetPrivate::needCloseChildWindows, dsi, &QDialog::accept);

		dsi->show();

		return;
	}

	void SimWidgetPrivate::showTrends()
	{
		// Get Trends list
		//
		std::vector<SimTrendsWidget*> trends = SimTrends::getTrendsList();

		// Choose trend
		//
		SimTrendsWidget* trendToActivate = nullptr;

		if (trends.empty() == true)
		{
			trendToActivate = nullptr; // if trendToActivate is nullptr, then create a new trend.
		}
		else
		{
			QMenu menu;

			QAction* newTrendAction = menu.addAction("New Trend...");
			newTrendAction->setData(QVariant::fromValue<int>(-1)); // Data -1 means, create new trend widget

			menu.addSeparator();

			for (size_t i = 0; i < trends.size(); i++)
			{
				QAction* a = menu.addAction(trends[i]->windowTitle());
				Q_ASSERT(a);

				a->setData(QVariant::fromValue<int>(static_cast<int>(i))); // Data is index in trend vector
			}

			QAction* triggeredAction = menu.exec(QCursor::pos());
			if (triggeredAction == nullptr)
			{
				return;
			}

			QVariant data = triggeredAction->data();

			bool ok = false;
			int trendIndex = data.toInt(&ok);

			if (trendIndex == -1)
			{
				trendToActivate = nullptr; // if trendToActivate is nullptr, then create a new trend.
			}
			else
			{
				if (ok == false || trendIndex < 0 || trendIndex >= static_cast<int>(trends.size()))
				{
					Q_ASSERT(ok == true);
					Q_ASSERT(trendIndex >= 0 && trendIndex < static_cast<int>(trends.size()));
					return;
				}

				trendToActivate = trends.at(trendIndex);
			}
		}

		// Start new trend or activate chosen one
		//
		if (trendToActivate == nullptr)
		{
			std::vector<AppSignalParam> appSignals;
			SimTrends::startTrendApp(m_simulator, appSignals, this);
		}
		else
		{
			SimTrends::activateTrendWindow(trendToActivate);
		}

		return;
	}

	bool SimWidgetPrivate::loadBuild(QString buildPath)
	{
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		bool ok = m_simulator->load(buildPath);

		QApplication::restoreOverrideCursor();

		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Cannot open project for simulation. For details see Output window."));
		}

		return ok;
	}

	void SimWidgetPrivate::addNewWindow()
	{
		qDebug() << "SimulatorWidget::addNewWindow()";

		SimWidget* widget = new SimWidget{m_ideLogFile,
										  m_simulator,
										  m_getProjectPathFunc,
										  m_propertyStorage,
										  m_dbProjectStateNotifier,
										  parentWidget(),
										  Qt::Window,
										  true,
										  m_masterWindow};
		widget->setWindowTitle(tr("Simulator"));

		widget->show();

		return;
	}

	void SimWidgetPrivate::takeSnapshot()
	{
		static QString lastUsed = m_simulator->projectName();
		static QString lastProject = m_simulator->projectName();

		if (lastProject != m_simulator->projectName())
		{
			lastUsed = m_simulator->projectName();
			lastProject = m_simulator->projectName();
		}

		QString fileName = QFileDialog::getSaveFileName(this,
														tr("Save Snapshot"),
														QString("%1.u7snap").arg(lastUsed),
														tr("Snapshot files (*.u7snap);;All Files (*.*)"));
		if (fileName.isEmpty() == true)
		{
			return;
		}

		QFileInfo fileInfo{fileName};
		QString snapshotId = fileInfo.completeBaseName();
		lastUsed = snapshotId;

		QApplication::setOverrideCursor(Qt::WaitCursor);
		auto data = m_simulator->control().takeSnapshot(snapshotId);
		QApplication::restoreOverrideCursor();

		if (data.isEmpty() == true)
		{
			QMessageBox::critical(this, qAppName(), tr("Snapshot failed. See the application log for details."));
			return;
		}

		// Save data to file with name snapshotId.u7snap
		//
		QFile file{fileName};

		if (file.open(QIODevice::WriteOnly) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to save snapshot file."));
			return;
		}

		auto written = file.write(data);
		file.close();

		if (written != data.size())
		{
			QMessageBox::critical(this, qAppName(), tr("Disk write error, file %1.").arg(file.fileName()));
			return;
		}
	}

	void SimWidgetPrivate::applySnapshot()
	{
		QString fileName = QFileDialog::getOpenFileName(this,
														tr("Open Snapshot"),
														QString("%1.u7snap").arg(m_simulator->projectName()),
														tr("Snapshot files (*.u7snap);;All Files (*.*)"));
		if (fileName.isEmpty() == true)
		{
			return;
		}

		QFile file{fileName};
		if (file.open(QIODevice::ReadOnly) == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to open snapshot file."));
			return;
		}

		QApplication::setOverrideCursor(Qt::WaitCursor);
		bool ok = m_simulator->control().applySnapshot(file.readAll());
		QApplication::restoreOverrideCursor();

		if (ok == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Failed to apply snapshot."));
			return;
		}

		return;
	}

	void SimWidgetPrivate::openLogicModuleTabPage(QString lmEquipmentId)
	{
		if (m_simulator->isLoaded() == false)
		{
			return;
		}

		// Check if such SimulatorControlPage already exists
		//
		SimLogicModulePage* cp = SimBasePage::logicModulePage(lmEquipmentId, m_tabWidget);

		if (cp != nullptr)
		{
			int tabIndex = m_tabWidget->indexOf(cp);
			if (tabIndex != -1)
			{
				m_tabWidget->setCurrentIndex(tabIndex);
			}
			else
			{
				cp->show();
				cp->activateWindow();
			}

			return;
		}

		// Create new SimLogicModulePage
		//
		auto logicModule = m_simulator->logicModule(lmEquipmentId);
		if (logicModule.has_value() == false)
		{
			assert(logicModule);
			return;
		}
		assert(lmEquipmentId == logicModule->equipmentId());

		SimLogicModulePage* controlPage = new SimLogicModulePage{m_simulator.get(), m_appSignalController, lmEquipmentId, m_tabWidget};

		int tabIndex = m_tabWidget->addTab(controlPage, lmEquipmentId);
		m_tabWidget->setTabIcon(tabIndex, QIcon{QPixmap{":/SimulatorUi/Images/SimLogicModuleIcon.svg"}});

		m_tabWidget->setCurrentIndex(tabIndex);

		connect(controlPage, &SimLogicModulePage::openSchemaRequest, this, &SimWidgetPrivate::openSchemaTabPage);
		connect(controlPage, &SimLogicModulePage::openCodePageRequest, this, &SimWidgetPrivate::openCodeTabPage);

		return;
	}

	void SimWidgetPrivate::openCodeTabPage(QString lmEquipmentId)
	{
		auto lm = m_simulator->logicModule(lmEquipmentId);
		if (lm.has_value() == false)
		{
			QMessageBox::critical(this, qAppName(), tr("Cannot find LogicModule %1").arg(lmEquipmentId));
			return;
		}

		SimCodePage* page = new SimCodePage{m_simulator.get(), lmEquipmentId, m_tabWidget};

		int tabIndex = m_tabWidget->addTab(page, lmEquipmentId);
		m_tabWidget->setCurrentIndex(tabIndex);

		return;
	}

	void SimWidgetPrivate::openConnectionTabPage(QString connectionId)
	{
		if (m_simulator->isLoaded() == false)
		{
			return;
		}

		// Check if such SimulatorControlPage already exists
		//
		SimConnectionPage* cp = SimBasePage::connectionPage(connectionId, m_tabWidget);

		if (cp != nullptr)
		{
			int tabIndex = m_tabWidget->indexOf(cp);
			if (tabIndex != -1)
			{
				m_tabWidget->setCurrentIndex(tabIndex);
			}
			else
			{
				cp->show();
				cp->activateWindow();
			}

			return;
		}

		// Create new SimConnectionPage
		//
		SimConnectionPage* page = new SimConnectionPage{m_simulator.get(), connectionId, m_tabWidget};

		int tabIndex = m_tabWidget->addTab(page, connectionId);
		m_tabWidget->setTabIcon(tabIndex, QIcon{QPixmap{":/SimulatorUi/Images/SimConnectionIcon.svg"}});

		m_tabWidget->setCurrentIndex(tabIndex);

		return;
	}

	void SimWidgetPrivate::openAppSchemasTabPage()
	{
		if (m_simulator->isLoaded() == false)
		{
			return;
		}

		// Check if such SimulatorControlPage already exists
		//
		SimSelectSchemaPage* cp = SimBasePage::selectSchemaPage(m_tabWidget);

		if (cp != nullptr)
		{
			int tabIndex = m_tabWidget->indexOf(cp);
			if (tabIndex != -1)
			{
				m_tabWidget->setCurrentIndex(tabIndex);
			}
			else
			{
				cp->show();
				cp->activateWindow();
			}

			return;
		}

		// Create new SimConnectionPage
		//
		SimSelectSchemaPage* page = new SimSelectSchemaPage{m_simulator.get(), m_tabWidget};

		int tabIndex = m_tabWidget->addTab(page, tr("AppLogic Schemas"));
		m_tabWidget->setTabIcon(tabIndex, QIcon{QPixmap{":/SimulatorUi/Images/SimAppLogicSchemas.svg"}});
		m_tabWidget->setTabToolTip(0,
								   tr("Application Logic Schemas\n"
									  "[CTRL + `]"));

		m_tabWidget->setCurrentIndex(tabIndex);

		connect(page, &SimSelectSchemaPage::openSchemaTabPage, this, &SimWidgetPrivate::openSchemaTabPage);

		return;
	}

	void SimWidgetPrivate::tabCloseRequest(int index)
	{
		QByteArray state = saveState();

		QWidget* w = m_tabWidget->widget(index);
		assert(w);

		delete w;

		restoreState(state);
		return;
	}

	void SimWidgetPrivate::tabCurrentChanged(int index)
	{
		// Show/hide close button for inactive tab bar
		//
		QTabBar::ButtonPosition closeSide =
			(QTabBar::ButtonPosition)style()->styleHint(QStyle::SH_TabBar_CloseButtonPosition, 0, m_tabWidget->tabBar());

		for (int i = 0; i < m_tabWidget->count(); i++)
		{
			QWidget* w = m_tabWidget->tabBar()->tabButton(i, closeSide);

			if (w != nullptr)
			{
				w->setVisible(i == index);
			}
		}

		return;
	}

	void SimWidgetPrivate::tabBarContextMenuRequest(const QPoint& pos)
	{
		assert(m_tabWidget);
		QTabBar* tabBar = m_tabWidget->tabBar();

		int tabIndex = tabBar->tabAt(pos);
		if (tabIndex == -1)
		{
			return;
		}

		SimBasePage* page = qobject_cast<SimBasePage*>(m_tabWidget->widget(tabIndex));
		if (page == nullptr)
		{
			assert(page);
			return;
		}

		return;
	}

	SimSchemaManager& SimWidgetPrivate::schemaManager()
	{
		return m_schemaManager;
	}

	const SimSchemaManager& SimWidgetPrivate::schemaManager() const
	{
		return m_schemaManager;
	}

	//
	//	SimulatorToolBar
	//
	SimToolBar::SimToolBar(const QString& title, QWidget* parent) :
		QToolBar{title, parent}
	{
		setMovable(false);
		setAcceptDrops(true);

		setObjectName("SimToolBar");

		setStyleSheet("QToolButton { padding-top: 3px; padding-bottom: 3px; padding-left: 3px; padding-right: 3px;}");
		setIconSize(iconSize() * 0.9);

		toggleViewAction()->setDisabled(true);

		return;
	}

	void SimToolBar::dragEnterEvent(QDragEnterEvent* event)
	{
		// Find Trend action
		//
		QWidget* trendActionWidget = nullptr;

		QList<QAction*> allActions = actions();
		for (QAction* a : allActions)
		{
			QVariant d = a->data();

			if (d.isValid() && d.typeId() == QMetaType::QString)
			{
				if (d.toString() == QLatin1String("IAmIndependentTrend"))
				{
					trendActionWidget = widgetForAction(a);
					trendActionWidget->setAcceptDrops(true);
				}
			}
		}

		if (trendActionWidget != nullptr && trendActionWidget->geometry().contains(event->position().toPoint()) &&
			event->mimeData()->hasFormat(AppSignalParamMimeType::value))
		{
			event->acceptProposedAction();
		}

		return;
	}

	void SimToolBar::dropEvent(QDropEvent* event)
	{
		// Find Trend action
		//
		QWidget* trendActionWidget = nullptr;
		QAction* trendAction = nullptr;

		QList<QAction*> allActions = actions();

		for (QAction* a : allActions)
		{
			QVariant d = a->data();
			if (d.isValid() && d.typeId() == QMetaType::QString)
			{
				if (d.toString() == QLatin1String("IAmIndependentTrend"))
				{
					trendAction = a;
					trendActionWidget = widgetForAction(trendAction);
				}
			}
		}

		if (trendAction != nullptr && trendActionWidget != nullptr && trendActionWidget->geometry().contains(event->position().toPoint()) &&
			event->mimeData()->hasFormat(AppSignalParamMimeType::value))
		{
			// Lets assume parent isMonitorMainWindow
			//
			SimWidgetPrivate* sw = dynamic_cast<SimWidgetPrivate*>(this->parent());
			if (sw == nullptr)
			{
				Q_ASSERT(sw);
				return;
			}

			// Load data from drag and drop
			//
			QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

			::Proto::AppSignalSet protoSetMessage;
			bool ok = protoSetMessage.ParseFromArray(data.constData(), static_cast<int>(data.size()));

			if (ok == false)
			{
				event->acceptProposedAction();
				return;
			}

			std::vector<AppSignalParam> appSignals;
			appSignals.reserve(protoSetMessage.appsignal_size());

			// Parse data
			//
			for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
			{
				const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

				AppSignalParam appSignalParam;
				ok = appSignalParam.load(appSignalMessage);

				if (ok == true)
				{
					appSignals.push_back(appSignalParam);
				}
			}

			if (appSignals.empty() == false)
			{
				sw->startTrends(appSignals);
			}
		}

		return;
	}
} // namespace SimUi
