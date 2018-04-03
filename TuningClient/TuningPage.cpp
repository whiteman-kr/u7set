#include "Settings.h"
#include "MainWindow.h"
#include "TuningPage.h"
#include <QKeyEvent>
#include <QPushButton>
#include "../VFrame30/DrawParam.h"
#include "DialogSignalInfo.h"

using namespace std;

//
// TuningItemModelMain
//


TuningModelClient::TuningModelClient(TuningSignalManager* tuningSignalManager, QWidget* parent):
	TuningModel(tuningSignalManager, parent)
{
}

void TuningModelClient::blink()
{
	m_blink = !m_blink;
}

bool TuningModelClient::hasPendingChanges()
{
	for (int row = 0; row < static_cast<int>(m_hashes.size()); row++)
	{
		Hash hash = m_hashes[row];

		bool ok = false;

		const TuningSignalState tss = m_tuningSignalManager->state(hash, &ok);

		if (tss.userModified() == true)
		{
			return true;
		}
	}

	return false;
}

QBrush TuningModelClient::backColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_hashes.size())
	{
		assert(false);
		return QBrush();
	}

	Hash hash = m_hashes[row];

	bool ok = false;

	if (displayIndex == static_cast<int>(Columns::Value))
	{
		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.controlIsEnabled() == false)
		{
			QColor color = QColor(Qt::gray);
			return QBrush(color);
		}

		if (state.valid() == false)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}

		if (m_blink == true && state.userModified() == true)
		{
			QColor color = QColor(Qt::yellow);
			return QBrush(color);
		}

		TuningValue tvDefault(defaultValue(asp));
		tvDefault.setType(asp.toTuningType());

		if (tvDefault != state.value())
		{
			QColor color = QColor(Qt::gray);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Valid))
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::LowLimit) || displayIndex == static_cast<int>(Columns::HighLimit))
	{
		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.limitsUnbalance(asp) == true)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::OutOfRange))
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.outOfRange() == true)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Default))
	{
		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningValue defaultVal = defaultValue(asp);

		if (defaultVal < asp.tuningLowBound() || defaultVal > asp.tuningHighBound())
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}

		QColor color = QColor(Qt::gray);
		return QBrush(color);
	}

	return QBrush();
}

QBrush TuningModelClient::foregroundColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_hashes.size())
	{
		assert(false);
		return QBrush();
	}

	Hash hash = m_hashes[row];

	bool ok = false;

	if (displayIndex == static_cast<int>(Columns::Valid))
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Value))
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.controlIsEnabled() == false)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}

		if (state.valid() == false)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}

		if (m_blink == true && state.userModified() == true)
		{
			QColor color = QColor(Qt::black);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::LowLimit) || displayIndex == static_cast<int>(Columns::HighLimit))
	{
		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.limitsUnbalance(asp) == true)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::OutOfRange))
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.outOfRange() == true)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == static_cast<int>(Columns::Default))
	{
		//int displayIndex = m_columnsIndexes[col];
		QColor color = QColor(Qt::white);
		return QBrush(color);
	}

	return QBrush();

}

Qt::ItemFlags TuningModelClient::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = TuningModel::flags(index);

	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_hashes.size())
	{
		assert(false);
		return f;
	}

	Hash hash = m_hashes[row];

	bool ok = false;

	if (displayIndex == static_cast<int>(Columns::Value))
	{
		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == true)
		{
			if (asp.isAnalog() == false)
			{
				f |= Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
			}
			else
			{
				f |= Qt::ItemIsEnabled | Qt::ItemIsEditable;
			}
		}

	}

	return f;
}

QVariant TuningModelClient::data(const QModelIndex& index, int role) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_hashes.size())
	{
		assert(false);
		return QVariant();
	}

	Hash hash = m_hashes[row];

	bool ok = false;

	AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

	TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

	if (role == Qt::CheckStateRole && displayIndex == static_cast<int>(Columns::Value) && asp.isDiscrete() == true && state.valid() == true)
	{
		return (state.modifiedValue().discreteValue() == 0 ? Qt::Unchecked : Qt::Checked);
	}

	return TuningModel::data(index, role);
}

bool TuningModelClient::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if (!index.isValid())
	{
		return false;
	}

	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_hashes.size())
	{
		assert(false);
		return false;
	}

	Hash hash = m_hashes[row];

	bool ok = false;

	AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

	TuningSignalState state = m_tuningSignalManager->state(hash, &ok);



	if (role == Qt::EditRole && displayIndex == static_cast<int>(TuningModel::Columns::Value))
	{
		bool ok = false;
		double v = value.toDouble(&ok);
		if (ok == false)
		{
			return false;
		}

		m_tuningSignalManager->setNewValue(asp.hash(), TuningValue(asp.toTuningType(), v));
		return true;
	}

	if (role == Qt::CheckStateRole && displayIndex == static_cast<int>(TuningModel::Columns::Value))
	{
		if ((Qt::CheckState)value.toInt() == Qt::Checked)
		{
			m_tuningSignalManager->setNewValue(asp.hash(), TuningValue(asp.toTuningType(), 1));
			return true;
		}
		else
		{
			m_tuningSignalManager->setNewValue(asp.hash(), TuningValue(asp.toTuningType(), 0));
			return true;
		}
	}
	return false;
}


//
// TuningTableView
//

bool TuningTableView::editorActive()
{
	return m_editorActive;
}

bool TuningTableView::edit(const QModelIndex&  index, EditTrigger trigger, QEvent* event)
{

	if (trigger == QAbstractItemView::EditKeyPressed)
	{
		TuningModel* m_model = dynamic_cast<TuningModel*>(model());
		if (m_model == nullptr)
		{
			assert(m_model);
			return false;
		}

		int row = index.row();

		Hash hash = m_model->hashByIndex(row);

		bool ok = false;

		if (row >= 0)
		{
			AppSignalParam asp = m_model->tuningSignalManager()->signalParam(hash, &ok);

			if (asp.isAnalog() == true)
			{
				m_editorActive = true;
			}
		}
	}

	return QTableView::edit(index, trigger, event);
}

void TuningTableView::closeEditor(QWidget* editor, QAbstractItemDelegate::EndEditHint hint)
{

	//qDebug() << "closeEditor";

	m_editorActive = false;

	QTableView::closeEditor(editor, hint);
}

//
// TuningPage
//

int TuningPage::m_instanceCounter = 0;

TuningPage::TuningPage(std::shared_ptr<TuningFilter> treeFilter, std::shared_ptr<TuningFilter> pageFilter, TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, QWidget* parent) :
	QWidget(parent),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient),
	m_treeFilter(treeFilter),
	m_pageFilter(pageFilter)
{

	//qDebug() << "TuningPage::TuningPage m_instanceCounter = " << m_instanceCounter;
	m_instanceNo = m_instanceCounter;
	m_instanceCounter++;

	//assert(m_treeFilter);		They can be nullptr
	//assert(m_buttonFilter);

	assert(m_tuningSignalManager);

	// Object List
	//
	m_objectList = new TuningTableView();

	QFont f = m_objectList->font();

	// Models and data
	//
	m_model = new TuningModelClient(m_tuningSignalManager, this);
	m_model->setFont(f.family(), f.pointSize(), false);
	m_model->setImportantFont(f.family(), f.pointSize(), true);

	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::CustomAppSignalID, 0.22));
	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::EquipmentID, 0.2));
	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::Caption, 0.15));
	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::Units, 0.05));
	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::Type, 0.1));

	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::Value, 0.07));
	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::LowLimit, 0.07));
	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::HighLimit, 0.07));
	m_columnsArray.push_back(std::make_pair(TuningModel::Columns::Default, 0.07));

	for (auto c : m_columnsArray)
	{
		m_model->addColumn(c.first);
	}

	// Filter controls
	//
	m_filterTypeCombo = new QComboBox();
	m_filterTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterType::All));
	m_filterTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterType::AppSignalID));
	m_filterTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterType::CustomAppSignalID));
	m_filterTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterType::EquipmentID));
	m_filterTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterType::Caption));

	connect(m_filterTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_FilterTypeIndexChanged(int)));

	m_filterTypeCombo->setCurrentIndex(0);

	m_filterEdit = new QLineEdit();
	connect(m_filterEdit, &QLineEdit::returnPressed, this, &TuningPage::slot_ApplyFilter);

	m_filterButton = new QPushButton(tr("Filter"));
	connect(m_filterButton, &QPushButton::clicked, this, &TuningPage::slot_ApplyFilter);

	// Button controls
	//

	m_setValueButton = new QPushButton(tr("Set Value"));
	connect(m_setValueButton, &QPushButton::clicked, this, &TuningPage::slot_setValue);

	m_setAllButton = new QPushButton(tr("Set All"));
	connect(m_setAllButton, &QPushButton::clicked, this, &TuningPage::slot_setAll);

	m_writeButton = new QPushButton(tr("Write"));
	connect(m_writeButton, &QPushButton::clicked, this, &TuningPage::slot_Write);

	m_undoButton = new QPushButton(tr("Undo"));
	connect(m_undoButton, &QPushButton::clicked, this, &TuningPage::slot_undo);


	m_bottomLayout = new QHBoxLayout();

	m_bottomLayout->addWidget(m_filterTypeCombo);
	m_bottomLayout->addWidget(m_filterEdit);
	m_bottomLayout->addWidget(m_filterButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_setValueButton);
	m_bottomLayout->addWidget(m_setAllButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_writeButton);
	m_bottomLayout->addWidget(m_undoButton);

	if (theConfigSettings.autoApply == false)
	{
		m_applyButton = new QPushButton(tr("Apply"));
		connect(m_applyButton, &QPushButton::clicked, this, &TuningPage::slot_Apply);
		m_bottomLayout->addWidget(m_applyButton);
	}

	m_mainLayout = new QVBoxLayout(this);

	m_mainLayout->addWidget(m_objectList);
	m_mainLayout->addLayout(m_bottomLayout);


	m_objectList->setModel(m_model);
	m_objectList->verticalHeader()->hide();
	m_objectList->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_objectList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_objectList->setSortingEnabled(true);
	m_objectList->setEditTriggers(QAbstractItemView::EditKeyPressed);

	m_objectList->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_objectList, &QWidget::customContextMenuRequested, this, &TuningPage::slot_listContextMenuRequested);

	connect(m_objectList->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &TuningPage::sortIndicatorChanged);

	connect(m_objectList, &QTableView::doubleClicked, this, &TuningPage::slot_tableDoubleClicked);

	m_objectList->installEventFilter(this);


	fillObjectsList();

	// Color

	if (m_pageFilter->isTab() == true && m_pageFilter->backColor().isValid() && m_pageFilter->textColor().isValid() && m_pageFilter->backColor() != m_pageFilter->textColor())
	{
		QPalette Pal(palette());

		Pal.setColor(QPalette::Background, m_pageFilter->backColor());
		setAutoFillBackground(true);
		setPalette(Pal);
	}
	else
	{
		m_mainLayout->setContentsMargins(0, 0, 0, 0);
	}

	connect(theMainWindow, &MainWindow::timerTick500, this, &TuningPage::slot_timerTick500);

}

TuningPage::~TuningPage()
{
	m_instanceCounter--;
	//qDebug() << "TuningPage::TuningPage m_instanceCounter = " << m_instanceCounter;
}

void TuningPage::fillObjectsList()
{
	std::vector<Hash> hashes;

	std::vector<Hash> pageHashes;

	//qDebug() << "FillObjectsList";

	//if (m_pageFilter != nullptr)
	//{
//		qDebug() << "Button " << m_pageFilter->caption();
//	}

	if (m_pageFilter != nullptr)
	{
		pageHashes = m_pageFilter->signalsHashes();
	}

	//qDebug() << "pageHashes.size() = " << pageHashes.size();

	// Tree Filter

	std::vector<Hash> treeHashes;

	if (m_treeFilter != nullptr && m_treeFilter->isRoot() == false)
	{
		if (m_treeFilter->isEmpty() == false)
		{
			treeHashes = m_treeFilter->signalsHashes();

			std::sort(treeHashes.begin(), treeHashes.end());

			//qDebug() << "treehashes.size() = " << treeHashes.size();

			hashes.reserve(pageHashes.size());

			// Place in hashes array only items that are found in treeHashes

			for (Hash pageHash : pageHashes)
			{
				if (std::binary_search(treeHashes.begin(), treeHashes.end(), pageHash) == true)
				{
					hashes.push_back(pageHash);
				}
			}
		}
	}
	else
	{
		hashes = pageHashes;
	}

	//qDebug() << "hashes.size() = " << hashes.size();

	//

	std::vector<Hash> filteredHashes;
	filteredHashes.reserve(hashes.size());

	std::vector<std::pair<Hash, TuningValue>> defaultValues;

	QString filter = m_filterEdit->text();

	FilterType filterType = FilterType::All;
	QVariant data = m_filterTypeCombo->currentData();
	if (data.isValid() == true)
	{
		filterType = static_cast<FilterType>(data.toInt());
	}

	bool ok = false;

	for (Hash hash : hashes)
	{
		const AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		if (ok == false)
		{
			assert(ok);
			continue;
		}

		bool modifyDefaultValue = false;
		TuningValue modifiedDefaultValue;

		if (treeHashes.empty() == false)
		{
			// Modify the default value from selected tree filter
			//

			TuningFilterValue filterValue;

			bool hasValue = m_treeFilter->value(hash, filterValue);

			if (hasValue == true)
			{
				modifyDefaultValue = true;
				modifiedDefaultValue = filterValue.value();
			}
		}
		else
		{
			if (m_pageFilter != nullptr)
			{
				// Modify the default value from page filter
				//

				TuningFilterValue filterValue;

				bool hasValue = m_pageFilter->value(hash, filterValue);

				if (hasValue == true)
				{
					modifyDefaultValue = true;
					modifiedDefaultValue = filterValue.value();
				}
			}
		}

		// Text filter
		//

		if (filter.length() != 0)
		{
			bool filterMatch = false;

			switch (filterType)
			{
			case FilterType::All:
				if (asp.appSignalId().contains(filter, Qt::CaseInsensitive) == true
						|| asp.customSignalId().contains(filter, Qt::CaseInsensitive) == true
						|| asp.equipmentId().contains(filter, Qt::CaseInsensitive) == true
						|| asp.caption().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::AppSignalID:
				if (asp.appSignalId().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::CustomAppSignalID:
				if (asp.customSignalId().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::EquipmentID:
				if (asp.equipmentId().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterType::Caption:
				if (asp.caption().contains(filter, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			}

			if (filterMatch == false)
			{
				continue;
			}
		}


		filteredHashes.push_back(hash);

		if (modifyDefaultValue == true)
		{
			defaultValues.push_back(std::make_pair(hash, modifiedDefaultValue));
		}
	}

	m_model->setHashes(filteredHashes);

	m_model->setDefaultValues(defaultValues);

	m_objectList->sortByColumn(m_sortColumn, m_sortOrder);
}

bool TuningPage::hasPendingChanges()
{
	return m_model->hasPendingChanges();
}

bool TuningPage::askForSavePendingChanges()
{
	bool hasPendingChanges = m_model->hasPendingChanges();

	if (hasPendingChanges == false)
	{
		return true;
	}

	int result = QMessageBox::warning(this, qAppName(), tr("Warning! Some values were modified but not written. Please select the following:"), tr("Write"), tr("Undo"), tr("Cancel"));

	if (result == 0)
	{
		if (write() == false)
		{
			return false;
		}
		return true;
	}

	if (result == 1)
	{
		undo();
		return true;
	}

	return false;
}

bool TuningPage::write()
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return false;
	}

	if (takeClientControl() == false)
	{
		return false;
	}

	QString str = tr("New values will be written:") + QString("\r\n\r\n");
	QString strValue;

	bool modifiedFound = false;
	int modifiedCount = 0;

	std::vector<Hash> hashes = m_model->hashes();

	bool ok = false;

	for (Hash hash : hashes)
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.userModified() == false)
		{
			continue;
		}

		if (state.controlIsEnabled() == false)
		{
			continue;
		}

		modifiedFound = true;
		modifiedCount++;
	}

	if (modifiedFound == false)
	{
		return false;
	}

	int listCount = 0;

	for (Hash hash : hashes)
	{
		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.userModified() == false)
		{
			continue;
		}

		if (state.controlIsEnabled() == false)
		{
			continue;
		}

		if (listCount >= 10)
		{
			str += tr("and %1 more values.").arg(modifiedCount - listCount);
			break;
		}

		if (asp.isAnalog() == true)
		{
			strValue = state.modifiedValue().toString(asp.precision());
		}
		else
		{
			strValue = state.modifiedValue().toString();
		}

		str += tr("%1 (%2) = %3\r\n").arg(asp.appSignalId()).arg(asp.caption()).arg(strValue);

		listCount++;
	}

	str += QString("\r\n") + tr("Are you sure you want to continue?");

	if (QMessageBox::warning(this, tr("Write Changes"),
							 str,
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return false;
	}

	std::vector<TuningWriteCommand> commands;

	for (Hash hash : hashes)
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.userModified() == false)
		{
			continue;
		}

		if (state.controlIsEnabled() == false)
		{
			continue;
		}

		TuningWriteCommand cmd(hash, state.modifiedValue());

		commands.push_back(cmd);
	}

	m_tuningTcpClient->writeTuningSignal(commands);

	return true;
}

bool TuningPage::apply()
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return false;
	}

	if (takeClientControl() == false)
	{
		return false;
	}

	if (QMessageBox::warning(this, qAppName(),
							 tr("Are you sure you want apply the changes?"),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return false;
	}

	// Get SOR counters

	bool sorActive = false;
	bool sorValid = false;

	int totalSorCount = m_tuningTcpClient->sourceSorCount(&sorActive, &sorValid);

	if (totalSorCount > 0)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!!!\r\n\r\nSOR Signal(s) are set in logic modules!\r\n\r\nIf you apply these changes, module can run into RUN SAFE STATE.\r\n\r\nAre you sure you STILL WANT TO APPLY the changes?"),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return false;
		}
	}

	m_tuningTcpClient->applyTuningSignals();

	return true;
}

void TuningPage::undo()
{
	slot_undo();
}

void TuningPage::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_model->sort(column, order);
}

void TuningPage::slot_setValue()
{
	QModelIndexList selection = m_objectList->selectionModel()->selectedRows();

	std::vector<int> selectedRows;

	for (const QModelIndex i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == true)
	{
		return;
	}

	bool first = true;
	TuningValue value;
	TuningValue defaultValue;
	bool sameValue = true;
	int precision = 0;
	TuningValue lowLimit;
	TuningValue highLimit;

	for (int i : selectedRows)
	{
		Hash hash = m_model->hashByIndex(i);

		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false)
		{
			return;
		}

		if (asp.isAnalog() == true)
		{
			if (state.limitsUnbalance(asp) == true)
			{
				QMessageBox::warning(this, tr("Set Value"), tr("There is limits mismatch in signal '%1'. Value setting is disabled.").arg(asp.customSignalId()));
				return;
			}
		}

		if (asp.isAnalog() == true)
		{
			if (asp.precision() > precision)
			{
				precision = asp.precision();
			}
		}

		if (first == true)
		{
			value = state.value();
			defaultValue = m_model->defaultValue(asp);
			lowLimit = asp.tuningLowBound();
			highLimit = asp.tuningHighBound();
			first = false;
		}
		else
		{
			if (asp.toTuningType() != value.type())
			{
				QMessageBox::warning(this, tr("Set Value"), tr("Please select objects of the same type."));
				return;
			}

			if (asp.isAnalog() == true)
			{
				if (lowLimit != asp.tuningLowBound() || highLimit != asp.tuningHighBound())
				{
					QMessageBox::warning(this, tr("Set Value"), tr("Selected objects have different input range."));
					return;
				}
			}

			if (defaultValue != m_model->defaultValue(asp))
			{
				QMessageBox::warning(this, tr("Set Value"), tr("Selected objects have different default values."));
				return;
			}

			if (value != state.value())
			{
				sameValue = false;
			}
		}
	}

	DialogInputTuningValue d(value, defaultValue, sameValue, lowLimit, highLimit, precision, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	TuningValue newValue = d.value();

	for (int i : selectedRows)
	{
		Hash hash = m_model->hashByIndex(i);

		m_tuningSignalManager->setNewValue(hash, newValue);
	}
}

void TuningPage::slot_tableDoubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);
	slot_setValue();
}

void TuningPage::slot_FilterTypeIndexChanged(int index)
{
	Q_UNUSED(index);
	fillObjectsList();
}

void TuningPage::slot_listContextMenuRequested(const QPoint& pos)
{
	QModelIndex index =	m_objectList->indexAt(pos);
	if (index.isValid() == false)
	{
		return;
	}

	Hash hash = m_model->hashByIndex(index.row());
	if (hash == UNDEFINED_HASH)
	{
		assert(false);
		return;
	}

	QMenu menu(this);

	bool found = false;

	AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &found);

	if (found == false)
	{
		assert(false);
		return;
	}

	QAction* a = new QAction(tr("%1 - %2").arg(asp.customSignalId()).arg(asp.caption()), &menu);

	auto f = [this, hash]() -> void
	{
		DialogSignalInfo* d = new DialogSignalInfo(hash, m_tuningTcpClient->instanceIdHash(), m_tuningSignalManager, this);
		d->show();
	};

	connect(a, &QAction::triggered, this, f);

	menu.addAction(a);

	menu.exec(QCursor::pos());

}

void TuningPage::slot_ApplyFilter()
{
	fillObjectsList();
}

void TuningPage::slot_treeFilterSelectionChanged(std::shared_ptr<TuningFilter> filter)
{
	m_treeFilter = filter;

	fillObjectsList();
}

void TuningPage::slot_pageFilterChanged(std::shared_ptr<TuningFilter> filter)
{
	m_pageFilter = filter;

	fillObjectsList();
}

bool TuningPage::eventFilter(QObject* object, QEvent* event)
{
	if (object == m_objectList && event->type()==QEvent::KeyPress)
	{
		QKeyEvent* pKeyEvent = static_cast<QKeyEvent*>(event);
		if(pKeyEvent->key() == Qt::Key_Return)
		{
			if (m_objectList->editorActive() == false)
			{
				slot_Write();
				return true;
			}
			return true;
		}

		if(pKeyEvent->key() == Qt::Key_Space)
		{
			invertValue();
			return true;
		}
	}

	return QWidget::eventFilter(object, event);
}

void TuningPage::resizeEvent(QResizeEvent *event)
{

	Q_UNUSED(event);

	if (m_objectList == nullptr)
	{
		return;
	}

	QSize s = m_objectList->size();

	double totalWidth = s.width() - 25;

	for (int c = 0; c < m_columnsArray.size(); c++)
	{
		auto columnInfo = m_columnsArray[c];

		int columnWidth = totalWidth * columnInfo.second;

		if (columnWidth >= s.width())
		{
			columnWidth = 100;
		}

		m_objectList->setColumnWidth(c, columnWidth);
	}

}

void TuningPage::invertValue()
{
	QModelIndexList selection = m_objectList->selectionModel()->selectedRows();

	std::vector<int> selectedRows;

	for (const QModelIndex i : selection)
	{
		selectedRows.push_back(i.row());
	}

	if (selectedRows.empty() == true)
	{
		return;
	}

	bool ok = false;

	for (int i : selectedRows)
	{
		Hash hash = m_model->hashByIndex(i);

		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false)
		{
			return;
		}

		if (asp.isDiscrete() == true)
		{
			TuningValue tv;

			tv.setType(TuningValueType::Discrete);

			tv.setDiscreteValue(0);

			if (state.modifiedValue().discreteValue() == 0)
			{
				tv.setDiscreteValue(1);
			}

			m_tuningSignalManager->setNewValue(hash, tv);
		}
	}
}

bool TuningPage::takeClientControl()
{
#ifdef Q_DEBUG
	if (theSettings.m_simulationMode == false)
#endif
	{
		if (m_tuningTcpClient->activeTuningSourceCount() == 0)
		{
			QMessageBox::critical(this, qAppName(),	 tr("No tuning sources with control enabled found."));

			return false;
		}
	}

	if (m_tuningTcpClient->singleLmControlMode() == true && m_tuningTcpClient->clientIsActive() == false)
	{
		QString equipmentId = m_tuningTcpClient->singleActiveTuningSource();

		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!\r\n\r\nCurrent client is not selected as active now.\r\n\r\nAre you sure you want to take control and activate the source %1?").arg(equipmentId),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return false;
		}

		m_tuningTcpClient->activateTuningSourceControl(equipmentId, true, true);
	}

	return true;
}

void TuningPage::slot_timerTick500()
{
	if  (isVisible() == true && m_model->rowCount() > 0)
	{

		//qDebug() << m_instanceNo;

		m_model->blink();

		// Update only visible dynamic items
		//
		int from = m_objectList->rowAt(0);
		int to = m_objectList->rowAt(m_objectList->height() - m_objectList->horizontalHeader()->height());

		if (from == -1)
		{
			from = 0;
		}

		if (to == -1)
		{
			to = m_model->rowCount() - 1;
		}

		// Redraw visible table items
		//
		for (int row = from; row <= to; row++)
		{
			//if (state->needRedraw() == true || state->userModified() == true)
			//{
			for (int col = 0; col < m_model->columnCount(); col++)
			{
				int displayIndex = m_model->columnIndex(col);

				if (displayIndex >= static_cast<int>(TuningModel::Columns::Value))
				{
					//QString str = QString("%1:%2").arg(row).arg(col);
					//qDebug() << str;

					m_objectList->update(m_model->index(row, col));
				}
			}
			//}
		}
	}

}

void TuningPage::slot_setAll()
{
	QMenu menu(this);


	// Check all signals to have correct limits
	{
		std::vector<Hash> hashes = m_model->hashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false)
			{
				continue;
			}

			if (asp.isAnalog() == true)
			{
				if (state.limitsUnbalance(asp) == true)
				{
					QMessageBox::warning(this, tr("Set All"), tr("There is limits mismatch in signal '%1'. Operation is disabled.").arg(asp.customSignalId()));
					return;
				}
			}
		}
	}

	// Set All To On
	QAction* actionAllToOn = new QAction(tr("Set All Discretes To On"), &menu);

	auto fAllToOn = [this]() -> void
	{
		std::vector<Hash> hashes = m_model->hashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false)
			{
				continue;
			}

			if (asp.isDiscrete() == true)
			{
				TuningValue tv;
				tv.setType(TuningValueType::Discrete);
				tv.setDiscreteValue(1);
				m_tuningSignalManager->setNewValue(hash, tv);
			}
		}
	};
	connect(actionAllToOn, &QAction::triggered, this, fAllToOn);

	// Set All To Onff
	QAction* actionAllToOff = new QAction(tr("Set All Discretes To Off"), &menu);

	auto fAllToOff = [this]() -> void
	{
		std::vector<Hash> hashes = m_model->hashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false)
			{
				continue;
			}

			if (asp.isDiscrete() == true)
			{
				TuningValue tv;
				tv.setType(TuningValueType::Discrete);
				tv.setDiscreteValue(0);
				m_tuningSignalManager->setNewValue(hash, tv);
			}
		}
	};

	connect(actionAllToOff, &QAction::triggered, this, fAllToOff);

	// Set All To Defaults
	QAction* actionAllToDefault = new QAction(tr("Set All To Defaults"), &menu);

	auto fAllToDefault = [this]() -> void
	{
		std::vector<Hash> hashes = m_model->hashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false)
			{
				continue;
			}

			TuningValue tvDefault = m_model->defaultValue(asp);

			if (tvDefault != state.value() && ok == true)
			{
				if(tvDefault < asp.tuningLowBound() || tvDefault > asp.tuningHighBound())
				{
					QString message = tr("Invalid default value '%1' in signal %2 [%3]").arg(tvDefault.toString(asp.precision())).arg(asp.appSignalId()).arg(asp.caption());
					QMessageBox::critical(this, qAppName(), message);
				}
				else
				{
					m_tuningSignalManager->setNewValue(hash, tvDefault);
				}
			}
		}
	};

	connect(actionAllToDefault, &QAction::triggered, this, fAllToDefault);

	// Run the menu

	menu.addAction(actionAllToOn);
	menu.addAction(actionAllToOff);
	menu.addSeparator();
	menu.addAction(actionAllToDefault);

	menu.exec(QCursor::pos());
}


void TuningPage::slot_undo()
{
	std::vector<Hash> hashes = m_model->hashes();

	bool ok = false;

	for (Hash hash : hashes)
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false)
		{
			continue;
		}

		m_tuningSignalManager->setNewValue(hash, state.value());
	}
}

void TuningPage::slot_Write()
{
	write();
}

void TuningPage::slot_Apply()
{
	apply();
}

