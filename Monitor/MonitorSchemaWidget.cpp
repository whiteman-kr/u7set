#include "MonitorSchemaWidget.h"
#include "../ClientLib/AppSignalManager.h"
#include "../VFrame30/AppSignalController.h"
#include "../VFrame30/IMatsSchemaItemAssociations.h"
#include "../lib/ITimeStats.h"
#include "Globals.h"
#include "MonitorMainWindow.h"
#include "MonitorSchemaManager.h"
#include "MonitorSchemaView.h"
#include "MonitorSignalInfo.h"


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
//	MonitorSchemaWidget
//
//
MonitorSchemaWidget::MonitorSchemaWidget(std::shared_ptr<VFrame30::Schema> schema,
										 MonitorSchemaManager* schemaManager,
										 VFrame30::AppSignalController* appSignalController,
										 VFrame30::LogController* logController,
										 ITimeStats* timeStats,
										 QWidget* parent) :
	VFrame30::ClientSchemaWidget(new MonitorSchemaView(schemaManager, this, appSignalController, logController, timeStats),
								 schema,
								 schemaManager,
								 parent),
	m_logController(logController),
	m_timeStats(timeStats)

{
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QWidget::customContextMenuRequested, this, &MonitorSchemaWidget::contextMenuRequested);

	createActions();

	Q_ASSERT(schema);

	auto context = VFrame30::Context::create(clientSchemaView());
	schema->setContext(std::move(context));

	return;
}

MonitorSchemaWidget::~MonitorSchemaWidget()
{
	qDebug() << Q_FUNC_INFO;
}

void MonitorSchemaWidget::createActions()
{
}

void MonitorSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Reset highlights
	//
	clientSchemaView()->setHighlightIds({});

	// Signals items
	//
	std::vector<SchemaItemPtr> items = itemsUnderCursor(pos);

	QStringList signalList;
	QStringList impactSignalList;
	QStringList loopbacks;

	auto context = VFrame30::Context::create(clientSchemaView());

	for (const SchemaItemPtr& item : items)
	{
		auto schemaItemAssociations = dynamic_cast<const VFrame30::IMatsSchemaItemAssociations*>(item.get());
		if (schemaItemAssociations == nullptr)
		{
			continue;
		}

		signalList += schemaItemAssociations->associatedAppSignalIds();
		impactSignalList += schemaItemAssociations->associatedImpactAppSignalIds();
		loopbacks += schemaItemAssociations->associatedLoopbackIds();
	}

	if (signalList.isEmpty() == false || impactSignalList.isEmpty() == false || loopbacks.isEmpty() == false)
	{
		auto f = [this](QString& s)
		{
			if (s.startsWith('@') == true)
			{
				s = appSignalManager().equipmentToAppSignalId(s);
			}
		};

		std::ranges::for_each(signalList, f);
		std::ranges::for_each(impactSignalList, f);

		signalContextMenu(signalList, impactSignalList, loopbacks, {});
	}

	return;
}

void MonitorSchemaWidget::signalContextMenu(QStringList appSignals,
											QStringList impactSignals,
											QStringList loopbacks,
											const QList<QMenu*>& customMenu)
{
	appSignals.sort();
	appSignals.removeDuplicates();

	impactSignals.sort();
	impactSignals.removeDuplicates();

	loopbacks.sort();
	loopbacks.removeDuplicates();

	// Compose menu
	//
	QSchemaMenu menu{this};

	// Schemas List
	//
	QMenu* schemasSubMenu = menu.addMenu(tr("Schemas"));

	std::set<QString> signalsSchemasSet;
	for (const QString& s : appSignals)
	{
		QStringList schemaIds = schemaManager()->configController().schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			signalsSchemasSet.insert(schemaId);
		}
	}

	std::set<QString> impactSignalsSchemasSet;
	for (const QString& s : impactSignals)
	{
		QStringList schemaIds = schemaManager()->configController().schemasByAppSignalId(s);

		for (const QString& schemaId : schemaIds)
		{
			impactSignalsSchemasSet.insert(schemaId);
		}
	}

	std::set<QString> loopbackSchemas;
	for (const QString& l : loopbacks)
	{
		QStringList schemaIds = schemaManager()->configController().schemasByLoopbackId(l);

		for (const QString& schemaId : schemaIds)
		{
			impactSignalsSchemasSet.insert(schemaId);
		}
	}

	// --
	//
	if (signalsSchemasSet.empty() == true &&
		impactSignalsSchemasSet.empty() == true &&
		loopbackSchemas.empty() == true)
	{
		schemasSubMenu->setDisabled(true);
	}
	else
	{
		for (const QString& schemaId : signalsSchemasSet)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, &loopbacks]()
			{
				if (schemaId != this->schemaId())
				{
					setSchema(schemaId, appSignals + impactSignals + loopbacks, false);
				}
			};

			QAction* a = schemasSubMenu->addAction(schemaId);

			a->setCheckable(true);
			a->setChecked(schema()->schemaId() == schemaId);

			connect(a, &QAction::triggered, this, f);
		}

		schemasSubMenu->addSeparator();

		for (const QString& schemaId : impactSignalsSchemasSet)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, &loopbacks]()
			{
				if (schemaId != this->schemaId())
				{
					setSchema(schemaId, appSignals + impactSignals + loopbacks, false);
				}
			};

			QString actionCaption = (schema()->schemaId() == schemaId) ? QString("-> %1").arg(schemaId) : schemaId;

			QAction* a = schemasSubMenu->addAction(actionCaption);
			connect(a, &QAction::triggered, this, f);
		}

		// Loopbacks
		//
		schemasSubMenu->addSeparator();

		for (const QString& schemaId : loopbackSchemas)
		{
			auto f = [this, schemaId, &appSignals, &impactSignals, &loopbacks]()
			{
				if (schemaId != this->schemaId())
				{
					setSchema(schemaId, appSignals + impactSignals + loopbacks, false);
				}
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
	QAction* appSignalSeparator = menu.addSeparator();
	appSignalSeparator->setText(tr("Signals"));

	std::list<AppSignalParam> appSignalParams;
	std::list<AppSignalParam> impactSignalsParams;

	qsizetype maxIdSize = 0;
	qsizetype maxCaptionSize = 0;

	for (const QString& s : appSignals)
	{
		bool ok = false;
		AppSignalParam signal = appSignalManager().signalParam(s, &ok);

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
		AppSignalParam signal = appSignalManager().signalParam(s, &ok);

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
	for (const auto& signal : appSignalParams)
	{
		auto signalAction = new QSignalUpdateAction{signal, &appSignalManager(), maxIdSize, maxCaptionSize, &menu};
		menu.addAction(signalAction);

		auto f = [this, signal]() -> void
		{
			signalInfo(signal.appSignalId());
		};

		connect(signalAction, &QAction::triggered, this, f);
	}

	// --
	//
	if (impactSignalsParams.empty() == false)
	{
		if (appSignals.empty() == false)
		{
			QAction* impactSignalSeparator = menu.addSeparator();
			impactSignalSeparator->setText(tr("Impact Signals"));
		}

		for (const auto& signal : impactSignalsParams)
		{
			auto signalAction = new QSignalUpdateAction{signal, &appSignalManager(), maxIdSize, maxCaptionSize, &menu};
			menu.addAction(signalAction);

			auto f = [this, signal]() -> void
			{
				signalInfo(signal.appSignalId());
			};

			connect(signalAction, &QAction::triggered, this, f);
		}
	}

	// --
	//
	menu.exec(QCursor::pos());

	return;
}

void MonitorSchemaWidget::signalInfo(QString appSignalId)
{
	MonitorSignalInfo::showDialog(appSignalId,
								  clientAppSignalManager(),
								  theApp.mainWindow()->tuningSignalManager(),
								  theApp.mainWindow()->tuningConnection(),
								  theApp.mainWindow()->tuningAuthorization(),
								  &theApp.mainWindow()->configController(),
								  theApp.mainWindow()->monitorCentralWidget());
	return;
}

MonitorSchemaView* MonitorSchemaWidget::monitorSchemaView()
{
	MonitorSchemaView* result = dynamic_cast<MonitorSchemaView*>(schemaView());
	Q_ASSERT(result);
	return result;
}

const MonitorSchemaView* MonitorSchemaWidget::monitorSchemaView() const
{
	const MonitorSchemaView* result = dynamic_cast<const MonitorSchemaView*>(schemaView());
	Q_ASSERT(result);
	return result;
}

IAppSignalManager& MonitorSchemaWidget::appSignalManager()
{
	return monitorSchemaView()->appSignalController()->appSignalManager();
}

const IAppSignalManager& MonitorSchemaWidget::appSignalManager() const
{
	return monitorSchemaView()->appSignalController()->appSignalManager();
}

ClientLib::AppSignalManager& MonitorSchemaWidget::clientAppSignalManager()
{
	try
	{
		return dynamic_cast<ClientLib::AppSignalManager&>(appSignalManager());
	}
	catch (std::bad_cast& e)
	{
		m_logController->writeAlert(tr("ClientLib::AppSignalManager is not available: %1. Terminate.").arg(e.what()));
		std::terminate();
	}
}

const ClientLib::AppSignalManager& MonitorSchemaWidget::clientAppSignalManager() const
{
	try
	{
		return dynamic_cast<const ClientLib::AppSignalManager&>(appSignalManager());
	}
	catch (std::bad_cast& e)
	{
		m_logController->writeAlert(tr("ClientLib::AppSignalManager is not available: %1. Terminate.").arg(e.what()));
		std::terminate();
	}
}

MonitorSchemaManager* MonitorSchemaWidget::schemaManager()
{
	return monitorSchemaView()->monitorSchemaManager();
}

const MonitorSchemaManager* MonitorSchemaWidget::schemaManager() const
{
	return monitorSchemaView()->monitorSchemaManager();
}
