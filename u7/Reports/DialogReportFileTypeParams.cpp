#include "DialogReportFileTypeParams.h"

#include <QPrinter>
#include <QPageSetupDialog>

using namespace ReportLib;

//
// DialogProjectDiffSections
//

DialogReportFileTypeParams::DialogReportFileTypeParams(const std::vector<ReportFileTypeParams>& fileTypeParams,
													   std::vector<ReportFileTypeParams> defaultFileTypeParams,
													   QWidget *parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_fileTypeParams(fileTypeParams),
	m_defaultFileTypeParams(defaultFileTypeParams)
{
	setWindowTitle(tr("Report Sections Page Setup"));
	setMinimumSize(540, 350);

	m_treeWidget = new QTreeWidget();

	QStringList l;
	l << tr("Section");
	l << tr("Page Size");
	l << tr("Orientation");
	l << tr("Margins, mm");
	m_treeWidget->setHeaderLabels(l);
	m_treeWidget->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item, int column){
		Q_UNUSED(item);
		Q_UNUSED(column);
		pageSetup();
	});

	QVBoxLayout* pbLayout = new QVBoxLayout();

	QPushButton* b = new QPushButton(tr("Page Setup..."));
	connect(b, &QPushButton::clicked, this, &DialogReportFileTypeParams::pageSetup);

	pbLayout->addWidget(b);

	b = new QPushButton(tr("Set to Default"));
	connect(b, &QPushButton::clicked, this, &DialogReportFileTypeParams::setToDefault);

	pbLayout->addWidget(b);
	pbLayout->addStretch();

	QHBoxLayout* topLayout = new QHBoxLayout();

	topLayout->addWidget(m_treeWidget);
	topLayout->addLayout(pbLayout);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();

	b = new QPushButton(tr("OK"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogReportFileTypeParams::accept);

	b = new QPushButton(tr("Cancel"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogReportFileTypeParams::reject);

	QVBoxLayout* ml = new QVBoxLayout();
	ml->addLayout(topLayout);
	ml->addLayout(buttonsLayout);

	setLayout(ml);

	fillTree();

	return;
}

std::vector<ReportFileTypeParams> DialogReportFileTypeParams::fileTypeParams() const
{
	return m_fileTypeParams;
}

void DialogReportFileTypeParams::pageSetup()
{
	QList<QTreeWidgetItem*> selectedItems =  m_treeWidget->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	int firstIndex = m_treeWidget->indexOfTopLevelItem(selectedItems[0]);
	if (firstIndex < 0 || firstIndex >= m_fileTypeParams.size())
	{
		Q_ASSERT(false);
		return;
	}

	const ReportFileTypeParams& firstFt = m_fileTypeParams[firstIndex];

	QPageLayout pageLayout = firstFt.pageLayout();

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

	for (QTreeWidgetItem* item : selectedItems)
	{
		int itemIndex = m_treeWidget->indexOfTopLevelItem(item);
		if (itemIndex < 0 || itemIndex >= m_fileTypeParams.size())
		{
			Q_ASSERT(false);
			return;
		}

		ReportFileTypeParams& ft = m_fileTypeParams[itemIndex];

		QPageLayout l(ft.pageLayout());

		l.setPageSize(QPageSize(id));
		l.setOrientation(d.printer()->pageLayout().orientation());
		l.setMargins(d.printer()->pageLayout().margins());

		ft.setPageLayout(l);
	}

	fillTree();

	return;
}

void DialogReportFileTypeParams::setToDefault()
{
	QList<QTreeWidgetItem*> selectedItems =  m_treeWidget->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		for (int i = 0; i < m_treeWidget->topLevelItemCount(); i++)
		{
			selectedItems.push_back(m_treeWidget->topLevelItem(i));
		}
	}

	for (QTreeWidgetItem* item : selectedItems)
	{
		int itemIndex = m_treeWidget->indexOfTopLevelItem(item);
		if (itemIndex < 0 || itemIndex >= m_fileTypeParams.size())
		{
			Q_ASSERT(false);
			return;
		}

		ReportFileTypeParams& ft = m_fileTypeParams[itemIndex];

		for (const ReportFileTypeParams& dft : m_defaultFileTypeParams)
		{
			if (dft.fileId() == ft.fileId())
			{
				ft.setPageLayout(dft.pageLayout());
				break;
			}
		}
	}

	fillTree();

	return;
}

void DialogReportFileTypeParams::fillTree()
{
	if (m_treeWidget->topLevelItemCount() != m_fileTypeParams.size())
	{
		m_treeWidget->clear();

		for (int i = 0; i < m_fileTypeParams.size(); i++)
		{
			m_treeWidget->addTopLevelItem(new QTreeWidgetItem());
		}
	}

	int itemIndex = 0;

	for (const ReportFileTypeParams& ft : m_fileTypeParams)
	{
		QTreeWidgetItem* item = m_treeWidget->topLevelItem(itemIndex++);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		QPageSize::PageSizeId id = QPageSize::id(ft.pageLayout().pageSize().sizePoints(), QPageSize::FuzzyOrientationMatch);
		if (id == QPageSize::Custom)
		{
			id = QPageSize::A4;
		}

		item->setText(0, ft.caption());
		item->setText(1, QPageSize(id).name());
		item->setText(2, ft.pageLayout().orientation() == QPageLayout::Portrait ? tr("Portrait") : tr("Landscape"));
		QMarginsF margins = ft.pageLayout().margins();
		item->setText(3, tr("l%1 t%2 r%3 b%4").arg(margins.left()).arg(margins.top()).arg(margins.right()).arg(margins.bottom()));
	}

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	return;
}
