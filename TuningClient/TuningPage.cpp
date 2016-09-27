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
	m_columnsNames<<tr("Default");
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
		m_columnsIndexes.push_back(Default);
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

		if (displayIndex == Default)
		{
			//int displayIndex = m_columnsIndexes[col];
			QColor color = QColor(Qt::gray);
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

		if (displayIndex == Default)
		{
			//int displayIndex = m_columnsIndexes[col];
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (role == Qt::FontRole)
	{
		//int col = index.column();
		//int displayIndex = m_columnsIndexes[col];

		//if (displayIndex == Value)
		//{
			QFont f = QFont("Arial", 10);
			f.setBold(true);
			return f;
		//}
	}

	if (role == Qt::TextAlignmentRole)
	{

		int col = index.column();
		int displayIndex = m_columnsIndexes[col];

		if (displayIndex >= Value)
		{
			return Qt::AlignCenter;
		}
		return Qt::AlignLeft + Qt::AlignVCenter;
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

		if (displayIndex == Default)
		{
			if (o.analog())
			{
				return tr("0.0");
			}
			else
			{
				return tr("No");
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
FilterButton::FilterButton(std::shared_ptr<ObjectFilter> filter, const QString& caption, QWidget* parent)
	:QPushButton(caption, parent)
{
	m_filter = filter;
	m_caption = caption;

	setCheckable(true);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

std::shared_ptr<ObjectFilter> FilterButton::filter()
{
	return m_filter;
}

void FilterButton::slot_toggled(bool checked)
{
	if (checked == true)
	{
		emit filterButtonClicked(m_filter);
	}

}

//
// TuningPage
//

TuningPage::TuningPage(int tuningPageIndex, std::shared_ptr<ObjectFilter> tabFilter, QWidget *parent) :
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
		std::shared_ptr<ObjectFilter> f = theFilters.topFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isButton() == false)
		{
			continue;
		}

		FilterButton* button = new FilterButton(f, f->caption());
		buttons.push_back(button);

		connect(button, &FilterButton::filterButtonClicked, this, &TuningPage::slot_filterButtonClicked);
	}

	// Child buttons
	//
	if (tabFilter != nullptr)
	{
		for (int i = 0; i < tabFilter->childFiltersCount(); i++)
		{
			std::shared_ptr<ObjectFilter> f = tabFilter->childFilter(i);
			if (f == nullptr)
			{
				assert(f);
				continue;
			}

			FilterButton* button = new FilterButton(f, f->caption());
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
		m_buttonFilter = buttons[0]->filter();
		buttons[0]->blockSignals(false);

	}

	// Object List
	//
	m_objectList = new QTableView();

	// Button controls
	//
	m_maskTypeCombo = new QComboBox();
	m_maskEdit = new QLineEdit();
	m_maskButton = new QPushButton("Apply Mask");

	m_setValueButton = new QPushButton("Set Value");
	m_setOnButton = new QPushButton("Set all to On");
	m_setOffButton = new QPushButton("Set all to Off");
	m_setToDefaultButton = new QPushButton("Set to Defaults");

	m_bottomLayout = new QHBoxLayout();

	m_bottomLayout->addWidget(m_maskTypeCombo);
	m_bottomLayout->addWidget(m_maskEdit);
	m_bottomLayout->addWidget(m_maskButton);
	m_bottomLayout->addStretch();
	m_bottomLayout->addWidget(m_setValueButton);
	m_bottomLayout->addWidget(m_setOnButton);
	m_bottomLayout->addWidget(m_setOffButton);
	m_bottomLayout->addWidget(m_setToDefaultButton);

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
			if (m_treeFilter->folder() == true)
			{
				continue;
			}

			bool result = true;

			ObjectFilter* treeFilter = m_treeFilter.get();
			while (treeFilter != nullptr)
			{
				if (treeFilter->match(o) == false)
				{
					result = false;
					break;
				}

				treeFilter = treeFilter->parentFilter();
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

void TuningPage::slot_filterButtonClicked(std::shared_ptr<ObjectFilter> filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	qDebug()<<"Filter button clicked: "<<filter->caption();

	m_buttonFilter = filter;

	fillObjectsList();

}

void TuningPage::slot_filterTreeChanged(std::shared_ptr<ObjectFilter> filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	qDebug()<<"Filter tree clicked: "<<filter->caption();

	m_treeFilter = filter;


	fillObjectsList();
}
