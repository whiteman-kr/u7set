#include "MonitorSchemaWidget.h"
#include "Globals.h"
#include "MonitorMainWindow.h"
#include "MonitorSchemaManager.h"
#include "MonitorSchemaView.h"
#include "MonitorSignalInfo.h"

#include <ClientLib/AppSignalManager.h>
#include <VFrame30/AppSignalController.h>
#include <VFrame30/IMatsSchemaItemAssociations.h>
#include <VFrame30/ITimeStats.h>

#include <exception>


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
		void timerEvent(QTimerEvent* /*event*/) override { setText(getActionText()); }

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
										   m_signalManager->signalState(m_signalParam.appSignalId()).value_or(AppSignalState{}) :
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
		AppSignalParam signalParam() const { return m_signalParam; }

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
										 VFrame30::ITimeStats* timeStats,
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

	monitorSchemaView()->updateConfiguration(schemaManager->configController().configuration());

	return;
}

MonitorSchemaWidget::~MonitorSchemaWidget()
{
	qDebug() << Q_FUNC_INFO;
}

void MonitorSchemaWidget::createActions() {}

void MonitorSchemaWidget::contextMenuRequested(const QPoint& pos)
{
	// Reset highlights
	//
	clientSchemaView()->setHighlightSignalIds({});

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

	auto transformEquipmentId = [this](QString& s)
	{
		if (s.startsWith('@') == true)
		{
			s = appSignalManager().equipmentToAppSignalId(s);
		}
	};

	std::ranges::for_each(signalList, transformEquipmentId);
	std::ranges::for_each(impactSignalList, transformEquipmentId);

	signalContextMenu(signalList, impactSignalList, loopbacks, {});
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

	QStringList equipmentIds; // Here will be added @equipmentId for appSignals and impactSignals.

	// Compose menu
	//
	QSchemaMenu menu{this};

	// Reset highlights.
	//
	if (clientSchemaView()->hasHighlightItems() == true)
	{
		QAction* a = menu.addAction(tr("Reset Highlights"));
		a->setEnabled(clientSchemaView()->hasHighlightItems());
		connect(a,
				&QAction::triggered,
				this,
				[this]()
				{
					clientSchemaView()->clearHighlightItems();
				});
	}

	if (appSignals.isEmpty() == true && impactSignals.isEmpty() == true && loopbacks.isEmpty() == true &&
		clientSchemaView()->hasHighlightItems() == false)
	{
		return;
	}

	// Schemas List
	//
	std::set<QString> signalsSchemasSet;
	for (const QString& s : appSignals)
	{
		// Find by app signal id
		//
		for (QStringList schemaIds = schemaManager()->configController().schemasByAppSignalId(s); const QString& schemaId : schemaIds)
		{
			signalsSchemasSet.insert(schemaId);
		}

		// Find by equipment id of input or output
		//
		auto signalParam = appSignalManager().signalParam(s);

		if (signalParam.has_value() == true && (signalParam->isInput() == true || signalParam->isOutput() == true))
		{
			QString equipmentId = "@" + signalParam->equipmentId();
			equipmentIds.push_back(equipmentId);

			for (QStringList schemaIds = schemaManager()->configController().schemasByAppSignalId(equipmentId);
				 const QString& schemaId : schemaIds)
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
		for (QStringList schemaIds = schemaManager()->configController().schemasByAppSignalId(s); const QString& schemaId : schemaIds)
		{
			impactSignalsSchemasSet.insert(schemaId);
		}

		// Find by equipment id of input or output
		//
		auto signalParam = appSignalManager().signalParam(s);

		if (signalParam.has_value() == true && (signalParam->isInput() == true || signalParam->isOutput() == true))
		{
			QString equipmentId = "@" + signalParam->equipmentId();
			equipmentIds.push_back(equipmentId);

			for (QStringList schemaIds = schemaManager()->configController().schemasByAppSignalId(equipmentId);
				 const QString& schemaId : schemaIds)
			{
				impactSignalsSchemasSet.insert(schemaId);
			}
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

	// Join all appSignals, impactSignals, loopbacks, and equipmentIds
	// for highlighting.
	//
	QStringList allIds = appSignals + impactSignals + loopbacks + equipmentIds;
	allIds.sort();
	allIds.removeDuplicates();

	// --
	//
	if (signalsSchemasSet.empty() == false || impactSignalsSchemasSet.empty() == false || loopbackSchemas.empty() == false)
	{
		menu.addSeparator();
		QMenu* schemasSubMenu = menu.addMenu(tr("Schemas"));
		schemasSubMenu->setToolTipsVisible(true);

		QFont mf;
#ifdef Q_OS_WIN
		mf.setFamily("Consolas");
#else
		mf = QFontDatabase::systemFont(QFontDatabase::FixedFont);
#endif
		schemasSubMenu->setFont(mf);

		qsizetype maxSchemaIdWidth = 0;
		{
			auto maxSchemaIdFunc = [](const auto& begin, const auto& end) -> qsizetype
			{
				auto maxSchemaIdIt = std::max_element(begin,
													  end,
													  [](const auto& lhs, const auto& rhs)
													  {
														  return lhs.size() < rhs.size();
													  });
				return maxSchemaIdIt != end ? static_cast<int>((*maxSchemaIdIt).size()) : 0;
			};

			maxSchemaIdWidth = std::max(maxSchemaIdFunc(signalsSchemasSet.begin(), signalsSchemasSet.end()), maxSchemaIdWidth);
			maxSchemaIdWidth = std::max(maxSchemaIdFunc(impactSignalsSchemasSet.begin(), impactSignalsSchemasSet.end()), maxSchemaIdWidth);
			maxSchemaIdWidth = std::max(maxSchemaIdFunc(loopbackSchemas.begin(), loopbackSchemas.end()), maxSchemaIdWidth);
		}

		const auto schemaDetailsSet = schemaManager()->configController().schemasDetailsSet();

		auto addSchemaMenuItem = [this, &allIds, schemasSubMenu, maxSchemaIdWidth, &schemaDetailsSet](QString schemaId)
		{
			auto f = [this, schemaId, allIds]()
			{
				if (schemaId != this->schemaId())
				{
					setSchema(schemaId, allIds, false);
				}
			};

			auto schemaCaption = schemaManager()->configController().schemaCaptionById(schemaId);

			QString text;
#if 0
			text = schemaCaption.isEmpty() ? schemaId : schemaCaption;
#else
			text = QString{"%1 | %2"}.arg(schemaId.leftJustified(maxSchemaIdWidth)).arg(schemaCaption);
#endif
			QAction* a = schemasSubMenu->addAction(text);

			auto schemaDetails = schemaDetailsSet.schemaDetails(schemaId);

			QString schemaTagsText;
			if (schemaDetails != nullptr)
			{
				QStringList tags;
				std::copy(schemaDetails->schemaTags().begin(), schemaDetails->schemaTags().end(), std::back_inserter(tags));
				schemaTagsText = tags.join(" ");
			}

			QString schemaPath = schemaDetails ? schemaDetails->m_path : QString{};

			auto tooltip = QString("<b>Schema:</b><br>%1<br>%2<br><b>Tags:</b><br>%3<br><b>Path:</b><br>%4")
							   .arg(schemaId)
							   .arg(schemaCaption)
							   .arg(schemaTagsText)
							   .arg(schemaPath);

			a->setToolTip(tooltip);
			a->setCheckable(true);
			a->setChecked(schema()->schemaId() == schemaId);

			connect(a, &QAction::triggered, this, f);
		};

		for (const QString& schemaId : signalsSchemasSet)
		{
			addSchemaMenuItem(schemaId);
		}

		schemasSubMenu->addSeparator();

		for (const QString& schemaId : impactSignalsSchemasSet)
		{
			addSchemaMenuItem(schemaId);
		}

		// Loopbacks
		//
		schemasSubMenu->addSeparator();

		for (const QString& schemaId : loopbackSchemas)
		{
			addSchemaMenuItem(schemaId);
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
		auto signal = appSignalManager().signalParam(s);

		if (signal.has_value() == false)
		{
			signal = AppSignalParam{};

			signal->setAppSignalId(s);
			signal->setCustomSignalId({});

			maxIdSize = std::max(maxIdSize, signal->appSignalId().size());
		}
		else
		{
			maxIdSize = std::max(maxIdSize, signal->customSignalId().size());
			maxCaptionSize = std::max(maxCaptionSize, signal->caption().size());
		}

		appSignalParams.push_back(std::move(*signal));
	}

	for (const QString& s : impactSignals)
	{
		auto signal = appSignalManager().signalParam(s);

		if (signal.has_value() == false)
		{
			signal = AppSignalParam{};

			signal->setAppSignalId(s);
			signal->setCustomSignalId({});

			maxIdSize = std::max(maxIdSize, signal->appSignalId().size());
		}
		else
		{
			maxIdSize = std::max(maxIdSize, signal->customSignalId().size());
			maxCaptionSize = std::max(maxCaptionSize, signal->caption().size());
		}

		impactSignalsParams.push_back(std::move(*signal));
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
								  &theApp.mainWindow()->monitorCentralWidget());
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
