#include "EditSchemaWidget.h"

#include "SchemaLayersDialog.h"
#include "ui_SchemaLayersDialog.h"

SchemaLayersDialog::SchemaLayersDialog(EditSchemaView* schemaView, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::SchemaLayersDialog),
	m_schemaView(schemaView)
{
	ui->setupUi(this);

	if (schemaView == nullptr)
	{
		Q_ASSERT(schemaView);
		return;
	}

	// Initialize list control
	//
	QStringList header;
	header << tr("Name");
	header << tr("Active");
	header << tr("Show");
	header << tr("Print");
	ui->m_layersList->setColumnCount(header.size());
	ui->m_layersList->setHeaderLabels(header);
	ui->m_layersList->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(ui->m_layersList, &QTreeWidget::customContextMenuRequested, this, &SchemaLayersDialog::onContextMenu);

	// Create context menu actions
	//
	m_activeAction = new QAction(QString("Active"), this);
	m_showAction = new QAction(QString("Show"), this);
	m_printAction = new QAction(QString("Print"), this);

	m_activeAction->setCheckable(true);
	m_showAction->setCheckable(true);
	m_printAction->setCheckable(true);

	connect(m_activeAction, &QAction::triggered, this, &SchemaLayersDialog::onActiveClick);
	connect(m_showAction, &QAction::triggered, this, &SchemaLayersDialog::onShowClick);
	connect(m_printAction, &QAction::triggered, this, &SchemaLayersDialog::onPrintClick);

	int index = 0;
	for (std::shared_ptr<VFrame30::SchemaLayer> l : m_schemaView->schema()->Layers)
	{
		m_name << l->name();
		m_show << l->show();
		m_print << l->print();

		if (l->guid() == m_schemaView->activeLayer()->guid())
		{
			m_activeIndex = index;
		}
		index++;
	}

	fillList(m_activeIndex);

	return;
}

SchemaLayersDialog::~SchemaLayersDialog()
{
	delete ui;
}

void SchemaLayersDialog::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(this);
	resize(screen.width() * 0.20, screen.height() * 0.15);

	move(screen.center() - rect().center());

	// --
	//
	assert(ui->m_layersList->columnCount() == 4);

	ui->m_layersList->resizeColumnToContents(1);
	ui->m_layersList->resizeColumnToContents(2);
	ui->m_layersList->resizeColumnToContents(3);

	return;
}

void SchemaLayersDialog::fillList(int selectedIndex)
{
	// Fill list and select the item with "selectedLayer" guid
	//
	ui->m_layersList->clear();

	QTreeWidgetItem* selectedItem = nullptr;

	QList<QTreeWidgetItem*> items;
	for (int i = 0; i < m_name.size(); i++)
	{
		QStringList s;
		s << m_name[i];
		s << QString(i == m_activeIndex ? tr("Yes") : tr(""));
		s << QString(m_show[i] ? tr("Yes") : tr(""));
		s << QString(m_print[i] ? tr("Yes") : tr(""));

		QTreeWidgetItem* item = new QTreeWidgetItem((QTreeWidget*)0, s);
		item->setData(0, Qt::UserRole, i);
		if (i == selectedIndex)
		{
			selectedItem = item;
		}
		items.append(item);
	}

	ui->m_layersList->insertTopLevelItems(0, items);

	if (selectedItem != nullptr)
	{
		ui->m_layersList->setCurrentItem(selectedItem);
	}
}

void SchemaLayersDialog::onContextMenu(const QPoint &pos)
{
	QTreeWidgetItem* item = ui->m_layersList->itemAt(pos);
	if (item == nullptr)
	{
		return;
	}

	m_cmIndex = item->data(0, Qt::UserRole).toInt();
	if (m_cmIndex == -1 || m_cmIndex >= m_show.size() || m_cmIndex >= m_print.size())
	{
		assert(false);
		return;
	}

	QMenu* menu = new QMenu(this);

	m_activeAction->setChecked(m_cmIndex == m_activeIndex);
	m_activeAction->setEnabled(m_show[m_cmIndex]);

	m_showAction->setChecked(m_show[m_cmIndex]);
	m_showAction->setEnabled(m_cmIndex != m_activeIndex);

	m_printAction->setChecked(m_print[m_cmIndex]);

	menu->addAction(m_activeAction);
	menu->addAction(m_showAction);
	menu->addAction(m_printAction);

	menu->exec(this->cursor().pos());
}


void SchemaLayersDialog::onActiveClick(bool /*checked*/)
{
	if (m_cmIndex < 0 || m_cmIndex >= m_name.size())
	{
		assert(false);
		return;
	}

	m_activeIndex = m_cmIndex;
	fillList(m_cmIndex);
}

void SchemaLayersDialog::onShowClick(bool checked)
{
	if (m_cmIndex < 0 || m_cmIndex >= m_show.size())
	{
		assert(false);
		return;
	}

	m_show[m_cmIndex] = checked;
	fillList(m_cmIndex);
}

void SchemaLayersDialog::onPrintClick(bool checked)
{
	if (m_cmIndex < 0 || m_cmIndex >= m_print.size())
	{
		assert(false);
		return;
	}

	m_print[m_cmIndex] = checked;
	fillList(m_cmIndex);
}

void SchemaLayersDialog::on_m_layersList_itemDoubleClicked(QTreeWidgetItem *item, int/* column*/)
{
	if (item == nullptr)
	{
		return;
	}

	m_cmIndex = item->data(0, Qt::UserRole).toInt();

	if (m_cmIndex < 0 || m_cmIndex >= m_name.size())
	{
		assert(false);
		return;
	}

	if (m_cmIndex < 0 || m_cmIndex >= m_show.size())
	{
		assert(false);
		return;
	}


	m_activeIndex = m_cmIndex;
	m_show[m_cmIndex] = true;

	accept();
}

void SchemaLayersDialog::on_SchemaLayersDialog_accepted()
{
	int index = 0;
	for (std::shared_ptr<VFrame30::SchemaLayer> l : m_schemaView->schema()->Layers)
	{
		l->setShow(m_show[index]);
		l->setPrint(m_print[index]);
		if (index == m_activeIndex)
		{
			m_schemaView->setActiveLayer(l);
		}

		index++;
	}
}
