#include "SimOverridePane.h"
#include "../../lib/AppSignalParam.h"
#include "../../Simulator/SimOverrideSignals.h"
#include "../../Proto/serialization.pb.h"
#include "SimOverrideValueWidget.h"


SimOverridePane::SimOverridePane(Sim::Simulator* simulator, DbController* dbc, QWidget* parent) :
	QWidget(parent),
    HasDbController(dbc),
	m_simulator(simulator)
{
	assert(m_simulator);

	m_treeWidget = new QOverrideListWidget(m_simulator, this);
	m_treeWidget->installEventFilter(this);

	m_treeWidget->setRootIsDecorated(false);
	m_treeWidget->setUniformRowHeights(true);

	m_treeWidget->setColumnCount(static_cast<int>(QOverrideTreeWidgetItem::Columns::ColumnCount));

	QStringList headerLabels;
	headerLabels << "No";
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
	bool ok = protoSetMessage.ParseFromArray(data.constData(), data.size());

	if (ok == false)
	{
		event->acceptProposedAction();
		return;
	}

	// Parse data
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

			AppSignalState state = m_simulator->appSignalManager().signalState(appSignalId, nullptr, false);
			bool isAlreadyOverriden = m_simulator->overrideSignals().isSignalInOverrideList(appSignalId);

			signalIds << appSignalId;
			signalValues.push_back(std::tuple{appSignalId, isAlreadyOverriden, state.value()});
		}
	}

	if (signalIds.isEmpty() == false)
	{
		// Check that signals are not optimized constants
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
			// Appartently signal already added, select it
			//
			selectSignal(signalIds.back());
		}

		// SetInitialValues to currents
		//
		for (auto&[appSignalId, isAlreadyOverriden, value] : signalValues)
		{
			if (isAlreadyOverriden == false)
			{
				m_simulator->overrideSignals().setValue(appSignalId, Sim::OverrideSignalMethod::Value, value);
			}
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
				QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->currentItem());
				if (item != nullptr)
				{
					// It makes toggle state (check box) from any selected column (not only for column 0 as by default)
					//
					item->setCheckState(0, item->checkState(0) == Qt::Checked ? Qt::Unchecked : Qt::Checked);
				}
			}
			return true;
		case Qt::Key_0:
			{
				QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->currentItem());
				if (item != nullptr)
				{
					QString appSignalId = item->m_overrideSignal.appSignalId();
					setValue(appSignalId, Sim::OverrideSignalMethod::Value, QVariant::fromValue<qint32>(0));
				}
			}
			return true;
		case Qt::Key_1:
			{
				QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->currentItem());
				if (item != nullptr)
				{
					QString appSignalId = item->m_overrideSignal.appSignalId();
					setValue(appSignalId, Sim::OverrideSignalMethod::Value, QVariant::fromValue<qint32>(1));
				}
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

	QString appSignalId;
	if (selectedItems.size() == 1)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(selectedItems.front());
		assert(item != nullptr);

		appSignalId = item->m_overrideSignal.appSignalId();
	}

	QMenu menu(this);

	// Set Value
	//
	QAction* setValueAction = menu.addAction(tr("Set Value..."),
											[this, &appSignalId]
											{
												showSetValueDialog(appSignalId);
											});

	setValueAction->setEnabled(!appSignalId.isEmpty());

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
	QAction* removeSignalAction = menu.addAction(tr("Remove Signal"),
												[this, &appSignalId]
												{
													removeSignal(appSignalId);
												});
	removeSignalAction->setShortcut(QKeySequence::Delete);
	removeSignalAction->setEnabled(!appSignalId.isEmpty());

	// Clear
	//
	QAction* clearAction = menu.addAction(tr("Clear"), [this]{clear();});
	clearAction->setEnabled(!selectedItems.empty());

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

	copyAction->setEnabled(!appSignalId.isEmpty());

	// --
	//
	menu.exec(event->globalPos());

	if (formatChanged == true)
	{
		updateValueColumn();
		SimOverrideUI::OverrideValueWidget::setViewOptions(appSignalId, m_currentBase, m_currentFormat, m_currentPrecision);
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

	for (int i = 0; i < count; i++)
	{
		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(m_treeWidget->topLevelItem(i));

		if (item == nullptr)
		{
			assert(item);
			return;
		}

		item->setSelected(item->m_overrideSignal.appSignalId() == appSignalId);

		if (item->isSelected() == true)
		{
			m_treeWidget->scrollToItem(item);
		}
	}

	return;
}

void SimOverridePane::itemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	QOverrideTreeWidgetItem* toItem = dynamic_cast<QOverrideTreeWidgetItem*>(item);
	assert(toItem);

	showSetValueDialog(toItem->m_overrideSignal.appSignalId());

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
	updateValueColumn();
}

void SimOverridePane::clear()
{
	m_simulator->overrideSignals().clear();
}

void SimOverridePane::removeSelectedSignals()
{
	QTreeWidgetItem* currentItem = m_treeWidget->currentItem();

	if (currentItem != nullptr)
	{
		int index = m_treeWidget->indexOfTopLevelItem(currentItem);

		QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(currentItem);
		assert(item != nullptr);

		QString appSignalId = item->m_overrideSignal.appSignalId();
		removeSignal(appSignalId);

		// Select next item
		//
		if (index < m_treeWidget->topLevelItemCount())
		{
			QTreeWidgetItem* nextItem = m_treeWidget->topLevelItem(index);
			assert(nextItem);

			if (nextItem != nullptr)
			{
				nextItem->setSelected(true);
				m_treeWidget->setCurrentItem(nextItem);
			}
		}
		else
		{
			if (m_treeWidget->topLevelItemCount() != 0)
			{
				QTreeWidgetItem* lastItem = m_treeWidget->topLevelItem(m_treeWidget->topLevelItemCount() - 1);
				assert(lastItem);

				if (lastItem != nullptr)
				{
					lastItem->setSelected(true);
					m_treeWidget->setCurrentItem(lastItem);
				}
			}
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
			if (m_simulator->overrideSignals().isSignalInOverrideList(appSignalId) == true)
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

	QString fileName = QFileDialog::getSaveFileName(this,
													tr("Save File"),
													QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/untitled.sow",
													tr("u7 Signal Override Workspace (*.sow);;All files (*.*)"));

	if (fileName.isEmpty() == true)
	{
		return;
	}

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

void SimOverridePane::showSetValueDialog(QString appSignalId)
{
	std::optional<Sim::OverrideSignalParam> osp = m_simulator->overrideSignals().overrideSignal(appSignalId);

	if (osp.has_value() == false)
	{
		assert(osp.has_value());
		return;
	}

	SimOverrideUI::OverrideValueWidget::showDialog(osp.value(), m_simulator, dbc(), this);
	SimOverrideUI::OverrideValueWidget::setViewOptions(osp.value().appSignalId(), m_currentBase, m_currentFormat, m_currentPrecision);

	return;
}

void SimOverridePane::setValue(QString appSignalId, Sim::OverrideSignalMethod method, const QVariant& value)
{
	m_simulator->overrideSignals().setValue(appSignalId, method, value);
}


QOverrideListWidget::QOverrideListWidget(Sim::Simulator* simulator, QWidget* parent) :
	QTreeWidget(parent),
	m_simulator(simulator)
{
	assert(m_simulator);
}

void QOverrideListWidget::mousePressEvent(QMouseEvent* event)
{
	if (QOverrideTreeWidgetItem* item = dynamic_cast<QOverrideTreeWidgetItem*>(this->itemAt(event->pos()));
		event->buttons().testFlag(Qt::LeftButton) == true &&
		item != nullptr)
	{
		m_dragStartPos = event->pos();
		m_dragAppSignalId = item->appSignalId();
	}
	else
	{
		m_dragStartPos = {};
		m_dragAppSignalId.clear();
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
	if (m_dragAppSignalId.isEmpty() == false &&
		event->buttons().testFlag(Qt::LeftButton) == true &&
		(event->pos() - m_dragStartPos).manhattanLength() >= QApplication::startDragDistance())
	{
		// Save signals to protobufer
		//
		::Proto::AppSignalSet protoSetMessage;

		bool ok = false;
		AppSignalParam signalParam = m_simulator->appSignalManager().signalParam(m_dragAppSignalId, &ok);

		if (ok == false)
		{
			return QTreeWidget::mouseMoveEvent(event);
		}

		assert(signalParam.appSignalId() == m_dragAppSignalId) ;

		::Proto::AppSignal* protoSignalMessage = protoSetMessage.add_appsignal();
		signalParam.save(protoSignalMessage);

		QByteArray data;
		data.resize(static_cast<int>(protoSetMessage.ByteSizeLong()));

		protoSetMessage.SerializeToArray(data.data(), static_cast<int>(protoSetMessage.ByteSizeLong()));

		// --
		//
		if (data.isEmpty() == false)
		{
			QDrag* drag = new QDrag(this);
			QMimeData* mimeData = new QMimeData;

			mimeData->setData(AppSignalParamMimeType::value, data);
			drag->setMimeData(mimeData);

			drag->exec(Qt::CopyAction);
		}
	}

	return QTreeWidget::mouseMoveEvent(event);
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
