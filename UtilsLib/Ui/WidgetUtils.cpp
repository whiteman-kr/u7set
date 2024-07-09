#ifndef UTILS_LIB_DOMAIN
#error Do not include this file in the project! Link UtilsLib instead.
#endif

#include "WidgetUtils.h"
#include "../WUtils.h"
#include <CommonLib/ConstStrings.h>

#include <QListView>
#include <QPushButton>
#include <QStandardItemModel>

void saveWindowPosition(QWidget* window, QString widgetKey)
{
	QSettings settings;
	QString screenSerialNumber = window->screen()->serialNumber();

	settings.setValue(widgetKey + "/screenSerialNumber", screenSerialNumber);
	settings.setValue(widgetKey + "/geometry", window->geometry());
}

void setWindowPosition(QWidget* window, QString widgetKey)
{
	if (window == nullptr)
	{
		return;
	}

	QSettings settings;
	QString screenSerialNumber = settings.value(widgetKey + "/screenSerialNumber").toString();

	QList<QScreen*> systemScreens = QGuiApplication::screens();
	if (systemScreens.empty() == true)
	{
		return;
	}

	QScreen* currentScreen = nullptr;

	for (auto s : systemScreens)
	{
		if (s->serialNumber() == screenSerialNumber)
		{
			currentScreen = s;
			break;
		}
	}

	if (currentScreen == nullptr)
	{
		currentScreen = systemScreens[0];
	}

	Q_ASSERT(currentScreen);

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

// -------------------------------------------------------------------------------------------------------
//
// TableDataVisibilityController::ColumnInfo class implementation
//
// -------------------------------------------------------------------------------------------------------

const QString TableDataVisibilityController::ColumnInfo::VISIBLE("visible");
const QString TableDataVisibilityController::ColumnInfo::HIDDEN("hidden");

QString TableDataVisibilityController::ColumnInfo::columnName() const
{
	return m_columnName;
}

void TableDataVisibilityController::ColumnInfo::setColumnName(const QString& colName)
{
	Q_ASSERT(colName.isEmpty() == false);

	m_columnName = colName;

	m_settingName = colName;
	m_settingName.replace(QStringLiteral("/"), QStringLiteral("|"));
	m_settingName.replace(QStringLiteral("\n"), QStringLiteral(" "));
}

QString TableDataVisibilityController::ColumnInfo::settingName() const
{
	return m_settingName;
}

int TableDataVisibilityController::ColumnInfo::position() const
{
	return m_position;
}

void TableDataVisibilityController::ColumnInfo::setPosition(int pos)
{
	if (pos < 0)
	{
		pos = 0;
	}

	m_position = pos;
}

bool TableDataVisibilityController::ColumnInfo::visible() const
{
	return m_visible;
}

void TableDataVisibilityController::ColumnInfo::setVisible(bool visible)
{
	m_visible = visible;
}

int TableDataVisibilityController::ColumnInfo::width() const
{
	return m_width;
}

void TableDataVisibilityController::ColumnInfo::setWidth(int width)
{
	m_width = width;
}

QString TableDataVisibilityController::ColumnInfo::saveParamsToString() const
{
	return QString("%1;%2;%3").arg(m_position).arg(m_visible ? VISIBLE : HIDDEN).arg(m_width);
}

void TableDataVisibilityController::ColumnInfo::readParamsFromString(const QString& str)
{
	m_position = -1;
	m_visible = true;
	m_width = 50;

	if (str.isEmpty() == true)
	{
		return;
	}

	QStringList paramStr = str.split(Separator::SEMICOLON, Qt::KeepEmptyParts);

	bool ok = true;

	if (paramStr.size() > 0)
	{
		m_position = paramStr[0].toInt(&ok);

		if (ok == false)
		{
			m_position = -1;
		}
	}

	if (paramStr.size() > 1)
	{
		m_visible = (paramStr[1] == VISIBLE);
	}

	if (paramStr.size() > 2)
	{
		m_width = paramStr[2].toInt(&ok);

		if (ok == false)
		{
			m_width = 50;
		}
	}
}

// -------------------------------------------------------------------------------------------------------
//
// TableDataVisibilityController class implementation
//
// -------------------------------------------------------------------------------------------------------

TableDataVisibilityController::TableDataVisibilityController(QTableView* parent,
															 const QString& settingsBranchName,
															 const QVector<int>& defaultVisibleColumnSet,
															 bool showAllDefaultColumns) :
	QObject(parent->horizontalHeader()),
	m_tableView(parent),
	m_settingBranchName(settingsBranchName),
	m_defaultVisibleColumnSet(defaultVisibleColumnSet),
	m_showAllDefaultColumns(showAllDefaultColumns)
{
	QHeaderView* horizontalHeader = m_tableView->horizontalHeader();

	horizontalHeader->setContextMenuPolicy(Qt::ActionsContextMenu);
	horizontalHeader->setSectionsMovable(true);

	checkNewColumns();

	QAction* columnsAction = new QAction("Rearrange columns", m_tableView);
	connect(columnsAction, &QAction::triggered, this, &TableDataVisibilityController::editColumnsVisibilityAndOrder);
	horizontalHeader->addAction(columnsAction);

	connect(horizontalHeader, &QHeaderView::sectionResized, this, &TableDataVisibilityController::onColumnResized);
	connect(horizontalHeader, &QHeaderView::sectionMoved, this, &TableDataVisibilityController::onColumnMoved);
}

TableDataVisibilityController::~TableDataVisibilityController()
{
	// delete old-style columns settings
	//
	m_settings.remove(QString("%1/ColumnPosition").arg(m_settingBranchName));
	m_settings.remove(QString("%1/ColumnWidth").arg(m_settingBranchName));
	m_settings.remove(QString("%1/ColumnVisibility").arg(m_settingBranchName));
}

void TableDataVisibilityController::saveColumnVisibility(int index, bool visible)
{
	if (isValidColumnIndex(index) == false)
	{
		Q_ASSERT(false);
		return;
	}

	ColumnInfo& ci = m_columnsInfo[index];

	ci.setVisible(visible);

	saveColumnInfo(ci);
}

void TableDataVisibilityController::saveColumnPosition(int index, int position)
{
	if (isValidColumnIndex(index) == false)
	{
		Q_ASSERT(false);
		return;
	}

	ColumnInfo& ci = m_columnsInfo[index];

	ci.setPosition(position);

	saveColumnInfo(ci);
}

void TableDataVisibilityController::saveColumnWidth(int index, int width)
{
	if (isValidColumnIndex(index) == false)
	{
		Q_ASSERT(false);
		return;
	}

	ColumnInfo& ci = m_columnsInfo[index];

	ci.setWidth(width);

	saveColumnInfo(ci);
}

int TableDataVisibilityController::getColumnPosition(int index) const
{
	if (isValidColumnIndex(index) == false)
	{
		Q_ASSERT(false);
		return -1;
	}

	return m_columnsInfo[index].position();
}

bool TableDataVisibilityController::getColumnVisibility(int index) const
{
	if (isValidColumnIndex(index) == false)
	{
		Q_ASSERT(false);
		return false;
	}

	return m_columnsInfo[index].visible();
}

int TableDataVisibilityController::getColumnWidth(int index) const
{
	if (isValidColumnIndex(index) == false)
	{
		Q_ASSERT(false);
		return 0;
	}

	return m_columnsInfo[index].width();
}

void TableDataVisibilityController::showColumn(int index, bool visible)
{
	if (isValidColumnIndex(index) == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_columnsInfo[index].setVisible(visible);

	m_tableView->horizontalHeader()->setSectionHidden(index, !visible);
}

void TableDataVisibilityController::relocateAllColumns()
{
	QHeaderView* horizontalHeader = m_tableView->horizontalHeader();
	std::vector<std::pair<int, int>> index2position;

	for (int i = 0; i < m_columnsInfo.count(); i++)
	{
		const ColumnInfo& ci = m_columnsInfo[i];

		if (ci.position() == -1)
		{
			continue;
		}

		index2position.emplace_back(i, ci.position());
	}

	std::sort(index2position.begin(), index2position.end(),
						[](std::pair<int, int> v1, std::pair<int, int> v2)
						{
							return v1.second < v2.second;
						});

	for (size_t i = 0; i < index2position.size(); i++)
	{
		auto i2p = index2position[i];
		int oldVisualIndex = horizontalHeader->visualIndex(i2p.first);
		int newVisualIndex = i2p.second;

		horizontalHeader->moveSection(oldVisualIndex, newVisualIndex);
	}
}

void TableDataVisibilityController::onColumnResized(int index, int oldSize, int newSize)
{
	Q_UNUSED(oldSize);
	saveColumnWidth(index, newSize);
}

void TableDataVisibilityController::onColumnMoved(int index, int oldVisualIndex, int newVisualIndex)
{
	Q_UNUSED(index);
	Q_UNUSED(oldVisualIndex);
	Q_UNUSED(newVisualIndex);

	saveAllHeaderGeomery();
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

void TableDataVisibilityController::saveAllHeaderGeomery()
{
	if (m_tableView->isEnabled() == false)
	{
		return;
	}

	auto header = m_tableView->horizontalHeader();

	int columnCount = header->count();

	for (int i = 0; i < columnCount; i++)
	{
		QString columnName = header->model()->headerData(i, header->orientation(), Qt::DisplayRole).toString();

		int index = m_columnsInfo.indexOf(columnName);

		if (index == -1)
		{
			Q_ASSERT(false);
			continue;
		}

		ColumnInfo& ci = m_columnsInfo[index];

		ci.setVisible(!header->isSectionHidden(i));
		ci.setPosition(header->visualIndex(i));
		ci.setWidth(header->sectionSize(i));

		saveColumnInfo(ci);
	}
}

void TableDataVisibilityController::checkNewColumns()
{
	auto* model = m_tableView->model();

	qsizetype columnCount = m_columnsInfo.count();
	int newColumnCount = model->columnCount();

	if (columnCount == newColumnCount)
	{
		return;
	}

	QSettings settings;
	QHeaderView* horizontalHeader = m_tableView->horizontalHeader();

	m_columnsInfo.clear();
	m_columnsInfo.reserve(newColumnCount);

	for (int i = 0; i < newColumnCount; i++)
	{
		QString columnName = model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString();

		if (m_columnsInfo.contains(columnName))
		{
			Q_ASSERT(false);	// Columns should be named differently
			continue;
		}

		ColumnInfo ci;

		loadColumnInfo(columnName, &ci);

		if (ci.width() <= 0)	// Looks like invisible
		{
			ci.setWidth(50);
		}

		bool visible = m_defaultVisibleColumnSet.contains(i);

		if (m_showAllDefaultColumns == false)
		{
			visible = ci.visible();
		}

		ci.setVisible(visible);

		m_columnsInfo.insert(columnName, ci);

		m_tableView->setColumnWidth(i, ci.width());

		horizontalHeader->setSectionHidden(i, !visible);
	}

	relocateAllColumns();
}

void TableDataVisibilityController::saveColumnInfo(const ColumnInfo& ci) const
{
	m_settings.setValue(QString("%1/Columns/%2").arg(m_settingBranchName).arg(ci.settingName()),
						ci.saveParamsToString());
	m_settings.sync();
}

void TableDataVisibilityController::loadColumnInfo(const QString& columnName, ColumnInfo* ci) const
{
	TEST_PTR_RETURN(ci);

	ci->setColumnName(columnName);

	QString paramsStr = m_settings.value(QString("%1/Columns/%2").arg(m_settingBranchName).arg(ci->settingName()),
								QString()).toString();

	if (paramsStr.isEmpty() == false)
	{
		ci->readParamsFromString(paramsStr);
	}
	else
	{
		// try read old format settings
		//
		int pos = m_settings.value(QString("%1/ColumnPosition/%2").arg(m_settingBranchName).arg(ci->settingName()), -1).toInt();
		bool visible = m_settings.value(QString("%1/ColumnVisibility/%2").arg(m_settingBranchName).arg(ci->settingName()), true).toBool();
		int width = m_settings.value(QString("%1/ColumnWidth/%2").arg(m_settingBranchName).arg(ci->settingName()), -1).toInt();

		if (width == -1)
		{
			width = 50;
		}

		ci->setPosition(pos);
		ci->setVisible(visible);
		ci->setWidth(width);

		saveColumnInfo(*ci);			// save in new format
	}
}

bool TableDataVisibilityController::isValidColumnIndex(int index) const
{
	return (index >= 0 && index < TO_INT(m_columnsInfo.size()));
}
// -------------------------------------------------------------------------------------------------------
//
// EditColumnsVisibilityDialog class implementation
//
// -------------------------------------------------------------------------------------------------------

EditColumnsVisibilityDialog::EditColumnsVisibilityDialog(QTableView* tableView, TableDataVisibilityController* controller) :
	QDialog(tableView, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint),
	m_controller(controller)
{
	setWindowTitle("Rearrange Columns");

	m_columnModel = new QStandardItemModel(this);
	m_header = tableView->horizontalHeader();

	int columnsCount = m_header->count();

	for (int i = 0; i < columnsCount; i++)
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

	bool currentChecked = item->checkState() == Qt::Checked;

	setHidden(logicalIndex, !currentChecked);

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

		selectedItem->setCheckState(currentChecked ? Qt::Checked : Qt::Unchecked);
		setHidden(logicalIndex, !currentChecked);
		m_controller->saveColumnVisibility(logicalIndex, currentChecked);
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
