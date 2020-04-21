#include "DialogTrendSignalPoints.h"
#include "ui_DialogTrendSignalPoints.h"
#include "DialogTrendSignalPoint.h"
#include "TrendSettings.h"
#include "TrendScale.h"

TrendPointsModel::TrendPointsModel(QObject* parent)
	: QAbstractTableModel(parent)
{
}

void TrendPointsModel::setSignalData(std::list<std::shared_ptr<TrendLib::OneHourData>>& signalData, const TrendLib::TrendSignalParam& trendSignal, E::TimeType timeType)
{
	m_trendSignal = trendSignal;

	// Delete old data
	//
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
		endRemoveRows();
	}

	// Set new data
	//
	m_signalData.swap(signalData);
	m_timeType = timeType;

	// Calculate new m_rowCount
	//
	m_rowCount = 0;

	for (const auto& oneHourDataPtr : m_signalData)
	{
		for (const auto& stateRecord : oneHourDataPtr->data)
		{
			m_rowCount += static_cast<int>(stateRecord.states.size());
		}
	}

	// Insert new rows
	//
	if (rowCount() > 0)
	{
		beginInsertRows(QModelIndex(), 0, rowCount() - 1);
		endInsertRows();
	}

	return;
}

int TrendPointsModel::stateItemIndex(const TrendLib::TrendStateItem& stateItem) const
{
	int currentIndex = 0;

	for (const auto& oneHourDataPtr : m_signalData)
	{
		for (const auto& stateRecord : oneHourDataPtr->data)
		{
			for (const TrendLib::TrendStateItem& item : stateRecord.states)
			{
				if (item.local == stateItem.local &&
					item.plant == stateItem.plant &&
					item.system == stateItem.system &&
					item.value == stateItem.value &&
					item.flags == stateItem.flags)
				{
					return currentIndex;
				}
				currentIndex++;
			}
		}
	}

	return -1;
}

TrendLib::TrendStateItem TrendPointsModel::stateItemByIndex(int index, int* oneHourIndex, int* recordIndex, int* stateIndex, bool* ok) const
{
	if (ok == nullptr || oneHourIndex == nullptr || recordIndex == nullptr || stateIndex == nullptr)
	{
		Q_ASSERT(ok);
		Q_ASSERT(oneHourIndex);
		Q_ASSERT(recordIndex);
		Q_ASSERT(stateIndex);
		return TrendLib::TrendStateItem();
	}

	int currentIndex = 0;
	*oneHourIndex = 0;
	*recordIndex = 0;
	*stateIndex = 0;
	*ok = 0;

	for (const auto& oneHourDataPtr : m_signalData)
	{
		*recordIndex = 0;

		for (const auto& stateRecord : oneHourDataPtr->data)
		{
			if (currentIndex <= index && index < currentIndex + stateRecord.states.size())
			{
				(*stateIndex) += static_cast<int>(stateRecord.states.size());

				*ok = true;

				return stateRecord.states[index - currentIndex];
			}

			currentIndex += static_cast<int>(stateRecord.states.size());

			(*recordIndex)++;
		}

		(*oneHourIndex)++;
	}

	*ok = false;
	return TrendLib::TrendStateItem();
}

int TrendPointsModel::rowCount(const QModelIndex& /*parent*/) const
{
	return m_rowCount;
}

int TrendPointsModel::columnCount(const QModelIndex& /*parent*/) const
{
	return static_cast<int>(Columns::Count);
}

QVariant TrendPointsModel::data(const QModelIndex& index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (index.column() == static_cast<int>(Columns::Number))
		{
			return QString("%1").arg(index.row());
		}

		bool ok = false;
		int oneHourIndex = 0;
		int recordIndex = 0;
		int stateIndex = 0;

		TrendLib::TrendStateItem stateItem = stateItemByIndex(index.row(), &oneHourIndex, &recordIndex, &stateIndex, &ok);
		if (ok == false)
		{
			Q_ASSERT(ok);
			return QVariant();
		}

		switch (index.column())
		{
			case static_cast<int>(Columns::Record):
				return QString("%1/%2").arg(oneHourIndex).arg(recordIndex);

			case static_cast<int>(Columns::Index):
				return QString("%1").arg(stateIndex);

			case static_cast<int>(Columns::Time):
				return stateItem.getTime(m_timeType).toDateTime().toString("dd.MM.yyyy hh:mm:ss.zzz");

			case static_cast<int>(Columns::Value):
				if (stateItem.isValid() == false)
				{
					return QString("???");
				}
				return TrendLib::TrendScale::scaleValueText(stateItem.value, m_scaleType, m_trendSignal);

		case static_cast<int>(Columns::Realtime):
			if (stateItem.isRealtimePoint() == true)
			{
				return QString("yes");
			}
			else
			{
				return QString("no");
			}
		default:
				Q_ASSERT(false);
		}
	}

	return QVariant();
}

QVariant TrendPointsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole)
	{
		if (orientation == Qt::Horizontal)
		{
			switch (section)
			{
			case static_cast<int>(Columns::Number):
				return QStringLiteral("#");
			case static_cast<int>(Columns::Record):
				return QStringLiteral("Record");
			case static_cast<int>(Columns::Index):
				return QStringLiteral("Index");
			case static_cast<int>(Columns::Time):
				return QStringLiteral("Time");
			case static_cast<int>(Columns::Value):
				return QStringLiteral("Value");
			case static_cast<int>(Columns::Realtime):
				return QStringLiteral("Realtime");
			default:
				Q_ASSERT(false);
			}
		}
	}

	return QVariant();
}


//--
//
DialogTrendSignalPoints::DialogTrendSignalPoints(const TrendLib::TrendSignalParam& trendSignal,
												 TrendLib::TrendSignalSet* trendSignalSet,
												 E::TimeType timeType, E::TrendMode trendMode,
												 QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogTrendSignalPoints),
	m_trendSignal(trendSignal),
	m_trendSignalSet(trendSignalSet),
	m_timeType(timeType),
	m_trendMode(trendMode)
{
	if (m_trendSignalSet == nullptr)
	{
		Q_ASSERT(m_trendSignalSet);
		return;
	}

	ui->setupUi(this);
	setWindowTitle(tr("Points - %1").arg(m_trendSignal.appSignalId()));

	// --
	//
	m_editStateItem.local = TimeStamp(QDateTime::currentDateTime()).timeStamp;
	m_editStateItem.system = m_editStateItem.local;
	m_editStateItem.plant = m_editStateItem.local;
	m_editStateItem.setValid(true);
	m_editStateItem.value = 0;

	// --
	//
	ui->tableView->setModel(&m_pointsModel);
	ui->tableView->setWordWrap(false);
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

	ui->tableView->horizontalHeader()->setSectionResizeMode(static_cast<int>(TrendPointsModel::Columns::Time), QHeaderView::Stretch);
	ui->tableView->horizontalHeader()->setSectionResizeMode(static_cast<int>(TrendPointsModel::Columns::Value), QHeaderView::Stretch);

	// Combo
	//
	ui->comboTimeType->blockSignals(true);
	ui->comboTimeType->addItem(tr("ServerTime"), static_cast<int>(E::TimeType::Local));
	ui->comboTimeType->addItem(tr("ServerTime +0UTC"), static_cast<int>(E::TimeType::System));
	ui->comboTimeType->addItem(tr("PlantTime"), static_cast<int>(E::TimeType::Plant));

	for (int i = 0; i < ui->comboTimeType->count(); i++)
	{
		if (ui->comboTimeType->itemData(i).toInt() == static_cast<int>(m_timeType))
		{
			ui->comboTimeType->setCurrentIndex(i);
			break;
		}
	}

	ui->comboTimeType->blockSignals(false);

	//
	//
	updatePoints();

	ui->tableView->resizeColumnsToContents();

	if (m_trendMode == E::TrendMode::Realtime || TrendLib::theSettings.m_allowPointsEditing == false)
	{
		ui->buttonAdd->setVisible(false);
		ui->buttonEdit->setVisible(false);
		ui->buttonRemove->setVisible(false);
	}
}

DialogTrendSignalPoints::~DialogTrendSignalPoints()
{
	delete ui;
}

void DialogTrendSignalPoints::updatePoints()
{
	std::list<std::shared_ptr<TrendLib::OneHourData>> signalData;

	m_trendSignalSet->getFullExistingTrendData(m_trendSignal.appSignalId(), m_timeType, &signalData);

	m_pointsModel.setSignalData(signalData, m_trendSignal, m_timeType);
}


void DialogTrendSignalPoints::on_buttonAdd_clicked()
{
	bool emptyModel = m_pointsModel.rowCount() == 0;

	std::vector<TrendLib::TrendStateItem> stateItems;
	stateItems.push_back(m_editStateItem);

	DialogTrendSignalPoint d(&stateItems, m_timeType, m_trendSignal, this);

	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	// Add new state item
	//
	m_editStateItem = stateItems[0];

	m_trendSignalSet->addTrendPoint(m_trendSignal.appSignalId(), m_timeType, m_editStateItem);

	updatePoints();

	if (emptyModel == true)
	{
		ui->tableView->resizeColumnsToContents();
	}

	// Select added item
	//
	int addedIndex = m_pointsModel.stateItemIndex(m_editStateItem);
	if (addedIndex != -1)
	{
		ui->tableView->selectRow(addedIndex);
		ui->tableView->scrollTo(m_pointsModel.index(addedIndex, 0));
	}

	// --
	//
	emit signalPointsChanged();

	return;
}

void DialogTrendSignalPoints::on_buttonEdit_clicked()
{
	QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
	if (indexes.empty() == true)
	{
		return;
	}

	std::vector<TrendLib::TrendStateItem> stateItems;
	std::vector<int> selectedRows;

	for (QModelIndex& mi : indexes)
	{
		if (mi.isValid() == false)
		{
			Q_ASSERT(false);
			return;
		}

		selectedRows.push_back(mi.row());

		int oneHourIndex = 0;
		int recordIndex = 0;
		int stateIndex = 0;

		bool ok = false;

		TrendLib::TrendStateItem item = m_pointsModel.stateItemByIndex(mi.row(), &oneHourIndex, &recordIndex, &stateIndex, &ok);
		if (ok == false)
		{
			Q_ASSERT(ok);
			return;
		}

		stateItems.push_back(item);
	}

	DialogTrendSignalPoint d(&stateItems, m_timeType, m_trendSignal, this);

	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	// Delete old points
	//
	std::sort(selectedRows.begin(), selectedRows.end(), std::greater<int>());

	for (int row : selectedRows)
	{
		if (m_trendSignalSet->removeTrendPoint(m_trendSignal.appSignalId(), row, m_timeType) == false)
		{
			Q_ASSERT(false);
			return;
		}
	}

	// Add new points
	//
	for (TrendLib::TrendStateItem stateItem : stateItems)
	{
		m_trendSignalSet->addTrendPoint(m_trendSignal.appSignalId(), m_timeType, stateItem);
	}

	if (stateItems.size() == 1)
	{
		m_editStateItem = stateItems[0];
	}

	//

	updatePoints();

	emit signalPointsChanged();

	return;
}

void DialogTrendSignalPoints::on_buttonRemove_clicked()
{
	QModelIndexList indexes = ui->tableView->selectionModel()->selectedRows();
	if (indexes.empty() == true)
	{
		return;
	}

	std::vector<int> selectedRows;

	for (QModelIndex& mi : indexes)
	{
		if (mi.isValid() == false)
		{
			Q_ASSERT(false);
			return;
		}

		selectedRows.push_back(mi.row());
	}

	if (selectedRows.empty() == true)
	{
		Q_ASSERT(false);
		return;
	}

	std::sort(selectedRows.begin(), selectedRows.end(), std::greater<int>());

	for (int row : selectedRows)
	{
		if (m_trendSignalSet->removeTrendPoint(m_trendSignal.appSignalId(), row, m_timeType) == false)
		{
			Q_ASSERT(false);
			return;
		}
	}

	updatePoints();

	if (m_pointsModel.rowCount() != 0)
	{
		int selectedRow = selectedRows[selectedRows.size() - 1];
		if (selectedRow > 0)
		{
			selectedRow--;
		}

		ui->tableView->selectRow(selectedRow);
	}

	emit signalPointsChanged();

	return;
}

void DialogTrendSignalPoints::on_comboTimeType_currentIndexChanged(int index)
{
	m_timeType = static_cast<E::TimeType>(ui->comboTimeType->itemData(index).toInt());

	updatePoints();

	return;
}

void DialogTrendSignalPoints::on_tableView_doubleClicked(const QModelIndex &index)
{
	Q_UNUSED(index);
	on_buttonEdit_clicked();
}
