#include "Settings.h"
#include "MainWindow.h"
#include "TuningPage.h"
#include <QKeyEvent>
#include <QPushButton>
#include "../VFrame30/DrawParam.h"
#include "TuningSignalInfo.h"
#include "DialogChooseFilter.h"

#include <QTableView>
#include <QInputDialog>


//
// TuningItemModelMain
//
TuningModelClient::TuningModelClient(TuningSignalManager* tuningSignalManager, TuningClientTcpClient* tuningTcpClient, const std::vector<QString>& valueColumnsAppSignalIdSuffixes, QWidget* parent):
	TuningModel(tuningSignalManager, valueColumnsAppSignalIdSuffixes, parent),
	m_tuningTcpClient(tuningTcpClient)
{
}

void TuningModelClient::blink()
{
	m_blink = !m_blink;
}

bool TuningModelClient::hasPendingChanges()
{
	for (int i = 0; i < static_cast<int>(m_allHashes.size()); i++)
	{
		Hash hash = m_allHashes[i];

		if (m_tuningSignalManager->newValueIsUnapplied(hash) == true)
		{
			return true;
		}
	}

	return false;
}

QBrush TuningModelClient::backColor(const QModelIndex& index) const
{
	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return QBrush();
	}

	bool ok = false;

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
		{
			assert(false);
			return QBrush();
		}

		Hash hash = hashByIndex(row, valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return QBrush();
		}

		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.controlIsEnabled() == false)
		{
			QColor color = theSettings.m_columnDisabledBackColor;
			return QBrush(color);
		}

		if (state.valid() == false)
		{
			QColor color = theSettings.m_columnErrorBackColor;
			return QBrush(color);
		}

		if (m_tuningTcpClient->writingIsEnabled(state) == false)
		{
			QColor color = theSettings.m_columnDisabledBackColor;
			return QBrush(color);
		}

		if (m_blink == true && m_tuningSignalManager->newValueIsUnapplied(hash) == true)
		{
			QColor color = theSettings.m_columnUnappliedBackColor;
			return QBrush(color);
		}

		TuningValue tvDefault(defaultValue(asp));
		tvDefault.setType(asp.tuningType());

		if (tvDefault != state.value())
		{
			QColor color = theSettings.m_columnDefaultMismatchBackColor;
			return QBrush(color);
		}
	}

	QColor color;

	const TuningModelHashSet& hashes = hashSetByIndex(row);

	for (int c = 0; c < MAX_VALUES_COLUMN_COUNT; c++)
	{
		const Hash hash = hashes.hash[c];

		if (hash == UNDEFINED_HASH)
		{
			continue;
		}

		if (columnType == static_cast<int>(TuningModelColumns::Valid))
		{
			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false)
			{
				color = theSettings.m_columnErrorBackColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::LowLimit) || columnType == static_cast<int>(TuningModelColumns::HighLimit))
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.limitsUnbalance(asp) == true)
			{
				color = theSettings.m_columnErrorBackColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::OutOfRange))
		{
			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.outOfRange() == true)
			{
				color = theSettings.m_columnErrorBackColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::Default))
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningValue defaultVal = defaultValue(asp);

			if (defaultVal < asp.tuningLowBound() || defaultVal > asp.tuningHighBound())
			{
				color = theSettings.m_columnErrorBackColor;
				break;
			}
		}
	}

	if (color.isValid() == true)
	{
		return QBrush(color);
	}

	return QBrush();
}

QBrush TuningModelClient::foregroundColor(const QModelIndex& index) const
{
	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return QBrush();
	}

	bool ok = false;

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
		{
			assert(false);
			return QBrush();
		}

		Hash hash = hashByIndex(row, valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return QBrush();
		}

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.controlIsEnabled() == false)
		{
			QColor color = theSettings.m_columnDisabledTextColor;
			return QBrush(color);
		}

		if (state.valid() == false)
		{
			QColor color = theSettings.m_columnErrorTextColor;
			return QBrush(color);
		}

        if (m_tuningTcpClient->writingIsEnabled(state) == false)
        {
			QColor color = theSettings.m_columnDisabledTextColor;
            return QBrush(color);
        }

		if (m_blink == true && m_tuningSignalManager->newValueIsUnapplied(hash) == true)
		{
			QColor color = theSettings.m_columnUnappliedTextColor;
			return QBrush(color);
		}
	}

	QColor color;

	const TuningModelHashSet& hashes = hashSetByIndex(row);

	for (int c = 0; c < MAX_VALUES_COLUMN_COUNT; c++)
	{
		const Hash hash = hashes.hash[c];

		if (hash == UNDEFINED_HASH)
		{
			continue;
		}

		if (columnType == static_cast<int>(TuningModelColumns::Valid))
		{
			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false)
			{
				color = theSettings.m_columnErrorTextColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::LowLimit) || columnType == static_cast<int>(TuningModelColumns::HighLimit))
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.limitsUnbalance(asp) == true)
			{
				color = theSettings.m_columnErrorTextColor;
				break;
			}
		}

		if (columnType == static_cast<int>(TuningModelColumns::OutOfRange))
		{
			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.outOfRange() == true)
			{
				color = theSettings.m_columnErrorTextColor;
				break;
			}
		}
	}

	if (color.isValid() == true)
	{
		return QBrush(color);
	}

	return QBrush();
}

Qt::ItemFlags TuningModelClient::flags(const QModelIndex& index) const
{
	Qt::ItemFlags f = TuningModel::flags(index);

	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return f;
	}

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
		{
			assert(false);
			return f;
		}

		Hash hash = hashByIndex(row, valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return f;
		}

		if (m_tuningSignalManager->newValueIsUnapplied(hash) == true)
		{
			f &= ~Qt::ItemIsSelectable;
		}

		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == true)
		{
			if (asp.isAnalog() == false)
			{
				if (m_tuningTcpClient->writingIsEnabled(state) == false)
				{
					f &= ~Qt::ItemIsEnabled;
				}
				else
				{
					f |= Qt::ItemIsUserCheckable;
				}
			}
			else
			{
				if (m_tuningTcpClient->writingIsEnabled(state) == false)
				{
					f &= ~Qt::ItemIsEnabled;
				}
				else
				{
					f |= Qt::ItemIsEditable;
				}
			}
		}
	}

	return f;
}

QVariant TuningModelClient::data(const QModelIndex& index, int role) const
{
	int col = index.column();
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return QVariant();
	}

	if (role == Qt::CheckStateRole && columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
		{
			assert(false);
			return QVariant();
		}

		Hash hash = hashByIndex(index.row(), valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			return QVariant();
		}

		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (asp.isDiscrete() == true && state.valid() == true)
		{
			quint32 discreteValue = 0;

			if (m_tuningSignalManager->newValueIsUnapplied(hash) == true)
			{
				discreteValue = m_tuningSignalManager->newValue(hash).discreteValue();
			}
			else
			{
				discreteValue = state.value().discreteValue();
			}

			return (discreteValue == 0 ? Qt::Unchecked : Qt::Checked);
		}
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
	int columnType = static_cast<int>(m_columnsTypes[col]);

	int row = index.row();
	if (row >= rowCount())
	{
		assert(false);
		return false;
	}

	if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst) && columnType <= static_cast<int>(TuningModelColumns::ValueLast))
	{
		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
		{
			assert(false);
			return false;
		}

		Hash hash = hashByIndex(row, valueColumn);

		if (hash == UNDEFINED_HASH)
		{
			assert(false);
			return false;
		}

		bool ok = false;

		AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (role == Qt::EditRole &&
				state.valid() == true &&
				state.controlIsEnabled() == true &&
				m_tuningTcpClient->writingIsEnabled(state) == true)
		{
			ok = false;
			double v = value.toDouble(&ok);
			if (ok == false)
			{
				return false;
			}

			m_tuningSignalManager->setNewValue(asp.hash(), TuningValue(asp.tuningType(), v));
			return true;
		}

		if (role == Qt::CheckStateRole &&
				state.valid() == true &&
				state.controlIsEnabled() == true &&
				m_tuningTcpClient->writingIsEnabled(state) == true)
		{
			if ((Qt::CheckState)value.toInt() == Qt::Checked)
			{
				m_tuningSignalManager->setNewValue(asp.hash(), TuningValue(asp.tuningType(), 1));
				return true;
			}
			else
			{
				m_tuningSignalManager->setNewValue(asp.hash(), TuningValue(asp.tuningType(), 0));
				return true;
			}
		}
	}

	return false;
}


//
// TuningTableView
//

TuningTableView::TuningTableView(TuningClientTcpClient* tuningTcpClient):
	QTableView(),
	m_tuningTcpClient(tuningTcpClient)
{

}

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

		int columnType = static_cast<int>(m_model->columnType(index.column()));

		int row = index.row();
		if (row >= m_model->rowCount())
		{
			assert(false);
			return false;
		}

		int valueColumn = columnType - static_cast<int>(TuningModelColumns::ValueFirst);
		if (valueColumn < 0 || valueColumn >= MAX_VALUES_COLUMN_COUNT)
		{
			return false;
		}

		Hash hash = m_model->hashByIndex(row, valueColumn);

		bool ok = false;

		if (row >= 0)
		{
			AppSignalParam asp = m_model->tuningSignalManager()->signalParam(hash, &ok);

			TuningSignalState state = m_model->tuningSignalManager()->state(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
			{
				return false;
			}

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
// TuningPageColumnsWidth
//

TuningPageColumnsWidth::TuningPageColumnsWidth()
{
	m_defaultWidthMap[TuningModelColumns::CustomAppSignalID] = 180;
	m_defaultWidthMap[TuningModelColumns::EquipmentID] = 180;
	m_defaultWidthMap[TuningModelColumns::AppSignalID] = 180;
	m_defaultWidthMap[TuningModelColumns::Caption] = 180;
	m_defaultWidthMap[TuningModelColumns::Units] = 70;
	m_defaultWidthMap[TuningModelColumns::Type] = 70;

	for (int i = 0; i < MAX_VALUES_COLUMN_COUNT; i++)
	{
		int valueColumn = static_cast<int>(TuningModelColumns::ValueFirst) + i;
		m_defaultWidthMap[static_cast<TuningModelColumns>(valueColumn)] = 70;
	}

	m_defaultWidthMap[TuningModelColumns::LowLimit] = 70;
	m_defaultWidthMap[TuningModelColumns::HighLimit] = 70;
	m_defaultWidthMap[TuningModelColumns::Default] = 70;
	m_defaultWidthMap[TuningModelColumns::Valid] = 70;
	m_defaultWidthMap[TuningModelColumns::OutOfRange] = 70;
}

bool TuningPageColumnsWidth::load(const QString& pageId)
{
	m_pageId = pageId;

	m_widthMap.clear();

	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
	QString value = settings.value(QString("PageColumnsWidth/%1").arg(m_pageId)).toString();

	QStringList l = value.split(';', QString::SkipEmptyParts);

	for (const QString& s : l)
	{
		QStringList pairList = s.split('=');
		if (pairList.size() != 2)
		{
			assert(false);
			return false;
		}

		TuningModelColumns column = static_cast<TuningModelColumns>(pairList[0].toInt());
		int width = pairList[1].toInt();

		setWidth(column, width);
	}


	return true;
}

bool TuningPageColumnsWidth::save() const
{
	if (m_pageId.isEmpty() == true)
	{
		assert(false);
		return false;
	}

	QString value;

	for (auto it : m_widthMap)
	{
		TuningModelColumns column = it.first;
		int width = it.second;

		value += QString("%1=%2;").arg(static_cast<int>(column)).arg(width);
	}

	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
	settings.setValue(QString("PageColumnsWidth/%1").arg(m_pageId), value);

	return true;
}

int TuningPageColumnsWidth::width(TuningModelColumns column) const
{
	auto it = m_widthMap.find(column);
	if (it != m_widthMap.end())
	{
		return it->second;
	}

	auto dit = m_defaultWidthMap.find(column);
	if (dit != m_defaultWidthMap.end())
	{
		return dit->second;
	}

	assert(false);
	return 100;

}

void TuningPageColumnsWidth::setWidth(TuningModelColumns column, int width)
{
	m_widthMap[column] = width;
}

//
// TuningPage
//

int TuningPage::m_instanceCounter = 0;

TuningPage::TuningPage(std::shared_ptr<TuningFilter> treeFilter,
					   std::shared_ptr<TuningFilter> pageFilter,
					   TuningSignalManager* tuningSignalManager,
					   TuningClientTcpClient* tuningTcpClient, TuningFilterStorage* tuningFilterStorage,
					   QWidget* parent) :
	QWidget(parent),
	m_tuningSignalManager(tuningSignalManager),
	m_tuningTcpClient(tuningTcpClient),
	m_tuningFilterStorage(tuningFilterStorage),
	m_treeFilter(treeFilter),
	m_pageFilter(pageFilter)
{

	//qDebug() << "TuningPage::TuningPage m_instanceCounter = " << m_instanceCounter;

	m_instanceNo = m_instanceCounter;
	m_instanceCounter++;

	//assert(m_treeFilter);		This can be nullptr

	if (m_pageFilter == nullptr)
	{
		Q_ASSERT(m_pageFilter);
		return;
	}

	assert(m_tuningSignalManager);
	assert(m_tuningFilterStorage);

	// Object List
	//
	m_objectList = new TuningTableView(m_tuningTcpClient);
    m_objectList->setWordWrap(false);


	// Models and data
	//

	// Get the tab filter (if page filter is button, take its parent, if no tab exists - create an empty temporary)

	TuningFilter* tabFilter = m_pageFilter.get();

	if (pageFilter->isButton() == true)
	{
		TuningFilter* parentFilter = m_pageFilter->parentFilter();

		if (parentFilter != nullptr && parentFilter->isTab() == true)
		{
			tabFilter = parentFilter;
		}
	}

	if (tabFilter == nullptr)
	{
		assert(false);	// page filter must exist !
		return;
	}

	std::vector<QString> valueColumnsAppSignalIdSuffixes = tabFilter->valueColumnsAppSignalIdSuffixes();

	m_model = new TuningModelClient(m_tuningSignalManager, m_tuningTcpClient, valueColumnsAppSignalIdSuffixes, this);

	QFont f = m_objectList->font();

	f.setBold(false);
	m_model->setFont(f);

	f.setBold(true);
	m_model->setImportantFont(f);

	if (tabFilter->columnCustomAppId() == true)
	{
		m_model->addColumn(TuningModelColumns::CustomAppSignalID);
	}
	if (tabFilter->columnAppId() == true)
	{
		m_model->addColumn(TuningModelColumns::AppSignalID);
	}
	if (tabFilter->columnEquipmentId() == true)
	{
		m_model->addColumn(TuningModelColumns::EquipmentID);
	}
	if (tabFilter->columnCaption() == true)
	{
		m_model->addColumn(TuningModelColumns::Caption);
	}
	if (tabFilter->columnUnits() == true)
	{
		m_model->addColumn(TuningModelColumns::Units);
	}
	if (tabFilter->columnType() == true)
	{
		m_model->addColumn(TuningModelColumns::Type);
	}

	int valuesColumnsCount = m_model->valueColumnsCount();
	for (int c = 0; c < valuesColumnsCount; c++)
	{
		int valueColumn = static_cast<int>(TuningModelColumns::ValueFirst) + c;
		m_model->addColumn(static_cast<TuningModelColumns>(valueColumn));
	}

	if (tabFilter->columnLimits() == true)
	{
		m_model->addColumn(TuningModelColumns::LowLimit);
		m_model->addColumn(TuningModelColumns::HighLimit);
	}
	if (tabFilter->columnDefault() == true)
	{
		m_model->addColumn(TuningModelColumns::Default);
	}
	if (tabFilter->columnValid() == true)
	{
		m_model->addColumn(TuningModelColumns::Valid);
	}
	if (tabFilter->columnOutOfRange() == true)
	{
		m_model->addColumn(TuningModelColumns::OutOfRange);
	}

	m_bottomLayout = new QHBoxLayout();


	// Filter controls
	//
	m_filterTypeCombo = new QComboBox();
	m_filterTypeCombo->addItem(tr("All Text"), static_cast<int>(FilterIDType::All));
	m_filterTypeCombo->addItem(tr("AppSignalID"), static_cast<int>(FilterIDType::AppSignalID));
	m_filterTypeCombo->addItem(tr("CustomAppSignalID"), static_cast<int>(FilterIDType::CustomAppSignalID));
	m_filterTypeCombo->addItem(tr("EquipmentID"), static_cast<int>(FilterIDType::EquipmentID));
	m_filterTypeCombo->addItem(tr("Caption"), static_cast<int>(FilterIDType::Caption));
	m_bottomLayout->addWidget(m_filterTypeCombo);

	connect(m_filterTypeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_FilterTypeIndexChanged(int)));

	m_filterTypeCombo->setCurrentIndex(0);

	// Masks combo

	m_filterTextCombo = new QComboBox();
	m_filterTextCombo->setEditable(true);
	m_filterTextCombo->setMinimumWidth(150);
	m_filterTextCombo->setInsertPolicy(QComboBox::NoInsert);
	m_bottomLayout->addWidget(m_filterTextCombo);

	// Load masks
	//
	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
	QStringList masks = settings.value(QString("Masks/%1").arg(tabFilter->ID())).toStringList();
	m_filterTextCombo->addItems(masks);
	m_filterTextCombo->setEditText(QString());

	QLineEdit* filterLineEdit = m_filterTextCombo->lineEdit();
	if (filterLineEdit == nullptr)
	{
		Q_ASSERT(filterLineEdit);
	}
	else
	{
		connect(filterLineEdit, &QLineEdit::returnPressed, this, &TuningPage::slot_ApplyFilter);
	}

	// Filter button

	m_filterButton = new QPushButton(tr("Filter"));
	m_bottomLayout->addWidget(m_filterButton);
	connect(m_filterButton, &QPushButton::clicked, this, &TuningPage::slot_ApplyFilter);

	m_bottomLayout->addSpacing(20);

	// Value filter controls
	//

	QLabel* l = new QLabel(tr("Value:"));
	m_bottomLayout->addWidget(l);

	m_filterValueCombo = new QComboBox();
	m_filterValueCombo->addItem(tr("Any Value"), static_cast<int>(FilterValueType::All));
	m_filterValueCombo->addItem(tr("Discrete 0"), static_cast<int>(FilterValueType::Zero));
	m_filterValueCombo->addItem(tr("Discrete 1"), static_cast<int>(FilterValueType::One));
	m_filterValueCombo->addItem(tr("Not Default"), static_cast<int>(FilterValueType::DefaultNotSet));
	m_bottomLayout->addWidget(m_filterValueCombo);

	connect(m_filterValueCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(slot_FilterValueIndexChanged(int)));

	m_filterValueCombo->setCurrentIndex(0);

	m_bottomLayout->addStretch();


	// Button controls
	//

	m_setValueButton = new QPushButton(tr("Set Value"));
	m_bottomLayout->addWidget(m_setValueButton);
	connect(m_setValueButton, &QPushButton::clicked, this, &TuningPage::slot_setValue);

	m_setAllButton = new QPushButton(tr("Set All"));
	m_bottomLayout->addWidget(m_setAllButton);
	connect(m_setAllButton, &QPushButton::clicked, this, &TuningPage::slot_setAll);

	m_bottomLayout->addStretch();

	m_writeButton = new QPushButton(tr("Write"));
	m_bottomLayout->addWidget(m_writeButton);
	connect(m_writeButton, &QPushButton::clicked, this, &TuningPage::slot_Write);

	m_undoButton = new QPushButton(tr("Undo"));
	m_bottomLayout->addWidget(m_undoButton);
	connect(m_undoButton, &QPushButton::clicked, this, &TuningPage::slot_undo);

	if (theConfigSettings.clientSettings.autoApply == false)
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

	// load column width

	m_columnWidthStorage.load(tabFilter->ID());

	for (int c = 0; c < m_model->columnCount(); c++)
	{
		int width = m_columnWidthStorage.width(m_model->columnType(c));

		m_objectList->setColumnWidth(c, width);
	}

	// Color

	if (m_pageFilter->isTab() == true && m_pageFilter->useColors() == true)
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

	//

	connect(theMainWindow, &MainWindow::timerTick500, this, &TuningPage::slot_timerTick500);

}

TuningPage::~TuningPage()
{
	for (int c = 0; c < m_model->columnCount(); c++)
	{
		m_columnWidthStorage.setWidth(m_model->columnType(c), m_objectList->columnWidth(c));
	}

	m_columnWidthStorage.save();

	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	// Save masks
	//

	if (m_pageFilter == nullptr)
	{
		Q_ASSERT(m_pageFilter);
		return;
	}

	QStringList masks;
	for (int i = 0; i < m_filterTextCombo->count(); i++)
	{
		masks.push_back(m_filterTextCombo->itemText(i));
	}
	settings.setValue(QString("Masks/%1").arg(m_pageFilter->ID()), masks);

	m_instanceCounter--;
	//qDebug() << "TuningPage::TuningPage m_instanceCounter = " << m_instanceCounter;
}

void TuningPage::fillObjectsList()
{
	//qDebug() << "FillObjectsList";

	//if (m_pageFilter != nullptr)
	//{
//		qDebug() << "Button " << m_pageFilter->caption();
//	}

	if (m_pageFilter == nullptr)
	{
		Q_ASSERT(m_pageFilter);
		return;
	}

	std::vector<Hash> hashes;

	std::vector<Hash> pageHashes = m_pageFilter->signalsHashes();

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

	QString mask = m_filterTextCombo->currentText();
	if (mask.isEmpty() == false)
	{
		if (m_filterTextCombo->findText(mask) == -1)
		{
			m_filterTextCombo->addItem(mask);
		}
		while (m_filterTextCombo->count() > 10)
		{
			m_filterTextCombo->removeItem(0);
		}
		m_filterTextCombo->setCurrentText(mask);
	}

	FilterIDType filterType = FilterIDType::All;
	QVariant data = m_filterTypeCombo->currentData();
	if (data.isValid() == true)
	{
		filterType = static_cast<FilterIDType>(data.toInt());
	}

	FilterValueType filterValue = FilterValueType::All;
	data = m_filterValueCombo->currentData();
	if (data.isValid() == true)
	{
		filterValue = static_cast<FilterValueType>(data.toInt());
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
			if (m_treeFilter == nullptr)
			{
				Q_ASSERT(m_treeFilter);
				continue;
			}

			if (m_treeFilter->filterSignalExists(hash) == true)
			{
				TuningFilterSignal filterSignal;

				ok = m_treeFilter->filterSignal(hash, filterSignal);
				if (ok == false)
				{
					Q_ASSERT(false);
					return;
				}

				if (filterSignal.useValue() == true)
				{
					modifyDefaultValue = true;
					modifiedDefaultValue = filterSignal.value();
				}
			}
		}
		else
		{
			// Modify the default value from page filter
			//
			if (m_pageFilter == nullptr)
			{
				Q_ASSERT(m_pageFilter);
				return;
			}

			if (m_pageFilter->filterSignalExists(hash) == true)
			{
				TuningFilterSignal filterSignal;

				ok = m_pageFilter->filterSignal(hash, filterSignal);
				if (ok == false)
				{
					Q_ASSERT(false);
					return;
				}

				if (filterSignal.useValue() == true)
				{
					modifyDefaultValue = true;
					modifiedDefaultValue = filterSignal.value();
				}
			}
		}

		// Value filter
		//

		if (filterValue != FilterValueType::All)
		{
			if (filterValue == FilterValueType::DefaultNotSet)
			{
				ok = false;

				const TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

				if (ok == false || state.valid() == false)
				{
					continue;
				}

				TuningValue tvDefault(m_model->defaultValue(asp));
				tvDefault.setType(asp.tuningType());

				if (tvDefault == state.value())
				{
					// Value is set to default
					continue;
				}
			}
			else
			{
				if (asp.isAnalog() == true)
				{
					continue;
				}

				if (asp.isDiscrete() == true)
				{
					ok = false;

					const TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

					if (ok == false || state.valid() == false)
					{
						continue;
					}

					if (filterValue == FilterValueType::Zero && state.value().discreteValue() != 0)
					{
						continue;
					}

					if (filterValue == FilterValueType::One && state.value().discreteValue() != 1)
					{
						continue;
					}
				}
			}
		}

		// Text filter
		//


		if (mask.isEmpty() == false)
		{
			bool filterMatch = false;

			switch (filterType)
			{
			case FilterIDType::All:
				if (asp.appSignalId().contains(mask, Qt::CaseInsensitive) == true
						|| asp.customSignalId().contains(mask, Qt::CaseInsensitive) == true
						|| asp.lmEquipmentId().contains(mask, Qt::CaseInsensitive) == true
						|| asp.caption().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::AppSignalID:
				if (asp.appSignalId().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::CustomAppSignalID:
				if (asp.customSignalId().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::EquipmentID:
				if (asp.lmEquipmentId().contains(mask, Qt::CaseInsensitive) == true)
				{
					filterMatch = true;
				}
				break;
			case FilterIDType::Caption:
				if (asp.caption().contains(mask, Qt::CaseInsensitive) == true)
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

	// Sort list
	//
	try
	{
		const std::pair<int, Qt::SortOrder>& sortData = m_sortData.at(m_pageFilter->ID());
		m_objectList->sortByColumn(sortData.first, sortData.second);
	}
	catch (std::out_of_range)
	{
		const std::pair<int, Qt::SortOrder> sortData = std::make_pair(0, Qt::AscendingOrder);
		m_sortData[m_pageFilter->ID()] = sortData;
		m_objectList->sortByColumn(sortData.first, sortData.second);
	}

	return;
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

	if (m_tuningTcpClient->takeClientControl(this) == false)
	{
		return false;
	}

	QString str = tr("New values will be written:") + QString("\n\n");
	QString strValue;

	bool modifiedFound = false;
	int modifiedCount = 0;

	std::vector<Hash> hashes = m_model->allHashes();

	bool ok = false;

	for (Hash hash : hashes)
	{
		if (m_tuningSignalManager->newValueIsUnapplied(hash) == false)
		{
			continue;
		}

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
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

		if (m_tuningSignalManager->newValueIsUnapplied(hash) == false)
		{
			continue;
		}

		if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
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
			strValue = m_tuningSignalManager->newValue(hash).toString(m_model->analogFormat(), asp.precision());
		}
		else
		{
			strValue = m_tuningSignalManager->newValue(hash).toString();
		}

		str += tr("%1 (%2) = %3\n").arg(asp.appSignalId()).arg(asp.caption()).arg(strValue);

		listCount++;
	}

	if (listCount == 0)
	{
		return false;
	}

	str += QString("\n") + tr("Are you sure you want to continue?");

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
		if (m_tuningSignalManager->newValueIsUnapplied(hash) == false)
		{
			continue;
		}

		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
		{
			continue;
		}

		TuningWriteCommand cmd(hash, m_tuningSignalManager->newValue(hash));

		commands.push_back(cmd);
	}

	if (commands.empty() == true)
	{
		return false;
	}

	m_tuningTcpClient->writeLogSignalChange(tr("'Write' button is pressed."));

	m_tuningTcpClient->writeTuningSignal(commands);

	return true;
}

void TuningPage::apply()
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return;
	}

	if (m_tuningTcpClient->takeClientControl(this) == false)
	{
		return;
	}

	if (QMessageBox::warning(this, qAppName(),
							 tr("Are you sure you want apply the changes?"),
							 QMessageBox::Yes | QMessageBox::No,
							 QMessageBox::No) != QMessageBox::Yes)
	{
		return;
	}

	// Get SOR counters

	TuningCounters rootCounters = m_tuningFilterStorage->root()->counters();

	if (rootCounters.sorCounter > 0)
	{
		if (QMessageBox::warning(this, qAppName(),
								 tr("Warning!!!\n\nSOR Signal(s) are set in logic modules!\n\nIf you apply these changes, module can run into RUN SAFE STATE.\n\nAre you sure you STILL WANT TO APPLY the changes?"),
								 QMessageBox::Yes | QMessageBox::No,
								 QMessageBox::No) != QMessageBox::Yes)
		{
			return;
		}
	}

	m_tuningTcpClient->applyTuningSignals();

	return;
}

void TuningPage::undo()
{
	slot_undo();
}

void TuningPage::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	if (m_pageFilter == nullptr)
	{
		Q_ASSERT(m_pageFilter);
		return;
	}

	m_sortData[m_pageFilter->ID()] = std::make_pair(column, order);
	m_objectList->sortByColumn(column, order);

	return;
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

	std::vector<Hash> selectedHashes;

	for (int row : selectedRows)
	{
		const TuningModelHashSet& hashSet = m_model->hashSetByIndex(row);

		for (int c = 0; c < m_model->valueColumnsCount(); c++)
		{
			Hash hash = hashSet.hash[c];

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			bool ok = false;

			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
			{
				continue;
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
				if (asp.tuningType() != value.type())
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

			selectedHashes.push_back(hash);
		}	// c
	}	// row

	if (selectedHashes.empty() == true)
	{
		return;
	}

	DialogInputTuningValue d(value, defaultValue, sameValue, lowLimit, highLimit, m_model->analogFormat(), precision, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	TuningValue newValue = d.value();

	for (Hash hash : selectedHashes)
	{
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

void TuningPage::slot_FilterValueIndexChanged(int index)
{
	FilterValueType filterValue = FilterValueType::All;
	QVariant data = m_filterValueCombo->currentData();
	if (data.isValid() == true)
	{
		filterValue = static_cast<FilterValueType>(data.toInt());
	}

	if (filterValue != FilterValueType::All)
	{
		m_filterValueCombo->setStyleSheet("QComboBox { color: red }");
	}
	else
	{
		m_filterValueCombo->setStyleSheet(QString());
	}

	Q_UNUSED(index);
	fillObjectsList();
}

void TuningPage::slot_listContextMenuRequested(const QPoint& pos)
{
	Q_UNUSED(pos);

	QModelIndexList mi = m_objectList->selectionModel()->selectedRows();

	QMenu menu(this);

	int menuSignalCount = 0;

	for (const QModelIndex& index : mi)
	{
		if (index.isValid() == false)
		{
			return;
		}

		const TuningModelHashSet& hashes = m_model->hashSetByIndex(index.row());

		for (int i = 0; i < m_model->valueColumnsCount(); i++)
		{
			Hash hash = hashes.hash[i];

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

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
				TuningSignalInfo* d = new TuningSignalInfo(hash, m_model->analogFormat(), m_tuningTcpClient->instanceIdHash(), m_tuningSignalManager, this);
				d->show();
			};

			connect(a, &QAction::triggered, this, f);

			menu.addAction(a);

			menuSignalCount++;

			if (menuSignalCount > 16)
			{
				a = new QAction(tr("..."), &menu);
				a->setEnabled(false);
				menu.addAction(a);
				break;
			}
		}

		if (menuSignalCount > 16)
		{
			break;
		}
	}

	if (menuSignalCount == 0)
	{
		return;
	}

	// Add additional commands

	menu.addSeparator();

	// View

	QMenu* submenuV = menu.addMenu(tr("Format"));

	QAction* a = new QAction(tr("Auto-select"), &menu);
	a->setCheckable(true);
	a->setChecked(m_model->analogFormat() == E::AnalogFormat::g_9_or_9e || m_model->analogFormat() == E::AnalogFormat::G_9_or_9E);
	connect(a, &QAction::triggered, this, [this]() { slot_setAnalogFormat(E::AnalogFormat::g_9_or_9e); });
	submenuV->addAction(a);

	a = new QAction(tr("Decimal (as [-]9.9)"), &menu);
	a->setCheckable(true);
	a->setChecked(m_model->analogFormat() == E::AnalogFormat::f_9);
	connect(a, &QAction::triggered, this, [this]() { slot_setAnalogFormat(E::AnalogFormat::f_9); });
	submenuV->addAction(a);

	a = new QAction(tr("Exponential (as [-]9.9e[+|-]999)"), &menu);
	a->setCheckable(true);
	a->setChecked(m_model->analogFormat() == E::AnalogFormat::e_9e || m_model->analogFormat() == E::AnalogFormat::E_9E);
	connect(a, &QAction::triggered, this, [this]() { slot_setAnalogFormat(E::AnalogFormat::e_9e); });
	submenuV->addAction(a);

	// More

	QMenu* submenuA = menu.addMenu(tr("More"));

	a = new QAction(tr("Add To New Filter..."), &menu);
	connect(a, &QAction::triggered, this, &TuningPage::slot_saveSignalsToNewFilter);
	submenuA->addAction(a);

	a = new QAction(tr("Add To Existing Filter..."), &menu);
	connect(a, &QAction::triggered, this, &TuningPage::slot_saveSignalsToExistingFilter);
	submenuA->addAction(a);

	// If AutoFilter filter exists, add Restore command

	std::shared_ptr<TuningFilter> root = m_tuningFilterStorage->root();

	if (root == nullptr)
	{
		assert(root);
		return;
	}

	std::shared_ptr<TuningFilter> autoCreatedFilter = root->childFilter(m_autoFilterCaption);
	if (autoCreatedFilter != nullptr)
	{
		submenuA->addSeparator();

		a = new QAction(tr("Restore Values From Filter..."), &menu);
		connect(a, &QAction::triggered, this, &TuningPage::slot_restoreValuesFromExistingFilter);
		submenuA->addAction(a);
	}


	menu.exec(QCursor::pos());

}

void TuningPage::slot_saveSignalsToNewFilter()
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return;
	}

	bool ok;
	QString filterName = QInputDialog::getText(this, tr("Add Signals To Filter"),
											tr("Enter the filter name:"), QLineEdit::Normal,
											tr("Name"), &ok);

	if (ok == false)
	{
		return;
	}

	std::shared_ptr<TuningFilter> root = m_tuningFilterStorage->root();

	if (root == nullptr)
	{
		assert(root);
		return;
	}

	// Get AutoFilter filter

	std::shared_ptr<TuningFilter> autoCreatedFilter = root->childFilter(m_autoFilterCaption);
	if (autoCreatedFilter == nullptr)
	{
		autoCreatedFilter = std::make_shared<TuningFilter>();

		QUuid uid = QUuid::createUuid();
		autoCreatedFilter->setID(uid.toString());
		autoCreatedFilter->setCaption(m_autoFilterCaption);
		autoCreatedFilter->setSource(TuningFilter::Source::User);
		autoCreatedFilter->setInterfaceType(TuningFilter::InterfaceType::Tree);

		root->addChild(autoCreatedFilter);
	}

	// Create Filter

	std::shared_ptr<TuningFilter> filter = std::make_shared<TuningFilter>();

	QUuid uid = QUuid::createUuid();
	filter->setID(uid.toString());
	filter->setCaption(filterName);
	filter->setSource(TuningFilter::Source::User);
	filter->setInterfaceType(TuningFilter::InterfaceType::Tree);

	autoCreatedFilter->addChild(filter);

	addSelectedSignalsToFilter(filter.get());
}

void TuningPage::slot_saveSignalsToExistingFilter()
{
	if (theMainWindow->userManager()->login(this) == false)
	{
		return;
	}

	std::shared_ptr<TuningFilter> root = m_tuningFilterStorage->root();

	if (root == nullptr)
	{
		assert(root);
		return;
	}

	// Get AutoFilter filter

	std::shared_ptr<TuningFilter> autoCreatedFilter = root->childFilter(m_autoFilterCaption);
	if (autoCreatedFilter == nullptr)
	{
		return;
	}

	DialogChooseFilter* d = new DialogChooseFilter(this, autoCreatedFilter.get(), TuningFilter::InterfaceType::Tree, TuningFilter::Source::User);

	if (d->exec() != QDialog::Accepted || d->chosenFilter() == nullptr)
	{
		return;
	}

	addSelectedSignalsToFilter(d->chosenFilter());

}

void TuningPage::slot_restoreValuesFromExistingFilter()
{
	std::shared_ptr<TuningFilter> root = m_tuningFilterStorage->root();

	if (root == nullptr)
	{
		assert(root);
		return;
	}

	// Get AutoFilter filter

	std::shared_ptr<TuningFilter> autoCreatedFilter = root->childFilter(m_autoFilterCaption);
	if (autoCreatedFilter == nullptr)
	{
		QMessageBox::warning(this, qAppName(), tr("No auto-created filters exist."));
		return;
	}

	DialogChooseFilter* d = new DialogChooseFilter(this, autoCreatedFilter.get(), TuningFilter::InterfaceType::Tree, TuningFilter::Source::User);

	if (d->exec() != QDialog::Accepted || d->chosenFilter() == nullptr)
	{
		return;
	}

	restoreSignalsFromFilter(d->chosenFilter());
}

void TuningPage::slot_setAnalogFormat(E::AnalogFormat analogFormat)
{
	m_model->setAnalogFormat(analogFormat);

	m_objectList->update();
}

void TuningPage::slot_ApplyFilter()
{
	if (m_filterTextCombo->currentText().isEmpty() == false)
	{
		m_filterTextCombo->setStyleSheet("QComboBox { color: red }");
		m_filterButton->setStyleSheet("QPushButton { color: red }");
	}
	else
	{
		m_filterTextCombo->setStyleSheet(QString());
		m_filterButton->setStyleSheet(QString());
	}

	fillObjectsList();
}

void TuningPage::slot_treeFilterSelectionChanged(std::shared_ptr<TuningFilter> filter)
{
	m_treeFilter = filter;

	fillObjectsList();
}

void TuningPage::slot_pageFilterChanged(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return;
	}

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

	for (int row : selectedRows)
	{
		const TuningModelHashSet& hashSet = m_model->hashSetByIndex(row);

		for (int c = 0; c < m_model->valueColumnsCount(); c++)
		{
			Hash hash = hashSet.hash[c];

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
			{
				continue;
			}

			if (asp.isDiscrete() == true)
			{
				TuningValue tv;

				tv.setType(TuningValueType::Discrete);

				tv.setDiscreteValue(0);

				if (m_tuningSignalManager->newValue(hash).discreteValue() == 0)
				{
					tv.setDiscreteValue(1);
				}

				m_tuningSignalManager->setNewValue(hash, tv);
			}
		}
	}
}

void TuningPage::addSelectedSignalsToFilter(TuningFilter* filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	int addedCount = 0;

	QModelIndexList mi = m_objectList->selectionModel()->selectedRows();

	for (const QModelIndex& index : mi)
	{
		if (index.isValid() == false)
		{
			assert(false);
			return;
		}

		const TuningModelHashSet& hashes = m_model->hashSetByIndex(index.row());

		for (int i = 0; i < m_model->valueColumnsCount(); i++)
		{
			Hash hash = hashes.hash[i];

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			bool found = false;

			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &found);

			if (found == false)
			{
				assert(false);
				return;
			}

			TuningSignalState state = m_tuningSignalManager->state(hash, &found);

			if (found == false)
			{
				continue;
			}

			TuningFilterSignal tv;

			tv.setAppSignalId(asp.appSignalId());

			if (state.valid() == true)
			{
				tv.setUseValue(true);
				tv.setValue(state.value());
			}

			filter->addFilterSignal(tv);

			addedCount++;
		}
	}

	if (addedCount == 0)
	{
		QMessageBox::warning(this, qAppName(), tr("No signals were added."));
		return;
	}

	QString errorMsg;

	if (m_tuningFilterStorage->save(theSettings.userFiltersFile(), &errorMsg, TuningFilter::Source::User) == false)
	{
		theLogFile->writeError(errorMsg);
		QMessageBox::critical(this, tr("Error"), errorMsg);
	}

	QMessageBox::information(this, qAppName(), tr("Adding signals complete."));

	QTimer::singleShot(500, theMainWindow, &MainWindow::slot_userFiltersChanged);
}

void TuningPage::restoreSignalsFromFilter(TuningFilter* filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	int restoredCount = 0;

	TuningFilterSignal tv;

	for (int r = 0; r < m_model->rowCount(); r++)
	{
		for (int c = 0; c < m_model->valueColumnsCount(); c++)
		{
			Hash hash = m_model->hashByIndex(r, c);

			if (hash == UNDEFINED_HASH)
			{
				continue;
			}

			bool exists = filter->filterSignalExists(hash);
			if (exists == true)
			{
				exists = filter->filterSignal(hash, tv);
				if (exists == false)
				{
					Q_ASSERT(false);
					return;
				}

				bool found = false;

				TuningSignalState state = m_tuningSignalManager->state(hash, &found);
				if (found == false)
				{
					continue;
				}

				if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
				{
					continue;
				}

				if (state.value() != tv.value())
				{
					m_tuningSignalManager->setNewValue(hash, tv.value());
					restoredCount++;
				}
			}
		}
	}

	if (restoredCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("No values restored from the filter for current signals."));
	}
	else
	{
		QMessageBox::warning(this, qAppName(), tr("%1 values were restored from the filter. Check them and apply the changes.").arg(restoredCount));
	}
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
			for (int col = 0; col < m_model->columnCount(); col++)
			{
				int columnType = static_cast<int>(m_model->columnType(col));

				if (columnType >= static_cast<int>(TuningModelColumns::ValueFirst))
				{
					//QString str = QString("%1:%2").arg(row).arg(col);
					//qDebug() << str;

					m_objectList->update(m_model->index(row, col));
				}
			}
		}
	}
}

void TuningPage::slot_setAll()
{
	QMenu menu(this);


	// Check all signals to have correct limits
	{
		std::vector<Hash> hashes = m_model->allHashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
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
		std::vector<Hash> hashes = m_model->allHashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
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
		std::vector<Hash> hashes = m_model->allHashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
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
		std::vector<Hash> hashes = m_model->allHashes();

		bool ok = false;

		for (Hash hash : hashes)
		{
			AppSignalParam asp = m_tuningSignalManager->signalParam(hash, &ok);

			TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

			if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
			{
				continue;
			}

			TuningValue tvDefault = m_model->defaultValue(asp);

			if (tvDefault != state.value() && ok == true)
			{
				if(tvDefault < asp.tuningLowBound() || tvDefault > asp.tuningHighBound())
				{
					QString message = tr("Invalid default value '%1' in signal %2 [%3]")
									  .arg(tvDefault.toString(m_model->analogFormat(), asp.precision()))
									  .arg(asp.appSignalId())
									  .arg(asp.caption());
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
	std::vector<Hash> hashes = m_model->allHashes();

	bool ok = false;

	for (Hash hash : hashes)
	{
		TuningSignalState state = m_tuningSignalManager->state(hash, &ok);

		if (state.valid() == false || state.controlIsEnabled() == false || m_tuningTcpClient->writingIsEnabled(state) == false)
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

