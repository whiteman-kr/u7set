#include "DialogSchemasReport.h"

#include <QPrinter>
#include <QPageSetupDialog>

using namespace Builder;
using namespace ReportLib;

//
// DialogSchemasReportTypeParams
//

DialogSchemasReportTypePageSetup::DialogSchemasReportTypePageSetup(const std::vector<SchemaTypesParams>& schemaTypesParams,
													   std::vector<SchemaTypesParams> defaultFileTypeParams,
													   QWidget *parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_schemaTypesParams(schemaTypesParams),
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
	m_treeWidget->setRootIsDecorated(false);

	connect(m_treeWidget, &QTreeWidget::itemDoubleClicked, [this](QTreeWidgetItem *item, int column){
		Q_UNUSED(item);
		Q_UNUSED(column);
		pageSetup();
	});

	QVBoxLayout* pbLayout = new QVBoxLayout();

	QPushButton* b = new QPushButton(tr("Page Setup..."));
	connect(b, &QPushButton::clicked, this, &DialogSchemasReportTypePageSetup::pageSetup);
	pbLayout->addWidget(b);

	b = new QPushButton(tr("Set to Default"));
	connect(b, &QPushButton::clicked, this, &DialogSchemasReportTypePageSetup::setToDefault);

	pbLayout->addWidget(b);
	pbLayout->addStretch();

	QHBoxLayout* topLayout = new QHBoxLayout();

	topLayout->addWidget(m_treeWidget);
	topLayout->addLayout(pbLayout);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();
	buttonsLayout->addStretch();

	b = new QPushButton(tr("OK"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogSchemasReportTypePageSetup::accept);

	b = new QPushButton(tr("Cancel"));
	buttonsLayout->addWidget(b);
	connect(b, &QPushButton::clicked, this, &DialogSchemasReportTypePageSetup::reject);

	QVBoxLayout* ml = new QVBoxLayout();
	ml->addLayout(topLayout);
	ml->addLayout(buttonsLayout);

	setLayout(ml);

	fillTree();

	return;
}

std::vector<SchemaTypesParams> DialogSchemasReportTypePageSetup::schemaTypesParams() const
{
	return m_schemaTypesParams;
}

void DialogSchemasReportTypePageSetup::pageSetup()
{
	QList<QTreeWidgetItem*> selectedItems =  m_treeWidget->selectedItems();
	if (selectedItems.isEmpty() == true)
	{
		return;
	}

	int firstIndex = m_treeWidget->indexOfTopLevelItem(selectedItems[0]);
	if (firstIndex < 0 || firstIndex >= m_schemaTypesParams.size())
	{
		Q_ASSERT(false);
		return;
	}

	const SchemaTypesParams& firstFt = m_schemaTypesParams[firstIndex];

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
		if (itemIndex < 0 || itemIndex >= m_schemaTypesParams.size())
		{
			Q_ASSERT(false);
			return;
		}

		SchemaTypesParams& ft = m_schemaTypesParams[itemIndex];

		QPageLayout l(ft.pageLayout());

		l.setPageSize(QPageSize(id));
		l.setOrientation(d.printer()->pageLayout().orientation());
		l.setMargins(d.printer()->pageLayout().margins());

		ft.setPageLayout(l);
	}

	fillTree();

	return;
}

void DialogSchemasReportTypePageSetup::setToDefault()
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
		if (itemIndex < 0 || itemIndex >= m_schemaTypesParams.size())
		{
			Q_ASSERT(false);
			return;
		}

		SchemaTypesParams& ft = m_schemaTypesParams[itemIndex];

		for (const SchemaTypesParams& dft : m_defaultFileTypeParams)
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

void DialogSchemasReportTypePageSetup::fillTree()
{
	if (m_treeWidget->topLevelItemCount() != m_schemaTypesParams.size())
	{
		m_treeWidget->clear();

		for (int i = 0; i < m_schemaTypesParams.size(); i++)
		{
			m_treeWidget->addTopLevelItem(new QTreeWidgetItem());
		}
	}

	int itemIndex = 0;

	for (const SchemaTypesParams& ft : m_schemaTypesParams)
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


//
// DialogSchemasReport
//
DialogSchemasReport::DialogSchemasReport(const QString& path,
										 const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
										 const std::vector<Builder::SchemaTypesParams>& defaultFileTypeParams,
										 const SchemasReportOptions& options,
										 QWidget *parent):
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_reportPath(path),
	m_schemaTypesParams(schemaTypesParams),
	m_defaultFileTypeParams(defaultFileTypeParams),
	m_options(options)
{
	setWindowTitle(tr("Create Schemas Album"));
	setMinimumWidth(500);

	// Report path
	//
	QLabel* label = new QLabel(tr("Album path:"));
	m_editReportPath = new QLineEdit(m_reportPath);

	QPushButton* browseButton = new QPushButton(tr("Browse..."));
	connect(browseButton, &QPushButton::clicked, this, &DialogSchemasReport::browseClicked);

	QHBoxLayout* reportPathLayout = new QHBoxLayout();
	reportPathLayout->addWidget(label);
	reportPathLayout->addWidget(m_editReportPath);
	reportPathLayout->addWidget(browseButton);

	// Schema types tree
	//
	m_schemaTypesTree = new QTreeWidget();
	m_schemaTypesTree->setHeaderLabels({tr("Schema type")});
	m_schemaTypesTree->setRootIsDecorated(false);
	for (const auto& stp : schemaTypesParams)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem({stp.caption()});
		item->setCheckState(0, stp.selected() ? Qt::Checked : Qt::Unchecked);
		m_schemaTypesTree->addTopLevelItem(item);
	}

	// Options buttons
	//
	//m_checkAddPageNumbers = new QCheckBox(tr("Add schemas information and page numbers"));
	//m_checkAddPageNumbers->setChecked(m_options.addPageNumbers);

	m_checkAddSignalsSources= new QCheckBox(tr("Add pages with schemas signals sources and destinations"));
	m_checkAddSignalsSources->setChecked(m_options.addLogicSchemaDetails);

	m_checkInfoMode = new QCheckBox(tr("Add schema items labels"));
	m_checkInfoMode->setChecked(m_options.infoMode);

	// Buttons layout
	//
	QPushButton* okButton = new QPushButton(tr("OK"));
	connect(okButton, &QPushButton::clicked, this, &DialogSchemasReport::okClicked);

	QPushButton* cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

	QHBoxLayout* buttonsLayout = new QHBoxLayout();

	QPushButton* pageSetupButton = new QPushButton(tr("Page Setup..."));
	connect(pageSetupButton, &QPushButton::clicked, this, &DialogSchemasReport::pageSetupClicked);

	buttonsLayout->addWidget(pageSetupButton);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);

	//
	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(reportPathLayout);
	mainLayout->addWidget(m_schemaTypesTree);
	//mainLayout->addWidget(m_checkAddPageNumbers);
	mainLayout->addWidget(m_checkAddSignalsSources);
	mainLayout->addWidget(m_checkInfoMode);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);
}

std::vector<Builder::SchemaTypesParams> DialogSchemasReport::schemaTypesParams() const
{
	return m_schemaTypesParams;
}

Builder::SchemasReportOptions DialogSchemasReport::options() const
{
	return m_options;
}

void DialogSchemasReport::okClicked()
{
	QString text = m_editReportPath->text();
	if (text.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Please enter the file name!"));
		m_editReportPath->setFocus();
		return;
	}
	m_reportPath = text;

	//m_options.addPageNumbers = m_checkAddPageNumbers->isChecked();
	m_options.addLogicSchemaDetails = m_checkAddSignalsSources->isChecked();
	m_options.infoMode = m_checkInfoMode->isChecked();

	int selectedCount = 0;

	for (int i = 0; i < m_schemaTypesTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_schemaTypesTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked)
		{
			selectedCount++;
		}

		m_schemaTypesParams[i].setSelected(item->checkState(0) == Qt::Checked);
	}

	if (selectedCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Please choose at least one file type!"));
		return;
	}

	QDialog::accept();
}

void DialogSchemasReport::browseClicked()
{
	QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"), QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	if (dir.isEmpty() == true)
	{
		return;
	}
	m_editReportPath->setText(QDir::toNativeSeparators(dir));

	return;
}

void DialogSchemasReport::pageSetupClicked()
{
	DialogSchemasReportTypePageSetup d(m_schemaTypesParams, m_defaultFileTypeParams, this);
	if (d.exec() == QDialog::Accepted)
	{
		m_schemaTypesParams = d.schemaTypesParams();
	}

	return;
}

void DialogSchemasReport::optionsClicked()
{

}
