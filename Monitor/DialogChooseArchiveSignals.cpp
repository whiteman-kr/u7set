#include "DialogChooseArchiveSignals.h"
#include "ui_DialogChooseArchiveSignals.h"
#include "Settings.h"

DialogChooseArchiveSignals::DialogChooseArchiveSignals(std::vector<AppSignalParam>& appSignals, QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogChooseArchiveSignals)
{
	ui->setupUi(this);

	// Set filter completer
	//
	m_filterCompleter = new QCompleter(theSettings.m_archiveSignalsDialogFilterCompleter, this);
	m_filterCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	ui->filterEdit->setCompleter(m_filterCompleter);

	// --
	//
	QStringList headerLabels;
	headerLabels << "SignalID";
	headerLabels << "Type";
	headerLabels << "Caption";

	ui->archiveSignals->setHeaderLabels(headerLabels);

	FilteredArchiveSignalsModel* model = new FilteredArchiveSignalsModel(theSignals.signalList(), ui->filteredSignals);
	ui->filteredSignals->setModel(model);

	// --
	//
	fillSignalList();

	// --
	//
	connect(ui->filteredSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseArchiveSignals::slot_filteredSignalsSelectionChanged);
	connect(ui->archiveSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseArchiveSignals::slot_archiveSignalsSelectionChanged);

	// --
	// --
	disableControls();

	ui->filteredSignals->header()->resizeSection(1, ui->filteredSignals->header()->sectionSizeHint(1));		// 1 is TypeColumn (A/D)
	ui->archiveSignals->header()->resizeSection(1 , ui->archiveSignals->header()->sectionSizeHint(1));			// 1 is TypeColumn (A/D)

	// Fill added signals
	//
	for (AppSignalParam& appSignal : appSignals)
	{
		addSignal(appSignal);
	}

	return;
}

DialogChooseArchiveSignals::~DialogChooseArchiveSignals()
{
	delete ui;
}

std::vector<AppSignalParam> DialogChooseArchiveSignals::acceptedSignals() const
{
	return m_acceptedSignals;
}

void DialogChooseArchiveSignals::fillSignalList()
{
	FilteredArchiveSignalsModel* model = dynamic_cast<FilteredArchiveSignalsModel*>(ui->filteredSignals->model());
	assert(model);

	model->filterSignals(ui->filterEdit->text());

	return;
}

void DialogChooseArchiveSignals::addSignal(const AppSignalParam& signal)
{
//	QString signalId = std::get<0>(signal);
//	QString type = std::get<1>(signal);
//	QString caption = std::get<2>(signal);

	if (archiveSignalsHasSignalId(signal.customSignalId()) == true)
	{
		// SignaID already presnt in ArchiveSignals
		//
		return;
	}

	if (ui->archiveSignals->topLevelItemCount() >= 12)
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

	QTreeWidgetItem* item = new QTreeWidgetItem(ui->archiveSignals, itemData);
	item->setData(0, Qt::UserRole, QVariant::fromValue(signal));

	ui->archiveSignals->addTopLevelItem(item);

	ui->archiveSignals->setCurrentItem(item, QItemSelectionModel::SelectCurrent);

	disableControls();

	return;
}

void DialogChooseArchiveSignals::removeSelectedSignal()
{
	assert(ui->archiveSignals);

	QModelIndex currentIndex = ui->archiveSignals->currentIndex();

	if (currentIndex.isValid() == true)
	{
		ui->archiveSignals->takeTopLevelItem(currentIndex.row());
	}

	disableControls();
	return;
}

bool DialogChooseArchiveSignals::archiveSignalsHasSignalId(QString signalId)
{
	int itemCount = ui->archiveSignals->topLevelItemCount();

	for (int i = 0; i < itemCount; i++)
	{
		QTreeWidgetItem* item = ui->archiveSignals->topLevelItem(i);
		assert(item);

		if (item->text(0) == signalId)
		{
			return true;
		}
	}

	return false;
}

void DialogChooseArchiveSignals::disableControls()
{
	assert(ui->filteredSignals);
	assert(ui->archiveSignals);

	const FilteredArchiveSignalsModel* fileterModel = dynamic_cast<const FilteredArchiveSignalsModel*>(ui->filteredSignals->model());
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
			enableAddButton = !archiveSignalsHasSignalId(signal.customSignalId());
		}
		else
		{
			enableAddButton = false;
		}
	}

	// Remove Signal Button
	//
	{
		QModelIndex index = ui->archiveSignals->currentIndex();

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
		enableRemoveAll = ui->archiveSignals->topLevelItemCount() > 0;
	}


	// --
	//
	ui->addSignalButton->setEnabled(enableAddButton);
	ui->removeSignalButton->setEnabled(enableRemoveButton);
	ui->removeAllSignalsButton->setEnabled(enableRemoveAll);

	return;
}

void DialogChooseArchiveSignals::on_addSignalButton_clicked()
{
	QModelIndex index = ui->filteredSignals->currentIndex();
	if (index.isValid() == false)
	{
		return;
	}

	const FilteredArchiveSignalsModel* model = dynamic_cast<const FilteredArchiveSignalsModel*>(index.model());

	if (model == nullptr)
	{
		assert(dynamic_cast<const FilteredArchiveSignalsModel*>(index.model()) != nullptr);
		return;
	}

	const AppSignalParam signal = model->signalByRow(index.row());
	addSignal(signal);

	return;
}

void DialogChooseArchiveSignals::on_removeSignalButton_clicked()
{
	removeSelectedSignal();
}

void DialogChooseArchiveSignals::on_removeAllSignalsButton_clicked()
{
	ui->archiveSignals->clear();

	disableControls();

	return;
}

void DialogChooseArchiveSignals::on_filterEdit_textChanged(const QString& /*arg*/)
{
	fillSignalList();
}


void DialogChooseArchiveSignals::on_filterEdit_editingFinished()
{
	QString arg = ui->filterEdit->text();

	if (theSettings.m_archiveSignalsDialogFilterCompleter.contains(arg) == false)
	{
		theSettings.m_archiveSignalsDialogFilterCompleter << arg;

		while (theSettings.m_archiveSignalsDialogFilterCompleter.size() > 1000)
		{
			theSettings.m_archiveSignalsDialogFilterCompleter.pop_front();
		}

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
		assert(completerModel);

		if (completerModel != nullptr)
		{
			completerModel->setStringList(theSettings.m_archiveSignalsDialogFilterCompleter);
		}
	}

	return;
}

void DialogChooseArchiveSignals::on_filteredSignals_doubleClicked(const QModelIndex& index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const FilteredArchiveSignalsModel* model = dynamic_cast<const FilteredArchiveSignalsModel*>(index.model());

	if (model == nullptr)
	{
		assert(dynamic_cast<const FilteredArchiveSignalsModel*>(index.model()) != nullptr);
		return;
	}

	const AppSignalParam signal = model->signalByRow(index.row());
	addSignal(signal);

	return;
}

void DialogChooseArchiveSignals::slot_filteredSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	disableControls();
}

void DialogChooseArchiveSignals::on_archiveSignals_doubleClicked(const QModelIndex& /*index*/)
{
	removeSelectedSignal();
}

void DialogChooseArchiveSignals::slot_archiveSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	disableControls();
}

//
//	FilteredArchiveSignalsModel
//
FilteredArchiveSignalsModel::FilteredArchiveSignalsModel(const std::vector<AppSignalParam>& signalss, QObject* parent)
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

void DialogChooseArchiveSignals::on_buttonBox_accepted()
{
	m_acceptedSignals.reserve(ui->archiveSignals->topLevelItemCount());

	for (int i = 0; i < ui->archiveSignals->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* treeItem = ui->archiveSignals->topLevelItem(i);
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

int FilteredArchiveSignalsModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(m_signalIndexes.size());
}

int FilteredArchiveSignalsModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 3;	// Columns: SignalID, Typem Caption
}

QVariant FilteredArchiveSignalsModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
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

QVariant FilteredArchiveSignalsModel::data(const QModelIndex& index, int role) const
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

void FilteredArchiveSignalsModel::filterSignals(QString filter)
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

AppSignalParam FilteredArchiveSignalsModel::signalByRow(int row) const
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



