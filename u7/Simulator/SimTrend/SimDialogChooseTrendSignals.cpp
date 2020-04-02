#include "SimDialogChooseTrendSignals.h"
#include "ui_SimDialogChooseTrendSignals.h"

SimDialogChooseTrendSignals::SimDialogChooseTrendSignals(Sim::AppSignalManager* signalManager,
														 std::vector<TrendLib::TrendSignalParam>& trendSignals,
														 QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogChooseTrendSignals)
{
	assert(signalManager);

	ui->setupUi(this);

	setWindowFlag(Qt::WindowContextHelpButtonHint, false);

	// Set filter completer
	//
	QSettings s;
	QStringList trendSignalsDialogFilterCompleter = s.value("SimDialogChooseTrendSignals/trendSignalsDialogFilterCompleter").toStringList();

	m_filterCompleter = new QCompleter(trendSignalsDialogFilterCompleter, this);
	m_filterCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	ui->filterEdit->setCompleter(m_filterCompleter);

	// --
	//
	QStringList headerLabels;
	headerLabels << "SignalID";
	headerLabels << "Type";
	headerLabels << "Caption";

	ui->trendSignals->setHeaderLabels(headerLabels);

	SimFilteredTrendSignalsModel* model = new SimFilteredTrendSignalsModel(signalManager->signalList(), ui->filteredSignals);
	ui->filteredSignals->setModel(model);

	// --
	//
	fillSignalList();

	// --
	//
	connect(ui->filteredSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SimDialogChooseTrendSignals::slot_filteredSignalsSelectionChanged);
	connect(ui->trendSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &SimDialogChooseTrendSignals::slot_trendSignalsSelectionChanged);

	// --
	// --
	disableControls();

	ui->filteredSignals->header()->resizeSection(1, ui->filteredSignals->header()->sectionSizeHint(1));		// 1 is TypeColumn (A/D)
	ui->trendSignals->header()->resizeSection(1 , ui->trendSignals->header()->sectionSizeHint(1));			// 1 is TypeColumn (A/D)

	// Fill added signals
	//
	for (TrendLib::TrendSignalParam& trendSignal : trendSignals)
	{
		AppSignalParam appSignal = trendSignal.toAppSignalParam();
		addSignal(appSignal);
	}

	return;
}

SimDialogChooseTrendSignals::~SimDialogChooseTrendSignals()
{
	delete ui;
}

std::vector<AppSignalParam> SimDialogChooseTrendSignals::acceptedSignals() const
{
	return m_acceptedSignals;
}

void SimDialogChooseTrendSignals::fillSignalList()
{
	SimFilteredTrendSignalsModel* model = dynamic_cast<SimFilteredTrendSignalsModel*>(ui->filteredSignals->model());
	Q_ASSERT(model);

	model->filterSignals(ui->filterEdit->text());

	return;
}

void SimDialogChooseTrendSignals::addSignal(const AppSignalParam& signal)
{
	if (trendSignalsHasSignalId(signal.customSignalId()) == true)
	{
		// SignaID already presnt in TrenSignals
		//
		return;
	}

	if (ui->trendSignals->topLevelItemCount() >= 16)
	{
		QMessageBox::critical(this, qAppName(), tr("The maximum number of signals reached."));
		return;
	}

	QString signalType;
	switch (signal.type())
	{
	case E::SignalType::Analog:		signalType = "A";	break;
	case E::SignalType::Discrete:	signalType = "D";	break;
	case E::SignalType::Bus:		signalType = "B";	break;
	default:
		Q_ASSERT(false);
	}

	QStringList itemData;
	itemData << signal.customSignalId();
	itemData << signalType;
	itemData << signal.caption();

	QString toolTip = QString("%1\n%2\n%3")
						.arg(signal.customSignalId())
						.arg(signal.appSignalId())
						.arg(signal.caption());

	QTreeWidgetItem* item = new QTreeWidgetItem(ui->trendSignals, itemData);
	item->setData(0, Qt::UserRole, QVariant::fromValue(signal));

	item->setToolTip(0, toolTip);
	item->setToolTip(1, toolTip);
	item->setToolTip(2, toolTip);

	ui->trendSignals->addTopLevelItem(item);
	ui->trendSignals->setCurrentItem(item, QItemSelectionModel::SelectCurrent);

	disableControls();

	return;
}

void SimDialogChooseTrendSignals::removeSelectedSignal()
{
	Q_ASSERT(ui->trendSignals);

	QModelIndex currentIndex = ui->trendSignals->currentIndex();

	if (currentIndex.isValid() == true)
	{
		ui->trendSignals->takeTopLevelItem(currentIndex.row());
	}

	disableControls();
	return;
}

bool SimDialogChooseTrendSignals::trendSignalsHasSignalId(QString signalId)
{
	int itemCount = ui->trendSignals->topLevelItemCount();

	for (int i = 0; i < itemCount; i++)
	{
		QTreeWidgetItem* item = ui->trendSignals->topLevelItem(i);
		Q_ASSERT(item);

		if (item->text(0) == signalId)
		{
			return true;
		}
	}

	return false;
}

void SimDialogChooseTrendSignals::disableControls()
{
	Q_ASSERT(ui->filteredSignals);
	Q_ASSERT(ui->trendSignals);

	const SimFilteredTrendSignalsModel* fileterModel = dynamic_cast<const SimFilteredTrendSignalsModel*>(ui->filteredSignals->model());
	if (fileterModel == nullptr)
	{
		Q_ASSERT(fileterModel != nullptr);
		return;
	}

	// --
	//
	bool enableAddButton = true;
	bool enableRemoveButton = true;
	bool enableRemoveAll = true;

	// Add Signal Button
	//
	{
		QModelIndex index = ui->filteredSignals->currentIndex();

		if (index.isValid() == true)
		{
			const AppSignalParam signal = fileterModel->signalByRow(index.row());
			enableAddButton = !trendSignalsHasSignalId(signal.customSignalId());
		}
		else
		{
			enableAddButton = false;
		}
	}

	// Remove Signal Button
	//
	{
		QModelIndex index = ui->trendSignals->currentIndex();

		if (index.isValid() == false ||
			index.row() < 0)
		{
			enableRemoveButton = false;
		}
		else
		{
			enableRemoveButton = true;
		}
	}

	// Remove All Signals Button
	//
	{
		enableRemoveAll = ui->trendSignals->topLevelItemCount() > 0;
	}


	// --
	//
	ui->addSignalButton->setEnabled(enableAddButton);
	ui->removeSignalButton->setEnabled(enableRemoveButton);
	ui->removeAllSignalsButton->setEnabled(enableRemoveAll);

	return;
}

void SimDialogChooseTrendSignals::on_addSignalButton_clicked()
{
	QModelIndex index = ui->filteredSignals->currentIndex();
	if (index.isValid() == false)
	{
		return;
	}

	const SimFilteredTrendSignalsModel* model = dynamic_cast<const SimFilteredTrendSignalsModel*>(index.model());

	if (model == nullptr)
	{
		Q_ASSERT(dynamic_cast<const SimFilteredTrendSignalsModel*>(index.model()) != nullptr);
		return;
	}

	const AppSignalParam signal = model->signalByRow(index.row());
	addSignal(signal);

	return;
}

void SimDialogChooseTrendSignals::on_removeSignalButton_clicked()
{
	removeSelectedSignal();
}

void SimDialogChooseTrendSignals::on_removeAllSignalsButton_clicked()
{
	ui->trendSignals->clear();

	disableControls();

	return;
}

void SimDialogChooseTrendSignals::on_filterEdit_textChanged(const QString& /*arg*/)
{
	fillSignalList();
}


void SimDialogChooseTrendSignals::on_filterEdit_editingFinished()
{
	QString arg = ui->filterEdit->text();

	QSettings s;
	QStringList trendSignalsDialogFilterCompleter = s.value("SimDialogChooseTrendSignals/trendSignalsDialogFilterCompleter").toStringList();

	if (trendSignalsDialogFilterCompleter.contains(arg) == false)
	{
		trendSignalsDialogFilterCompleter << arg;

		while (trendSignalsDialogFilterCompleter.size() > 1000)
		{
			trendSignalsDialogFilterCompleter.pop_front();
		}

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(trendSignalsDialogFilterCompleter);
		}

		s.setValue("SimDialogChooseTrendSignals/trendSignalsDialogFilterCompleter", trendSignalsDialogFilterCompleter);
	}

	return;
}

void SimDialogChooseTrendSignals::on_filteredSignals_doubleClicked(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const SimFilteredTrendSignalsModel* model = dynamic_cast<const SimFilteredTrendSignalsModel*>(index.model());

	if (model == nullptr)
	{
		Q_ASSERT(dynamic_cast<const SimFilteredTrendSignalsModel*>(index.model()) != nullptr);
		return;
	}

	const AppSignalParam signal = model->signalByRow(index.row());
	addSignal(signal);

	return;
}

void SimDialogChooseTrendSignals::slot_filteredSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	disableControls();
}

void SimDialogChooseTrendSignals::on_trendSignals_doubleClicked(const QModelIndex& /*index*/)
{
	removeSelectedSignal();
}

void SimDialogChooseTrendSignals::slot_trendSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	disableControls();
}

void SimDialogChooseTrendSignals::on_buttonBox_accepted()
{
	m_acceptedSignals.reserve(ui->trendSignals->topLevelItemCount());

	for (int i = 0; i < ui->trendSignals->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* treeItem = ui->trendSignals->topLevelItem(i);
		Q_ASSERT(treeItem);

		QVariant signalVariant = treeItem->data(0, Qt::UserRole);

		Q_ASSERT(signalVariant.isNull() == false);
		Q_ASSERT(signalVariant.isValid() == true);

		AppSignalParam signalParam = signalVariant.value<AppSignalParam>();

		Q_ASSERT(signalParam.customSignalId() == treeItem->text(0));

		m_acceptedSignals.push_back(signalParam);
	}

	return;
}

//
//		SimFilteredTrendSignalsModel
//
SimFilteredTrendSignalsModel::SimFilteredTrendSignalsModel(const std::vector<AppSignalParam>& signalss, QObject* parent)
	: QAbstractTableModel(parent),
	m_signals(signalss)
{
	std::sort(m_signals.begin(), m_signals.end(),
			[](const AppSignalParam& s1, const AppSignalParam& s2) -> bool
			{
				return s1.customSignalId() < s2.customSignalId();
			});

	m_signalIndexes.reserve(m_signals.size());

	// Init m_startWithArrays.
	// m_startWithArrays keeps vector of indexes for "StartWith"
	//
	size_t siganlCount = m_signals.size();

	for (size_t index = 0; index < siganlCount; index ++)
	{
		const AppSignalParam& appSignal = m_signals[index];
		QString customSignalId = appSignal.customSignalId().toLower();

		if (customSignalId.isEmpty() == true)
		{
			Q_ASSERT(customSignalId.isEmpty() == false);
			continue;
		}

		QString firstLetter = customSignalId.at(0);

		auto foundStartWithIt = m_startWithArrays.find(firstLetter);
		if (foundStartWithIt == m_startWithArrays.end())
		{
			std::vector<int> signalIndexes;
			signalIndexes.reserve(8192);

			signalIndexes.push_back(static_cast<int>(index));

			m_startWithArrays[firstLetter] = signalIndexes;
		}
		else
		{
			std::vector<int>& signalIndexes = foundStartWithIt->second;
			signalIndexes.push_back(static_cast<int>(index));
		}
	}

}

int SimFilteredTrendSignalsModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(m_signalIndexes.size());
}

int SimFilteredTrendSignalsModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 3;	// Columns: SignalID, Typem Caption
}

QVariant SimFilteredTrendSignalsModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (section)
			{
			case 0:
				return QString("SignalID");
			case 1:
				return QString("Type");
			case 2:
				return QString("Caption");
			}
		}
	}

	return QVariant();
}

QVariant SimFilteredTrendSignalsModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	switch (role)
	{
	case Qt::DisplayRole:
		{
			if (row < 0 || row >= static_cast<int>(m_signalIndexes.size()))
			{
				Q_ASSERT(row >= 0 && row < static_cast<int>(m_signalIndexes.size()));
				return QVariant();
			}

			int signalIndex = m_signalIndexes[row];

			if (signalIndex < 0 ||
				signalIndex >= static_cast<int>(m_signals.size()))
			{
				Q_ASSERT(signalIndex >= 0 &&  signalIndex < static_cast<int>(m_signals.size()));
				return QVariant();
			}

			const AppSignalParam& signalParam = m_signals[signalIndex];

			switch (col)
			{
			case 0:
				return signalParam.customSignalId();
			case 1:
				switch (signalParam.type())
				{
				case E::SignalType::Analog:		return QString("A");
				case E::SignalType::Discrete:	return QString("D");
				case E::SignalType::Bus:		return QString("B");
				default:
					Q_ASSERT(false);
					return QVariant();
				}
			case 2:
				return signalParam.caption();
			default:
				Q_ASSERT(false);
				return QVariant();
			}
		}
		break;
	case Qt::ToolTipRole:
		{
			if (row < 0 || row >= static_cast<int>(m_signalIndexes.size()))
			{
				Q_ASSERT(row >= 0 && row < static_cast<int>(m_signalIndexes.size()));
				return QVariant();
			}

			int signalIndex = m_signalIndexes[row];

			if (signalIndex < 0 ||
				signalIndex >= static_cast<int>(m_signals.size()))
			{
				Q_ASSERT(signalIndex >= 0 &&  signalIndex < static_cast<int>(m_signals.size()));
				return QVariant();
			}

			const AppSignalParam& signalParam = m_signals[signalIndex];

			QString toolTip = QString("%1\n%2\n%3")
								.arg(signalParam.customSignalId())
								.arg(signalParam.appSignalId())
								.arg(signalParam.caption());

			return toolTip;

		}
	default:
		return QVariant();
	}
}

void SimFilteredTrendSignalsModel::filterSignals(QString filter)
{
	beginResetModel();

	QString filterText = filter.trimmed().toLower();

	if (filterText.isEmpty() == true)
	{
		// No filter, add all signals
		//
		m_signalIndexes.clear();

		int signalCount = static_cast<int>(m_signals.size());
		for (int i = 0; i < signalCount; i++)
		{
			m_signalIndexes.push_back(i);
		}

		endResetModel();
		return;
	}

	auto foundStartWithIt = m_startWithArrays.find(filterText.left(1));		// m_startWithArrays keeps only firts letter
	if (foundStartWithIt == m_startWithArrays.end())
	{
		m_signalIndexes.clear();
	}
	else
	{
		std::vector<int>& signalIndexes = foundStartWithIt->second;

		m_signalIndexes.clear();

		for (int index : signalIndexes)
		{
			if (index < 0 || index >= static_cast<int>(m_signals.size()))
			{
				Q_ASSERT(index >= 0 && index < static_cast<int>(m_signals.size()));
				continue;
			}

			const AppSignalParam& signal = m_signals[index];

			// if filterText.size() == 1 then we already filrtered it by getting data from m_startWithArrays
			//
			if (filterText.size() == 1 ||
				signal.customSignalId().startsWith(filterText, Qt::CaseInsensitive) == true)
			{

				m_signalIndexes.push_back(index);
			}
		}
	}

	endResetModel();

	return;
}

AppSignalParam SimFilteredTrendSignalsModel::signalByRow(int row) const
{
	if (row < 0 || row >= static_cast<int>(m_signalIndexes.size()))
	{
		Q_ASSERT(row >= 0 && row < static_cast<int>(m_signalIndexes.size()));
		return AppSignalParam();
	}

	int signalIndex = m_signalIndexes[row];

	if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
	{
		Q_ASSERT(signalIndex >= 0 && signalIndex < static_cast<int>(m_signals.size()));
		return AppSignalParam();
	}

	return m_signals[signalIndex];
}



