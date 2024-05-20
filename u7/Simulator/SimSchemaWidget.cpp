#include "SimSchemaWidget.h"
#include "SimIdeSimulator.h"
#include "SimSchemaManager.h"
#include "SimSchemaView.h"
#include "SimWidget.h"

#include <Simulator/SimAppSignalManager.h>
#include <Simulator/SimSoftware.h>
#include <VFrame30/IMatsSchemaItemAssociations.h>
#include <VFrame30/AppSignalController.h>
#include <VFrame30/LogicSchema.h>
#include <VFrame30/Context.h>

namespace
{
	class QSignalUpdateAction : public QAction
	{
	public:
		explicit QSignalUpdateAction(const AppSignalParam& signalParam,
									 const IAppSignalManager* signalManager,
									 qsizetype maxIdSize,
									 qsizetype maxCaptionSize,
									 QObject* parent) :
			QAction{parent},
			m_signalParam{signalParam},
			m_signalManager{signalManager},
			m_maxIdSize{maxIdSize},
			m_maxCaptionSize{maxCaptionSize}
		{
			Q_ASSERT(m_signalManager);

			setText(getActionText());
			startTimer(200);

			return;
		}

	protected:
		void timerEvent(QTimerEvent* /*event*/) override
		{
			setText(getActionText());
		}

		QString getActionText()
		{
			QString str;

			if (m_signalParam.customSignalId().isEmpty() == true)
			{
				// There is no such signal.
				//
				str = m_signalParam.appSignalId();
			}
			else
			{
				AppSignalState state = m_signalManager ?
										   m_signalManager->signalState(m_signalParam.appSignalId(), nullptr) :
										   AppSignalState{};

				QString stateText;
				if (state.isValid() == false)
				{
					stateText = " ? ";
				}
				else
				{
					// Print signal value.
					//
					int precision = (m_signalParam.isAnalog() && m_signalParam.analogSignalFormat() == E::AnalogAppSignalFormat::Float32) ?
										m_signalParam.precision() :
										0;

					stateText = QString{"%1"}.arg(state.value(), 8, 'f', precision);
				}

				str = QString{"%1 | %2 |%3"}
						  .arg(m_signalParam.customSignalId().leftJustified(m_maxIdSize))
						  .arg(m_signalParam.caption().leftJustified(m_maxCaptionSize))
						  .arg(stateText);
			}

			return str;
		}

	public:
		AppSignalParam signalParam() const
		{
			return m_signalParam;
		}

	private:
		const AppSignalParam m_signalParam;
		const IAppSignalManager* m_signalManager{};

		qsizetype m_maxIdSize{};
		qsizetype m_maxCaptionSize{};
	};

	class QSchemaMenu : public QMenu
	{
	public:
		QSchemaMenu(QWidget* parent = nullptr) :
			QMenu{parent}
		{
#ifdef Q_OS_WIN
			QFont f;
			f.setFamily("Consolas");
			setFont(f);
#else
			// QFont f;
			// f.setFamily("DejaVu Sans Mono");  // https://ianyepan.github.io/posts/system-default-monospace-fonts-pt1/
			// f.setFamily("DejaVu Sans Mono Book");  // https://ianyepan.github.io/posts/system-default-monospace-fonts-pt1/
			// setFont(f);
			setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
#endif
		}

	protected:
		void mousePressEvent(QMouseEvent* event) override
		{
			m_dragSignalParam = {};
			m_dragStartPosition = {};

			QSignalUpdateAction* dragAction = dynamic_cast<QSignalUpdateAction*>(activeAction());

			if (event->button() == Qt::LeftButton && dragAction != nullptr)
			{
				m_dragStartPosition = event->pos();
				m_dragSignalParam = dragAction->signalParam();
			}

			return QMenu::mousePressEvent(event);
		}

		void mouseMoveEvent(QMouseEvent* event) override
		{
			if (m_dragSignalParam.customSignalId().isEmpty() == true)
			{
				QMenu::mouseMoveEvent(event);
				return;
			}

			if (!(event->buttons() & Qt::LeftButton))
			{
				QMenu::mouseMoveEvent(event);
				return;
			}

			if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
			{
				QMenu::mouseMoveEvent(event);
				return;
			}

			// Save signals to protobuf.
			//
			::Proto::AppSignalSet protoSetMessage;
			m_dragSignalParam.save(protoSetMessage.add_appsignal());

			QByteArray data;
			data.resize(static_cast<int>(protoSetMessage.ByteSizeLong()));

			protoSetMessage.SerializeToArray(data.data(), static_cast<int>(protoSetMessage.ByteSizeLong()));

			// --
			//
			if (data.isEmpty() == false)
			{
				QDrag* drag = new QDrag{this->parentWidget()};
				QMimeData* mimeData = new QMimeData;

				mimeData->setData(AppSignalParamMimeType::value, data);
				drag->setMimeData(mimeData);

				// Close this menu, if it is not closed then interface can freeze.
				//
				close();
				m_dragSignalParam = {};

				drag->exec();

				return;
			}

			return;
		}

	private:
		AppSignalParam m_dragSignalParam;
		QPoint m_dragStartPosition;
	};
} // namespace


//
//
//	SimSchemaWidget
//
//
SimSchemaWidget::SimSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
								 SimSchemaManager* schemaManager,
								 VFrame30::AppSignalController* appSignalController,
								 SimIdeSimulator* simulator,
								 QWidget* parent) :
	VFrame30::ClientSchemaWidget(new SimSchemaView{schemaManager}, schema, schemaManager, parent),
	m_simulator(simulator),
	m_logController(simulator->log())
{
	Q_ASSERT(m_simulator);
	Q_ASSERT(schema);

	clientSchemaView()->setAppSignalController(appSignalController);
	clientSchemaView()->setLogController(&m_logController);

	auto context = VFrame30::Context::create(clientSchemaView());
	schema->setContext(std::move(context));

	setFrameShape(QFrame::NoFrame);

	// --
	//
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &SimSchemaWidget::contextMenuRequested);

	createActions();

	// --
	//
	connect(m_simulator, &SimIdeSimulator::projectUpdated, this, &SimSchemaWidget::updateProject);

	// If the project was already loaded and we open another SimWidget then call this slot manually.
	// It sets monitorId ans calls OnShwoScript
	//
	updateProject();

	return;
}

SimSchemaWidget::~SimSchemaWidget()
{
}

void SimSchemaWidget::createActions()
{
	m_zoomInAction = new QAction(tr("Zoom In"), this);
	m_zoomInAction->setStatusTip(tr("Zoom in schema view"));
	m_zoomInAction->setEnabled(true);
	m_zoomInAction->setShortcut(QKeySequence::ZoomIn);
	connect(m_zoomInAction, &QAction::triggered, this, &SimSchemaWidget::zoomIn);

	m_zoomOutAction = new QAction(tr("Zoom Out"), this);
	m_zoomOutAction->setStatusTip(tr("Zoom out schema view"));
	m_zoomOutAction->setEnabled(true);
	m_zoomOutAction->setShortcut(QKeySequence::ZoomOut);
	connect(m_zoomOutAction, &QAction::triggered, this, &SimSchemaWidget::zoomOut);

	m_zoom100Action = new QAction(tr("Zoom 100%"), this);
	m_zoom100Action->setStatusTip(tr("Set zoom to 100%"));
	m_zoom100Action->setEnabled(true);
	m_zoom100Action->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Asterisk));
	connect(m_zoom100Action, &QAction::triggered, this, &SimSchemaWidget::zoom100);

	m_zoomToFitAction = new QAction(tr("Fit to Screen"), this);
	m_zoomToFitAction->setStatusTip(tr("Set zoom to fit the screen"));
	m_zoomToFitAction->setEnabled(true);
	m_zoomToFitAction->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Slash));
	connect(m_zoomToFitAction, &QAction::triggered, this, &SimSchemaWidget::zoomToFit);

	addAction(m_zoomInAction);
	addAction(m_zoomOutAction);
	addAction(m_zoom100Action);
	addAction(m_zoomToFitAction);

	return;
}

SimWidget* SimSchemaWidget::simWidget()
{
	SimWidget* result = nullptr;

	QWidget* parent = this->parentWidget();
	while (parent != nullptr)
	{
		result = dynamic_cast<SimWidget*>(parent);
		if (result != nullptr)
		{
			break;
		}

		parent = parent->parentWidget();
	}

	return result;
}


void SimSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Reset highlight
	//
	clientSchemaView()->setHighlightIds({});

	// Signals items
	//
	std::vector<SchemaItemPtr> items = itemsUnderCursor(pos);

	QStringList appSignals;
	QStringList impactSignals;
	QString loopbackId;

	auto context = VFrame30::Context::create(simSchemaView());

	if (items.empty() == false)
	{
		for (const SchemaItemPtr& item : items)
		{
			auto schemaItemAssociations = dynamic_cast<const VFrame30::IMatsSchemaItemAssociations*>(item.get());
			if (schemaItemAssociations == nullptr)
			{
				continue;
			}

			appSignals += schemaItemAssociations->associatedAppSignalIds();
			impactSignals += schemaItemAssociations->associatedImpactAppSignalIds();
			loopbackId += schemaItemAssociations->associatedLoopbackIds().join(", ");
		}
	}

	auto f = [sim = m_simulator](QString& s)
		{
			if (s.startsWith('@') == true)
			{
				s = sim->appSignalManager().equipmentToAppSignalId(s);
			}
		};

	std::ranges::for_each(appSignals, f);
	std::ranges::for_each(impactSignals, f);

	signalContextMenu(appSignals, impactSignals, loopbackId, {});

	return;
}

void SimSchemaWidget::signalContextMenu(QStringList appSignals,
										QStringList impactSignals,
										QString loopbackId,
										const QList<QMenu*> customMenu)
{
	// To set, it will sort list and exclude same ids
	//
	appSignals.sort();
	appSignals.removeDuplicates();

	impactSignals.sort();
	impactSignals.removeDuplicates();

	QStringList equipmentIds; // Here will be added @equipmentId for appSignals and impactSignals.

	// --
	//
	SimWidget* thisSimWidget = simWidget();

	if (thisSimWidget == nullptr)
	{
		Q_ASSERT(thisSimWidget);
		return;
	}

	// Compose menu
	//
	QSchemaMenu menu{this};

	// Schemas List
	//
	QMenu* schemasSubMenu = menu.addMenu(tr("Schemas"));

	std::set<QString> signalsSchemasSet;
	for (const QString& s : appSignals)
	{
		// Find schema by appSignalId
		//
		for (const QString& schemaId : m_simulator->schemasByAppSignalId(s))
		{
			signalsSchemasSet.insert(schemaId);
		}

		// Find by equipment id of input or output
		//
		bool found = false;
		auto signalParam = m_simulator->appSignalManager().signalParam(s, &found);

		if (found == true && (signalParam.isInput() == true || signalParam.isOutput() == true))
		{
			QString equipmentId = "@" + signalParam.equipmentId();
			equipmentIds.push_back(equipmentId);

			for (QStringList schemaIds = m_simulator->schemaDetails().schemasByAppSignalId(equipmentId);
				 const QString & schemaId : schemaIds)
			{
				signalsSchemasSet.insert(schemaId);
			}
		}
	}

	std::set<QString> impactSignalsSchemasSet;
	for (const QString& s : impactSignals)
	{
		// Find by app signal id
		//
		for (const QString& schemaId : m_simulator->schemasByAppSignalId(s))
		{
			impactSignalsSchemasSet.insert(schemaId);
		}

		// Find by equipment id of input or output
		//
		bool found = false;
		auto signalParam = m_simulator->appSignalManager().signalParam(s, &found);

		if (found == true && (signalParam.isInput() == true || signalParam.isOutput() == true))
		{
			QString equipmentId = "@" + signalParam.equipmentId();
			equipmentIds.push_back(equipmentId);

			for (QStringList schemaIds = m_simulator->schemaDetails().schemasByAppSignalId(equipmentId);
				 const QString & schemaId : schemaIds)
			{
				impactSignalsSchemasSet.insert(schemaId);
			}
		}
	}

	// Join all appSignals, impactSignals, loopbacks, and equipmentIds
	// for highlighting.
	//
	QStringList allIds = appSignals + impactSignals + equipmentIds;
	allIds << loopbackId;

	allIds.sort();
	allIds.removeDuplicates();

	// --
	//
	std::set<QString> loopbackSchemas;
	if (loopbackId.isEmpty() == false)
	{
		QStringList schemaIds = m_simulator->schemasByLoopbackId(loopbackId);

		for (const QString& schemaId : schemaIds)
		{
			loopbackSchemas.insert(schemaId);
		}
	}

	if (signalsSchemasSet.empty() == true &&
		impactSignalsSchemasSet.empty() == true &&
		loopbackSchemas.empty() == true)
	{
		schemasSubMenu->setDisabled(true);
	}
	else
	{
		// App Signals
		//
		for (const QString& schemaId : signalsSchemasSet)
		{
			auto f = [schemaId, &allIds, thisSimWidget]() -> void
			{
				thisSimWidget->openSchemaTabPage(schemaId, allIds);
			};

			QAction* a = schemasSubMenu->addAction(schemaId);

			a->setCheckable(true);
			a->setChecked(schema()->schemaId() == schemaId);

			connect(a, &QAction::triggered, this, f);
		}

		// Impact signals
		//
		schemasSubMenu->addSeparator();

		for (const QString& schemaId : impactSignalsSchemasSet)
		{
			auto f = [schemaId, &allIds, thisSimWidget]() -> void
			{
				thisSimWidget->openSchemaTabPage(schemaId, allIds);
			};

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}

		// Loopback
		//
		schemasSubMenu->addSeparator();

		for (const QString& schemaId : loopbackSchemas)
		{
			auto f = [schemaId, &allIds, thisSimWidget]() -> void
			{
				thisSimWidget->openSchemaTabPage(schemaId, allIds);
			};

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}
	}

	// Custom menus
	//
	if (customMenu.isEmpty() == false)
	{
		for (auto cm : customMenu)
		{
			menu.addActions(cm->actions());
		}
	}

	// SignalInfo list
	//
	std::list<AppSignalParam> appSignalParams;
	std::list<AppSignalParam> impactSignalsParams;

	qsizetype maxIdSize = 0;
	qsizetype maxCaptionSize = 0;

	for (const QString& s : appSignals)
	{
		bool ok = false;
		AppSignalParam signal = m_simulator->appSignalManager().signalParam(s, &ok);

		if (ok == false)
		{
			signal.setAppSignalId(s);
			signal.setCustomSignalId({});

			maxIdSize = std::max(maxIdSize, signal.appSignalId().size());
		}
		else
		{
			maxIdSize = std::max(maxIdSize, signal.customSignalId().size());
			maxCaptionSize = std::max(maxCaptionSize, signal.caption().size());
		}

		appSignalParams.push_back(std::move(signal));
	}

	for (const QString& s : impactSignals)
	{
		bool ok = false;
		AppSignalParam signal = m_simulator->appSignalManager().signalParam(s, &ok);

		if (ok == false)
		{
			signal.setAppSignalId(s);
			signal.setCustomSignalId({});

			maxIdSize = std::max(maxIdSize, signal.appSignalId().size());
		}
		else
		{
			maxIdSize = std::max(maxIdSize, signal.customSignalId().size());
			maxCaptionSize = std::max(maxCaptionSize, signal.caption().size());
		}

		impactSignalsParams.push_back(std::move(signal));
	}

	// --
	//
	if (appSignalParams.empty() == false)
	{
		QAction* appSignalSeparator = menu.addSeparator();
		appSignalSeparator->setText(tr("Signals"));

		for (const auto& signal : appSignalParams)
		{
			auto signalAction = new QSignalUpdateAction{signal, &m_simulator->appSignalManager(), maxIdSize, maxCaptionSize, &menu};
			menu.addAction(signalAction);

			auto f = [signal, thisSimWidget]() -> void
			{
				thisSimWidget->signalInfo(signal.appSignalId());
			};

			connect(signalAction, &QAction::triggered, this, f);
		}
	}

	if (impactSignalsParams.empty() == false)
	{
		if (appSignalParams.empty() == false)
		{
			QAction* impactSignalSeparator = menu.addSeparator();
			impactSignalSeparator->setText(tr("Impact Signals"));
		}

		for (const auto& signal : impactSignalsParams)
		{
			auto signalAction = new QSignalUpdateAction{signal, &m_simulator->appSignalManager(), maxIdSize, maxCaptionSize, &menu};
			menu.addAction(signalAction);

			auto f = [signal, thisSimWidget]() -> void
			{
				thisSimWidget->signalInfo(signal.appSignalId());
			};

			connect(signalAction, &QAction::triggered, this, f);
		}
	}

	// The list of Monitors
	//
	if (QStringList monitors = m_simulator->software().monitors();
		monitors.isEmpty() == false)
	{
		menu.addSeparator();
		QMenu* monitorsSubMenu = menu.addMenu(tr("Monitors"));

		for (const QString& m : monitors)
		{
			QAction* a = monitorsSubMenu->addAction(m);

			a->setCheckable(true);
			a->setChecked(m == simSchemaView()->monitorId());

			auto f = [m, this]()
			{
				QSettings s;
				s.setValue("Simulator/" + m_simulator->projectName(), m);

				simSchemaView()->setMonitorId(m, true);
			};

			connect(a, &QAction::triggered, this, f);
		}
	}

	// --
	//
	menu.exec(QCursor::pos());

	return;
}

void SimSchemaWidget::updateProject()
{
	// Update MonitorEquipment
	//
	clientSchemaView()->setMonitorEquipment(m_simulator->monitorEquipment());

	// Set MonitorID
	//
	QSettings s;
	QString monitorId = s.value("Simulator/" + m_simulator->projectName()).toString();

	QStringList monitors = m_simulator->software().monitors();
	if (monitors.contains(monitorId) == false)
	{
		monitorId.clear();
	}

	if (monitorId.isEmpty() == true && monitors.isEmpty() == false)
	{
		monitorId = monitors.front();
	}

	simSchemaView()->setMonitorId(monitorId, false); // Do not emit update (false) as it will lead to recursion

	// Update schema
	//
	if (m_simulator->isLoaded() == true)
	{
		auto context = VFrame30::Context::create(clientSchemaView());
		auto newSchema = schemaManager()->schema(schemaId(), std::move(context));

		if (newSchema == nullptr)
		{
			// Schema could be deleted or renamed
			// create a dummy schema
			//
			newSchema = std::make_shared<VFrame30::LogicSchema>();
		}

		BaseSchemaWidget::setSchema(newSchema, true);
	}

	// Run onShowScript
	//
	schema()->onShowEvent(clientSchemaView()->jsEngine(), clientSchemaView()->logFile());

	return;
}

SimSchemaView* SimSchemaWidget::simSchemaView()
{
	SimSchemaView* result = dynamic_cast<SimSchemaView*>(schemaView());
	assert(result);
	return result;
}

const SimSchemaView* SimSchemaWidget::simSchemaView() const
{
	const SimSchemaView* result = dynamic_cast<const SimSchemaView*>(schemaView());
	assert(result);
	return result;
}
