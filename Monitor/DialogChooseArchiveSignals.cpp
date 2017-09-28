#include "DialogChooseArchiveSignals.h"
#include "ui_DialogChooseArchiveSignals.h"
#include "Settings.h"


DialogChooseArchiveSignals::ArchiveSignalType DialogChooseArchiveSignals::m_lastSignalType = DialogChooseArchiveSignals::ArchiveSignalType::AllSignals;
QString DialogChooseArchiveSignals::m_lastSchemaId;


DialogChooseArchiveSignals::DialogChooseArchiveSignals(
		const std::vector<VFrame30::SchemaDetails>& schemaDetails,
		const ArchiveSource& init,
		QWidget* parent) :
	QDialog(parent),
	ui(new Ui::DialogChooseArchiveSignals),
	m_schemasDetails(schemaDetails)
{
	ui->setupUi(this);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint | Qt::WindowMaximizeButtonHint);

	// Fill SignalType Combo
	//
	assert(ui->signalTypeCombo);
	ui->signalTypeCombo->addItem(tr("All Signals"), QVariant::fromValue<ArchiveSignalType>(ArchiveSignalType::AllSignals));
	ui->signalTypeCombo->addItem(tr("Analog Signals"), QVariant::fromValue<ArchiveSignalType>(ArchiveSignalType::AnalogSignals));
	ui->signalTypeCombo->addItem(tr("Discrete Signals"), QVariant::fromValue<ArchiveSignalType>(ArchiveSignalType::DiscreteSignals));

	int currentSignalTypeIndex = ui->signalTypeCombo->findData(QVariant::fromValue<ArchiveSignalType>(m_lastSignalType));
	assert(currentSignalTypeIndex != -1);

	if (currentSignalTypeIndex != -1)
	{
		ui->signalTypeCombo->setCurrentIndex(currentSignalTypeIndex);
	}

	connect(ui->signalTypeCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogChooseArchiveSignals::signalTypeCurrentIndexChanged);

	// Fill Schema Combo
	//
	assert(ui->schemaCombo);
	ui->schemaCombo->addItem(tr("All Schemas"), QVariant::fromValue<QString>(QString()));
	for (const VFrame30::SchemaDetails& schema : m_schemasDetails)
	{
		ui->schemaCombo->addItem(schema.m_schemaId + " - " + schema.m_caption, QVariant(schema.m_schemaId));
	}

	int schemaIndex = ui->schemaCombo->findData(QVariant(m_lastSchemaId));
	if (schemaIndex != -1)
	{
		ui->schemaCombo->setCurrentIndex(schemaIndex);
	}

	connect(ui->schemaCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogChooseArchiveSignals::schemaCurrentIndexChanged);

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
	assert(ui->timeTypeCombo);
	ui->timeTypeCombo->addItem(tr("Server Time"), QVariant::fromValue(E::TimeType::Local));
	ui->timeTypeCombo->addItem(tr("Server Time UTC%100").arg(QChar(0x00B1)), QVariant::fromValue(E::TimeType::System));
	ui->timeTypeCombo->addItem(tr("Plant Time"), QVariant::fromValue(E::TimeType::Plant));

	int currentTimeType = ui->timeTypeCombo->findData(QVariant::fromValue(init.timeType));
	assert(currentTimeType != -1);

	if (currentTimeType != -1)
	{
		ui->timeTypeCombo->setCurrentIndex(currentTimeType);
	}

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

	FilteredArchiveSignalsModel* model = new FilteredArchiveSignalsModel(theSignals.signalList(),
																		 m_schemasDetails,
																		 ui->filteredSignals);
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
	for (const AppSignalParam& appSignal : init.acceptedSignals)
	{
		addSignal(appSignal);
	}

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
	assert(signaType == ArchiveSignalType::AllSignals ||
		   signaType == ArchiveSignalType::AnalogSignals ||
		   signaType == ArchiveSignalType::DiscreteSignals);

	// Get schema id, empty if schema is not required
	//
	QString schemaId = ui->schemaCombo->currentData().toString();

	// Apply filter to model
	//
	FilteredArchiveSignalsModel* model = dynamic_cast<FilteredArchiveSignalsModel*>(ui->filteredSignals->model());
	assert(model);

	model->filterSignals(signaType, ui->filterEdit->text(), schemaId);

	return;
}

void DialogChooseArchiveSignals::addSignal(const AppSignalParam& signal)
{
	if (archiveSignalsHasSignalId(signal.customSignalId()) == true)
	{
		// SignaID already presnt in ArchiveSignals
		//
		return;
	}

	if (ui->archiveSignals->topLevelItemCount() >= ARCH_REQUEST_MAX_SIGNALS)
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

	QString toolTip = QString("%1\n%2\n%3")
						.arg(signal.customSignalId())
						.arg(signal.appSignalId())
						.arg(signal.caption());

	QTreeWidgetItem* item = new QTreeWidgetItem(ui->archiveSignals, itemData);
	item->setData(0, Qt::UserRole, QVariant::fromValue(signal));

	item->setToolTip(0, toolTip);
	item->setToolTip(1, toolTip);
	item->setToolTip(2, toolTip);

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

void DialogChooseArchiveSignals::signalTypeCurrentIndexChanged(int /*index*/)
{
	filterSignals();
}

void DialogChooseArchiveSignals::schemaCurrentIndexChanged(int /*index*/)
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
FilteredArchiveSignalsModel::FilteredArchiveSignalsModel(const std::vector<AppSignalParam>& signalss,
														 const std::vector<VFrame30::SchemaDetails>& schemasDetails,
														 QObject* parent)
	: QAbstractTableModel(parent),
	m_signals(signalss),
	m_schemasDetails(schemasDetails)
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

	return;
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

	m_lastSignalType = ui->signalTypeCombo->currentData().value<ArchiveSignalType>();
	m_lastSchemaId = ui->schemaCombo->currentData().toString();

	m_result.acceptedSignals.reserve(ui->archiveSignals->topLevelItemCount());

	for (int i = 0; i < ui->archiveSignals->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* treeItem = ui->archiveSignals->topLevelItem(i);
		assert(treeItem);

		QVariant signalVariant = treeItem->data(0, Qt::UserRole);

		assert(signalVariant.isNull() == false);
		assert(signalVariant.isValid() == true);

		AppSignalParam signalParam = signalVariant.value<AppSignalParam>();

		assert(signalParam.customSignalId() == treeItem->text(0));

		m_result.acceptedSignals.push_back(signalParam);
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
	case Qt::ToolTipRole:
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

			QString toolTip = QString("%1\n%2\n%3")
								.arg(signalParam.customSignalId())
								.arg(signalParam.appSignalId())
								.arg(signalParam.caption());

			return toolTip;

		}
		break;
	default:
		return QVariant();
	}

	return QVariant();
}

void FilteredArchiveSignalsModel::filterSignals(DialogChooseArchiveSignals::ArchiveSignalType signalType, QString signalIdFilter, QString schemaId)
{
	beginResetModel();

	QString filterText = signalIdFilter.trimmed().toLower();

	auto sit = std::find_if(m_schemasDetails.begin(), m_schemasDetails.end(),
				[&schemaId](const VFrame30::SchemaDetails& details)
				{
					return details.m_schemaId == schemaId;
				});

	int schemaIndex = -1;
	if (sit != m_schemasDetails.end())
	{
		schemaIndex = std::distance(m_schemasDetails.begin(), sit);
		assert(schemaIndex < m_schemasDetails.size());
	}

	if (filterText.isEmpty() == true)
	{
		// No filter, add all signals by other filters
		//
		m_signalIndexes.clear();

		int signalCount = static_cast<int>(m_signals.size());
		for (int i = 0; i < signalCount; i++)
		{
			if ((signalType == DialogChooseArchiveSignals::ArchiveSignalType::AllSignals) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::AnalogSignals && m_signals[i].isAnalog() == true) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::DiscreteSignals && m_signals[i].isDiscrete() == true))
			{
			}
			else
			{
				continue;
			}

			if (schemaId.isEmpty() == true ||
				(schemaIndex != -1 && m_schemasDetails[schemaIndex].m_signals.count(m_signals[i].appSignalId()) != 0))
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
			}
			else
			{
				continue;
			}

			if ((signalType == DialogChooseArchiveSignals::ArchiveSignalType::AllSignals) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::AnalogSignals && signal.isAnalog() == true) ||
				(signalType == DialogChooseArchiveSignals::ArchiveSignalType::DiscreteSignals && signal.isDiscrete() == true))
			{
			}
			else
			{
				continue;
			}

			if (schemaId.isEmpty() == true ||
				(schemaIndex != -1 && m_schemasDetails[schemaIndex].m_signals.count(signal.appSignalId()) != 0))
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



