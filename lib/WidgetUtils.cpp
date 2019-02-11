#include "WidgetUtils.h"
#include <QDialog>
#include <QStandardItemModel>
#include <QSettings>
#include <QApplication>
#include <QTableView>
#include <QHeaderView>
#include <QListView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QAction>

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

	QRect screenRect = QApplication::desktop()->screenGeometry(screenNumber);
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

TableDataVisibilityController::TableDataVisibilityController(QTableView* parent, QString settingsBranchName) :
	QObject(parent->horizontalHeader()),
	m_signalsView(parent),
	m_settingBranchName(settingsBranchName)
{
	auto* model = m_signalsView->model();
	int columnCount = model->columnCount();
	m_columnNameList.reserve(columnCount);
	for (int i = 0; i < columnCount; i++)
	{
		m_columnNameList.push_back(model->headerData(i, Qt::Horizontal, Qt::DisplayRole).toString());
	}

	QMap<int, int> positionMap;
	QSettings settings;
	QHeaderView* horizontalHeader = m_signalsView->horizontalHeader();

	for (int i = 0; i < columnCount; i++)
	{
		QString columnName = QString("%1").arg(m_columnNameList[i].replace("/", "|")).replace("\n", " ");
		m_signalsView->setColumnWidth(i, settings.value(m_settingBranchName + "/ColumnWidth/" + columnName, m_signalsView->columnWidth(i)).toInt());
		horizontalHeader->setSectionHidden(i, !settings.value(m_settingBranchName + "/ColumnVisibility/" + columnName, true).toBool());
		int position = settings.value(m_settingBranchName + "/ColumnPosition/" + columnName, i).toInt();
		positionMap.insert(position, i);
	}

	for (int i = 0; i < columnCount; i++)
	{
		int logicalIndex = positionMap[i];
		int oldVisualIndex = horizontalHeader->visualIndex(logicalIndex);
		horizontalHeader->moveSection(oldVisualIndex, i);
	}

	QAction* columnsAction = new QAction("Columns", m_signalsView);
	connect(columnsAction, &QAction::triggered, this, &TableDataVisibilityController::editColumnsVisibilityAndOrder);
	horizontalHeader->addAction(columnsAction);
	connect(horizontalHeader, &QHeaderView::sectionResized, this, &TableDataVisibilityController::saveColumnWidth);
}

TableDataVisibilityController::~TableDataVisibilityController()
{
	auto header = m_signalsView->horizontalHeader();
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

void TableDataVisibilityController::editColumnsVisibilityAndOrder()
{
	QDialog dlg(nullptr, Qt::WindowTitleHint | Qt::WindowSystemMenuHint | Qt::WindowCloseButtonHint);
	QStandardItemModel *model = new QStandardItemModel(&dlg);
	auto header = m_signalsView->horizontalHeader();
	for (int i = 0; i < header->count(); i++)
	{
		auto item = new QStandardItem;
		item->setCheckable(true);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
		model->setItem(i, item);
	}

	// Helper functions
	//
	auto isHidden = [header](int logicalIndex){
		return header->isSectionHidden(logicalIndex) || header->sectionSize(logicalIndex) == 0;
	};
	auto updateHidden = [model](int visualIndex, bool hidden) {
		model->setData(model->index(visualIndex, 0), hidden ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
	};
	auto setHidden = [header](int logicalIndex, bool hidden) {
		header->setSectionHidden(logicalIndex, hidden);
		if (!hidden && header->sectionSize(logicalIndex) == 0)
		{
			header->resizeSection(logicalIndex, header->defaultSectionSize());
		}
	};

	QAbstractItemModel* tableModel = m_signalsView->model();
	if (tableModel == nullptr)
	{
		assert(false);
		return;
	}

	// Update state of items from signal table header
	//
	auto updateItems = [=](){
		for (int i = 0; i < header->count(); i++)
		{
			int logicalIndex = header->logicalIndex(i);
			updateHidden(i, isHidden(logicalIndex));
			model->setData(model->index(i, 0), tableModel->headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString().replace('\n', ' '), Qt::DisplayRole);
		}
	};
	updateItems();

	// Child widgets layout
	//
	QListView* listView = new QListView(&dlg);
	listView->setModel(model);
	listView->setCurrentIndex(model->index(0,0));
	QHBoxLayout* hl = new QHBoxLayout;
	hl->addWidget(listView);
	QVBoxLayout* vl = new QVBoxLayout;
	hl->addLayout(vl);
	QPushButton* upButton = new QPushButton("Up", &dlg);
	vl->addWidget(upButton);
	QPushButton* downButton = new QPushButton("Down", &dlg);
	vl->addWidget(downButton);
	vl->addStretch();
	dlg.setLayout(hl);

	//Window geometry
	//
	setWindowPosition(&dlg, "ColumnsVisibilityDialog");

	// Show/Hide column
	//
	connect(model, &QStandardItemModel::itemChanged, [=](QStandardItem* item){
		int visualIndex = item->row();
		int logicalIndex = header->logicalIndex(visualIndex);
		setHidden(logicalIndex, item->checkState() != Qt::Checked);

		saveColumnVisibility(logicalIndex, item->checkState() == Qt::Checked);

		//Check if no visible column left
		//
		for (int i = 0; i < model->rowCount(); i++)
		{
			if (!isHidden(i))
			{
				return;
			}
		}
		setHidden(0, false);
		saveColumnVisibility(0, true);
		updateHidden(header->visualIndex(0), false);
	});

	// Move column left (move item up)
	//
	connect(upButton, &QPushButton::pressed, [=](){
		int visualIndex = listView->currentIndex().row();
		if (visualIndex == 0)
		{
			return;
		}

		header->moveSection(visualIndex, visualIndex - 1);

		listView->setCurrentIndex(model->index(visualIndex - 1, 0));
		updateItems();
		saveColumnPosition(header->logicalIndex(visualIndex), visualIndex);
		saveColumnPosition(header->logicalIndex(visualIndex - 1), visualIndex - 1);
	});

	// Move column right (move item down)
	//
	connect(downButton, &QPushButton::pressed, [=](){
		int visualIndex = listView->currentIndex().row();
		if (visualIndex == model->rowCount() - 1)
		{
			return;
		}

		header->moveSection(visualIndex, visualIndex + 1);

		listView->setCurrentIndex(model->index(visualIndex + 1, 0));
		updateItems();
		saveColumnPosition(header->logicalIndex(visualIndex), visualIndex);
		saveColumnPosition(header->logicalIndex(visualIndex + 1), visualIndex + 1);
	});

	dlg.exec();

	saveWindowPosition(&dlg, "ColumnsVisibilityDialog");
}

void TableDataVisibilityController::saveColumnWidth(int index)
{
	int width = m_signalsView->columnWidth(index);
	if (width == 0)
	{
		return;
	}

	QSettings settings;
	settings.setValue((m_settingBranchName + "/ColumnWidth/%1").arg(m_columnNameList[index].replace("/", "|")).replace("\n", " "), width);
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
