#include "ArchiveModelView.h"
#include "Settings.h"

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
		return QVariant();
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

	case ArchiveColumns::Time:
		return tr("Time");
	}

	assert(false);
	return QVariant();
}

QVariant ArchiveModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int column = index.column();

	return data(row, column, role);
}

QVariant ArchiveModel::data(int row, int column, int role) const
{
	if (row >= m_archive.size())
	{
		return QVariant();
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
			{
				auto sit = m_appSignals.find(m_cachedSignalState.hash());
				if (sit == m_appSignals.end())
				{
					// State from differtent signal!!! ArchiveService has returned something wrong?
					//
					assert(false);
				}
				else
				{
					result = sit->second.appSignalId();
				}
			}
			break;

		case ArchiveColumns::CustomSignalId:
			{
				auto sit = m_appSignals.find(m_cachedSignalState.hash());
				if (sit == m_appSignals.end())
				{
					// State from differtent signal!!! ArchiveService has returned something wrong?
					//
					assert(false);
				}
				else
				{
					result = sit->second.customSignalId();
				}
			}
			break;
		case ArchiveColumns::Caption:
			{
				auto sit = m_appSignals.find(m_cachedSignalState.hash());
				if (sit == m_appSignals.end())
				{
					// State from differtent signal!!! ArchiveService has returned something wrong?
					//
					assert(false);
				}
				else
				{
					result = sit->second.caption();
				}
			}
			break;
		case ArchiveColumns::State:
			{
				auto sit = m_appSignals.find(m_cachedSignalState.hash());
				if (sit == m_appSignals.end())
				{
					// State from differtent signal!!! ArchiveService has returned something wrong?
					//
					assert(false);
				}
				else
				{
					result = getValueString(m_cachedSignalState, sit->second);
				}
			}
			break;
		case ArchiveColumns::Valid:
			{
				result = m_cachedSignalState.isValid() ? "" : "no";
			}
			break;
		case ArchiveColumns::Time:
			{
				const TimeStamp& ts = m_cachedSignalState.time(m_timeType);
				result = ts.toDateTime().toString("dd/MM/yyyy HH:mm:ss.zzz");
			}
			break;
		default:
			assert(false);
		}

		return result;
	}

	if (role == Qt::TextAlignmentRole &&
		(column ==  static_cast<int>(ArchiveColumns::Row) ||
		 column ==  static_cast<int>(ArchiveColumns::State) ||
		column ==  static_cast<int>(ArchiveColumns::Valid))
		)
	{
		return QVariant(Qt::AlignCenter);
	}

	if (role == Qt::ToolTipRole)
	{
		updateCachedState(row);		// m_cachedSignalState -- state for row
		AppSignalParam signalParam;

		auto sit = m_appSignals.find(m_cachedSignalState.hash());
		if (sit == m_appSignals.end())
		{
			// State from differtent signal!!! ArchiveService has returned something wrong?
			//
			assert(false);
		}
		else
		{
			signalParam = sit->second;
		}

		QString typeStr;
		switch (signalParam.type())
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
			assert(false);
		}

		QString toolTip = QString("StateIndex: %1\n"
								  "SignalID: %2\n"
								  "AppSignalID: %3\n"
								  "Caption: %4\n"
								  "Type: %5\n"
								  "Value: %6 (%7)\n"
								  "Flags: %8\n"
								  "Time: %9 (%10)\n"
								  "ServerTime: %11\n"
								  "ServerTime +0UTC: %12\n"
								  "PlantTime: %13")
						  .arg(row + 1)
						  .arg(signalParam.customSignalId())
						  .arg(signalParam.appSignalId())
						  .arg(signalParam.caption())
						  .arg(typeStr)
						  .arg(getValueString(m_cachedSignalState, signalParam))
								.arg(m_cachedSignalState.m_value)
						  .arg(QString::number(m_cachedSignalState.m_flags.all, 2))
						  .arg(m_cachedSignalState.time(m_timeType).toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"))
								.arg(E::valueToString<E::TimeType>(m_timeType))
						  .arg(m_cachedSignalState.time().system.toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"))			//"ServerTime: %12\n"
						  .arg(m_cachedSignalState.time().local.toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"))			//"ServerTime +0UTC: %13\n"
						  .arg(m_cachedSignalState.time().plant.toDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz"));			//"PlantTime: %14"



		return toolTip;
	}

	return QVariant();
}

QString ArchiveModel::getValueString(const AppSignalState& state, const AppSignalParam& signalParam) const
{
	E::SignalType signalType = signalParam.type();
	QString result;

	switch (signalType)
	{
	case E::SignalType::Analog:
		if (m_cachedSignalState.isValid() == false)
		{
			result = nonValidString;
		}
		else
		{
			result = QString::number(state.value());
		}
		break;
	case E::SignalType::Discrete:
		if (m_cachedSignalState.isValid() == false)
		{
			result = nonValidString;
		}
		else
		{
			result = QString::number(state.value());
		}
		break;
	default:
		result = tr("Unsuported");
	}

	return result;
}

void ArchiveModel::updateCachedState(int row) const
{
	if (m_cachedStateIndex == row)
	{
		return;
	}

	m_cachedSignalState = m_archive.state(row);
	m_cachedStateIndex = row;

	return;
}

void ArchiveModel::setParams(const std::vector<AppSignalParam>& appSignals, E::TimeType timeType)
{
	m_appSignals.clear();

	for (const AppSignalParam& asp : appSignals)
	{
		Hash h = ::calcHash(asp.appSignalId());
		m_appSignals[h] = asp;
	}

	m_timeType = timeType;

	return;
}

void ArchiveModel::addData(std::shared_ptr<ArchiveChunk> chunk)
{
	if (chunk == nullptr)
	{
		assert(chunk);
		return;
	}

	qDebug() << "ArchiveModel::addData, chunk size " << chunk->states.size();

	int first = m_archive.size();
	int last = first + static_cast<int>(chunk->states.size()) - 1;

	beginInsertRows(QModelIndex(), first, last);

	m_archive.addChunk(chunk);

	endInsertRows();
	return;
}

void ArchiveModel::clear()
{
	beginResetModel();

	m_archive.clear();
	m_cachedStateIndex = -1;

	endResetModel();
	return;
}

std::vector<AppSignalParam> ArchiveModel::appSignals()
{
	std::vector<AppSignalParam> result;
	result.reserve(m_appSignals.size());

	for (const std::pair<Hash, AppSignalParam>& p : m_appSignals)
	{
		result.push_back(p.second);
	}

	return result;
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

	horizontalHeader()->restoreState(theSettings.m_archiveHorzHeader);

	qRegisterMetaType<ArchiveColumns>("ArchiveColumns");

	// --
	//
	copyAction = new QAction(tr("Copy"), nullptr);
	copyAction->setShortcut(QKeySequence::Copy);
	connect(copyAction, &QAction::triggered, this, &ArchiveView::copySelection);

	addAction(copyAction);

	return;
}

ArchiveView::~ArchiveView()
{
	theSettings.m_archiveHorzHeader = horizontalHeader()->saveState();
}

void ArchiveView::contextMenuEvent(QContextMenuEvent* event)
{
	ArchiveModel* archiveModel = qobject_cast<ArchiveModel*>(model());
	assert(archiveModel);

	QMenu menu(this);

	// Add action to show signal info dialog
	//
	std::vector<AppSignalParam> apppSignals = archiveModel->appSignals();

	if (apppSignals.empty() == false)
	{
		for (const AppSignalParam& signal : apppSignals)
		{
			QAction* action = menu.addAction(signal.customSignalId() + " - " + signal.caption());

			QString appSignalId = signal.appSignalId();
			connect(action, &QAction::triggered, this, [this, appSignalId]()
				{
					qDebug() << "emit requestToShowSignalInfo " << appSignalId ;
					emit requestToShowSignalInfo(appSignalId);
				});
		}

		menu.addSeparator();
	}

	// Add actions to remove specific signal from archive model
	//
	if (apppSignals.empty() == false)
	{
		for (const AppSignalParam& signal : apppSignals)
		{
			QAction* action = menu.addAction(QLatin1String("Remove ") + signal.customSignalId());

			QString appSignalId = signal.appSignalId();
			connect(action, &QAction::triggered, this, [this, appSignalId]()
				{
					emit requestToRemoveSignal(appSignalId);
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

	actionsData.emplace_back(ArchiveColumns::Row, tr("Row"));
	actionsData.emplace_back(ArchiveColumns::AppSignalId, tr("AppSignalID"));
	actionsData.emplace_back(ArchiveColumns::CustomSignalId, tr("SignalID"));
	actionsData.emplace_back(ArchiveColumns::Caption, tr("Caption"));
	actionsData.emplace_back(ArchiveColumns::State, tr("State"));
	actionsData.emplace_back(ArchiveColumns::Valid, tr("Valid"));
	actionsData.emplace_back(ArchiveColumns::Time, tr("Time"));

	for (std::pair<ArchiveColumns, QString> ad : actionsData)
	{
		QAction* action = new QAction(ad.second, nullptr);
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
		assert(action);
		return ;
	}

	int column = action->data().value<int>();

	if (column >= static_cast<int>(ArchiveColumns::ColumnCount))
	{
		assert(column < static_cast<int>(ArchiveColumns::ColumnCount));
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
	assert(archiveModel);

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

	qSort(selectedIndexes);

	QString str;
	str.reserve(4096);
	QTextStream out(&str);

	QString cellText;
	int lastRow = selectedIndexes.front().row();

	for (const QModelIndex& index : selectedIndexes)
	{
		if (lastRow != index.row())
		{
			out << endl;
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
