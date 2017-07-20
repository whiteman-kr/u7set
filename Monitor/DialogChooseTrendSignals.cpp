#include "DialogChooseTrendSignals.h"
#include "ui_DialogChooseTrendSignals.h"

DialogChooseTrendSignals::DialogChooseTrendSignals(QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogChooseTrendSignals)
{
	ui->setupUi(this);

	// --
	//
	QStringList headerLabels;
	headerLabels << "SignalID";
	headerLabels << "Type";
	headerLabels << "Caption";

	ui->trendSignals->setHeaderLabels(headerLabels);

	FilteredTrendSignalsModel* model = new FilteredTrendSignalsModel(theSignals.signalList(), ui->filteredSignals);
	ui->filteredSignals->setModel(model);

	// --
	//
	fillSignalList();

	// --
	//
	connect(ui->filteredSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseTrendSignals::slot_filteredSignalsSelectionChanged);
	connect(ui->trendSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseTrendSignals::slot_trendSignalsSelectionChanged);

	// --
	// --
	disableControls();

	ui->filteredSignals->header()->resizeSection(1, ui->filteredSignals->header()->sectionSizeHint(1));		// 1 is TypeColumn (A/D)
	ui->trendSignals->header()->resizeSection(1 , ui->trendSignals->header()->sectionSizeHint(1));			// 1 is TypeColumn (A/D)

	return;
}

DialogChooseTrendSignals::~DialogChooseTrendSignals()
{
	delete ui;
}

std::vector<AppSignalParam> DialogChooseTrendSignals::acceptedSignals() const
{
	return m_acceptedSignals;
}

void DialogChooseTrendSignals::fillSignalList()
{
	FilteredTrendSignalsModel* model = dynamic_cast<FilteredTrendSignalsModel*>(ui->filteredSignals->model());
	assert(model);

	model->filterSignals(ui->filterEdit->text());

	return;
}

void DialogChooseTrendSignals::addSignal(const AppSignalParam& signal)
{
//	QString signalId = std::get<0>(signal);
//	QString type = std::get<1>(signal);
//	QString caption = std::get<2>(signal);

	if (trendSignalsHasSignalId(signal.customSignalId()) == true)
	{
		// SignaID already presnt in TrenSignals
		//
		return;
	}

	if (ui->trendSignals->topLevelItemCount() >= 12)
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
		assert(false);
	}

	QStringList itemData;
	itemData << signal.customSignalId();
	itemData << signalType;
	itemData << signal.caption();

	QTreeWidgetItem* item = new QTreeWidgetItem(ui->trendSignals, itemData);
	item->setData(0, Qt::UserRole, QVariant::fromValue(signal));

	ui->trendSignals->addTopLevelItem(item);

	ui->trendSignals->setCurrentItem(item, QItemSelectionModel::SelectCurrent);

	disableControls();

	return;
}

void DialogChooseTrendSignals::removeSelectedSignal()
{
	assert(ui->trendSignals);

	QModelIndex currentIndex = ui->trendSignals->currentIndex();

	if (currentIndex.isValid() == true)
	{
		ui->trendSignals->takeTopLevelItem(currentIndex.row());
	}

	disableControls();
	return;
}

bool DialogChooseTrendSignals::trendSignalsHasSignalId(QString signalId)
{
	int itemCount = ui->trendSignals->topLevelItemCount();

	for (int i = 0; i < itemCount; i++)
	{
		QTreeWidgetItem* item = ui->trendSignals->topLevelItem(i);
		assert(item);

		if (item->text(0) == signalId)
		{
			return true;
		}
	}

	return false;
}

void DialogChooseTrendSignals::disableControls()
{
	assert(ui->filteredSignals);
	assert(ui->trendSignals);

	const FilteredTrendSignalsModel* fileterModel = dynamic_cast<const FilteredTrendSignalsModel*>(ui->filteredSignals->model());
	if (fileterModel == nullptr)
	{
		assert(fileterModel != nullptr);
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

void DialogChooseTrendSignals::on_addSignalButton_clicked()
{
	QModelIndex index = ui->filteredSignals->currentIndex();
	if (index.isValid() == false)
	{
		return;
	}

	const FilteredTrendSignalsModel* model = dynamic_cast<const FilteredTrendSignalsModel*>(index.model());

	if (model == nullptr)
	{
		assert(dynamic_cast<const FilteredTrendSignalsModel*>(index.model()) != nullptr);
		return;
	}

	const AppSignalParam signal = model->signalByRow(index.row());
	addSignal(signal);

	return;
}

void DialogChooseTrendSignals::on_removeSignalButton_clicked()
{
	removeSelectedSignal();
}

void DialogChooseTrendSignals::on_removeAllSignalsButton_clicked()
{
	ui->trendSignals->clear();

	disableControls();

	return;
}

void DialogChooseTrendSignals::on_filterEdit_textChanged(const QString& /*arg1*/)
{
	fillSignalList();
}

void DialogChooseTrendSignals::on_filteredSignals_doubleClicked(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const FilteredTrendSignalsModel* model = dynamic_cast<const FilteredTrendSignalsModel*>(index.model());

	if (model == nullptr)
	{
		assert(dynamic_cast<const FilteredTrendSignalsModel*>(index.model()) != nullptr);
		return;
	}

	const AppSignalParam signal = model->signalByRow(index.row());
	addSignal(signal);

	return;
}

void DialogChooseTrendSignals::slot_filteredSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	disableControls();
}

void DialogChooseTrendSignals::on_trendSignals_doubleClicked(const QModelIndex& /*index*/)
{
	removeSelectedSignal();
}

FilteredTrendSignalsModel::FilteredTrendSignalsModel(const std::vector<AppSignalParam>& signalss, QObject* parent)
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
			assert(customSignalId.isEmpty() == false);
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

void DialogChooseTrendSignals::slot_trendSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	disableControls();
}

void DialogChooseTrendSignals::on_buttonBox_accepted()
{
	m_acceptedSignals.reserve(ui->trendSignals->topLevelItemCount());

	for (int i = 0; i < ui->trendSignals->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* treeItem = ui->trendSignals->topLevelItem(i);
		assert(treeItem);

		QVariant signalVariant = treeItem->data(0, Qt::UserRole);

		assert(signalVariant.isNull() == false);
		assert(signalVariant.isValid() == true);

		AppSignalParam signalParam = signalVariant.value<AppSignalParam>();

		assert(signalParam.customSignalId() == treeItem->text(0));

		m_acceptedSignals.push_back(signalParam);
	}

	return;
}

int FilteredTrendSignalsModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(m_signalIndexes.size());
}

int FilteredTrendSignalsModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 3;	// Columns: SignalID, Typem Caption
}

QVariant FilteredTrendSignalsModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
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

QVariant FilteredTrendSignalsModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	switch (role)
	{
	case Qt::DisplayRole:
		{
			if (row < 0 || row >= static_cast<int>(m_signalIndexes.size()))
			{
				assert(row >= 0 && row < static_cast<int>(m_signalIndexes.size()));
				return QVariant();
			}

			int signalIndex = m_signalIndexes[row];

			if (signalIndex < 0 ||
				signalIndex >= static_cast<int>(m_signals.size()))
			{
				assert(signalIndex >= 0 &&  signalIndex < static_cast<int>(m_signals.size()));
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
					assert(false);
					return QVariant();
				}
			case 2:
				return signalParam.caption();
			default:
				assert(false);
				return QVariant();
			}
		}
		break;
	default:
		return QVariant();
	}

	return QVariant();
}

void FilteredTrendSignalsModel::filterSignals(QString filter)
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
				assert(index >= 0 && index < static_cast<int>(m_signals.size()));
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

AppSignalParam FilteredTrendSignalsModel::signalByRow(int row) const
{
	if (row < 0 || row >= static_cast<int>(m_signalIndexes.size()))
	{
		assert(row >= 0 && row < static_cast<int>(m_signalIndexes.size()));
		return AppSignalParam();
	}

	int signalIndex = m_signalIndexes[row];

	if (signalIndex < 0 || signalIndex >= static_cast<int>(m_signals.size()))
	{
		assert(signalIndex >= 0 && signalIndex < static_cast<int>(m_signals.size()));
		return AppSignalParam();
	}

	return m_signals[signalIndex];
}

