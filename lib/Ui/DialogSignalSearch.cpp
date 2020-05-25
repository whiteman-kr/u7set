#include "Stable.h"
#include "DialogSignalSearch.h"

SignalSearchSorter::SignalSearchSorter(std::vector<AppSignalParam>* appSignalParamVec, Columns sortColumn, Qt::SortOrder sortOrder):
	m_sortColumn(sortColumn),
	m_sortOrder(sortOrder),
	m_appSignalParamVec(appSignalParamVec)
{
	if (appSignalParamVec == nullptr)
	{
		Q_ASSERT(appSignalParamVec);
	}
}

bool SignalSearchSorter::sortFunction(int index1, int index2, const SignalSearchSorter* pThis)
{
	if (m_appSignalParamVec == nullptr)
	{
		Q_ASSERT(m_appSignalParamVec);
		return index1 < index2;
	}

	if (index1 < 0 || index1 >= pThis->m_appSignalParamVec->size() ||
			index2 < 0 || index2 >= pThis->m_appSignalParamVec->size())
	{
		Q_ASSERT(false);
		return index1 < index2;
	}

	const AppSignalParam& o1 = (*pThis->m_appSignalParamVec)[index1];
	const AppSignalParam& o2 = (*pThis->m_appSignalParamVec)[index2];

	switch (m_sortColumn)
	{
	case Columns::SignalID:
		{
			v1 = o1.customSignalId();
			v2 = o2.customSignalId();
		}
		break;
	case Columns::AppSignalID:
		{
			v1 = o1.appSignalId();
			v2 = o2.appSignalId();
		}
		break;
	case Columns::EquipmentID:
		{
			v1 = o1.equipmentId();
			v2 = o2.equipmentId();
		}
		break;
	case Columns::Caption:
		{
			v1 = o1.caption();
			v2 = o2.caption();
		}
		break;

	case Columns::Channel:
		{
			v1 = static_cast<int>(o1.channel());
			v2 = static_cast<int>(o2.channel());
		}
		break;

	case Columns::Type:
		{
			if (o1.isDiscrete() == true || o2.isDiscrete() == true)
			{
				v1 = static_cast<int>(o1.inOutType());
				v2 = static_cast<int>(o2.inOutType());
				break;
			}

			if (o1.type() == o2.type())
			{
				if (o1.analogSignalFormat() == o2.analogSignalFormat())
				{
					v1 = static_cast<int>(o1.inOutType());
					v2 = static_cast<int>(o2.inOutType());
				}
				else
				{
					v1 = static_cast<int>(o1.analogSignalFormat());
					v2 = static_cast<int>(o2.analogSignalFormat());
				}
			}
			else
			{
				v1 = static_cast<int>(o1.type());
				v2 = static_cast<int>(o2.type());
			}
		}
		break;

	case Columns::Units:
		{
			v1 = o1.unit();
			v2 = o2.unit();
		}
		break;

	case Columns::LowValidRange:
		{
			v1 = o1.lowValidRange();
			v2 = o2.lowValidRange();
		}
		break;

	case Columns::HighValidRange:
		{
			v1 = o1.highValidRange();
			v2 = o2.highValidRange();
		}
		break;

	case Columns::LowEngineeringUnits:
		{
			v1 = o1.lowEngineeringUnits();
			v2 = o2.lowEngineeringUnits();
		}
		break;

	case Columns::HighEngineeringUnits:
		{
			v1 = o1.highEngineeringUnits();
			v2 = o2.highEngineeringUnits();
		}
		break;

	case Columns::EnableTuning:
		{
			v1 = o1.enableTuning();
			v2 = o2.enableTuning();
		}
		break;

	case Columns::TuningDefaultValue:
		{
			v1 = o1.tuningDefaultValue().toDouble();
			v2 = o2.tuningDefaultValue().toDouble();
		}
		break;

	default:
		{
			Q_ASSERT(false);
			v1 = index1;
			v2 = index2;
		}
	}

	if (v1 == v2)
	{
		return index1 < index2;
	}

	if (m_sortOrder == Qt::AscendingOrder)
		return v1 < v2;
	else
		return v1 > v2;
}

//
// SignalSearchItemModel
//

SignalSearchItemModel::SignalSearchItemModel(QObject *parent):
	QAbstractItemModel(parent)
{
	// Fill column names
	//
	m_columnsNames << tr("Signal ID");
	m_columnsNames << tr("Caption");

	return;
}

QModelIndex SignalSearchItemModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
	{
		return QModelIndex();
	}

	if (parent.isValid() == false)	// only top elements are used
	{
		return createIndex(row, column);
	}

	Q_ASSERT(false);
	return QModelIndex();
}

QModelIndex SignalSearchItemModel::parent(const QModelIndex &index) const
{
	Q_UNUSED(index);
	return QModelIndex();

}

int SignalSearchItemModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (int)m_columnsNames.size();

}

int SignalSearchItemModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return (int)m_signals.size();
}

QVariant SignalSearchItemModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole)
	{
		int col = index.column();
		if (col < 0 || col >= m_columnsNames.size())
		{
			Q_ASSERT(false);
			return QVariant();
		}

		int row = index.row();
		if (row >= m_signals.size())
		{
			Q_ASSERT(false);
			return QVariant();
		}

		Columns displayIndex = static_cast<Columns>(col);

		//
		// Get signal now
		//

		const AppSignalParam& s = m_signals[row];

		switch (displayIndex)
		{
		case Columns::SignalID:
		{
			return s.customSignalId();
		}

		case Columns::Caption:
		{
			return s.caption();
		}

		default:
			Q_ASSERT(false);
		}

		return QVariant();
	} // End of if (role == Qt::DisplayRole)

	return QVariant();
}

QVariant SignalSearchItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
	{
		if (section < 0 || section >= m_columnsNames.size())
		{
			Q_ASSERT(false);
			return QVariant();
		}

		return m_columnsNames.at(section);
	}

	return QVariant();
}

AppSignalParam SignalSearchItemModel::getSignal(const QModelIndex& index) const
{
	int row = index.row();
	if (row >= m_signals.size())
	{
		Q_ASSERT(false);
		return AppSignalParam();
	}

	return m_signals[row];
}

void SignalSearchItemModel::setSignals(std::vector<AppSignalParam>* signalsVector)
{
	if (signalsVector == nullptr)
	{
		Q_ASSERT(signalsVector);
		return;
	}

	if (rowCount() > 0)
	{
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);

		removeRows(0, rowCount());

		m_signals.clear();

		endRemoveRows();
	}

	if (signalsVector->empty() == true)
	{
		return;
	}

	//

	beginInsertRows(QModelIndex(), 0, static_cast<int>(signalsVector->size()) - 1);

	std::swap(m_signals, *signalsVector);

	insertRows(0, static_cast<int>(m_signals.size()));

	endInsertRows();
}

//
// DialogSignalSearchSettings
//

void DialogSignalSearchSettings::restore()
{
	QSettings s;

	pos = s.value("DialogSignalSearch/pos", QPoint(-1, -1)).toPoint();
	geometry = s.value("DialogSignalSearch/geometry").toByteArray();
	columnCount = s.value("DialogSignalSearch/columnCount").toInt();
	columnWidth = s.value("DialogSignalSearch/columnWidth").toByteArray();
}

void DialogSignalSearchSettings::store()
{
	QSettings s;

	s.setValue("DialogSignalSearch/pos", pos);
	s.setValue("DialogSignalSearch/geometry", geometry);
	s.setValue("DialogSignalSearch/columnCount", columnCount);
	s.setValue("DialogSignalSearch/columnWidth", columnWidth);
}

//
// DialogSignalSearch
//

QString DialogSignalSearch::m_signalId;

DialogSignalSearch::DialogSignalSearch(QWidget *parent, IAppSignalManager* appSignalManager) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint),
	m_model(this),
	m_appSignalManager(appSignalManager)
{
	setAttribute(Qt::WA_DeleteOnClose);

	setWindowTitle(tr("Signal Search"));

	if(m_appSignalManager == nullptr)
	{
		Q_ASSERT(m_appSignalManager);
		return;
	}

	m_signals = m_appSignalManager->signalList();

	// Setup UI
	//
	QLabel* l = new QLabel("SignalID");
	m_editSignalID = new QLineEdit();
	connect(m_editSignalID, &QLineEdit::textEdited, this, &DialogSignalSearch::textEdited);

	QHBoxLayout* hl = new QHBoxLayout();
	hl->addWidget(l);
	hl->addWidget(m_editSignalID);

	m_tableView = new QTableView();
	m_tableView->horizontalHeader()->setHighlightSections(false);

	m_labelFound = new QLabel();

	QHBoxLayout* bl = new QHBoxLayout();
	bl->addWidget(m_labelFound);

	bl->addStretch();

	QPushButton* b = new QPushButton(tr("Open"));
	connect(b, &QPushButton::clicked, this, &DialogSignalSearch::openClicked);
	bl->addWidget(b);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(hl);
	mainLayout->addWidget(m_tableView);
	mainLayout->addLayout(bl);

	setLayout(mainLayout);

	connect(this, &DialogSignalSearch::finished, this, &DialogSignalSearch::finished);

	setMinimumSize(400, 450);

	// set model
	//
	m_tableView->setModel(&m_model);
	m_tableView->verticalHeader()->hide();
	m_tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	m_tableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	m_tableView->horizontalHeader()->setStretchLastSection(false);
	m_tableView->setGridStyle(Qt::PenStyle::NoPen);
	m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_tableView, &QTreeView::customContextMenuRequested,this, &DialogSignalSearch::prepareContextMenu);
	connect(m_tableView, &QTableView::doubleClicked, this, &DialogSignalSearch::tableDoubleClicked);

	int fontHeight = fontMetrics().height() + 4;

	QHeaderView *verticalHeader = m_tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(fontHeight);

	// Restore window pos
	//
	DialogSignalSearchSettings settings;
	settings.restore();

	if (settings.pos.x() != -1 && settings.pos.y() != -1)
	{
		move(settings.pos);
		restoreGeometry(settings.geometry);
	}

	// Restore columns width
	//
	QDataStream stream(&settings.columnWidth, QIODevice::ReadOnly);

	for (int i = 0; i < settings.columnCount; i++)
	{
		int width;
		stream >> width;
		m_tableView->setColumnWidth(i, width);
	}

	//

	m_editSignalID->setText(m_signalId);
	m_editSignalID->setPlaceholderText(tr("Enter SignalID here"));

	// Sort m_signals by SignalId
	//

	int signalsCount = static_cast<int>(m_signals.size());

	std::vector<int> signalsIndexVector;	// Index vector for m_signals
	signalsIndexVector.resize(signalsCount);

	for (int i = 0; i < signalsCount; i++)
	{
		signalsIndexVector[i] = i;
	}

	std::sort(signalsIndexVector.begin(), signalsIndexVector.end(), SignalSearchSorter(&m_signals));

	std::vector<AppSignalParam> sortedSignals;
	sortedSignals.resize(signalsCount);

	for (int i = 0; i < signalsCount; i++)
	{
		sortedSignals[i] = m_signals[signalsIndexVector[i]];
	}

	m_signals.swap(sortedSignals);

	//


	search();
}

DialogSignalSearch::~DialogSignalSearch()
{
}

void DialogSignalSearch::signalsUpdated()
{
	m_signals = m_appSignalManager->signalList();

	m_editSignalID->blockSignals(true);
	m_editSignalID->setText(QString());
	m_editSignalID->blockSignals(false);

	m_signalId.clear();
	search();

	return;
}

void DialogSignalSearch::textEdited(const QString &arg1)
{
	m_signalId = arg1;
	search();

	return;
}

void DialogSignalSearch::search()
{
	std::vector<AppSignalParam> foundSignals;

	if (m_signalId.isEmpty() == true)
	{
		foundSignals = m_signals;
	}
	else
	{
		for (const AppSignalParam& s : m_signals)
		{
			if (s.customSignalId().contains(m_signalId, Qt::CaseInsensitive) == false)
			{
				continue;
			}
			foundSignals.push_back(s);
		}
	}

	m_labelFound->setText(QString("Signals found: %1").arg(foundSignals.size()));

	m_model.setSignals(&foundSignals);
}

void DialogSignalSearch::finished(int result)
{
	Q_UNUSED(result);

	// Save columns width
	//
	DialogSignalSearchSettings settings;

	QDataStream stream(&settings.columnWidth, QIODevice::WriteOnly);

	settings.columnCount = m_model.columnCount();

	for (int i = 0; i < settings.columnCount; i++)
	{
		stream << (int)m_tableView->columnWidth(i);
	}

	// Save window position
	//
	settings.pos = pos();
	settings.geometry = saveGeometry();

	settings.store();
}

void DialogSignalSearch::openClicked()
{
	QModelIndex index = m_tableView->currentIndex();
	if (index.isValid() == false)
	{
		return;
	}

	tableDoubleClicked(index);

	return;
}

void DialogSignalSearch::tableDoubleClicked(const QModelIndex &index)
{
	if (index.isValid() == false)
	{
		return;
	}

	const AppSignalParam& signal = m_model.getSignal(index);

	emit signalInfo(signal.appSignalId());

	return;
}

void DialogSignalSearch::prepareContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	QModelIndex index = m_tableView->indexAt(pos);
	if (index.isValid() == false)
	{
		return;
	}

	const AppSignalParam& signal = m_model.getSignal(index);

	QStringList list;
	list << signal.appSignalId();

	emit signalContextMenu(list);

	return;
}

DialogSignalSearchSettings theDialogSignalSearchSettings;
