#include "DialogChooseArchiveSignals.h"
#include "ui_DialogChooseArchiveSignals.h"

#include "../OnlineLib/SocketIO.h"

#include <ClientLib/AppSignalManager.h>
#include <AppSignalLists/SignalList.h>

//
//
//	DialogChooseArchiveSignals
//
//
DialogChooseArchiveSignals::ArchiveSignalType DialogChooseArchiveSignals::s_lastSignalType = DialogChooseArchiveSignals::ArchiveSignalType::AllSignals;
QString DialogChooseArchiveSignals::s_lastServer;

using namespace MonitorInternal;

DialogChooseArchiveSignals::DialogChooseArchiveSignals(const ClientLib::AppSignalManager& signalManager,
													   const std::vector<SoftwareEndpoint::ArchiveService>& archiveServices,
													   const ArchiveSource& init,
													   const AppSignalLists::AppSignalListSet& lists,
													   QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogChooseArchiveSignals),
	m_archiveServices(archiveServices),
	m_appSignalListSet(lists),
	s_allServers(tr("All Servers"))
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMaximizeButtonHint);

	// Fill SignalType Combo
	//
	Q_ASSERT(ui->signalTypeCombo);
	ui->signalTypeCombo->addItem(tr("All Signals"), QVariant::fromValue<ArchiveSignalType>(ArchiveSignalType::AllSignals));
	ui->signalTypeCombo->addItem(tr("Analog Signals"), QVariant::fromValue<ArchiveSignalType>(ArchiveSignalType::AnalogSignals));
	ui->signalTypeCombo->addItem(tr("Discrete Signals"), QVariant::fromValue<ArchiveSignalType>(ArchiveSignalType::DiscreteSignals));

	if (int currentSignalTypeIndex = ui->signalTypeCombo->findData(QVariant::fromValue<ArchiveSignalType>(s_lastSignalType));
		currentSignalTypeIndex != -1)
	{
		ui->signalTypeCombo->setCurrentIndex(currentSignalTypeIndex);
	}

	connect(ui->signalTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogChooseArchiveSignals::signalTypeCurrentIndexChanged);

	// Fill Schema Combo
	//
	std::ranges::sort(m_archiveServices, [](const auto& a, const auto& b) {return a.shortenId < b.shortenId;});

	fillServerCombo();

	connect(ui->serverCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogChooseArchiveSignals::serverCurrentIndexChanged);

	// Fill Start/End date/time
	//
	QDateTime startTime = init.requestStartTime.toDateTime();
	QDateTime endTime = init.requestEndTime.toDateTime();

	ui->startDateEdit->setDate(startTime.date());
	ui->startTimeEdit->setTime(startTime.time());

	ui->endDateEdit->setDate(endTime.date());
	ui->endTimeEdit->setTime(endTime.time());

	// TimeType Combo
	//
	Q_ASSERT(ui->timeTypeCombo);
	ui->timeTypeCombo->addItem(tr("Server Time"), QVariant::fromValue(E::TimeType::Local));
	ui->timeTypeCombo->addItem(tr("Server Time UTC%100").arg(QChar(0x00B1)), QVariant::fromValue(E::TimeType::System));
	ui->timeTypeCombo->addItem(tr("Plant Time"), QVariant::fromValue(E::TimeType::Plant));

	if (int currentTimeType = ui->timeTypeCombo->findData(QVariant::fromValue(init.timeType));
		currentTimeType != -1)
	{
		ui->timeTypeCombo->setCurrentIndex(currentTimeType);
	}

	//AppSignalLists
	//
	connect(&m_appSignalListSet,
			&AppSignalLists::AppSignalListSet::updatePerformed,
			this,
			&DialogChooseArchiveSignals::fillAppSignalLists);
	fillAppSignalLists();

	// Remove periodic records checkbox
	//
	Q_ASSERT(ui->removePeriodicCheckbox);
	ui->removePeriodicCheckbox->setChecked(init.removePeriodicRecords);

	// Set filter completer
	//
	auto archiveSignalsDialogFilterCompleter = QSettings{}.value("DialogChooseArchiveSignals/filter").toStringList();

	m_filterCompleter = new QCompleter(archiveSignalsDialogFilterCompleter, this);
	m_filterCompleter->setCaseSensitivity(Qt::CaseInsensitive);

	ui->filterEdit->setCompleter(m_filterCompleter);

	// --
	//
	QStringList headerLabels;
	headerLabels << tr("SignalID");
	headerLabels << tr("Type");
	headerLabels << tr("Caption");
	headerLabels << tr("Server");

	ui->archiveSignals->setHeaderLabels(headerLabels);

	std::vector<AppSignalParam> signalParams = signalManager.signalList();

	std::vector<ArchiveSignal> signalParamsSources;
	signalParamsSources.reserve(signalParams.size());

	for (const AppSignalParam& sp : signalParams)
	{
		// Make signal copy for each ArchiveService which has this signal
		//
		for (const SoftwareEndpoint::ArchiveService& archiveService : m_archiveServices)
		{
			if (signalManager.dataServiceHasSignal(archiveService.appDataServiceId, sp.appSignalId()) == true)
			{
				signalParamsSources.emplace_back(sp, archiveService.equipmentId, archiveService.shortenId);
			}
		}
	}

	FilteredArchiveSignalsModel* model = new FilteredArchiveSignalsModel{std::move(signalParamsSources), ui->filteredSignals};
	ui->filteredSignals->setModel(model);
	
	// --
	//
	fillSignalList();

	// --
	//
	connect(ui->filteredSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseArchiveSignals::slot_filteredSignalsSelectionChanged);
	connect(ui->archiveSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseArchiveSignals::slot_archiveSignalsSelectionChanged);
	connect(ui->listCombo,
			static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
			this,
			&DialogChooseArchiveSignals::listComboIndexChanged);


	// --
	// --
	updateControls();

	ui->filteredSignals->header()->resizeSection(1, ui->filteredSignals->header()->sectionSizeHint(1));		// 1 is TypeColumn (A/D)
	ui->archiveSignals->header()->resizeSection(1 , ui->archiveSignals->header()->sectionSizeHint(1));		// 1 is TypeColumn (A/D)

	// Fill added signals
	//
	std::ranges::for_each(init.acceptedSignals,
		[this](const ArchiveSignal& appSignal)
		{
			addSignal(appSignal);
		});

	return;
}

DialogChooseArchiveSignals::~DialogChooseArchiveSignals()
{
	delete ui;
}

ArchiveSource DialogChooseArchiveSignals::accpetedResult() const
{
	return m_result;
}

void DialogChooseArchiveSignals::fillServerCombo()
{
	Q_ASSERT(ui->serverCombo);

	ui->serverCombo->clear();

	// Fill combo box
	//
	ui->serverCombo->addItem(s_allServers, {});

	for (const SoftwareEndpoint::ArchiveService& srv : m_archiveServices)
	{
		ui->serverCombo->addItem(srv.shortenId, QVariant{srv.equipmentId});
	}

	// Set current item in combo box
	//
	if (int serverIndex = ui->serverCombo->findData(QVariant{s_lastServer});
		serverIndex != -1)
	{
		ui->serverCombo->setCurrentIndex(serverIndex);
	}

	return;
}

void DialogChooseArchiveSignals::fillAppSignalLists()
{
	// Refresh AppSignalLists combo
	//
	QString selectedList = ui->listCombo->currentData().toString();

	ui->listCombo->blockSignals(true);

	ui->listCombo->clear();
	ui->listCombo->addItem(tr("Not selected"), QString());

	// Remove previously set filter
	//
	if (selectedList.isEmpty() == false)
	{
		fillSignalList();
	}

	// Fill lists combo
	//
	const auto lists = m_appSignalListSet.lists();

	for (const auto& list : lists)
	{
		ui->listCombo->addItem(tr("[%1] %2").arg(list->id()).arg(list->caption()), list->id());
	}
	if (lists.empty() == true)
	{
		ui->listCombo->setEnabled(false);
	}

	ui->listCombo->blockSignals(false);
}

void DialogChooseArchiveSignals::fillSignalList()
{
	filterSignals();
	return;
}

void DialogChooseArchiveSignals::filterSignals()
{
	// Get SignalType
	//
	ArchiveSignalType signaType = ui->signalTypeCombo->currentData().value<ArchiveSignalType>();

	Q_ASSERT(signaType == ArchiveSignalType::AllSignals ||
			 signaType == ArchiveSignalType::AnalogSignals ||
			 signaType == ArchiveSignalType::DiscreteSignals);

	// Get ArchiveServiceId
	//
	QString server = ui->serverCombo->currentData().toString();

	// signalIdFilter
	//
	QString signalIdFilter = ui->filterEdit->text();
	
	// appSignalList
	//
	std::optional<AppSignalLists::AppSignalList*> appSignalList;

	QString selectedList = ui->listCombo->currentData().toString();
	if (selectedList.isEmpty() == false)
	{
		std::shared_ptr<AppSignalLists::AppSignalList> list = m_appSignalListSet.get(selectedList);
		if (list != nullptr)
		{
			appSignalList = list.get();
		}
	}

	// Apply filter to model
	//
	FilteredArchiveSignalsModel* model = dynamic_cast<FilteredArchiveSignalsModel*>(ui->filteredSignals->model());
	Q_ASSERT(model);

	model->filterSignals(server, signaType, appSignalList, signalIdFilter);

	return;
}

void DialogChooseArchiveSignals::addSignal(const ArchiveSignal& archiveSignal)
{
	if (signalAlreadyPresent(archiveSignal.signalParam.customSignalId(), archiveSignal.archiveServiceId) == true)
	{
		// SignaID already present in ArchiveSignals
		//
		return;
	}

	if (ui->archiveSignals->topLevelItemCount() >= ARCH_REQUEST_MAX_SIGNALS)
	{
		QMessageBox::critical(this, qAppName(), tr("The maximum number of signals reached."));
		return;
	}

	const AppSignalParam& signalParam = archiveSignal.signalParam;

	QString signalType;
	switch (signalParam.type())
	{
	case E::SignalType::Analog:		signalType = "A";	break;
	case E::SignalType::Discrete:	signalType = "D";	break;
	case E::SignalType::Bus:		signalType = "B";	break;
	}

	Q_ASSERT(signalType.isEmpty() == false);

	QStringList itemData;
	itemData << signalParam.customSignalId();
	itemData << signalType;
	itemData << signalParam.caption();
	itemData << archiveSignal.archiveServiceShortenId;

	QString toolTip = QString{"%1\n%2\n%3\nServer: %4 (%5)"}
						.arg(archiveSignal.signalParam.customSignalId())
						.arg(archiveSignal.signalParam.appSignalId())
						.arg(archiveSignal.signalParam.caption())
						.arg(archiveSignal.archiveServiceShortenId)
						.arg(archiveSignal.archiveServiceId);

	QTreeWidgetItem* item = new QTreeWidgetItem(ui->archiveSignals, itemData);
	item->setData(0, Qt::UserRole, QVariant::fromValue(archiveSignal));
	item->setData(3, Qt::UserRole, QVariant::fromValue(archiveSignal.archiveServiceId));

	item->setToolTip(0, toolTip);
	item->setToolTip(1, toolTip);
	item->setToolTip(2, toolTip);
	item->setToolTip(3, toolTip);

	ui->archiveSignals->addTopLevelItem(item);
	ui->archiveSignals->setCurrentItem(item, QItemSelectionModel::SelectCurrent);

	updateControls();

	return;
}

void DialogChooseArchiveSignals::removeSelectedSignal()
{
	Q_ASSERT(ui->archiveSignals);

	QModelIndex currentIndex = ui->archiveSignals->currentIndex();

	if (currentIndex.isValid() == true)
	{
		ui->archiveSignals->takeTopLevelItem(currentIndex.row());
	}

	updateControls();
	return;
}

bool DialogChooseArchiveSignals::signalAlreadyPresent(const QString& customSignalId, const QString& archiveServiceId)
{
	int itemCount = ui->archiveSignals->topLevelItemCount();

	for (int i = 0; i < itemCount; i++)
	{
		QTreeWidgetItem* item = ui->archiveSignals->topLevelItem(i);
		Q_ASSERT(item);

		QString itemArchiveServiceId = item->data(3, Qt::UserRole).toString();
		Q_ASSERT(itemArchiveServiceId.isEmpty() == false);

		if (item->text(0) == customSignalId && itemArchiveServiceId == archiveServiceId)
		{
			return true;
		}
	}

	return false;
}

void DialogChooseArchiveSignals::updateControls()
{
	Q_ASSERT(ui->filteredSignals);
	Q_ASSERT(ui->archiveSignals);

	const FilteredArchiveSignalsModel* fileterModel = dynamic_cast<const FilteredArchiveSignalsModel*>(ui->filteredSignals->model());
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
	try
	{
		QModelIndex index = ui->filteredSignals->currentIndex();

		if (index.isValid() == true)
		{
			ArchiveSignal signal = fileterModel->signalByRow(index.row());
			enableAddButton = !signalAlreadyPresent(signal.signalParam.customSignalId(), signal.archiveServiceId);
		}
		else
		{
			enableAddButton = false;
		}
	}
	catch (std::out_of_range&)
	{
		enableAddButton = false;
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

void DialogChooseArchiveSignals::signalTypeCurrentIndexChanged(int /*index*/)
{
	filterSignals();
}

void DialogChooseArchiveSignals::serverCurrentIndexChanged(int /*index*/)
{
	filterSignals();
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
		Q_ASSERT(dynamic_cast<const FilteredArchiveSignalsModel*>(index.model()) != nullptr);
		return;
	}

	try
	{
		auto signal = model->signalByRow(index.row());
		addSignal(signal);
	}
	catch (std::out_of_range&)
	{
	}

	return;
}

void DialogChooseArchiveSignals::on_removeSignalButton_clicked()
{
	removeSelectedSignal();
}

void DialogChooseArchiveSignals::on_removeAllSignalsButton_clicked()
{
	ui->archiveSignals->clear();

	updateControls();

	return;
}

void DialogChooseArchiveSignals::on_filterEdit_textChanged(const QString& /*arg*/)
{
	fillSignalList();
}


void DialogChooseArchiveSignals::on_filterEdit_editingFinished()
{
	QString arg = ui->filterEdit->text();

	if (auto archiveSignalsDialogFilterCompleter = QSettings{}.value("DialogChooseArchiveSignals/filter").toStringList();
		archiveSignalsDialogFilterCompleter.contains(arg) == false)
	{
		archiveSignalsDialogFilterCompleter << arg;

		while (archiveSignalsDialogFilterCompleter.size() > 1000)
		{
			archiveSignalsDialogFilterCompleter.pop_front();
		}

		QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
		Q_ASSERT(completerModel);

		if (completerModel != nullptr)
		{
			QSettings{}.setValue("DialogChooseArchiveSignals/filter", archiveSignalsDialogFilterCompleter);
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
		Q_ASSERT(dynamic_cast<const FilteredArchiveSignalsModel*>(index.model()) != nullptr);
		return;
	}

	try
	{
		auto signal = model->signalByRow(index.row());
		addSignal(signal);
	}
	catch (std::out_of_range&)
	{
	}

	return;
}

void DialogChooseArchiveSignals::slot_filteredSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
{
	updateControls();
}

void DialogChooseArchiveSignals::on_archiveSignals_doubleClicked(const QModelIndex& /*index*/)
{
	removeSelectedSignal();
}

void DialogChooseArchiveSignals::slot_archiveSignalsSelectionChanged(const QItemSelection& /*selected*/,
																	 const QItemSelection& /*deselected*/)
{
	updateControls();
}

void DialogChooseArchiveSignals::listComboIndexChanged(int /*index*/)
{
	fillSignalList();
}

void DialogChooseArchiveSignals::on_buttonBox_accepted()
{
	m_result.timeType = ui->timeTypeCombo->currentData().value<E::TimeType>();

	QDateTime startTime;
	startTime.setDate(ui->startDateEdit->date());
	startTime.setTime(ui->startTimeEdit->time());

	QDateTime endTime;
	endTime.setDate(ui->endDateEdit->date());
	endTime.setTime(ui->endTimeEdit->time());

	m_result.requestStartTime = TimeStamp(startTime);
	m_result.requestEndTime = TimeStamp(endTime);

	m_result.removePeriodicRecords = ui->removePeriodicCheckbox->isChecked();

	s_lastSignalType = ui->signalTypeCombo->currentData().value<ArchiveSignalType>();
	s_lastServer = ui->serverCombo->currentData().toString();

	m_result.acceptedSignals.reserve(ui->archiveSignals->topLevelItemCount());

	for (int i = 0; i < ui->archiveSignals->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* treeItem = ui->archiveSignals->topLevelItem(i);
		Q_ASSERT(treeItem);

		QVariant signalVariant = treeItem->data(0, Qt::UserRole);

		Q_ASSERT(signalVariant.isNull() == false);
		Q_ASSERT(signalVariant.isValid() == true);
		Q_ASSERT(signalVariant.canConvert<ArchiveSignal>() == true);

		ArchiveSignal archiveSignal = signalVariant.value<ArchiveSignal>();

		Q_ASSERT(archiveSignal.signalParam.customSignalId() == treeItem->text(0));

		m_result.acceptedSignals.push_back(std::move(archiveSignal));
	}

	return;
}

//
//
//	FilteredArchiveSignalsModel
//
//
FilteredArchiveSignalsModel::FilteredArchiveSignalsModel(std::vector<ArchiveSignal>&& signalss, QObject* parent)
	: QAbstractTableModel(parent),
	m_signals(std::move(signalss))
{

	std::sort(m_signals.begin(), m_signals.end(),
			[](const auto& a, const auto& b) -> bool
			{
				return std::make_tuple(a.signalParam.customSignalId(), a.archiveServiceId) <
					   std::make_tuple(b.signalParam.customSignalId(), b.archiveServiceId);
			});

	m_signalIndexes.reserve(m_signals.size());

	// Init m_startWithArrays.
	// m_startWithArrays keeps vector of indexes for "StartWith"
	//
	for (size_t index = 0, signalCount = m_signals.size(); index < signalCount; index++)
	{
		const AppSignalParam& appSignal = m_signals[index].signalParam;
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
			std::vector<size_t> signalIndexes;
			signalIndexes.reserve(8192);

			signalIndexes.push_back(static_cast<int>(index));

			m_startWithArrays[firstLetter] = signalIndexes;
		}
		else
		{
			std::vector<size_t>& signalIndexes = foundStartWithIt->second;
			signalIndexes.push_back(static_cast<int>(index));
		}
	}

	return;
}

int FilteredArchiveSignalsModel::rowCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(m_signalIndexes.size());
}

int FilteredArchiveSignalsModel::columnCount(const QModelIndex& /*parent*/) const
{
	return 4;	// ColumnType::SignalId, ... , ColumnType::Server
}

QVariant FilteredArchiveSignalsModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (static_cast<ColumnType>(section))
			{
			case ColumnType::SignalId:
				return tr("SignalID");

			case ColumnType::Type:
				return tr("Type");

			case ColumnType::Caption:
				return tr("Caption");

			case ColumnType::Server:
				return tr("Server");
			}

			Q_ASSERT(false);
			return {};
		}
	}

	return {};
}

QVariant FilteredArchiveSignalsModel::data(const QModelIndex& index, int role) const
{
	size_t row = index.row();
	ColumnType col = static_cast<ColumnType>(index.column());

	switch (role)
	{
	case Qt::DisplayRole:
		{
			if (row >= m_signalIndexes.size())
			{
				Q_ASSERT(row < m_signalIndexes.size());
				return {};
			}

			size_t signalIndex = m_signalIndexes[row];
			if (signalIndex >= m_signals.size())
			{
				Q_ASSERT(signalIndex < m_signals.size());
				return {};
			}

			const ArchiveSignal& archiveSignal = m_signals[signalIndex];

			switch (col)
			{
			case ColumnType::SignalId:
				return archiveSignal.signalParam.customSignalId();
			case ColumnType::Type:
				switch (archiveSignal.signalParam.type())
				{
				case E::SignalType::Analog:		return QString{"A"};
				case E::SignalType::Discrete:	return QString{"D"};
				case E::SignalType::Bus:		return QString{"B"};
				default:
					Q_ASSERT(false);
					return {};
				}
			case ColumnType::Caption:
				return archiveSignal.signalParam.caption();
			case ColumnType::Server:
				return archiveSignal.archiveServiceShortenId;
			default:
				Q_ASSERT(false);
				return {};
			}
		}
		break;
	case Qt::ToolTipRole:
		{
			if (row >= m_signalIndexes.size())
			{
				Q_ASSERT(row < m_signalIndexes.size());
				return {};
			}

			size_t signalIndex = m_signalIndexes[row];

			if (signalIndex >= m_signals.size())
			{
				Q_ASSERT(signalIndex < m_signals.size());
				return {};
			}

			const ArchiveSignal& archiveSignal = m_signals[signalIndex];

			QString toolTip = QString{"%1\n%2\n%3\nServer: %4 (%5)"}
								.arg(archiveSignal.signalParam.customSignalId())
								.arg(archiveSignal.signalParam.appSignalId())
								.arg(archiveSignal.signalParam.caption())
								.arg(archiveSignal.archiveServiceShortenId)
								.arg(archiveSignal.archiveServiceId);

			return toolTip;
		}
		break;
	default:
		return {};
	}
}

void FilteredArchiveSignalsModel::filterSignals(QString server, DialogChooseArchiveSignals::ArchiveSignalType signalType, std::optional<AppSignalLists::AppSignalList*> appSignalList, const QString& signalIdFilter)
{
	// Get hashes list filtered by signal list
	//
	std::set<Hash> appSignalListHashes;
	if (appSignalList.has_value() == true)
	{
		Q_ASSERT(appSignalList.value());
		appSignalListHashes = appSignalList.value()->listHashesCache();
	}

	//
	//	
	beginResetModel();

	QString filterText = signalIdFilter.trimmed().toLower();
	server = server.trimmed();

	if (filterText.isEmpty() == true)
	{
		// No text filter, add all signals by other filters
		//
		m_signalIndexes.clear();

		for (size_t i = 0, signalCount = m_signals.size(); i < signalCount; i++)
		{
			const ArchiveSignal& s = m_signals[i];

			if (appSignalList.has_value() == true)
			{
				if (appSignalListHashes.contains(s.signalParam.hash()) == false)
				{
					continue;
				}
			}

			if ((signalType == DialogChooseArchiveSignals::ArchiveSignalType::AllSignals) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::AnalogSignals && s.signalParam.isAnalog() == true) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::DiscreteSignals && s.signalParam.isDiscrete() == true))
			{
			}
			else
			{
				continue;
			}

			if (server.isEmpty() == true ||
				s.archiveServiceId == server)
			{
			}
			else
			{
				continue;
			}

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
		std::vector<size_t>& signalIndexes = foundStartWithIt->second;

		m_signalIndexes.clear();

		for (size_t index : signalIndexes)
		{
			if (index >= m_signals.size())
			{
				Q_ASSERT(index < m_signals.size());
				continue;
			}

			const ArchiveSignal& s = m_signals[index];

			if (appSignalList.has_value() == true)
			{
				if (appSignalListHashes.contains(s.signalParam.hash()) == false)
				{
					continue;
				}
			}

			// if filterText.size() == 1 then we already filrtered it by getting data from m_startWithArrays
			//
			if (filterText.size() == 1 ||
				s.signalParam.customSignalId().startsWith(filterText, Qt::CaseInsensitive) == true)
			{
			}
			else
			{
				continue;
			}

			if ((signalType == DialogChooseArchiveSignals::ArchiveSignalType::AllSignals) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::AnalogSignals && s.signalParam.isAnalog() == true) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::DiscreteSignals && s.signalParam.isDiscrete() == true))
			{
			}
			else
			{
				continue;
			}

			if (server.isEmpty() == true ||
				s.archiveServiceId == server)
			{
			}
			else
			{
				continue;
			}

			m_signalIndexes.push_back(index);
		}
	}

	endResetModel();

	return;
}

ArchiveSignal FilteredArchiveSignalsModel::signalByRow(int row) const	// can throw std::out_of_range()
{
	size_t r = static_cast<size_t>(row);

	if (r >= m_signalIndexes.size())
	{
		Q_ASSERT(r < m_signalIndexes.size());
		throw std::out_of_range("FilteredArchiveSignalsModel::signalByRow(r) where r is out of range");
	}

	size_t signalIndex = m_signalIndexes[r];

	if (signalIndex >= m_signals.size())
	{
		Q_ASSERT(signalIndex < m_signals.size());
		throw std::out_of_range("FilteredArchiveSignalsModel::signalByRow(r) where signalIndex is out of range");
	}

	return m_signals[signalIndex];
}


