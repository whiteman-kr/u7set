#include "SimOverridePane.h"
#include "SimOverrideValueWidget.h"
#include <Simulator/SimOverrideSignals.h>

SimOverridePane::SimOverridePane(Sim::Simulator* simulator, DbController* dbc, QWidget* parent) :
	QWidget(parent),
    HasDbController(dbc),
	m_simulator(simulator)
{
	assert(m_simulator);

	m_treeWidget = new QOverrideListWidget(m_simulator, this);

	m_treeWidget->installEventFilter(this);

	// --
	//
	QHBoxLayout* layout = new QHBoxLayout;
	layout->setContentsMargins(0, 0, 0, 0);

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
	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, this, &SimOverridePane::itemDoubleClicked);
	connect(m_treeWidget, &QTreeWidget::itemChanged, this, &SimOverridePane::itemChanged);

	connect(&m_simulator->overrideSignals(), &Sim::OverrideSignals::signalsChanged, this, &SimOverridePane::signalsChanged);
	connect(&m_simulator->overrideSignals(), &Sim::OverrideSignals::stateChanged, this, &SimOverridePane::signalStateChanged);

	// --
	//
	QSettings settings;

	m_currentBase = settings.value("SimulatorWidget/SimOverridenSignals/m_currentBase", 10).toInt();
	m_currentFormat = static_cast<E::AnalogFormat>(settings.value("SimulatorWidget/SimOverridenSignals/m_currentFormat", 'g').toInt());
	m_currentPrecision = settings.value("SimulatorWidget/SimOverridenSignals/m_currentPrecision", -1).toInt();

	startTimer(std::chrono::milliseconds{200});

	return;
}

SimOverridePane::~SimOverridePane()
{
	QSettings settings;

	QByteArray headerState = m_treeWidget->header()->saveState();
	settings.setValue("SimulatorWidget/SimOverridenSignals/ListHeader", headerState);

	settings.setValue("SimulatorWidget/SimOverridenSignals/m_currentBase", m_currentBase);
	settings.setValue("SimulatorWidget/SimOverridenSignals/m_currentFormat", static_cast<int>(m_currentFormat));
	settings.setValue("SimulatorWidget/SimOverridenSignals/m_currentPrecision", m_currentPrecision);

	return;
}

void SimOverridePane::timerEvent(QTimerEvent* /*event*/)
{
	updateValueColumn();
}

void SimOverridePane::dragEnterEvent(QDragEnterEvent* event)
{
	if (event->mimeData()->hasFormat(AppSignalParamMimeType::value))
	{
		event->acceptProposedAction();
	}

	return;
}

void SimOverridePane::dropEvent(QDropEvent* event)
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
	bool ok = protoSetMessage.ParseFromArray(data.constData(), static_cast<int>(data.size()));

	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	// Parse data.
	//
	QStringList signalIds;
	std::vector<std::tuple<QString, bool, double>> signalValues;
	
	signalIds.reserve(protoSetMessage.appsignal_size());
	signalValues.reserve(protoSetMessage.appsignal_size());

	for (int i = 0; i < protoSetMessage.appsignal_size(); i++)
	{
		const ::Proto::AppSignal& appSignalMessage = protoSetMessage.appsignal(i);

		AppSignalParam appSignalParam;
		ok = appSignalParam.load(appSignalMessage);

		if (ok == true)
		{
			QString appSignalId = appSignalParam.appSignalId();

			// get state without applied override.
			//
			AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, nullptr, false);
			bool isAlreadyOverriden = m_simulator->overrideSignals().containsSignal(appSignalId);

			signalIds << appSignalId;
			signalValues.push_back(std::tuple{appSignalId, isAlreadyOverriden, state.value()});
		}
	}

	if (signalIds.isEmpty() == false)
	{
		// Check that signals are not optimized constants.
		//
		for (const QString& id : signalIds)
		{
			std::optional<AppSignal> sp = m_simulator->appSignalManager().signalParamExt(id);

			if (sp.has_value() == false)
			{
				QMessageBox::critical(this, qAppName(), tr("Signal %1 not found.").arg(id));
				return;
			}

			if (sp->isConst() == true)
			{
				QMessageBox::critical(this, qAppName(), tr("Value for signal %1 cannot be is overriden as it was optimized to const.").arg(id));
				return;
			}
		}

		// --
		//
		int actuallyAdded = m_simulator->overrideSignals().addSignals(signalIds);
		if (actuallyAdded == 0)
		{
			// Apparently signal already added, select it.
			//
			selectSignal(signalIds.back());
		}

		// SetInitialValues to currents.
		//
		std::vector<Sim::OverrideSetValueData> overrideData;
		overrideData.reserve(signalValues.size());

		for (auto&[appSignalId, isAlreadyOverriden, value] : signalValues)
		{
			if (isAlreadyOverriden == false)
			{
				overrideData.emplace_back(appSignalId, Sim::OverrideSignalMethod::Value, value);
			}
		}

		if (overrideData.empty() == false)
		{
			m_simulator->overrideSignals().setValues(overrideData);
		}
	}

	return;
}


bool SimOverridePane::eventFilter(QObject* obj, QEvent* event)
{
	if (obj == m_treeWidget && event->type() == QEvent::KeyPress)
	{
		switch (static_cast<QKeyEvent*>(event)->key())
		{
		case Qt::Key::Key_Delete:
			{
				removeSelectedSignals();
			}
			return true;
		case Qt::Key::Key_Insert:
			{
				addSignal();
			}
			return true;
		case Qt::Key::Key_Space:
			{
				// Switch discrete signal 1 - 0 - 1 - 0...
				//
				QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();

				std::sort(selectedItems.begin(), selectedItems.end(), [this](QTreeWidgetItem* i1, QTreeWidgetItem* i2) {
					return m_treeWidget->indexOfTopLevelItem(i1) < m_treeWidget->indexOfTopLevelItem(i2);
				});

				std::vector<Sim::OverrideSetValueData> overrideData;
				overrideData.reserve(selectedItems.size());

				for (QTreeWidgetItem* selectedItem : selectedItems)
				{
					QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItem);
					assert(item != nullptr);

					if (item->m_overrideSignal.signalType() == E::SignalType::Discrete)
					{
						quint16 currentValue = item->m_overrideSignal.value().value<quint16>();
						currentValue = currentValue ? 0 : 1;

						QString appSignalId = item->m_overrideSignal.appSignalId();

						overrideData.emplace_back(appSignalId, Sim::OverrideSignalMethod::Value, QVariant::fromValue<qint32>(currentValue));
					}
				}

				if (overrideData.empty() == false)
				{
					m_simulator->overrideSignals().setValues(overrideData);
				}

				// Turn off/on override for current signal
				//
//				QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->currentItem());
//				if (item != nullptr)
//				{
//					// It makes toggle state (check box) from any selected column (not only for column 0 as by default)
//					//
//					item->setCheckState(0, item->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
//				}

				updateValueColumn();
			}
			return true;
		case Qt::Key_0:
			{
				QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();

				std::sort(selectedItems.begin(), selectedItems.end(), [this](QTreeWidgetItem* i1, QTreeWidgetItem* i2) {
					return m_treeWidget->indexOfTopLevelItem(i1) < m_treeWidget->indexOfTopLevelItem(i2);
				});

				std::vector<Sim::OverrideSetValueData> overrideData;
				overrideData.reserve(selectedItems.size());

				for (QTreeWidgetItem* selectedItem : selectedItems)
				{
					QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItem);
					assert(item != nullptr);

					if (item->m_overrideSignal.signalType() == E::SignalType::Discrete)
					{
						quint16 currentValue = item->m_overrideSignal.value().value<quint16>();
						currentValue = currentValue ? 0 : 1;

						QString appSignalId = item->m_overrideSignal.appSignalId();

						overrideData.emplace_back(appSignalId, Sim::OverrideSignalMethod::Value, QVariant::fromValue<qint32>(currentValue));
					}
				}

				if (overrideData.empty() == false)
				{
					m_simulator->overrideSignals().setValues(overrideData);
				}

				updateValueColumn();
			}
			return true;
		case Qt::Key_1:
			{
				QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();

				std::sort(selectedItems.begin(), selectedItems.end(), [this](QTreeWidgetItem* i1, QTreeWidgetItem* i2) {
					return m_treeWidget->indexOfTopLevelItem(i1) < m_treeWidget->indexOfTopLevelItem(i2);
				});

				std::vector<Sim::OverrideSetValueData> overrideData;
				overrideData.reserve(selectedItems.size());

				for (QTreeWidgetItem* selectedItem : selectedItems)
				{
					QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItem);
					assert(item != nullptr);

					if (item->m_overrideSignal.signalType() == E::SignalType::Discrete)
					{
						quint16 currentValue = item->m_overrideSignal.value().value<quint16>();
						currentValue = currentValue ? 0 : 1;

						QString appSignalId = item->m_overrideSignal.appSignalId();
						
						overrideData.emplace_back(appSignalId, Sim::OverrideSignalMethod::Value, QVariant::fromValue<qint32>(currentValue));
					}
				}

				if (overrideData.empty() == false)
				{
					m_simulator->overrideSignals().setValues(overrideData);
				}

				updateValueColumn();
			}
			return true ;
		}

		return false;		// return false to process event
	}

	return false;			// return false to process event
}

void SimOverridePane::contextMenuEvent(QContextMenuEvent* event)
{
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();

	QStringList appSignalIds;
	appSignalIds.reserve(selectedItems.size());

	bool signalsHaveTheSameType = true;
	std::optional<Sim::OverrideSignalParam> firstSignal;

	for (QTreeWidgetItem* selectedItem : selectedItems)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItem);
		assert(item != nullptr);

		appSignalIds.push_back(item->m_overrideSignal.appSignalId());

		// Check if selected isgnals have the same type and data format.
		//
		if (firstSignal.has_value() == false)
		{
			firstSignal = item->m_overrideSignal;
		}
		else
		{
			signalsHaveTheSameType &= firstSignal->sameType(item->m_overrideSignal);
		}
	}

	QMenu menu{this};


	// Set Value
	//
	QAction* setValueAction = menu.addAction(tr("Set Value..."),
											[this, &appSignalIds, signalsHaveTheSameType]
											{
												if (signalsHaveTheSameType == true && appSignalIds.empty() == false)
												{
													showSetValueDialog(appSignalIds);
												}
											});
	setValueAction->setEnabled(signalsHaveTheSameType == true && appSignalIds.empty() == false);

	menu.setDefaultAction(setValueAction);

	// Add Signal
	//
	QAction* addSignalAction = menu.addAction(tr("Add Signal..."),
											[this]
											{
												addSignal();
											});
	addSignalAction->setShortcut(Qt::Key::Key_Insert);

	// Remove Signal
	//
	QAction* removeSignalAction = menu.addAction(tr("Remove Signal(s)"),
												[this]
												{
													removeSelectedSignals();
												});
	removeSignalAction->setShortcut(QKeySequence::Delete);
	removeSignalAction->setEnabled(appSignalIds.isEmpty() == false);

	// Clear
	//
	QAction* clearAction = menu.addAction(tr("Clear All"), [this](){ clear(); });
	Q_UNUSED(clearAction);

	// ---------------------------------------
	//
	menu.addSeparator()->setText(tr("Workspace"));

	// Save to files
	//
	QAction* saveWorkspaceAction = menu.addAction(tr("Save Worksapce..."), [this]{	saveWorkspace();	});
	saveWorkspaceAction->setEnabled(m_treeWidget->topLevelItemCount() != 0);

	// Save to files
	//
	QAction* restoreWorkspaceAction = menu.addAction(tr("Restore Workspace..."), [this]{	restoreWorkspace();	});
	Q_UNUSED(restoreWorkspaceAction);

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

	copyAction->setEnabled(appSignalIds.size() == 1);

	// --
	//
	menu.exec(event->globalPos());

	if (formatChanged == true)
	{
		updateValueColumn();
		SimOverrideUI::OverrideDialog::setViewOptions(appSignalIds, m_currentBase, m_currentFormat, m_currentPrecision);
	}

	return;
}

void SimOverridePane::updateValueColumn()
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

		QString appSignalId = treeItem->m_overrideSignal.appSignalId();

		auto it = std::find_if(currentSignals.begin(), currentSignals.end(),
								[&appSignalId](const Sim::OverrideSignalParam& osp)
								{
			                        return osp.appSignalId() == appSignalId;
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

		Qt::CheckState cs = osp.enabled() ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;

		treeItem->setCheckState(0, cs);
	}

	return;
}

void SimOverridePane::fillListWidget(const std::vector<Sim::OverrideSignalParam>& overrideSignals)
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

void SimOverridePane::selectSignal(QString appSignalId)
{
	int count = m_treeWidget->topLevelItemCount();
	m_treeWidget->setCurrentItem(nullptr);

	for (int i = 0; i < count; i++)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->topLevelItem(i));

		if (item == nullptr)
		{
			assert(item);
			return;
		}

		if (item->m_overrideSignal.appSignalId() == appSignalId)
		{
			item->setSelected(true);
			m_treeWidget->setCurrentItem(item);
			m_treeWidget->scrollToItem(item);
		}
		else
		{
			item->setSelected(false);
		}
	}

	return;
}

void SimOverridePane::itemDoubleClicked(QTreeWidgetItem* /*item*/, int /*column*/)
{
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();

	QStringList appSignalIds;
	appSignalIds.reserve(selectedItems.size());

	bool signalsHaveTheSameType = true;
	std::optional<Sim::OverrideSignalParam> firstSignal;

	QString detailsText;

	for (QTreeWidgetItem* selectedItem : selectedItems)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItem);
		assert(item != nullptr);

		appSignalIds.push_back(item->m_overrideSignal.appSignalId());

		// Check if selected isgnals have the same type and data format.
		//
		if (firstSignal.has_value() == false)
		{
			firstSignal = item->m_overrideSignal;
		}
		else
		{
			signalsHaveTheSameType &= firstSignal->sameType(item->m_overrideSignal);
		}

		if (item->m_overrideSignal.signalType() == E::SignalType::Analog)
		{
			detailsText += QString{"%1, type: %2, data format: %3\n"}
						   .arg(item->m_overrideSignal.appSignalId())
						   .arg(E::valueToString(item->m_overrideSignal.signalType()))
						   .arg(E::valueToString(item->m_overrideSignal.dataFormat()));
		}

		if (item->m_overrideSignal.signalType() == E::SignalType::Discrete)
		{
			detailsText += QString{"%1, type: %2\n"}
						   .arg(item->m_overrideSignal.appSignalId())
						   .arg(E::valueToString(item->m_overrideSignal.signalType()));
		}

	}

	if (signalsHaveTheSameType == false)
	{
		QMessageBox mb{this};
		mb.setIcon(QMessageBox::Warning);
		mb.setText(tr("Selected signals must have the same type and data format."));
		mb.setDetailedText(detailsText);

		mb.exec();
		return;
	}

	showSetValueDialog(appSignalIds);

	return;
}

void SimOverridePane::itemChanged(QTreeWidgetItem* item, int column)
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

void SimOverridePane::signalsChanged(QStringList addedAppSignalIds)
{
	auto overrideSignals = m_simulator->overrideSignals().overrideSignals();

	fillListWidget(overrideSignals);

	if (addedAppSignalIds.size() == 1)
	{
		selectSignal(addedAppSignalIds.front());
	}

	return;
}

void SimOverridePane::signalStateChanged(QStringList /*appSignalId*/)
{
	// This slot can be called very frequently if the FastForward mode is on.
	// To avoid UI freezing, skip updating values if they come too often.
	// Also, signal states are sometimes updated by timer.
	//
	if (m_signalStateSlotTimer.isValid() == false)
	{
		m_signalStateSlotTimer.start();
	}
	else
	{
		if (m_signalStateSlotTimer.elapsed() < 50)	// ms
		{
			return;
		}
	}

	m_signalStateSlotTimer.restart();

	// --
	//
	updateValueColumn();

	return;
}

void SimOverridePane::clear()
{
	m_simulator->overrideSignals().clear();
}

void SimOverridePane::removeSelectedSignals()
{
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();

	std::sort(selectedItems.begin(), selectedItems.end(), [this](QTreeWidgetItem* i1, QTreeWidgetItem* i2) {
		return m_treeWidget->indexOfTopLevelItem(i1) < m_treeWidget->indexOfTopLevelItem(i2);
	});

	QStringList appSignalIds;
	appSignalIds.reserve(selectedItems.size());

	QString belowSignalId;

	for (QTreeWidgetItem* selectedItem : selectedItems)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItem);
		assert(item != nullptr);

		QString appSignalId  = item->m_overrideSignal.appSignalId();
		appSignalIds.push_back(appSignalId);

		if (item == selectedItems.back())
		{
			if (auto itemBelow = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->itemBelow(item));
				itemBelow != nullptr)
			{
				belowSignalId = itemBelow->m_overrideSignal.appSignalId();
			}
		}
	}

	for (const QString& appSignalId : appSignalIds)
	{
		removeSignal(appSignalId);
	}

	if (belowSignalId.isEmpty() == false)
	{
		selectSignal(belowSignalId);
	}
	else
	{
		if (m_treeWidget->topLevelItemCount() != 0)
		{
			auto lastItem = m_treeWidget->topLevelItem(m_treeWidget->topLevelItemCount() - 1);
			lastItem->setSelected(true);
		}
	}

	return;
}

void SimOverridePane::removeSignal(QString appSignalId)
{
	m_simulator->overrideSignals().removeSignal(appSignalId);
}

void SimOverridePane::addSignal()
{
	QString defaultText;

	const QClipboard* clipboard = QApplication::clipboard();
	const QMimeData* mimeData = clipboard->mimeData();

	if (mimeData->hasText() == true && mimeData->text().trimmed().isEmpty() == false)
	{
		defaultText = mimeData->text().trimmed();
	}

	do
	{
		bool ok = false;
		QString signalId = QInputDialog::getText(this,
													tr("Add Signal to Override"),
													tr("SignalID:"),
													QLineEdit::Normal,
													defaultText,
													&ok).trimmed();

		if (ok == true && signalId.isEmpty() == false)
		{
			QString appSignalId;

			if (signalId.at(0) == QChar('#'))
			{
				appSignalId = signalId;
			}
			else
			{
				// To add signal to override, AppSignalId is required, so go and get it
				//
				Hash appSignalIdHash = m_simulator->appSignalManager().customToAppSignal(::calcHash(signalId));

				AppSignalParam appSignalParam = m_simulator->appSignalManager().signalParam(appSignalIdHash, &ok);
				if (ok == false)
				{
					QMessageBox::critical(this, qAppName(), tr("Signal %1 not found.").arg(signalId));
					defaultText = signalId;
					continue;
				}

				appSignalId = appSignalParam.appSignalId();
			}

			// If signal already added to simulation, just select it
			//
			if (m_simulator->overrideSignals().containsSignal(appSignalId) == true)
			{
				selectSignal(appSignalId);
				return;
			}

			// Get current signal value, and set it as default
			//
			AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, nullptr, false);

			// Add signal to override list
			//
			m_simulator->overrideSignals().addSignals(QStringList{} << appSignalId);

			// Set default value to override, is actual signal state
			//
			m_simulator->overrideSignals().setValue(appSignalId, Sim::OverrideSignalMethod::Value, state.value());

			selectSignal(appSignalId);
		}

		break;
	}
	while (true);

	return;
}

void SimOverridePane::saveWorkspace()
{
	if (m_treeWidget->topLevelItemCount() == 0)
	{
		return;
	}

	static QString path{QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/untitled.sow"};
	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													path + QDir::separator() + "untitled.sow",
													tr("u7 Signal Override Workspace (*.sow);;All files (*.*)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}
	path = QFileInfo(fileName).path(); // store path for next time

	m_simulator->overrideSignals().saveWorkspace(fileName);
	return;
}

void SimOverridePane::restoreWorkspace()
{
	QString fileName = QFileDialog::getOpenFileName(this,
													tr("Open File"),
													QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
													tr("u7 Signal Override Workspace (*.sow);;All files (*.*)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}

	m_simulator->overrideSignals().loadWorkspace(fileName);
	return;
}

void SimOverridePane::showSetValueDialog(const QStringList& appSignalIds)
{
	std::vector<Sim::OverrideSignalParam> overrideValueSignals;
	overrideValueSignals.reserve(appSignalIds.size());

	for (const QString& appSignalId : appSignalIds)
	{
		std::optional<Sim::OverrideSignalParam> osp = m_simulator->overrideSignals().overrideSignal(appSignalId);

		if (osp.has_value() == false)
		{
			assert(osp.has_value());
			return;
		}

		overrideValueSignals.emplace_back(osp.value());
	}

	SimOverrideUI::OverrideDialog::showDialog(overrideValueSignals, *m_simulator, dbc(), this);
	SimOverrideUI::OverrideDialog::setViewOptions(appSignalIds, m_currentBase, m_currentFormat, m_currentPrecision);

	return;
}

void SimOverridePane::setValue(QString appSignalId, Sim::OverrideSignalMethod method, const QVariant& value)
{
	m_simulator->overrideSignals().setValue(appSignalId, method, value);
}


namespace
{
	class NoCurrentRectItemDelegate : public QStyledItemDelegate
	{
	public:
		NoCurrentRectItemDelegate(QObject* parent) :
			QStyledItemDelegate(parent)
		{
		}

	protected:
		void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
		{
			QStyleOptionViewItem itemOption{option};
			itemOption.state &= ~QStyle::State_HasFocus;

			QStyledItemDelegate::paint(painter, itemOption, index);
			return;
		}
	};
}


QOverrideListWidget::QOverrideListWidget(Sim::Simulator* simulator, QWidget* parent) :
	QTreeWidget(parent),
	m_simulator(simulator)
{
	assert(m_simulator);

	setRootIsDecorated(false);
	setUniformRowHeights(true);

	setColumnCount(static_cast<int>(QOverrideTreeWidgetItem::Columns::ColumnCount));

	QStringList headerLabels;
	headerLabels << "No";
	headerLabels << "SignalID";
	headerLabels << "Caption";
	headerLabels << "Type";
	headerLabels << "Override Value";

	assert(headerLabels.size() == static_cast<int>(QOverrideTreeWidgetItem::Columns::ColumnCount));

	setHeaderLabels(headerLabels);

	// Set selection and draw focused cell workaround.
	//
	setSelectionMode(QAbstractItemView::ExtendedSelection);
	setItemDelegate(new NoCurrentRectItemDelegate(this));

	return;
}

void QOverrideListWidget::mousePressEvent(QMouseEvent* event)
{
	// Get all selected items
	//
	QList<QTreeWidgetItem*> selectedItems = this->selectedItems();

	QStringList appDataIds;
	appDataIds.reserve(selectedItems.size());

	for (QTreeWidgetItem* selectedItem : selectedItems)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItem);
		assert(item != nullptr);

		appDataIds.push_back(item->m_overrideSignal.appSignalId());
	}


	// --
	//
	if (appDataIds.isEmpty() == false && event->buttons().testFlag(Qt::LeftButton) == true)
	{
		m_dragStartPos = event->pos();
		m_dragAppSignalIds = std::move(appDataIds);
	}
	else
	{
		m_dragStartPos = {};
		m_dragAppSignalIds.clear();
	}

	if (state() == DragSelectingState ||
		state() == DraggingState)
	{
		// This fixes problem of lost click after starting drag
		// This code is just taken from inet
		//
		setState(NoState);
	}

	return QTreeWidget::mousePressEvent(event);
}

void QOverrideListWidget::mouseMoveEvent(QMouseEvent* event)
{
	QTreeWidget::mouseMoveEvent(event);

	if (m_dragAppSignalIds.isEmpty() == false &&
		event->buttons().testFlag(Qt::LeftButton) == true &&
		(event->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance())
	{
		// Save signals to protobufer
		//
		::Proto::AppSignalSet protoSetMessage;

		for (const QString& appSignalId : m_dragAppSignalIds)
		{
			bool ok = false;

			AppSignalParam signalParam = m_simulator->appSignalManager().signalParam(appSignalId, &ok);
			if (ok == false)
			{
				continue;
			}

			assert(signalParam.appSignalId() == appSignalId) ;

			::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
			signalParam.save(protoSignalMessage);
		}

		QByteArray data;
		data.resize(static_cast<int>(protoSetMessage.ByteSizeLong()));

		protoSetMessage.SerializeToArray(data.data(), static_cast<int>(protoSetMessage.ByteSizeLong()));

		// --
		//
		if (protoSetMessage.appsignal_size() > 0 && data.isEmpty() == false)
		{
			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new QMimeData;

			mimeData->setData(AppSignalParamMimeType::value, data);
			drag->setMimeData(mimeData);

			drag->exec(Qt::CopyAction);
		}
	}

	return;
}


QOverrideTreeWidgetItem::QOverrideTreeWidgetItem(const Sim::OverrideSignalParam& overrideSignal) :
	QTreeWidgetItem(),
	m_overrideSignal(overrideSignal)
{
	QString type = E::valueToString(overrideSignal.signalType());
	if (overrideSignal.signalType() == E::SignalType::Analog)
	{
		type = E::valueToString(overrideSignal.dataFormat());
	}

	this->setText(static_cast<int>(Columns::Index), QString::number(overrideSignal.index()));
	this->setText(static_cast<int>(Columns::CustomSignalId), overrideSignal.customSignalId());
	this->setText(static_cast<int>(Columns::Caption), overrideSignal.caption());
	this->setText(static_cast<int>(Columns::Type), type);
	this->setText(static_cast<int>(Columns::Value), overrideSignal.valueString());

	this->setCheckState(0, overrideSignal.enabled() ? Qt::Checked : Qt::Unchecked);

	return;
}

QOverrideTreeWidgetItem::~QOverrideTreeWidgetItem()
{
}

QString QOverrideTreeWidgetItem::appSignalId() const
{
	return m_overrideSignal.appSignalId();
}

bool QOverrideTreeWidgetItem::operator < (const QTreeWidgetItem& other) const
{
	int column = treeWidget()->sortColumn();

	switch (static_cast<Columns>(column))
	{
	case Columns::Index:
		return text(column).toInt() < other.text(column).toInt();
	case Columns::Value:
		return text(column).toDouble() < other.text(column).toDouble();
	default:
		return text(column).toLower() < other.text(column).toLower();
	}
}
