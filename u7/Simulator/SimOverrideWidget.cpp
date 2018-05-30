#include "SimOverrideWidget.h"
#include <QHBoxLayout>
#include <QMenu>
#include <QActionGroup>
#include <QSettings>
#include <QClipboard>
#include <QHeaderView>
#include <QDragEnterEvent>
#include "../../lib/AppSignal.h"

SimOverrideWidget::SimOverrideWidget(Sim::Simulator* simulator, QWidget* parent) :
	QWidget(parent),
	m_simulator(simulator)
{
	assert(m_simulator);

	m_treeWidget = new QTreeWidget(this);

	m_treeWidget->setRootIsDecorated(false);
	m_treeWidget->setUniformRowHeights(true);

	m_treeWidget->setColumnCount(static_cast<int>(QOverrideTreeWidgetItem::Columns::ColumnCount));

	QStringList headerLabels;
	headerLabels << "SignalID";
	headerLabels << "Caption";
	headerLabels << "Type";
	headerLabels << "Override Value";

	assert(headerLabels.size() == static_cast<int>(QOverrideTreeWidgetItem::Columns::ColumnCount));

	m_treeWidget->setHeaderLabels(headerLabels);

	// --
	//
	QHBoxLayout* layout = new QHBoxLayout;

	layout->insertWidget(0, m_treeWidget);

	setLayout(layout);

	// --
	//
	QByteArray headerState = QSettings().value("SimulatorWidget/SimOverridenSignals/ListHeader").toByteArray();
	if (headerState.isEmpty() == false)
	{
		m_treeWidget->header()->restoreState(headerState);
	}

	m_treeWidget->setSortingEnabled(true);

	// Darg and Drop fo rsignals
	//
	setAcceptDrops(true);

	// --
	//
	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &SimOverrideWidget::itemDoubleClicked);
	connect(m_treeWidget, &QTreeWidget::itemChanged, this, &SimOverrideWidget::itemChanged);

	connect(&m_simulator->overrideSignals(), &Sim::OverrideSignals::signalsChanged, this, &SimOverrideWidget::signalsChanged);
	connect(&m_simulator->overrideSignals(), &Sim::OverrideSignals::stateChanged, this, &SimOverrideWidget::signalStateChanged);

	// --
	//
	QSettings settings;

	m_currentBase = settings.value("SimulatorWidget/SimOverridenSignals/m_currentBase", 10).toInt();
	m_currentFormat = static_cast<E::AnalogFormat>(settings.value("SimulatorWidget/SimOverridenSignals/m_currentFormat", 'g').toInt());
	m_currentPrecision = settings.value("SimulatorWidget/SimOverridenSignals/m_currentPrecision", -1).toInt();

	return;
}

SimOverrideWidget::~SimOverrideWidget()
{
	QSettings settings;

	QByteArray headerState = m_treeWidget->header()->saveState();
	settings.setValue("SimulatorWidget/SimOverridenSignals/ListHeader", headerState);

	settings.setValue("SimulatorWidget/SimOverridenSignals/m_currentBase", m_currentBase);
	settings.setValue("SimulatorWidget/SimOverridenSignals/m_currentFormat", static_cast<int>(m_currentFormat));
	settings.setValue("SimulatorWidget/SimOverridenSignals/m_currentPrecision", m_currentPrecision);

	return;
}

void SimOverrideWidget::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void SimOverrideWidget::dropEvent(QDropEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value) == false)
	{
		assert(event->mimeData()->hasFormat(AppSignalParamMimeType::value) == true);
		event->setDropAction(Qt::DropAction::IgnoreAction);
		event->accept();
		return;
	}

	QByteArray data = event->mimeData()->data(AppSignalParamMimeType::value);

	::Proto::AppSignalSet protoSetMessage;
	bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	// Parse data
	//
	QStringList signalIds;

	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		AppSignalParam appSignalParam;
		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			signalIds << appSignalParam.appSignalId();
		}
	}

	if (signalIds.isEmpty() == false)
	{
		int actuallyAdded = m_simulator->overrideSignals().addSignals(signalIds);

		if (actuallyAdded == 0)
		{
			// Appartently signal already added, select it
			//
			selectSignal(signalIds.back());
		}
	}

	return;
}

void SimOverrideWidget::contextMenuEvent(QContextMenuEvent* event)
{
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();

	QString appSignalId;
	if (selectedItems.size() == 1)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItems.front());
		assert(item != nullptr);

		appSignalId = item->m_overrideSignal.m_appSignalId;
	}

	QMenu menu(this);

	// Set Value
	//
	QAction* setValueAction = menu.addAction(tr("Set Value..."),
											[this, &appSignalId]
											{
												setValue(appSignalId);
											});

	setValueAction->setEnabled(!appSignalId.isEmpty());

	menu.setDefaultAction(setValueAction);

	// Remove Signal
	//
	QAction* removeSignalAction = menu.addAction(tr("Remove Signal"),
												[this, &appSignalId]
												{
													removeSignal(appSignalId);
												});

	removeSignalAction->setEnabled(!appSignalId.isEmpty());

	// Clear
	//
	QAction* clearAction = menu.addAction(tr("Clear"), [this]{clear();});
	clearAction->setEnabled(!selectedItems.empty());

	// Dispaly format menu
	// Radix: 10 or 16
	// E::AnalogFormat m_currentFormat = E::AnalogFormat::g_9_or_9e;	// Current format for floating point signals
	// int m_currentPrecision = 4;
	//
	bool formatChanged = false;

	menu.addSeparator()->setText(tr("Integer Radix"));
	QAction* actionDec = menu.addAction("DEC",
										[this, &formatChanged]
										{
											this->m_currentBase = 10;
											formatChanged = true;
										});
	QAction* actionHex = menu.addAction("HEX",
										[this, &formatChanged]
										{
											this->m_currentBase = 16;
											formatChanged = true;
										});

	QActionGroup radixGroup(this);
	radixGroup.addAction(actionDec);
	radixGroup.addAction(actionHex);

	actionDec->setCheckable(true);
	actionHex->setCheckable(true);

	actionDec->setChecked(m_currentBase == 10);
	actionHex->setChecked(m_currentBase == 16);

	// Dispaly format menu
	// E::AnalogFormat m_currentFormat = E::AnalogFormat::g_9_or_9e;	// Current format for floating point signals
	//
	menu.addSeparator()->setText(tr("Float Format"));

	QAction* format_g_9_or_9e = menu.addAction("Auto",
												[this, &formatChanged]()
												{
													this->m_currentFormat = E::AnalogFormat::g_9_or_9e;
													formatChanged = true;
												});

	QAction* format_e_9e = menu.addAction("Exp",
												[this, &formatChanged]()
												{
													this->m_currentFormat = E::AnalogFormat::e_9e;
													formatChanged = true;
												});

	QAction* format_f_9 = menu.addAction("Numeric",
												[this, &formatChanged]()
												{
													this->m_currentFormat = E::AnalogFormat::f_9;
													formatChanged = true;
												});

	QActionGroup floatFormatGroup(this);
	floatFormatGroup.addAction(format_g_9_or_9e);
	floatFormatGroup.addAction(format_e_9e);
	floatFormatGroup.addAction(format_f_9);

	format_e_9e->setCheckable(true);
	format_f_9->setCheckable(true);
	format_g_9_or_9e->setCheckable(true);

	format_e_9e->setChecked(m_currentFormat == E::AnalogFormat::e_9e);
	format_f_9->setChecked(m_currentFormat == E::AnalogFormat::f_9);
	format_g_9_or_9e->setChecked(m_currentFormat == E::AnalogFormat::g_9_or_9e);

	// Dispaly format menu
	// m_currentPrecision
	//
	menu.addSeparator()->setText(tr("Float Precision"));

	QAction* precision_auto = menu.addAction("Precision Auto",
											[this, &formatChanged]()
											{
												this->m_currentPrecision = -1;
												formatChanged = true;
											});

	QAction* precision_4 = menu.addAction("Precision 4",
											[this, &formatChanged]()
											{
												this->m_currentPrecision = 4;
												formatChanged = true;
											});

	QAction* precision_8 = menu.addAction("Precision 8",
											[this, &formatChanged]()
											{
												this->m_currentPrecision = 8;
												formatChanged = true;
											});

	QAction* precision_12 = menu.addAction("Precision 12",
											[this, &formatChanged]()
											{
												this->m_currentPrecision = 12;
												formatChanged = true;
											});

	QAction* precision_16 = menu.addAction("Precision 16",
											[this, &formatChanged]()
											{
												this->m_currentPrecision = 16;
												formatChanged = true;
											});

	QActionGroup precisiontGroup(this);
	precisiontGroup.addAction(precision_auto);
	precisiontGroup.addAction(precision_4);
	precisiontGroup.addAction(precision_8);
	precisiontGroup.addAction(precision_12);
	precisiontGroup.addAction(precision_16);

	precision_auto->setCheckable(true);
	precision_4->setCheckable(true);
	precision_8->setCheckable(true);
	precision_12->setCheckable(true);
	precision_16->setCheckable(true);

	precision_auto->setChecked(m_currentPrecision == -1);
	precision_4->setChecked(m_currentPrecision == 4);
	precision_8->setChecked(m_currentPrecision == 8);
	precision_12->setChecked(m_currentPrecision == 12);
	precision_16->setChecked(m_currentPrecision == 16);

	// --
	//
	menu.addSeparator()->setText(tr("Copy"));

	QAction* copyAction = menu.addAction(tr("Copy Value"),
										[this]()
										{
											auto selected = m_treeWidget->selectedItems();
											if (selected.empty() == false)
											{
												QString text = selected.back()->text(static_cast<int>(QOverrideTreeWidgetItem::Columns::Value));

												QClipboard* clipboard = QGuiApplication::clipboard();
												assert(clipboard);

												clipboard->setText(text);
											}
										});

	copyAction->setEnabled(!appSignalId.isEmpty());

	// --
	//
	menu.exec(event->globalPos());

	if (formatChanged == true)
	{
		updateValueColumn();
	}

	return;
}

void SimOverrideWidget::updateValueColumn()
{
	auto currentSignals = m_simulator->overrideSignals().overrideSignals();

	int itemCount = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < itemCount; i++)
	{
		auto treeItem = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->topLevelItem(i));

		if (treeItem == nullptr)
		{
			assert(treeItem);
			return;
		}

		QString appSignalId = treeItem->m_overrideSignal.m_appSignalId;

		auto it = std::find_if(currentSignals.begin(), currentSignals.end(),
								[&appSignalId](const Sim::OverrideSignalParam& osp)
								{
									return osp.m_appSignalId == appSignalId;
								});

		if (it == currentSignals.end())
		{
			// Signot is not in the list, nothing critical, but how it happened?
			//
			assert(false);
			continue;
		}

		const Sim::OverrideSignalParam& osp = *it;
		treeItem->m_overrideSignal = osp;

		treeItem->setText(static_cast<int>(QOverrideTreeWidgetItem::Columns::Value),
						  osp.valueString(m_currentBase, m_currentFormat, m_currentPrecision));

		Qt::CheckState cs = osp.m_enabled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;

		treeItem->setCheckState(0, cs);
	}

	return;
}

void SimOverrideWidget::fillListWidget(const std::vector<Sim::OverrideSignalParam>& overrideSignals)
{
	assert(m_treeWidget);

	QList<QTreeWidgetItem*> newItems;
	newItems.reserve(static_cast<int>(overrideSignals.size()));

	for (const Sim::OverrideSignalParam& osp : overrideSignals)
	{
		QTreeWidgetItem* item = new QOverrideTreeWidgetItem(osp);
		newItems.push_back(item);
	}

	m_treeWidget->clear();
	m_treeWidget->addTopLevelItems(newItems);

	updateValueColumn();

	return;
}

void SimOverrideWidget::selectSignal(QString appSignalId)
{
	int count = m_treeWidget->topLevelItemCount();

	for (int i = 0; i < count; i++)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->topLevelItem(i));

		if (item == nullptr)
		{
			assert(item);
			return;
		}

		item->setSelected(item->m_overrideSignal.m_appSignalId == appSignalId);

		if (item->isSelected() == true)
		{
			m_treeWidget->scrollToItem(item);
		}
	}

	return;
}

void SimOverrideWidget::itemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	QOverrideTreeWidgetItem* toItem = dynamic_cast<QOverrideTreeWidgetItem*>(item);
	assert(toItem);

	setValue(toItem->m_overrideSignal.m_appSignalId);

	return;
}

void SimOverrideWidget::itemChanged(QTreeWidgetItem* item, int column)
{
	assert(item);

	if (column == 0)
	{
		QOverrideTreeWidgetItem* ovItem = dynamic_cast<QOverrideTreeWidgetItem*>(item);
		assert(ovItem);

		bool checked = ovItem->checkState(0) == Qt::Checked;

		m_simulator->overrideSignals().setEnable(ovItem->appSignalId(), checked);
	}

	return;
}

void SimOverrideWidget::signalsChanged(QStringList addedAppSignalIds)
{
	auto overrideSignals = m_simulator->overrideSignals().overrideSignals();

	fillListWidget(overrideSignals);

	if (addedAppSignalIds.size() == 1)
	{
		selectSignal(addedAppSignalIds.front());
	}

	return;
}

void SimOverrideWidget::signalStateChanged(QString appSignalId)
{
	updateValueColumn();
}

void SimOverrideWidget::clear()
{
	m_simulator->overrideSignals().clear();
}

void SimOverrideWidget::removeSignal(QString appSignalId)
{
	m_simulator->overrideSignals().removeSignal(appSignalId);
}

void SimOverrideWidget::setValue(QString appSignalId)
{
	//..
}


QOverrideTreeWidgetItem::QOverrideTreeWidgetItem(const Sim::OverrideSignalParam& overrideSignal) :
	QTreeWidgetItem(),
	m_overrideSignal(overrideSignal)
{
	QString type = E::valueToString(overrideSignal.m_signalType);
	if (overrideSignal.m_signalType == E::SignalType::Analog)
	{
		type = E::valueToString(overrideSignal.m_dataFormat);
	}

	this->setText(static_cast<int>(Columns::CustomSignalId), overrideSignal.m_customSignalId);
	this->setText(static_cast<int>(Columns::Caption), overrideSignal.m_caption);
	this->setText(static_cast<int>(Columns::Type), type);
	this->setText(static_cast<int>(Columns::Value), overrideSignal.valueString());

	this->setCheckState(0, overrideSignal.m_enabled ? Qt::Checked : Qt::Unchecked);

	return;
}

QOverrideTreeWidgetItem::~QOverrideTreeWidgetItem()
{
}

QString QOverrideTreeWidgetItem::appSignalId() const
{
	return m_overrideSignal.m_appSignalId;
}
