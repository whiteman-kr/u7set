#include "ArchiveModelView.h"

//
//
//		ArchiveModel
//
//
ArchiveModel::ArchiveModel(QObject* parent) :
	QAbstractTableModel(parent)
{
}

int ArchiveModel::rowCount(const QModelIndex& /*parent*/) const
{
	return m_archive.size();
}

int ArchiveModel::columnCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(ArchiveColumns::ColumnCount);
}

QVariant ArchiveModel::headerData(int section, Qt::Orientation /*orientation*/, int role /*= Qt::DisplayRole*/) const
{
	if (role != Qt::DisplayRole)
	{
		return {};
	}

	switch (static_cast<ArchiveColumns>(section))
	{
	case ArchiveColumns::Row:
		return tr("Row");

	case ArchiveColumns::AppSignalId:
		return tr("AppSignalID");

	case ArchiveColumns::CustomSignalId:
		return tr("SignalID");

	case ArchiveColumns::Caption:
		return tr("Caption");

	case ArchiveColumns::State:
		return tr("State");

	case ArchiveColumns::Valid:
		return tr("Valid");

	case ArchiveColumns::StateAvailable:
		return tr("StateAvailable");

	case ArchiveColumns::Simulated:
		return tr("Simulated");

	case ArchiveColumns::Blocked:
		return tr("Blocked");

	case ArchiveColumns::Mismatch:
		return tr("Mismatch");

	case ArchiveColumns::OutOfLimits:
		return tr("OutOfLimits");

	case ArchiveColumns::ArchivingReason:
		return tr("ArchivingReason");

	case ArchiveColumns::Time:
		return tr("Time");

	case ArchiveColumns::Server:
		return tr("Server");

	case ArchiveColumns::ColumnCount:
		Q_ASSERT(false);
		break;		
	}

	Q_ASSERT(false);
	return {};
}

QVariant ArchiveModel::data(const QModelIndex& index, int role) const
{
	return data(index.row(), index.column(), role);
}

QVariant ArchiveModel::data(int row, int column, int role) const
{
	if (row >= m_archive.size())
	{
		return {};
	}

	if (role == Qt::DisplayRole)
	{
		QVariant result;
		updateCachedState(row);		// m_cachedSignalState -- state for row

		switch (static_cast<ArchiveColumns>(column))
		{
		case ArchiveColumns::Row:
			result = row + 1;
			break;

		case ArchiveColumns::AppSignalId:
			if (auto sit = m_archiveSignalsMap.find(m_cachedSignalState.appState.hash());
				sit == m_archiveSignalsMap.end())
			{
				// State from differtent signal!!! ArchiveService has returned something wrong?
				//
				Q_ASSERT(false);
			}
			else
			{
				result = sit->second.signalParam.appSignalId();
			}
			break;

		case ArchiveColumns::CustomSignalId:
			if (auto sit = m_archiveSignalsMap.find(m_cachedSignalState.appState.hash());
				sit == m_archiveSignalsMap.end())
			{
				// State from differtent signal!!! ArchiveService has returned something wrong?
				//
				Q_ASSERT(false);
			}
			else
			{
				result = sit->second.signalParam.customSignalId();
			}
			break;
		case ArchiveColumns::Caption:
			if (auto sit = m_archiveSignalsMap.find(m_cachedSignalState.appState.hash());
				sit == m_archiveSignalsMap.end())
			{
				// State from differtent signal!!! ArchiveService has returned something wrong?
				//
				Q_ASSERT(false);
			}
			else
			{
				result = sit->second.signalParam.caption();
			}
			break;
		case ArchiveColumns::State:
			if (auto sit = m_archiveSignalsMap.find(m_cachedSignalState.appState.hash());
				sit == m_archiveSignalsMap.end())
			{
				// State from differtent signal!!! ArchiveService has returned something wrong?
				//
				Q_ASSERT(false);
			}
			else
			{
				result = getValueString(m_cachedSignalState.appState, sit->second);
			}
			break;
		case ArchiveColumns::Valid:
			{
				result = m_cachedSignalState.appState.isValid() ? QString() : QStringLiteral("no");
			}
			break;
		case ArchiveColumns::StateAvailable:
			{
				result = m_cachedSignalState.appState.m_flags.stateAvailable ? QString() : QStringLiteral("no");
			}
			break;
		case ArchiveColumns::Simulated:
			{
				result = m_cachedSignalState.appState.m_flags.simulated ? QStringLiteral("yes") : QString();
			}
			break;
		case ArchiveColumns::Blocked:
			{
				result = m_cachedSignalState.appState.m_flags.blocked ? QStringLiteral("yes") : QString();
			}
			break;
		case ArchiveColumns::Mismatch:
			{
				result = m_cachedSignalState.appState.m_flags.mismatch ? QStringLiteral("yes") : QString();
			}
			break;
		case ArchiveColumns::OutOfLimits:
			{
				QStringList resultString;

				if (m_cachedSignalState.appState.m_flags.belowLowLimit == true)
				{
					resultString << QStringLiteral("LOW ");
				}
				if (m_cachedSignalState.appState.m_flags.aboveHighLimit == true)
				{
					resultString << QStringLiteral("HIGH ");
				}

				result = resultString.join(' ');
			}
			break;
		case ArchiveColumns::ArchivingReason:
			{
				QStringList resultString;

				if (m_cachedSignalState.appState.m_flags.validityChange == true)
				{
					resultString << tr("VALIDITY");
				}
				if (m_cachedSignalState.appState.m_flags.simBlockMismatchChange == true)
				{
					resultString << tr("SIMLOCK");
				}
				if (m_cachedSignalState.appState.m_flags.limitFlagsChange == true)
				{
					resultString << tr("LIMIT");
				}
				if (m_cachedSignalState.appState.m_flags.autoPoint == true)
				{
					resultString << tr("AUTO");
				}
				if (m_cachedSignalState.appState.m_flags.fineAperture == true)
				{
					resultString << tr("FINEAP");
				}
				if (m_cachedSignalState.appState.m_flags.coarseAperture == true)
				{
					resultString << tr("COARSEAP");
				}

				result = resultString.join(' ');
			}
			break;
		case ArchiveColumns::Time:
			{
				const TimeStamp& ts = m_cachedSignalState.appState.time(m_timeType);
				result = ts.toDateTime().toString("dd/MM/yyyy HH:mm:ss.zzz");
			}
			break;
		case ArchiveColumns::Server:
			{
				result = m_cachedSignalState.archiveServiceShortenId;
			}
			break;
		default:
			Q_ASSERT(false);
		}

		return result;
	}

	if (role == Qt::TextAlignmentRole &&
		(column ==  static_cast<int>(ArchiveColumns::Row) ||
		 column ==  static_cast<int>(ArchiveColumns::State) ||
		 column ==  static_cast<int>(ArchiveColumns::Valid) ||
		 column ==  static_cast<int>(ArchiveColumns::StateAvailable) ||
		 column ==  static_cast<int>(ArchiveColumns::Simulated) ||
		 column ==  static_cast<int>(ArchiveColumns::Blocked) ||
		 column ==  static_cast<int>(ArchiveColumns::Mismatch)))
	{
		return {Qt::AlignCenter};
	}

	if (role == Qt::ToolTipRole)
	{
		updateCachedState(row);		// m_cachedSignalState -- state for row
		ArchiveSignalParam signalParam;

		if (auto sit = m_archiveSignalsMap.find(m_cachedSignalState.appState.hash());
			sit == m_archiveSignalsMap.end())
		{
			// State from differtent signal!!! ArchiveService has returned something wrong?
			//
			Q_ASSERT(false);
		}
		else
		{
			signalParam = sit->second;
		}

		QString typeStr;
		switch (signalParam.signalParam.type())
		{
		case E::SignalType::Analog:
			typeStr = tr("Analog");
			break;
		case E::SignalType::Discrete:
			typeStr = tr("Discrete");
			break;
		case E::SignalType::Bus:
			typeStr = tr("Bus");
			break;
		default:
			Q_ASSERT(false);
		}

		QString toolTip = tr("StateIndex: %1\n"
								  "SignalID: %2\n"
								  "AppSignalID: %3\n"
								  "Caption: %4\n"
								  "Type: %5\n"
								  "Value: %6 (%7)\n"
								  "Flags: %8\n"
								  "Time: %9 (%10)\n"
								  "ServerTime: %11\n"
								  "ServerTime +0UTC: %12\n"
								  "PlantTime: %13\n"
								  "Server: %14")
						  .arg(row + 1)
						  .arg(signalParam.signalParam.customSignalId())
						  .arg(signalParam.signalParam.appSignalId())
						  .arg(signalParam.signalParam.caption())
						  .arg(typeStr)
						  .arg(getValueString(m_cachedSignalState.appState, signalParam))
						  .arg(m_cachedSignalState.appState.m_value)
						  .arg(QString::number(m_cachedSignalState.appState.m_flags.all, 2))
						  .arg(m_cachedSignalState.appState.time(m_timeType).toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"))
						  .arg(E::valueToString<E::TimeType>(m_timeType))
						  .arg(m_cachedSignalState.appState.time().system.toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"))			//"ServerTime: %12\n"
						  .arg(m_cachedSignalState.appState.time().local.toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"))			//"ServerTime +0UTC: %13\n"
						  .arg(m_cachedSignalState.appState.time().plant.toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"))			//"PlantTime: %14"
						  .arg(m_cachedSignalState.archiveServiceShortenId);


		return toolTip;
	}

	return {};
}

QString ArchiveModel::getValueString(const AppSignalState& state, const ArchiveSignalParam& signalParam) const
{
	E::SignalType signalType = signalParam.signalParam.type();
	QString result;

	switch (signalType)
	{
	case E::SignalType::Analog:
		if (m_cachedSignalState.appState.isValid() == false)
		{
			result = QString("%1 (%2)")
						.arg(NonValidString)
						.arg(AppSignalState::toString(state.value(),
													  signalParam.viewType,
													  E::AnalogFormat::f_9,
													  signalParam.signalParam.analogSignalFormat(),
													  signalParam.precision));
		}
		else
		{
			result = AppSignalState::toString(state.value(),
											  signalParam.viewType,
											  E::AnalogFormat::f_9,
											  signalParam.signalParam.analogSignalFormat(),
											  signalParam.precision);
		}
		break;
	case E::SignalType::Discrete:
		if (m_cachedSignalState.appState.isValid() == false)
		{
			result = QString("%1 (%2)")
						.arg(NonValidString)
						.arg(QString::number(state.value()));
		}
		else
		{
			result = QString::number(state.value());
		}
		break;
	case E::SignalType::Bus:
		result = tr("Unsuported");
		break;
	default:
		assert(false);
	}

	return result;
}

void ArchiveModel::updateCachedState(int row) const
{
	if (m_cachedStateIndex == row)
	{
		return;
	}

	if (row < 0)
	{
		m_cachedStateIndex = -1;
		return;
	}

	m_cachedStateIndex = row;
	m_cachedSignalState = m_archive.state(row);

	return;
}

void ArchiveModel::setParams(const std::vector<ArchiveSignal>& archiveSignals, E::TimeType timeType)
{
	// Update m_appSignalsMap with new records and remove unwanted ones
	//
	{
		std::map<Hash, ArchiveSignalParam> oldAppSignalsMap;
		oldAppSignalsMap.swap(m_archiveSignalsMap);

		for (const ArchiveSignal& archSignal : archiveSignals)
		{
			Hash h = ::calcHash(archSignal.signalParam.appSignalId());

			if (auto oldSignalIt = oldAppSignalsMap.find(h);
				oldSignalIt != oldAppSignalsMap.end())
			{
				auto nh = oldAppSignalsMap.extract(oldSignalIt);
				m_archiveSignalsMap.insert(std::move(nh));
			}
			else
			{
				m_archiveSignalsMap.emplace(h, archSignal);
			}
		}
	}

	// Update m_archiveSignalsVector
	//
	{
		std::vector<ArchiveSignal> oldVector = std::move(m_archiveSignalsVector);
		m_archiveSignalsVector.clear();

		for (const ArchiveSignal& archSignal : archiveSignals)
		{
			auto oldSignalIt = std::find_if(oldVector.begin(), oldVector.end(),
				[&archSignal](const ArchiveSignal& as)
				{
					return as.signalParam.appSignalId() == archSignal.signalParam.appSignalId() &&
						   as.archiveServiceShortenId == archSignal.archiveServiceShortenId;
				});

			if (oldSignalIt != oldVector.end())
			{
				m_archiveSignalsVector.push_back(*oldSignalIt);
			}
			else
			{
				m_archiveSignalsVector.push_back(archSignal);
			}
		}
	}


	// Set time type
	//
	m_timeType = timeType;

	return;
}

void ArchiveModel::addData(ArchiveRequestResult&& chunk)
{
	if (chunk.states.empty() == true)
	{
		return;
	}

	qDebug() << "ArchiveModel::addData, chunk size " << chunk.states.size();

	beginResetModel();

	m_archive.addChunk(std::move(chunk), m_timeType);
	updateCachedState(-1);

	endResetModel();
	return;
}

void ArchiveModel::clear()
{
	beginResetModel();

	m_archive.clear();
	updateCachedState(-1);

	endResetModel();
	return;
}

void ArchiveModel::removeSignal(QString appSignalId, QString archiveServiceId)
{
	beginResetModel();

	m_archive.removeSignal(appSignalId, archiveServiceId);
	updateCachedState(-1);

	endResetModel();
	return;
}

std::vector<ArchiveSignal> ArchiveModel::archiveSignals()
{
	return m_archiveSignalsVector;
}

std::vector<ArchiveSignalParam> ArchiveModel::appSignals()
{
	std::vector<ArchiveSignalParam> result;
	result.reserve(m_archiveSignalsMap.size());

	for (const auto& p : m_archiveSignalsMap)
	{
		result.push_back(p.second);
	}

	return result;
}

const ArchiveSignalParam& ArchiveModel::signalParam(int row) const
{
	Q_ASSERT(row < m_archive.size());
	const AppSignalState& signalState = m_archive.state(row).appState;

	if (auto it = m_archiveSignalsMap.find(signalState.hash());
		 it == m_archiveSignalsMap.end())
	{
		return InvalidSignalParam;
	}
	else
	{
		return it->second;
	}
}

bool ArchiveModel::setShowParams(Hash signalHash, E::ValueViewType viewType, int precision)
{
	if (auto it = m_archiveSignalsMap.find(signalHash);
		it == m_archiveSignalsMap.end())
	{
		return false;
	}
	else
	{
		it->second.viewType = viewType;
		it->second.precision = precision;

		return true;
	}
}

//
//
//		ArchiveView
//
//
ArchiveView::ArchiveView(QWidget* parent) :
	QTableView(parent)
{
	verticalHeader()->hide();
	verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader()->setDefaultSectionSize(verticalHeader()->minimumSectionSize() + verticalHeader()->minimumSectionSize() / 10);

	//setSelectionBehavior(QAbstractItemView::SelectRows);
	setSelectionMode(QAbstractItemView::ExtendedSelection);

	// --
	//
	horizontalHeader()->setHighlightSections(false);
	horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(horizontalHeader(), &QWidget::customContextMenuRequested, this, &ArchiveView::headerColumnContextMenuRequested);

	auto archiveHorzHeader = QSettings{}.value("ArchiveWindow/horzHeader").toByteArray();
	horizontalHeader()->restoreState(archiveHorzHeader);

	qRegisterMetaType<ArchiveColumns>("ArchiveColumns");

	// --
	//
	copyAction = new QAction(tr("Copy"), this);
	copyAction->setShortcut(QKeySequence::Copy);
	connect(copyAction, &QAction::triggered, this, &ArchiveView::copySelection);

	addAction(copyAction);

	return;
}

ArchiveView::~ArchiveView()
{
	QSettings s{};

	s.setValue("ArchiveWindow/horzHeader", horizontalHeader()->saveState());
	s.setValue("ArchiveWindow/horzHeaderCount", static_cast<int>(ArchiveColumns::ColumnCount));

	return;
}

void ArchiveView::contextMenuEvent(QContextMenuEvent* event)
{
	ArchiveModel* archiveModel = qobject_cast<ArchiveModel*>(model());
	Q_ASSERT(archiveModel);

	QMenu menu(this);

	// SignalViewParams
	//
	if (QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
		selectedIndexes.isEmpty() == false)
	{
		std::set<int> selectedRows;

		for (const QModelIndex& mi : selectedIndexes)
		{
			selectedRows.insert(mi.row());
		}

		if (selectedRows.size() == 1)
		{
			// Show View sub menu only for one row
			//
			ArchiveSignalParam archiveSignalParam = archiveModel->signalParam(*selectedRows.begin());

			if (archiveSignalParam.signalParam.isAnalog() == true)
			{
				QMenu* viewMenu = menu.addMenu(tr("View %1").arg(archiveSignalParam.signalParam.customSignalId()));
				QList<QAction*> actions;

				// Precision
				// Copy/Paste from DialogSignalInfo::ContextMenu
				//
				QString strPrecision = ".";

				QActionGroup *precisionGroup = new QActionGroup(this);
				precisionGroup->setExclusive(true);

				for (int i = 0; i < 10; i++)
				{
					QAction* a = new QAction(strPrecision, &menu);

					auto f = [&archiveSignalParam, i, archiveModel]() -> void
							 {
								archiveModel->setShowParams(archiveSignalParam.signalParam.hash(), archiveSignalParam.viewType, i);
							 };

					connect(a, &QAction::triggered, this, f);

					a->setCheckable(true);

					if (i == archiveSignalParam.precision)
					{
						a->setChecked(true);
					}

					precisionGroup->addAction(a);

					strPrecision += "0";
				}

				viewMenu->addActions(precisionGroup->actions());

				//
				QAction* separator = new QAction(&menu);
				separator->setSeparator(true);
				viewMenu->addAction(separator);

				// View type
				//
				QActionGroup *viewGroup = new QActionGroup(this);
				viewGroup->setExclusive(true);

				for (int i = 0; i < static_cast<int>(E::ValueViewType::Count); i++)
				{
					QAction* a = new QAction(E::valueToString<E::ValueViewType>(i), &menu);

					auto f = [&archiveSignalParam, i, archiveModel]() -> void
							 {
								archiveModel->setShowParams(archiveSignalParam.signalParam.hash(), static_cast<E::ValueViewType>(i), archiveSignalParam.precision);
							 };

					connect(a, &QAction::triggered, this, f);

					a->setCheckable(true);

					if (i == static_cast<int>(archiveSignalParam.viewType))
					{
						a->setChecked(true);
					}

					viewGroup->addAction(a);
				}

				viewMenu->addActions(viewGroup->actions());
			}
		}
	}

	// Add action to show "SignalInfoDialog"
	//
	if (std::vector<ArchiveSignalParam> appSignals = archiveModel->appSignals();
		appSignals.empty() == false)
	{
		for (const ArchiveSignalParam& archSignal : appSignals)
		{
			QAction* action = menu.addAction(archSignal.signalParam.customSignalId() + " - " + archSignal.signalParam.caption());

			QString appSignalId = archSignal.signalParam.appSignalId();
			connect(action, &QAction::triggered, this, [this, appSignalId]()
				{
					qDebug() << "emit requestToShowSignalInfo " << appSignalId ;
					emit requestToShowSignalInfo(appSignalId);
				});
		}

		menu.addSeparator();
	}

	// Add actions to "Remove" specific signal from archive model
	//
	if (std::vector<ArchiveSignal> archiveSignals = archiveModel->archiveSignals();
		archiveSignals.empty() == false)
	{
		for (const ArchiveSignal& archSignal : archiveSignals)
		{
			QAction* action = menu.addAction(tr("Remove ") + archSignal.signalParam.customSignalId() + "(" + archSignal.archiveServiceShortenId + ")");

			QString appSignalId = archSignal.signalParam.appSignalId();
			QString archiveServiceId = archSignal.archiveServiceId;
			connect(action, &QAction::triggered, this, [this, appSignalId, archiveServiceId]()
				{
					emit requestToRemoveSignal(appSignalId, archiveServiceId);
				});
		}

		menu.addSeparator();
	}

	// Add action to copy selected rows to the cllipboard
	//
	QAction* a1 = menu.addAction(tr("Copy"));
	a1->setEnabled(selectionModel()->hasSelection());
	connect(a1, &QAction::triggered, this, &ArchiveView::requestToCopySelection);
	connect(this, &ArchiveView::requestToCopySelection, this, &ArchiveView::copySelection);			// Can be coonected from QAction::triggered directly, but...

	menu.addSeparator();

	// Signals...
	//
	QAction* a4 = menu.addAction(tr("Signals..."));
	connect(a4, &QAction::triggered, this, &ArchiveView::requestToSetSignals);

	// Show menu
	//
	menu.exec(event->globalPos());

	return;
}

void ArchiveView::headerColumnContextMenuRequested(const QPoint& pos)
{
	QMenu menu(this);

	QList<QAction*> actions;

	std::vector<std::pair<ArchiveColumns, QString>> actionsData;
	actionsData.reserve(static_cast<int>(ArchiveColumns::ColumnCount));

	ArchiveModel* archiveModel = dynamic_cast<ArchiveModel*>(model());
	if (archiveModel == nullptr)
	{
		Q_ASSERT(archiveModel);
		return;
	}

	for(int i = 0; i < archiveModel->columnCount(); i++)
	{
		actionsData.emplace_back(static_cast<ArchiveColumns>(i), archiveModel->headerData(i, Qt::Horizontal).toString());
	}

	for (std::pair<ArchiveColumns, QString> ad : actionsData)
	{
		QAction* action = new QAction(ad.second, this);
		action->setData(QVariant::fromValue(ad.first));
		action->setCheckable(true);
		action->setChecked(!horizontalHeader()->isSectionHidden(static_cast<int>(ad.first)));

		if (horizontalHeader()->count() - horizontalHeader()->hiddenSectionCount() == 1 &&
			action->isChecked() == true)
		{
			action->setEnabled(false);			// Impossible to uncheck the last column
		}

		connect(action, &QAction::toggled, this, &ArchiveView::headerColumnToggled);

		actions << action;
	}

	menu.exec(actions, mapToGlobal(pos), 0, this);
	return;
}

void ArchiveView::headerColumnToggled(bool checked)
{
	QAction* action = dynamic_cast<QAction*>(sender());

	if (action == nullptr)
	{
		Q_ASSERT(action);
		return ;
	}

	int column = action->data().value<int>();

	if (column >= static_cast<int>(ArchiveColumns::ColumnCount))
	{
		Q_ASSERT(column < static_cast<int>(ArchiveColumns::ColumnCount));
		return;
	}

	if (checked == true)
	{
		showColumn(column);
	}
	else
	{
		hideColumn(column);
	}

	return;
}

void ArchiveView::copySelection()
{
	ArchiveModel* archiveModel = qobject_cast<ArchiveModel*>(model());
	Q_ASSERT(archiveModel);

	QModelIndexList selectedIndexes = selectionModel()->selectedIndexes();
	if (selectedIndexes.isEmpty() == true)
	{
		return;
	}

	if (selectedIndexes.size() == 1)
	{
		QString str = archiveModel->data(selectedIndexes.front(), Qt::DisplayRole).toString();
		qApp->clipboard()->setText(str);
		return;
	}

	std::sort(selectedIndexes.begin(), selectedIndexes.end());

	QString str;
	str.reserve(4096);
	QTextStream out(&str);

	QString cellText;
	int lastRow = selectedIndexes.front().row();

	for (const QModelIndex& index : selectedIndexes)
	{
		if (lastRow != index.row())
		{
			out << Qt::endl;
		}

		cellText = archiveModel->data(index, Qt::DisplayRole).toString();

		if (cellText.contains(';') == true)
		{
			// If cell contains semicolon it must be enclosed in quotes
			//
			cellText.prepend('"');
			cellText.append('"');
		}

		out << cellText << ";";

		// --
		//
		lastRow = index.row();
	}

	out.flush();
	qApp->clipboard()->setText(str);

	return;
}
