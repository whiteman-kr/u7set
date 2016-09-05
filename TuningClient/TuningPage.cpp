#include "TuningPage.h"

//
//SnapshotItemModel
//

using namespace std;

TuningItemModel::TuningItemModel(QObject* parent)
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


	//if (theSettings.m_signalSnapshotColumnCount == 0)
	{
		m_columnsIndexes.push_back(SignalID);
		m_columnsIndexes.push_back(EquipmentID);
		m_columnsIndexes.push_back(Caption);
		m_columnsIndexes.push_back(Units);
		m_columnsIndexes.push_back(Type);

		//m_columnsIndexes.push_back(LocalTime);
		m_columnsIndexes.push_back(Value);
		m_columnsIndexes.push_back(Valid);
		m_columnsIndexes.push_back(Underflow);
		m_columnsIndexes.push_back(Overflow);
	}
	/*else
	{
		const int* begin = reinterpret_cast<int*>(theSettings.m_signalSnapshotColumns.data());
		const int* end = begin + theSettings.m_signalSnapshotColumnCount;

		std::vector<int> buffer(begin, end);
		m_columnsIndexes = buffer;
	}*/

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

	beginInsertRows(QModelIndex(), 0, (int)objectsIndexes.size());

	m_objectsIndexes = objectsIndexes;

	insertRows(0, (int)objectsIndexes.size());

	endInsertRows();

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

		TuningObject* o = theObjects.object(row);
		if (o == nullptr)
		{
			assert(o);
			return QVariant();
		}

		int displayIndex = m_columnsIndexes[col];

		if (displayIndex == SignalID)
		{
			return o->customAppSignalID();
		}

		if (displayIndex == EquipmentID)
		{
			return o->equipmentID();
		}

		if (displayIndex == AppSignalID)
		{
			return o->appSignalID();
		}

		if (displayIndex == Caption)
		{
			return o->caption();
		}

		if (displayIndex == Units)
		{
			return o->units();
		}

		if (displayIndex == Type)
		{
			return o->analog() ? "Analog" : "Discrete";
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
			return (o->valid() == true) ? tr("Yes") : tr("No");
		}

		if (displayIndex == Underflow)
		{
			return (o->underflow() == true) ? tr("Yes") : tr("No");
		}

		if (displayIndex == Overflow)
		{
			return (o->overflow() == true) ? tr("Yes") : tr("No");
		}




		return QVariant();
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
//SnapshotItemModel
//
TuningItemProxyModel::TuningItemProxyModel(TuningItemModel *sourceModel, QObject *parent) :
	QSortFilterProxyModel(parent),
	m_sourceModel(sourceModel)
{
	setSourceModel(sourceModel);
}

bool TuningItemProxyModel::filterAcceptsRow(int source_row, const QModelIndex &) const
{
	/*const Signal* s = m_sourceModel->signal(source_row);
	if (s == nullptr)
	{
		assert(s);
		return false;
	}

	if (m_strIdMasks.isEmpty() == false)
	{
		QString strId = s->customAppSignalID().trimmed();
		for (QString idMask : m_strIdMasks)
		{
			QRegExp rx(idMask.trimmed());
			rx.setPatternSyntax(QRegExp::Wildcard);
			if (rx.exactMatch(strId))
			{
				return true;
			}
		}
		return false;
	}*/

	return true;
}

bool TuningItemProxyModel::lessThan(const QModelIndex &left,
									   const QModelIndex &right) const
{
	 /*const Signal* s1 = m_sourceModel->signal(left.row());
	 const Signal* s2 = m_sourceModel->signal(right.row());

	 if (s1 == nullptr || s2 == nullptr)
	 {
		 assert(false);
		 return false;
	 }

	 std::vector<int> ci = m_sourceModel->columnsIndexes();

	 int sc = sortColumn();
	 if (sc < 0 || sc >= ci.size())
	 {
		assert(false);
		return false;
	 }

	 static AppSignalState st1;
	 static AppSignalState st2;

	 int c = ci[sc];
	 if (c >= SnapshotItemModel::SystemTime)
	 {
		 st1 = theSignals.signalState(s1->appSignalID());
		 st2 = theSignals.signalState(s2->appSignalID());
	 }

	 QVariant v1;
	 QVariant v2;


	 switch (c)
	 {
	 case SnapshotItemModel::SignalID:
	 {
		 v1 = s1->customAppSignalID();
		 v2 = s2->customAppSignalID();
	 }
		 break;
	 case SnapshotItemModel::EquipmentID:
	 {
		 v1 = s1->equipmentID();
		 v2 = s2->equipmentID();
	 }
		 break;
	 case SnapshotItemModel::AppSignalID:
	 {
		 v1 = s1->appSignalID();
		 v2 = s2->appSignalID();
	 }
		 break;
	 case SnapshotItemModel::Caption:
	 {
		 v1 = s1->caption();
		 v2 = s2->caption();
	 }
		 break;
	 case SnapshotItemModel::Units:
	 {
		 v1 = s1->unitID();
		 v2 = s2->unitID();
	 }
		 break;
	 case SnapshotItemModel::Type:
	 {
		 if (s1->type() == s2->type())
		 {
			 if (s1->dataFormat() == s2->dataFormat())
			 {
				 v1 = s1->inOutTypeInt();
				 v2 = s2->inOutTypeInt();
			 }
			 else
			 {
				 v1 = s1->dataFormatInt();
				 v2 = s2->dataFormatInt();
			 }
		 }
		 else
		 {
			 v1 = s1->typeInt();
			 v2 = s2->typeInt();
		 }
	 }
		 break;
	 case SnapshotItemModel::SystemTime:
	 {
		 v1 = st1.time.system;
		 v2 = st2.time.system;
	 }
		 break;
	 case SnapshotItemModel::LocalTime:
	 {
		 v1 = st1.time.local;
		 v2 = st2.time.local;
	 }
		 break;
	 case SnapshotItemModel::PlantTime:
	 {
		 v1 = st1.time.plant;
		 v2 = st2.time.plant;
	 }
		 break;
	 case SnapshotItemModel::Value:
	 {
		 if (s1->isAnalog() == s2->isAnalog())
		 {
			v1 = st1.value;
			v2 = st2.value;
		 }
		 else
		 {
			 v1 = s1->isAnalog();
			 v2 = s2->isAnalog();
		 }
	 }
		 break;
	 case SnapshotItemModel::Valid:
	 {
		 v1 = st1.flags.valid;
		 v2 = st2.flags.valid;
	 }
		 break;
	 case SnapshotItemModel::Underflow:
	 {
		 v1 = st1.flags.underflow;
		 v2 = st2.flags.underflow;
	 }
		 break;
	 case SnapshotItemModel::Overflow:
	 {
		 v1 = st1.flags.overflow;
		 v2 = st2.flags.overflow;
	 }
		 break;
	 default:
		 assert(false);
		 return false;
	 }

	 if (v1 == v2)
	 {
		 return s1->customAppSignalID() < s2->customAppSignalID();
	 }

	 return v1 < v2;
	 */
	return true;

}


void TuningItemProxyModel::refreshFilters()
{
	invalidateFilter();
}

int TuningItemProxyModel::objectIndex(const QModelIndex& mi)
{
	return m_sourceModel->objectIndex(mapToSource(mi).row());
}

//
// TuningPage
//
FilterButton::FilterButton(const QString& filterId, const QString& caption, QWidget* parent)
	:QPushButton(caption, parent)
{
	m_filterId = filterId;
	m_caption = caption;
}

//
// TuningPage
//

TuningPage::TuningPage(std::shared_ptr<ObjectFilter> tabFilter, QWidget *parent) :
	QWidget(parent),
	m_tabFilter(tabFilter)
{
	// Top buttons
	//
	for (auto f : theFilters.filters)
	{
		if (f->isButton() == false)
		{
			continue;
		}

		FilterButton* button = new FilterButton(f->strID(), f->caption());
		m_buttons.push_back(button);
	}

	// Child buttons
	//
	if (tabFilter != nullptr)
	{
		for(auto f : tabFilter->childFilters)
		{
			FilterButton* button = new FilterButton(f->strID(), f->caption());
			m_buttons.push_back(button);
		}
	}

	if (m_buttons.empty() == false)
	{
		m_buttonsLayout = new QHBoxLayout();

		for (auto b: m_buttons)
		{
			m_buttonsLayout->addWidget(b);
		}
		m_buttonsLayout->addStretch();
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
	// crete models
	//
	m_model = new TuningItemModel(this);


	for (int i = 0; i < theObjects.objectsCount(); i++)
	{
		m_objectsIndexes.push_back(i);
	}

	m_model->setObjectsIndexes(m_objectsIndexes);

	m_proxyModel = new TuningItemProxyModel(m_model, this);

	m_objectList->setModel(m_proxyModel);
	m_objectList->verticalHeader()->hide();
	m_objectList->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
	m_objectList->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_objectList->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	//m_objectList->setSortingEnabled(true);
	//m_objectList->sortByColumn(0, Qt::AscendingOrder);

}
