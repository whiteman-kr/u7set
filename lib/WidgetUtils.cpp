#include "WidgetUtils.h"
#include <QStandardItemModel>
#include <QSettings>
#include <QApplication>
#include <QTableView>
#include <QHeaderView>
#include <QListView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>
#include <QDesktopWidget>
#include <QScreen>

void saveWindowPosition(QWidget* window, QString widgetKey)
{
	QSettings settings;
	int screenNumber = QApplication::desktop()->screenNumber(window);
	settings.setValue(widgetKey + "/screenNumber", screenNumber);
	settings.setValue(widgetKey + "/geometry", window->geometry());
}

void setWindowPosition(QWidget* window, QString widgetKey)
{
	if (window == nullptr)
	{
		return;
	}
	QSettings settings;
	int screenNumber = settings.value(widgetKey + "/screenNumber", QApplication::desktop()->screenNumber(window)).toInt();

	if (QGuiApplication::screens().size() <= screenNumber)
	{
		return;
	}
	QScreen* currentScreen = QGuiApplication::screens()[screenNumber];
	if (currentScreen == nullptr)
	{
		return;
	}
	QRect screenRect = currentScreen->geometry();
	QPoint center = screenRect.center();

	QRect baseWindowRect = screenRect;
	baseWindowRect.setSize(QSize(screenRect.width() * 2 / 3, screenRect.height() * 2 / 3));
	baseWindowRect.moveCenter(center);

	QRect windowRect = settings.value(widgetKey + "/geometry", baseWindowRect).toRect();

	if (windowRect.height() > screenRect.height())
	{
		windowRect.setHeight(screenRect.height());
	}
	if (windowRect.width() > screenRect.width())
	{
		windowRect.setWidth(screenRect.width());
	}

	if (windowRect.left() < 0)
	{
		windowRect.moveLeft(0);
	}
	if (windowRect.left() + windowRect.width() > screenRect.right())
	{
		windowRect.moveLeft(screenRect.right() - windowRect.width());
	}

	if (windowRect.top() < 0)
	{
		windowRect.moveTop(0);
	}
	if (windowRect.top() + windowRect.height() > screenRect.bottom())
	{
		windowRect.moveTop(screenRect.bottom() - windowRect.height());
	}

	window->setGeometry(windowRect);
}

TableDataVisibilityController::TableDataVisibilityController(QTableView* parent, const QString& settingsBranchName, const QVector<int>& defaultVisibleColumnSet, bool showAllDefaultColumns) :
	QObject(parent->horizontalHeader()),
	m_tableView(parent),
	m_settingBranchName(settingsBranchName),
	m_defaultVisibleColumnSet(defaultVisibleColumnSet),
	m_showAllDefaultColumns(showAllDefaultColumns)
{
	connect(parent, &QObject::destroyed, this, &TableDataVisibilityController::saveAllHeaderGeomery, Qt::DirectConnection);

	QHeaderView* horizontalHeader = m_tableView->horizontalHeader();

	horizontalHeader->setContextMenuPolicy(Qt::ActionsContextMenu);
	horizontalHeader->setSectionsMovable(true);

	checkNewColumns();

	QAction* columnsAction = new QAction("Rearrange columns", m_tableView);
	connect(columnsAction, &QAction::triggered, this, &TableDataVisibilityController::editColumnsVisibilityAndOrder);
	horizontalHeader->addAction(columnsAction);
	connect(horizontalHeader, &QHeaderView::sectionResized, this, &TableDataVisibilityController::saveColumnWidth);
}

TableDataVisibilityController::~TableDataVisibilityController()
{
}

void TableDataVisibilityController::editColumnsVisibilityAndOrder()
{
	EditColumnsVisibilityDialog dlg(m_tableView, this);

	//Window geometry
	//
	setWindowPosition(&dlg, m_settingBranchName + "ColumnsVisibilityDialog");

	dlg.exec();

	saveWindowPosition(&dlg, m_settingBranchName + "ColumnsVisibilityDialog");
	saveAllHeaderGeomery();
}

void TableDataVisibilityController::saveColumnWidth(int index)
{
	int width = m_tableView->columnWidth(index);
	if (width == 0)
	{
		return;
	}

	QSettings settings;
	settings.setValue((m_settingBranchName + "/ColumnWidth/%1").arg(m_columnNameList[index].replace("/", "|")).replace("\n", " "), width);
}

void TableDataVisibilityController::saveAllHeaderGeomery()
{
	if (m_tableView->isEnabled() == false)
	{
		return;
	}
	auto header = m_tableView->horizontalHeader();
	int columnCount = m_columnNameList.count();
	for (int i = 0; i < columnCount; i++)
	{
		bool visible = !header->isSectionHidden(i);
		if (visible)
		{
			saveColumnWidth(i);
		}
		saveColumnVisibility(i, visible);

		saveColumnPosition(i, header->visualIndex(i));
	}
}

void TableDataVisibilityController::checkNewColumns()
{
	auto* model = m_tableView->model();

	int columnCount = m_columnNameList.count();
	int newColumnCount = model->columnCount();

	if (columnCount == newColumnCount)
	{
		return;
	}

	QSettings settings;
	QHeaderView* horizontalHeader = m_tableView->horizontalHeader();

	m_columnNameList.clear();
	m_columnNameList.reserve(newColumnCount);

	for (int i = 0; i < newColumnCount; i++)
	{
		QString columnName = model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();
		if (m_columnNameList.contains(columnName))
		{
			assert(false);	// Columns should be named differently
		}
		m_columnNameList.push_back(columnName);

		columnName = columnName.replace("/", "|").replace("\n", " ");
		int columnWidth = settings.value(m_settingBranchName + "/ColumnWidth/" + columnName, -1).toInt();
		if (columnWidth == -1)
		{
			m_tableView->resizeColumnToContents(i);
		}
		else
		{
			m_tableView->setColumnWidth(i, columnWidth);
		}

		bool visible = m_defaultVisibleColumnSet.contains(i);
		if (m_showAllDefaultColumns == false)
		{
			visible = settings.value(m_settingBranchName + "/ColumnVisibility/" + columnName, visible).toBool();
		}
		horizontalHeader->setSectionHidden(i, !visible);
	}

	relocateAllColumns();
}

void TableDataVisibilityController::saveColumnVisibility(int index, bool visible)
{
	QSettings settings;
	settings.setValue((m_settingBranchName + "/ColumnVisibility/%1").arg(m_columnNameList[index].replace("/", "|")).replace("\n", " "), visible);
}

void TableDataVisibilityController::saveColumnPosition(int index, int position)
{
	QSettings settings;
	settings.setValue((m_settingBranchName + "/ColumnPosition/%1").arg(m_columnNameList[index].replace("/", "|")).replace("\n", " "), position);
}

bool TableDataVisibilityController::getColumnVisibility(int index)
{
	QSettings settings;
	bool visible = m_tableView->horizontalHeader()->isSectionHidden(index);
	return settings.value((m_settingBranchName + "/ColumnVisibility/%1").arg(m_columnNameList[index].replace("/", "|")).replace("\n", " "), visible).toBool();
}

int TableDataVisibilityController::getColumnPosition(int index)
{
	QSettings settings;
	int position = m_tableView->horizontalHeader()->visualIndex(index);
	return settings.value((m_settingBranchName + "/ColumnPosition/%1").arg(m_columnNameList[index].replace("/", "|")).replace("\n", " "), position).toInt();
}

int TableDataVisibilityController::getColumnWidth(int index)
{
	QSettings settings;
	int width = m_tableView->columnWidth(index);
	return settings.value((m_settingBranchName + "/ColumnWidth/%1").arg(m_columnNameList[index].replace("/", "|")).replace("\n", " "), width).toInt();
}

void TableDataVisibilityController::showColumn(int index, bool visible)
{
	m_tableView->horizontalHeader()->setSectionHidden(index, !visible);
}

void TableDataVisibilityController::relocateAllColumns()
{
	QSettings settings;
	QHeaderView* horizontalHeader = m_tableView->horizontalHeader();
	std::vector<std::pair<int, int>> index2position;

	for (int i = 0; i < m_columnNameList.count(); i++)
	{
		QString columnName = QString("%1").arg(m_columnNameList[i].replace("/", "|")).replace("\n", " ");
		int position = settings.value(m_settingBranchName + "/ColumnPosition/" + columnName, -1).toInt();
		if (position == -1)
		{
			continue;
		}
		index2position.push_back(std::make_pair(i, position));
	}

	std::sort(index2position.begin(), index2position.end(), [](std::pair<int, int> v1, std::pair<int, int> v2){ return v1.second < v2.second; });

	for (size_t i = 0; i < index2position.size(); i++)
	{
		auto i2p = index2position[i];
		int oldVisualIndex = horizontalHeader->visualIndex(i2p.first);
		int newVisualIndex = i2p.second;

		horizontalHeader->moveSection(oldVisualIndex, newVisualIndex);
	}
}

EditColumnsVisibilityDialog::EditColumnsVisibilityDialog(QTableView* tableView, TableDataVisibilityController* controller) :
	QDialog(tableView, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
	m_controller(controller)
{
	setWindowTitle("Rearrange Columns");

	m_columnModel = new QStandardItemModel(this);
	m_header = tableView->horizontalHeader();
	for (int i = 0; i < m_header->count(); i++)
	{
		auto item = new QStandardItem;
		item->setCheckable(true);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		m_columnModel->setItem(i, item);
	}

	m_tableModel = tableView->model();
	if (m_tableModel == nullptr)
	{
		assert(false);
		return;
	}

	// Child widgets layout
	//
	m_columnList = new QListView(this);
	m_columnList->setModel(m_columnModel);
	m_columnList->setSelectionMode(QAbstractItemView::ExtendedSelection);
	m_columnList->setCurrentIndex(m_columnModel->index(0,0));
	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(m_columnList);
	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);
	QPushButton* upButton = new QPushButton("Up", this);
	vl->addWidget(upButton);
	QPushButton* downButton = new QPushButton("Down", this);
	vl->addWidget(downButton);
	vl->addStretch();
	setLayout(hl);

	// Show/Hide column
	//
	connect(m_columnModel, &QStandardItemModel::itemChanged, this, &EditColumnsVisibilityDialog::changeVisibility, Qt::DirectConnection);

	// Move column left (move item up)
	//
	connect(upButton, &QPushButton::pressed, this, &EditColumnsVisibilityDialog::moveUp);

	// Move column right (move item down)
	//
	connect(downButton, &QPushButton::pressed, this, &EditColumnsVisibilityDialog::moveDown);

	// Update state of items from signal table header
	//
	updateItems();
}

void EditColumnsVisibilityDialog::updateItems(QList<int> selectedLogicalIndexes, int currentLogicalIndex)
{
	QItemSelectionModel* selectionModel = m_columnList->selectionModel();
	selectionModel->clearSelection();

	for (int i = 0; i < m_header->count(); i++)
	{
		int logicalIndex = m_header->logicalIndex(i);
		updateHidden(i, isHidden(logicalIndex));
		QModelIndex index = m_columnModel->index(i, 0);
		m_columnModel->setData(index, m_tableModel->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString().replace('\n', ' '), Qt::DisplayRole);

		if (logicalIndex == currentLogicalIndex)
		{
			selectionModel->select(index, QItemSelectionModel::Current);
		}

		if (selectedLogicalIndexes.contains(logicalIndex))
		{
			selectionModel->select(index, QItemSelectionModel::Select);
		}
	}
}

bool EditColumnsVisibilityDialog::isHidden(int logicalIndex)
{
	return m_header->isSectionHidden(logicalIndex) || m_header->sectionSize(logicalIndex) == 0;
}

void EditColumnsVisibilityDialog::updateHidden(int visualIndex, bool hidden)
{
	m_columnModel->setData(m_columnModel->index(visualIndex, 0), hidden ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
}

void EditColumnsVisibilityDialog::setHidden(int logicalIndex, bool hidden)
{
	m_header->setSectionHidden(logicalIndex, hidden);
	if (!hidden && m_header->sectionSize(logicalIndex) == 0)
	{
		m_header->resizeSection(logicalIndex, m_header->defaultSectionSize());
	}
}

void EditColumnsVisibilityDialog::moveUp()
{
	m_changingItems = true;
	QList<int> selectedIndexes;
	QList<int> selectedLogicalIndexes;
	int currentLogicalIndex = -1;

	QModelIndexList&& list = m_columnList->selectionModel()->selectedIndexes();

	selectedIndexes.reserve(list.size());
	selectedLogicalIndexes.reserve(list.size());

	foreach (const QModelIndex& index, list)
	{
		selectedIndexes.push_back(index.row());
		selectedLogicalIndexes.push_back(m_header->logicalIndex(index.row()));
	}

	QModelIndex currentIndex = m_columnList->currentIndex();
	if (currentIndex.isValid())
	{
		currentLogicalIndex = m_header->logicalIndex(currentIndex.row());
	}

	std::sort(selectedIndexes.begin(), selectedIndexes.end());

	for (int i = 0; i < selectedIndexes.count(); i++)
	{
		if (selectedIndexes[i] == 0)
		{
			continue;
		}

		if (i == 0 || selectedIndexes[i - 1] != selectedIndexes[i] - 1)
		{
			m_header->moveSection(selectedIndexes[i], selectedIndexes[i] - 1);
			selectedIndexes[i]--;
		}
	}

	updateItems(selectedLogicalIndexes, currentLogicalIndex);
	m_changingItems = false;
}

void EditColumnsVisibilityDialog::moveDown()
{
	m_changingItems = true;
	QList<int> selectedIndexes;
	QList<int> selectedLogicalIndexes;
	int currentLogicalIndex = -1;

	QModelIndexList&& list = m_columnList->selectionModel()->selectedIndexes();

	selectedIndexes.reserve(list.size());
	selectedLogicalIndexes.reserve(list.size());

	foreach (const QModelIndex& index, list)
	{
		selectedIndexes.push_back(index.row());
		selectedLogicalIndexes.push_back(m_header->logicalIndex(index.row()));
	}

	QModelIndex currentIndex = m_columnList->currentIndex();
	if (currentIndex.isValid())
	{
		currentLogicalIndex = m_header->logicalIndex(currentIndex.row());
	}

	std::sort(selectedIndexes.begin(), selectedIndexes.end(), std::greater<int>());

	for (int i = 0; i < selectedIndexes.count(); i++)
	{
		if (selectedIndexes[i] == m_columnModel->rowCount() - 1)
		{
			continue;
		}

		if (i == 0 || selectedIndexes[i - 1] != selectedIndexes[i] + 1)
		{
			m_header->moveSection(selectedIndexes[i], selectedIndexes[i] + 1);
			selectedIndexes[i]++;
		}
	}

	updateItems(selectedLogicalIndexes, currentLogicalIndex);
	m_changingItems = false;
}

void EditColumnsVisibilityDialog::changeVisibility(QStandardItem* item)
{
	if (m_changingItems == true)
	{
		return;
	}

	// Apply visibility for current item
	//
	int visualIndex = item->row();
	int logicalIndex = m_header->logicalIndex(visualIndex);
	setHidden(logicalIndex, item->checkState() != Qt::Checked);

	m_controller->saveColumnVisibility(logicalIndex, item->checkState() == Qt::Checked);

	// In case if user selected multiple items
	//
	m_changingItems = true;
	QModelIndexList&& list = m_columnList->selectionModel()->selectedIndexes();
	foreach(const QModelIndex& index, list)
	{
		if (index.row() == visualIndex)
		{
			continue;
		}

		logicalIndex = m_header->logicalIndex(index.row());

		QStandardItem* selectedItem = m_columnModel->item(index.row());
		if (selectedItem->checkState() == Qt::Checked)
		{
			selectedItem->setCheckState(Qt::Unchecked);

			setHidden(logicalIndex, true);
			m_controller->saveColumnVisibility(logicalIndex, false);
		}
		else
		{
			selectedItem->setCheckState(Qt::Checked);

			setHidden(logicalIndex, false);
			m_controller->saveColumnVisibility(logicalIndex, true);
		}
	}
	m_changingItems = false;

	//Check if no visible column left
	//
	for (int i = 0; i < m_columnModel->rowCount(); i++)
	{
		if (isHidden(i) == false)
		{
			return;
		}
	}

	m_changingItems = true;
	setHidden(0, false);
	m_controller->saveColumnVisibility(0, true);
	updateHidden(m_header->visualIndex(0), false);
	m_changingItems = false;
}
