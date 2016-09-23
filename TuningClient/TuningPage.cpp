#include "Settings.h"
#include "TuningPage.h"

//
//SnapshotItemModel
//

using namespace std;

TuningItemModel::TuningItemModel(int tuningPageIndex, QObject* parent)
	:QAbstractItemModel(parent)
{
	// Fill column names

	m_columnsNames<<tr("Signal ID");
	m_columnsNames<<tr("Equipment ID");
	m_columnsNames<<tr("App Signal ID");
	m_columnsNames<<tr("Caption");
	m_columnsNames<<tr("Units");
	m_columnsNames<<tr("Type");

	m_columnsNames<<tr("Value");
	m_columnsNames<<tr("Valid");
	m_columnsNames<<tr("Underflow");
	m_columnsNames<<tr("Overflow");

	TuningPageSettings* pageSettings = &theSettings.m_tuningPageSettings[tuningPageIndex];
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	if (pageSettings->m_columnCount == 0)
	{
		m_columnsIndexes.push_back(SignalID);
		m_columnsIndexes.push_back(EquipmentID);
		m_columnsIndexes.push_back(Caption);
		m_columnsIndexes.push_back(Units);
		m_columnsIndexes.push_back(Type);

		m_columnsIndexes.push_back(Value);
		m_columnsIndexes.push_back(Valid);
		m_columnsIndexes.push_back(Underflow);
		m_columnsIndexes.push_back(Overflow);
	}
	else
	{
		m_columnsIndexes  = pageSettings->m_columnsIndexes;
	}

}

void TuningItemModel::setObjectsIndexes(const std::vector<int>& objectsIndexes)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_objectsIndexes.clear();

		endRemoveRows();
	}

	//
	if (objectsIndexes.size() > 0)
	{

		beginInsertRows(QModelIndex(), 0, (int)objectsIndexes.size() - 1);

		m_objectsIndexes = objectsIndexes;

		insertRows(0, (int)objectsIndexes.size());

		endInsertRows();
	}

}

std::vector<int> TuningItemModel::columnsIndexes()
{
	return m_columnsIndexes;

}

void TuningItemModel::setColumnsIndexes(std::vector<int> columnsIndexes)
{
	if (columnCount() > 0)
	{
		beginRemoveColumns(QModelIndex(), 0, columnCount() - 1);

		removeColumns(0, columnCount());

		m_columnsIndexes.clear();

		endRemoveColumns();
	}

	beginInsertColumns(QModelIndex(), 0, (int)columnsIndexes.size() - 1);

	m_columnsIndexes = columnsIndexes;

	insertColumns(0, (int)m_columnsIndexes.size());

	endInsertColumns();

}

QStringList TuningItemModel::columnsNames()
{
	return m_columnsNames;
}


void TuningItemModel::update()
{
	emit QAbstractItemModel::dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

int TuningItemModel::objectIndex(int index)
{
	if (index < 0 || index >= m_objectsIndexes.size())
	{
		assert(false);
		return -1;
	}
	return m_objectsIndexes[index];

}

QModelIndex TuningItemModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

QModelIndex TuningItemModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();

}

int TuningItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (int)m_columnsIndexes.size();

}

int TuningItemModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (int)m_objectsIndexes.size();

}

QVariant TuningItemModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::BackgroundRole)
	{
		int col = index.column();
		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == Value)
		{
			//int displayIndex = m_columnsIndexes[col];
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (role == Qt::ForegroundRole)
	{
		int col = index.column();
		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == Value)
		{
			//int displayIndex = m_columnsIndexes[col];
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}

	}

	if (role == Qt::FontRole)
	{
		int col = index.column();
		int displayIndex = m_columnsIndexes[col];

		//if (displayIndex == Value)
		//{
			QFont f = QFont("Arial", 10);
			f.setBold(true);
			return f;
		//}
	}

	if (role == Qt::DisplayRole)
	{
		int row = index.row();
		if (row >= m_objectsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		int col = index.column();

		if (col < 0 || col >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		//QString str = QString("%1:%2").arg(row).arg(col);
		//qDebug()<<str;

		TuningObject o = theObjects.object(m_objectsIndexes[row]);

		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == SignalID)
		{
			return o.customAppSignalID();
		}

		if (displayIndex == EquipmentID)
		{
			return o.equipmentID();
		}

		if (displayIndex == AppSignalID)
		{
			return o.appSignalID();
		}

		if (displayIndex == Caption)
		{
			return o.caption();
		}

		if (displayIndex == Units)
		{
			return o.units();
		}

		if (displayIndex == Type)
		{
			return o.analog() ? "Analog" : "Discrete";
		}

		//
		// State
		//



		if (displayIndex == Value)
		{
			/*if (o->Valid == true)
				{
					if (s->isDiscrete())
					{
						return ((int)state.value == s->normalState()) ? tr("No") : tr("Yes");
					}
					if (s->isAnalog())
					{
						QString str = QString::number(state.value, 'f', s->decimalPlaces());
						if (state.flags.underflow == true)
						{
							str += tr(" [Underflow");
						}

						return str;
					}
				}
				else*/
			{
				return tr("???");
			}
		}


		if (displayIndex == Valid)
		{
			return (o.valid() == true) ? tr("Yes") : tr("No");
		}

		if (displayIndex == Underflow)
		{
			return (o.underflow() == true) ? tr("Yes") : tr("No");
		}

		if (displayIndex == Overflow)
		{
			return (o.overflow() == true) ? tr("Yes") : tr("No");
		}
	}
	return QVariant();
}

QVariant TuningItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section < 0 || section >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		int displayIndex = m_columnsIndexes[section];
		return m_columnsNames.at(displayIndex);
	}

	return QVariant();
}

//
// TuningPage
//
FilterButton::FilterButton(Hash hash, const QString& caption, QWidget* parent)
	:QPushButton(caption, parent)
{
	m_filterHash = hash;
	m_caption = caption;

	setCheckable(true);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

Hash FilterButton::filterHash()
{
	return m_filterHash;
}

void FilterButton::slot_toggled(bool checked)
{
	if (checked == true)
	{
		emit filterButtonClicked(m_filterHash);
	}

}

//
// TuningPage
//

TuningPage::TuningPage(int tuningPageIndex, ObjectFilter *tabFilter, QWidget *parent) :
	m_tuningPageIndex(tuningPageIndex),
	QWidget(parent),
	m_tabFilter(tabFilter)
{
	// Reserve place for tuning page settings and copy existing
	//
	if (theSettings.m_tuningPageSettings.size() <= m_tuningPageIndex)
	{
		std::vector<TuningPageSettings> m_tuningPageSettings2 = theSettings.m_tuningPageSettings;

		theSettings.m_tuningPageSettings.resize(m_tuningPageIndex + 1);
		for (int i = 0; i < m_tuningPageSettings2.size(); i++)
		{
			theSettings.m_tuningPageSettings[i] = m_tuningPageSettings2[i];
		}
	}


	std::vector<FilterButton*> buttons;

	// Top buttons
	//
	int count = theFilters.topFilterCount();
	for (int i = 0; i < count; i++)
	{
		ObjectFilter* f = theFilters.topFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isButton() == false)
		{
			continue;
		}

		FilterButton* button = new FilterButton(f->hash(), f->caption());
		buttons.push_back(button);

		connect(button, &FilterButton::filterButtonClicked, this, &TuningPage::slot_filterButtonClicked);
	}

	// Child buttons
	//
	if (tabFilter != nullptr)
	{
		for (int i = 0; i < tabFilter->childFiltersCount(); i++)
		{
			ObjectFilter* f = tabFilter->childFilter(i);
			if (f == nullptr)
			{
				assert(f);
				continue;
			}

			FilterButton* button = new FilterButton(f->hash(), f->caption());
			buttons.push_back(button);

			connect(button, &FilterButton::filterButtonClicked, this, &TuningPage::slot_filterButtonClicked);

		}
	}

	if (buttons.empty() == false)
	{
		m_filterButtonGroup = new QButtonGroup(this);

		m_filterButtonGroup->setExclusive(true);

		m_buttonsLayout = new QHBoxLayout();

		for (auto b: buttons)
		{
			m_filterButtonGroup->addButton(b);
			m_buttonsLayout->addWidget(b);
		}

		m_buttonsLayout->addStretch();

		// Set the first button checked
		//
		buttons[0]->blockSignals(true);
		buttons[0]->setChecked(true);
		m_buttonFilter = theFilters.filter(buttons[0]->filterHash());
		buttons[0]->blockSignals(false);

	}

	// Object List
	//
	m_objectList = new QTableView();

	// Button controls
	//
	m_maskTypeCombo = new QComboBox();
	m_maskEdit = new QLineEdit();
	m_maskButton = new QPushButton("Apply");

	m_applyButton = new QPushButton("Apply");
	m_restoreButton = new QPushButton("Restore");

	m_bottomLayout = new QHBoxLayout();

	m_bottomLayout->addWidget(m_maskTypeCombo);
	m_bottomLayout->addWidget(m_maskEdit);
	m_bottomLayout->addWidget(m_maskButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_applyButton);
	m_bottomLayout->addWidget(m_restoreButton);

	m_mainLayout = new QVBoxLayout(this);

	if (m_buttonsLayout != nullptr)
	{
		m_mainLayout->addLayout(m_buttonsLayout);
	}

	m_mainLayout->addWidget(m_objectList);
	m_mainLayout->addLayout(m_bottomLayout);

	// Models and data
	//
	m_model = new TuningItemModel(m_tuningPageIndex, this);


	m_objectList->setModel(m_model);
	m_objectList->verticalHeader()->hide();
	m_objectList->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_objectList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	//m_objectList->setSortingEnabled(true);
	//m_objectList->sortByColumn(0, Qt::AscendingOrder);

	TuningPageSettings* pageSettings = &theSettings.m_tuningPageSettings[tuningPageIndex];
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	for (int i = 0; i < pageSettings->m_columnCount; i++)
	{
		m_objectList->setColumnWidth(i, pageSettings->m_columnsWidth[i]);
	}

	fillObjectsList();
}

TuningPage::~TuningPage()
{
	TuningPageSettings* pageSettings = &theSettings.m_tuningPageSettings[m_tuningPageIndex];
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	pageSettings->m_columnsIndexes = m_model->columnsIndexes();
	pageSettings->m_columnCount = (int)pageSettings->m_columnsIndexes.size();

	pageSettings->m_columnsWidth.resize(pageSettings->m_columnCount);
	for (int i = 0; i < pageSettings->m_columnCount; i++)
	{
		pageSettings->m_columnsWidth[i] = m_objectList->columnWidth(i);
	}
}

void TuningPage::fillObjectsList()
{
	m_objectsIndexes.clear();

	for (int i = 0; i < theObjects.objectsCount(); i++)
	{
		TuningObject o = theObjects.object(i);

		if (m_treeFilter != nullptr)
		{
			bool result = true;

			ObjectFilter* treeFilter = m_treeFilter;
			while (treeFilter != nullptr)
			{
				if (treeFilter->match(o) == false)
				{
					result = false;
					break;
				}
				treeFilter = treeFilter->parent();
			}
			if (result == false)
			{
				continue;
			}
		}

		if (m_tabFilter != nullptr)
		{
			if (m_tabFilter->match(o) == false)
			{
				continue;
			}
		}

		if (m_buttonFilter != nullptr)
		{
			if (m_buttonFilter->match(o) == false)
			{
				continue;
			}
		}

		m_objectsIndexes.push_back(i);
	}

	m_model->setObjectsIndexes(m_objectsIndexes);
}

void TuningPage::slot_filterButtonClicked(Hash hash)
{
	qDebug()<<"Filter button clicked: "<<hash;

	m_buttonFilter = theFilters.filter(hash);

	if (m_buttonFilter == nullptr)
	{
		assert(m_buttonFilter);
		return;
	}

	fillObjectsList();

}

void TuningPage::slot_filterTreeChanged(Hash hash)
{
	qDebug()<<"Filter tree clicked: "<<hash;

	m_treeFilter = theFilters.filter(hash);

	if (m_treeFilter == nullptr)
	{
		assert(m_treeFilter);
		return;
	}

	fillObjectsList();
}
