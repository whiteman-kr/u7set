#include "DialogSignalSearch.h"
#include "ui_DialogSignalSearch.h"
#include "MonitorMainWindow.h"
#include "MonitorCentralWidget.h"
#include "Stable.h"
#include "Settings.h"

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

	assert(false);
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
			assert(false);
			return QVariant();
		}

		int row = index.row();
		if (row >= m_signals.size())
		{
			assert(false);
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
			assert(false);
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
			assert(false);
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
		assert(false);
		return AppSignalParam();
	}

	return m_signals[row];
}

void SignalSearchItemModel::setSignals(std::vector<AppSignalParam>* signalsVector)
{
	if (signalsVector == nullptr)
	{
		assert(signalsVector);
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
// DialogSignalSearch
//

QString DialogSignalSearch::m_signalId = "";

DialogSignalSearch::DialogSignalSearch(QWidget *parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogSignalSearch),
	m_model(this)
{
	ui->setupUi(this);

	// Restore window pos
	//
	if (theSettings.m_signalSearchPos.x() != -1 && theSettings.m_signalSearchPos.y() != -1)
	{
		move(theSettings.m_signalSearchPos);
		restoreGeometry(theSettings.m_signalSearchGeometry);
	}

	// set model
	//
	ui->tableView->setModel(&m_model);
	ui->tableView->verticalHeader()->hide();
	ui->tableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
	ui->tableView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	ui->tableView->horizontalHeader()->setStretchLastSection(false);
	ui->tableView->setGridStyle(Qt::PenStyle::NoPen);
	ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui->tableView, &QTreeView::customContextMenuRequested,this, &DialogSignalSearch::prepareContextMenu);

	int fontHeight = fontMetrics().height() + 4;

	QHeaderView *verticalHeader = ui->tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(fontHeight);

	// Restore columns width
	//
	QDataStream stream(&theSettings.m_signalSearchColumnWidth, QIODevice::ReadOnly);

	for (int i = 0; i < theSettings.m_signalSearchColumnCount; i++)
	{
		int width;
		stream >> width;
		ui->tableView->setColumnWidth(i, width);
	}

	//

	ui->editSignalID->setText(m_signalId);
	ui->editSignalID->setPlaceholderText(tr("Enter SignalID here"));

	m_signals = theSignals.signalList();

	search();
}

DialogSignalSearch::~DialogSignalSearch()
{
	delete ui;
}

void DialogSignalSearch::on_editSignalID_textEdited(const QString &arg1)
{
	m_signalId = arg1;
	search();

}

void DialogSignalSearch::search()
{
	std::vector<AppSignalParam> foundSignals;

	for (const AppSignalParam& s : m_signals)
	{
		if (s.customSignalId().startsWith(m_signalId, Qt::CaseInsensitive) == false)
		{
			continue;
		}
		foundSignals.push_back(s);
	}

	ui->labelFound->setText(QString("Signals found: %1").arg(foundSignals.size()));

	m_model.setSignals(&foundSignals);
}

void DialogSignalSearch::on_DialogSignalSearch_finished(int result)
{
	Q_UNUSED(result);

	// Save columns width
	//
	theSettings.m_signalSearchColumnWidth.clear();

	QDataStream stream(&theSettings.m_signalSearchColumnWidth, QIODevice::WriteOnly);

	theSettings.m_signalSearchColumnCount = m_model.columnCount();
	for (int i = 0; i < theSettings.m_signalSearchColumnCount; i++)
	{
		stream << (int)ui->tableView->columnWidth(i);
	}

	// Save window position
	//
	theSettings.m_signalSearchPos = pos();
	theSettings.m_signalSearchGeometry = saveGeometry();
}

void DialogSignalSearch::prepareContextMenu(const QPoint& pos)
{
	Q_UNUSED(pos);

	if (theMonitorMainWindow == nullptr)
	{
		assert(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		assert(cw);
		return;
	}

	QModelIndex index = ui->tableView->indexAt(pos);
	if (index.isValid() == false)
	{
		return;
	}

	const AppSignalParam& signal = m_model.getSignal(index);
	cw->currentTab()->signalContextMenu(QStringList() << signal.appSignalId());

	return;
}

void DialogSignalSearch::on_tableView_doubleClicked(const QModelIndex &index)
{
	if (index.isValid() == false)
	{
		return;
	}

	if (theMonitorMainWindow == nullptr)
	{
		assert(theMonitorMainWindow);
		return;
	}

	MonitorCentralWidget* cw = dynamic_cast<MonitorCentralWidget*>(theMonitorMainWindow->centralWidget());
	if (cw == nullptr)
	{
		assert(cw);
		return;
	}

	const AppSignalParam& signal = m_model.getSignal(index);
	cw->currentTab()->signalInfo(signal.appSignalId());

	return;
}
