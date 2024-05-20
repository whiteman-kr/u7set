#include "DialogReportPageSetup.h"

using namespace Builder;

//
// DialogReportPageSetup
//

DialogReportPageSetup::DialogReportPageSetup(const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
											 const std::vector<Builder::SchemaTypesParams>& defaultFileTypeParams,
											 QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_schemaTypesParams(schemaTypesParams),
	m_defaultFileTypeParams(defaultFileTypeParams)
{
	setWindowTitle(tr("Report Sections Page Setup"));
	setMinimumSize(600, 400);

	m_treeWidget = new QTreeWidget();

	QStringList l;
	l << tr("Section");
	l << tr("Page Size");
	l << tr("Orientation");
	l << tr("Margins, mm");
	l << tr("No Margins (1:1)");
	m_treeWidget->setHeaderLabels(l);
	m_treeWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item, int column){
		Q_UNUSED(item);
		Q_UNUSED(column);
		pageSetup();
	});

	QVBoxLayout* pbLayout = new QVBoxLayout();

	QPushButton* b = new QPushButton(tr("Page Setup..."));
	connect(b, &QPushButton::clicked, this, &DialogReportPageSetup::pageSetup);
	pbLayout->addWidget(b);

	b = new QPushButton(tr("Reset to Defaults"));
	connect(b, &QPushButton::clicked, this, &DialogReportPageSetup::setToDefault);

	pbLayout->addWidget(b);
	pbLayout->addStretch();

	QHBoxLayout* topLayout = new QHBoxLayout();

	topLayout->addWidget(m_treeWidget);
	topLayout->addLayout(pbLayout);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();

	b = new QPushButton(tr("OK"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogReportPageSetup::accept);

	b = new QPushButton(tr("Cancel"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogReportPageSetup::reject);

	QVBoxLayout* ml = new QVBoxLayout();
	ml->addLayout(topLayout);
	ml->addLayout(buttonsLayout);

	setLayout(ml);

	fillTree();

	return;
}

std::vector<SchemaTypesParams> DialogReportPageSetup::schemaTypesParams() const
{
	return m_schemaTypesParams;
}

void DialogReportPageSetup::pageSetup()
{
	QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	QPageLayout editLayout;

	// Process only items without children, because parent items does not contain a layout
	//
	QList<QTreeWidgetItem*> selectedEditItems;
	for (auto selectedItem : selectedItems)
	{
		if (selectedItem->childCount() == 0)
		{
			selectedEditItems.push_back(selectedItem);
		}
	}
	if (selectedEditItems.size() == 0)
	{
		return;
	}

	// Get current data from selected item
	//
	int selectedItemIndex = selectedEditItems[0]->data(m_typeIndexColumn, Qt::UserRole).toInt();
	if (selectedItemIndex < 0 || selectedItemIndex >= m_schemaTypesParams.size())
	{
		Q_ASSERT(false);
		return;
	}

	int selectedLayoutIndex = selectedEditItems[0]->data(m_layoutIndexColumn, Qt::UserRole).toInt();
	if (selectedLayoutIndex < 0 || selectedLayoutIndex >= m_schemaTypesParams[selectedItemIndex].pageLayoutCount())
	{
		Q_ASSERT(false);
		return;
	}

	const SchemaTypesParams& firstFt = m_schemaTypesParams[selectedItemIndex];
	QPageLayout pageLayout = firstFt.pageLayout(selectedLayoutIndex);

	QPrinter printer(QPrinter::HighResolution);

	QPageSize::PageSizeId id = QPageSize::id(pageLayout.pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
	if (id == QPageSize::Custom)
	{
		id = QPageSize::A4;
	}

	printer.setFullPage(true);
	printer.setPageSize(QPageSize(id));
	printer.setPageOrientation(pageLayout.orientation());
	printer.setPageMargins(pageLayout.margins(), QPageLayout::Unit::Millimeter);

	QPageSetupDialog d(&printer, this);
	if (d.exec() != QDialog::Accepted)
	{
		return;
	}

	id = QPageSize::id(d.printer()->pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);

	for (QTreeWidgetItem* item : selectedEditItems)
	{
		// Set data for all selected items
		//
		int itemIndex = item->data(m_typeIndexColumn, Qt::UserRole).toInt();
		if (itemIndex < 0 || itemIndex >= m_schemaTypesParams.size())
		{
			Q_ASSERT(false);
			return;
		}

		int layoutIndex = item->data(m_layoutIndexColumn, Qt::UserRole).toInt();
		if (layoutIndex < 0 || layoutIndex >= m_schemaTypesParams[itemIndex].pageLayoutCount())
		{
			Q_ASSERT(false);
			return;
		}

		SchemaTypesParams& ft = m_schemaTypesParams[itemIndex];

		QPageLayout l(ft.pageLayout(layoutIndex));

		l.setPageSize(QPageSize(id));
		l.setOrientation(d.printer()->pageLayout().orientation());
		l.setMargins(d.printer()->pageLayout().margins());

		ft.setPageLayout(layoutIndex, l);
	}

	saveOptions();

	fillTree();
	
	return;
}

void DialogReportPageSetup::setToDefault()
{
QList<QTreeWidgetItem*> selectedItems = m_treeWidget->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	QPageLayout editLayout;

	// Process only items without children, because parent items does not contain a layout
	//
	QList<QTreeWidgetItem*> selectedEditItems;
	for (auto selectedItem : selectedItems)
	{
		if (selectedItem->childCount() == 0)
		{
			selectedEditItems.push_back(selectedItem);
		}
	}
	if (selectedEditItems.size() == 0)
	{
		return;
	}

	for (QTreeWidgetItem* item : selectedEditItems)
	{
		int selectedItemIndex = item->data(m_typeIndexColumn, Qt::UserRole).toInt();
		if (selectedItemIndex < 0 || selectedItemIndex >= m_schemaTypesParams.size())
		{
			Q_ASSERT(false);
			return;
		}

		int selectedLayoutIndex = item->data(m_layoutIndexColumn, Qt::UserRole).toInt();
		if (selectedLayoutIndex < 0 || selectedLayoutIndex >= m_schemaTypesParams[selectedItemIndex].pageLayoutCount())
		{
			Q_ASSERT(false);
			return;
		}

		SchemaTypesParams& ft = m_schemaTypesParams[selectedItemIndex];

		ft.setPageLayout(selectedLayoutIndex, m_defaultFileTypeParams[selectedItemIndex].pageLayout(selectedLayoutIndex));
		ft.setNoMargins(selectedLayoutIndex, m_defaultFileTypeParams[selectedItemIndex].noMargins(selectedLayoutIndex));
	}

	saveOptions();

	fillTree();

	return;
}

void DialogReportPageSetup::fillTree()
{
	if (m_treeWidget->topLevelItemCount() != m_schemaTypesParams.size())
	{
		m_treeWidget->clear();

		for (int itemIndex = 0; itemIndex < m_schemaTypesParams.size(); itemIndex++)
		{
			const SchemaTypesParams& ft = m_schemaTypesParams[itemIndex];

			QTreeWidgetItem* item = new QTreeWidgetItem();
			m_treeWidget->addTopLevelItem(item);

			if (ft.pageLayoutCount() > 1)
			{
				// Add child items if there is more than one layout
				//
				for (int layoutIndex = 0; layoutIndex < ft.pageLayoutCount(); layoutIndex++)
				{
					QTreeWidgetItem* layoutItem = new QTreeWidgetItem();
					item->addChild(layoutItem);
				}
			}
		}
	}
	
	for (int itemIndex = 0; itemIndex < m_schemaTypesParams.size(); itemIndex++)
	{
		const SchemaTypesParams& ft = m_schemaTypesParams[itemIndex];

		QTreeWidgetItem* item = m_treeWidget->topLevelItem(itemIndex);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}
		item->setText(0, ft.caption());

		for (int layoutIndex = 0; layoutIndex < ft.pageLayoutCount(); layoutIndex++)
		{
			const QPageLayout& layout = ft.pageLayout(layoutIndex);

			int col = 0;

			QTreeWidgetItem* layoutItem;
			if (ft.pageLayoutCount() > 1)
			{
				layoutItem = item->child(layoutIndex);
				layoutItem->setText(col, ft.pageLayoutCaption(layoutIndex));
			}
			else
			{
				layoutItem = item;
			}
			if (layoutItem == nullptr)
			{
				Q_ASSERT(layoutItem);
				return;
			}
			col++;

			QPageSize::PageSizeId id = QPageSize::id(layout.pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
			if (id == QPageSize::Custom)
			{
				id = QPageSize::A4;
			}

			layoutItem->setText(col++, QPageSize(id).name());
			layoutItem->setText(col++, layout.orientation() == QPageLayout::Portrait ? tr("Portrait") : tr("Landscape"));
			QMarginsF margins = layout.margins();
			layoutItem->setText(col++, tr("l%1 t%2 r%3 b%4").arg(margins.left()).arg(margins.top()).arg(margins.right()).arg(margins.bottom()));
			layoutItem->setCheckState(m_noMarginsColumn, ft.noMargins(layoutIndex) ? Qt::Checked : Qt::Unchecked);

			// Every child (layout) item has data: column 0 - index of schema type, 1 - index of page layout
			//
			layoutItem->setData(m_typeIndexColumn, Qt::UserRole, itemIndex);	
			layoutItem->setData(m_layoutIndexColumn, Qt::UserRole, layoutIndex);
		}

		if (item->childCount() > 0)
		{
			item->setExpanded(true);
		}
	}

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	return;
}

void DialogReportPageSetup::saveOptions()
{
	for (int itemIndex = 0; itemIndex < m_treeWidget->topLevelItemCount(); itemIndex++)
	{
		QTreeWidgetItem* item = m_treeWidget->topLevelItem(itemIndex);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		SchemaTypesParams& ft = m_schemaTypesParams[itemIndex];

		for (int layoutIndex = 0; layoutIndex < ft.pageLayoutCount(); layoutIndex++)
		{
			QTreeWidgetItem* layoutItem;
			if (ft.pageLayoutCount() > 1)
			{
				layoutItem = item->child(layoutIndex);
			}
			else
			{
				layoutItem = item;
			}
			if (layoutItem == nullptr)
			{
				Q_ASSERT(layoutItem);
				return;
			}

			ft.setNoMargins(layoutIndex, layoutItem->checkState(m_noMarginsColumn) == Qt::Checked);
		}
	}
}

void DialogReportPageSetup::accept()
{
	saveOptions();
		
	QDialog::accept();
}