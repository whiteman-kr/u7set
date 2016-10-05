#include "Settings.h"
#include "TuningPage.h"

using namespace std;

//
// TuningItemSorter
//

TuningItemSorter::TuningItemSorter(int column, Qt::SortOrder order):
	m_column(column),
	m_order(order)
{
}

bool TuningItemSorter::sortFunction(const TuningObject& o1, const TuningObject& o2, int column, Qt::SortOrder order) const
{

	QVariant v1;
	QVariant v2;

	switch (static_cast<TuningItemModel::Columns>(column))
	{
	case TuningItemModel::Columns::CustomAppSignalID:
		{
			v1 = o1.customAppSignalID();
			v2 = o2.customAppSignalID();
		}
		break;
	case TuningItemModel::Columns::EquipmentID:
		{
			v1 = o1.equipmentID();
			v2 = o2.equipmentID();
		}
		break;
	case TuningItemModel::Columns::AppSignalID:
		{
			v1 = o1.appSignalID();
			v2 = o2.appSignalID();
		}
		break;
	case TuningItemModel::Columns::Caption:
		{
			v1 = o1.caption();
			v2 = o2.caption();
		}
		break;
	case TuningItemModel::Columns::Units:
		{
			v1 = o1.units();
			v2 = o2.units();
		}
		break;
	case TuningItemModel::Columns::Type:
		{
			v1 = o1.analog();
			v2 = o2.analog();
		}
		break;

	case TuningItemModel::Columns::Default:
		{
			if (o1.analog() == o2.analog())
			{
				v1 = o1.value();
				v2 = o2.value();
			}
			else
			{
				v1 = o1.analog();
				v2 = o2.analog();
			}
		}
		break;
	case TuningItemModel::Columns::Value:
		{
			if (o1.analog() == o2.analog())
			{
				v1 = o1.value();
				v2 = o2.value();
			}
			else
			{
				v1 = o1.analog();
				v2 = o2.analog();
			}
		}
		break;
	case TuningItemModel::Columns::Valid:
		{
			v1 = o1.valid();
			v2 = o2.valid();
		}
		break;
	case TuningItemModel::Columns::Underflow:
		{
			v1 = o1.underflow();
			v2 = o2.underflow();
		}
		break;
	case TuningItemModel::Columns::Overflow:
		{
			v1 = o1.overflow();
			v2 = o2.overflow();
		}
		break;
	default:
		assert(false);
		return false;
	}

	if (v1 == v2)
	{
		return o1.customAppSignalID() < o2.customAppSignalID();
	}

	if (order == Qt::AscendingOrder)
		return v1 < v2;
	else
		return v1 > v2;
}

//
// TuningItemModel
//

TuningItemModel::TuningItemModel(QObject* parent)
  :QAbstractItemModel(parent)
{
	// Fill column names

	m_columnsNames<<tr("Custom AppSignal ID");
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
}

TuningItemModel::~TuningItemModel()
{
	if (m_font != nullptr)
	{
		delete m_font;
		m_font = nullptr;
	}
}

void TuningItemModel::setObjectsIndexes(const std::vector<TuningObject>& allObjects, const std::vector<int>& objectsIndexes)
{
	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_objects.clear();

		endRemoveRows();
	}

	//
	if (objectsIndexes.size() > 0)
	{

		beginInsertRows(QModelIndex(), 0, (int)objectsIndexes.size() - 1);

		for (int index : objectsIndexes)
		{
			m_objects.push_back(allObjects[index]);
		}

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

int TuningItemModel::columnIndex(int index) const
{
	if (index <0 || index >= m_columnsIndexes.size())
	{
		assert(false);
		return -1;
	}

	return m_columnsIndexes[index];
}

void TuningItemModel::updateStates(int from, int to)
{
	/*if (m_signalsTable.size() == 0)
	{
		return;
	}
	if (from >= m_signalsTable.size() || to >= m_signalsTable.size())
	{
		assert(false);
		return;
	}

	std::vector<Hash> requestHashes;
	requestHashes.reserve(to - from);

	std::vector<AppSignalState> requestStates;

	for (int i = from; i <= to; i++)
	{
		requestHashes.push_back(m_signalsTable[i].first);
	}

	int count = theSignals.signalState(requestHashes, &requestStates);

	if (count != requestHashes.size() || count != requestStates.size())
	{
		assert(false);
		return;
	}

	int state = 0;
	for (int i = from; i <= to; i++)
	{
		m_signalsTable[i].second = requestStates[state];
		state++;
	}*/

	return;
}

TuningObject TuningItemModel::object(int index)
{
	if (index < 0 || index >= m_objects.size())
	{
		assert(false);
		return TuningObject();
	}
	return m_objects[index];
}

void TuningItemModel::setFont(const QString& fontName, int fontSize, bool fontBold)
{
	if (m_font != nullptr)
	{
		delete m_font;
	}
	m_font = new QFont(fontName, fontSize, fontBold);
}

void TuningItemModel::addColumn(Columns column)
{
	m_columnsIndexes.push_back(column);
}

QModelIndex TuningItemModel::index(int row, int column, const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return createIndex(row, column);
}

void TuningItemModel::sort(int column, Qt::SortOrder order)
{
	if (column < 0 || column >= m_columnsIndexes.size())
	{
		assert(false);
		return;
	}

	int sortColumnIndex = m_columnsIndexes[column];

	std::sort(m_objects.begin(), m_objects.end(), TuningItemSorter(sortColumnIndex, order));

	if (m_objects.empty() == false)
	{
		emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
	}

	return;
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
	return (int)m_objects.size();

}

QVariant TuningItemModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::BackgroundRole)
	{
		return backColor(index);
	}

	if (role == Qt::ForegroundRole)
	{
		return foregroundColor(index);
	}

	if (m_font != nullptr && role == Qt::FontRole)
	{
		return *m_font;
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
		//QString str = QString("%1:%2").arg(row).arg(col);
		//qDebug()<<str;
		int col = index.column();

		if (col < 0 || col >= m_columnsIndexes.size())
		{
			assert(false);
			return QVariant();
		}

		int row = index.row();
		if (row >= m_objects.size())
		{
			assert(false);
			return QVariant();
		}

		const TuningObject& o = m_objects[row];

		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == CustomAppSignalID)
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
			if (o.valid() == true)
			{
				if (o.analog() == false)
				{
					return ((int)o.value().toDouble() == 0 ? tr("No") : tr("Yes"));
				}
				else
				{
					QString str = QString::number(o.value().toDouble(), 'f', o.decimalPlaces());
					if (o.underflow() == true)
					{
						str += tr(" [Underflow]");
					}
					if (o.overflow() == true)
					{
						str += tr(" [Overflow]");
					}

					return str;
				}
			}
			else
			{
				return tr("?");
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

QBrush TuningItemModel::backColor(const QModelIndex& index) const
{
	Q_UNUSED(index);
	return QBrush();
}

QBrush TuningItemModel::foregroundColor(const QModelIndex& index) const
{
	Q_UNUSED(index);
	return QBrush();
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
// TuningItemModelMain
//


TuningItemModelMain::TuningItemModelMain(int tuningPageIndex, QObject* parent)
	:TuningItemModel(parent)
{
	TuningPageSettings* pageSettings = &theSettings.m_tuningPageSettings[tuningPageIndex];
	if (pageSettings == nullptr)
	{
		assert(pageSettings);
		return;
	}

	if (pageSettings->m_columnCount == 0)
	{
		addColumn(CustomAppSignalID);
		addColumn(EquipmentID);
		addColumn(Caption);
		addColumn(Units);
		addColumn(Type);

		addColumn(Value);
		addColumn(Default);
		addColumn(Valid);
		addColumn(Underflow);
		addColumn(Overflow);
	}
	else
	{
		m_columnsIndexes  = pageSettings->m_columnsIndexes;
	}
}

QBrush TuningItemModelMain::backColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_objects.size())
	{
		assert(false);
		return QBrush();
	}

	if (displayIndex == Value)
	{
		const TuningObject& o = m_objects[row];

		if (o.valid() == false)
		{
			QColor color = QColor(Qt::red);
			return QBrush(color);
		}
	}

	if (displayIndex == Default)
	{
		QColor color = QColor(Qt::gray);
		return QBrush(color);
	}

	return QBrush();
}

QBrush TuningItemModelMain::foregroundColor(const QModelIndex& index) const
{
	int col = index.column();
	int displayIndex = m_columnsIndexes[col];

	int row = index.row();
	if (row >= m_objects.size())
	{
		assert(false);
		return QBrush();
	}

	if (displayIndex == Value)
	{
		const TuningObject& o = m_objects[row];

		if (o.valid() == false)
		{
			QColor color = QColor(Qt::white);
			return QBrush(color);
		}
	}

	if (displayIndex == Default)
	{
		//int displayIndex = m_columnsIndexes[col];
		QColor color = QColor(Qt::white);
		return QBrush(color);
	}

	return QBrush();

}


//
// TuningPage
//
FilterButton::FilterButton(std::shared_ptr<TuningFilter> filter, const QString& caption, QWidget* parent)
	:QPushButton(caption, parent)
{
	m_filter = filter;
	m_caption = caption;

	setCheckable(true);

	connect(this, &QPushButton::toggled, this, &FilterButton::slot_toggled);
}

std::shared_ptr<TuningFilter> FilterButton::filter()
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

TuningPage::TuningPage(int tuningPageIndex, std::shared_ptr<TuningFilter> tabFilter, QWidget *parent) :
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
	int count = theFilters.m_root->childFiltersCount();
	for (int i = 0; i < count; i++)
	{
		std::shared_ptr<TuningFilter> f = theFilters.m_root->childFilter(i);
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
			std::shared_ptr<TuningFilter> f = tabFilter->childFilter(i);
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
	m_model = new TuningItemModelMain(m_tuningPageIndex, this);
	//m_model->setFont("Ms Sans Serif", 10, true);


	m_objectList->setModel(m_model);
	m_objectList->verticalHeader()->hide();
	m_objectList->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_objectList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_objectList->setSortingEnabled(true);

	connect(m_objectList->horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, &TuningPage::sortIndicatorChanged);

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
	m_objectList->resizeColumnsToContents();

	m_updateStateTimerId = startTimer(500);

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
	std::vector<int> objectsIndexes;

	std::vector<TuningObject> objects = theObjects.objects();

	for (int i = 0; i < objects.size(); i++)
	{
		const TuningObject& o = objects[i];

		// Root filter
		//
		if (theFilters.m_root->match(o) == false)
		{
			continue;
		}

		// Tree Filter
		//
		if (m_treeFilter != nullptr)
		{
			bool result = true;

			TuningFilter* treeFilter = m_treeFilter.get();
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

		// Tab Filter
		//

		if (m_tabFilter != nullptr)
		{
			if (m_tabFilter->match(o) == false)
			{
				continue;
			}
		}

		// Button Filter
		//

		if (m_buttonFilter != nullptr)
		{
			if (m_buttonFilter->match(o) == false)
			{
				continue;
			}
		}

		objectsIndexes.push_back(i);
	}

	m_model->setObjectsIndexes(objects, objectsIndexes);
	m_objectList->sortByColumn(m_sortColumn, m_sortOrder);
}

void TuningPage::slot_filterButtonClicked(std::shared_ptr<TuningFilter> filter)
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

void TuningPage::sortIndicatorChanged(int column, Qt::SortOrder order)
{
	m_sortColumn = column;
	m_sortOrder = order;

	m_model->sort(column, order);
}


void TuningPage::slot_filterTreeChanged(std::shared_ptr<TuningFilter> filter)
{
	if (filter == nullptr)
	{
		qDebug()<<"Filter tree removed.";
	}
	else
	{
		qDebug()<<"Filter tree clicked: "<<filter->caption();
	}

	m_treeFilter = filter;

	fillObjectsList();
}

void TuningPage::timerEvent(QTimerEvent* event)
{
	assert(event);

	if  (event->timerId() == m_updateStateTimerId && m_model->rowCount() > 0 && isVisible() == true)
	{
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

		// Update signal states
		//
		m_model->updateStates(from, to);

		// Redraw visible table items
		//
		for (int col = 0; col < m_model->columnCount(); col++)
		{
			int displayIndex = m_model->columnIndex(col);

			if (displayIndex >= static_cast<int>(TuningItemModel::Columns::Value))
			{
				for (int row = from; row <= to; row++)
				{
					m_objectList->update(m_model->index(row, col));
				}
			}
		}
	}
}
