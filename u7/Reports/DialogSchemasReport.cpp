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


VariablesWidget::VariablesWidget(const std::map<QString, QString>& variables):
	m_variables(variables)
{
	QVBoxLayout* variablesLayout = new QVBoxLayout(this);
	m_variablesTree = new QTreeWidget();
	m_variablesTree->setHeaderLabels(QStringList() << "Name"
												   << "Value");

	for (const auto& [name, value] : m_variables)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << name << value);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		m_variablesTree->addTopLevelItem(item);
	}
	variablesLayout->addWidget(m_variablesTree);
	m_variablesTree->resizeColumnToContents(0);
	m_variablesTree->resizeColumnToContents(1);

	QHBoxLayout* buttonLayout = new QHBoxLayout();
	variablesLayout->addLayout(buttonLayout);

	QPushButton* b = new QPushButton("Add");
	connect(b, &QPushButton::clicked, this, &VariablesWidget::onAddVariableClicked);
	buttonLayout->addWidget(b);

	b = new QPushButton("Remove");
	connect(b, &QPushButton::clicked, this, &VariablesWidget::onRemoveVariableClicked);
	buttonLayout->addWidget(b);

	buttonLayout->addStretch();

	return;
}

std::map<QString, QString> VariablesWidget::getVariables() const
{
	std::map<QString, QString> variables;
	for (int i = 0; i < m_variablesTree->topLevelItemCount(); i++)
	{
		QString name = m_variablesTree->topLevelItem(i)->text(0);
		QString value = m_variablesTree->topLevelItem(i)->text(1);
		if (name.isEmpty() == false)
		{
			variables[name] = value;
		}
	}
	return variables;
}


void VariablesWidget::onAddVariableClicked()
{
	QStringList l;
	for (int i = 0; i < m_variablesTree->topLevelItemCount(); i++)
	{
		l.push_back(m_variablesTree->topLevelItem(i)->text(0));
	}

	QString name;
	int value = m_variablesTree->topLevelItemCount();
	do
	{
		name = tr("Variable%1").arg(++value);
	} while (l.contains(name) == true);


	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << name << tr("Value"));
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	m_variablesTree->addTopLevelItem(item);
	m_variablesTree->resizeColumnToContents(0);
	m_variablesTree->resizeColumnToContents(1);
}

void VariablesWidget::onRemoveVariableClicked()
{
	const auto items = m_variablesTree->selectedItems();
	for (const auto& item : items)
	{
		int index = m_variablesTree->indexOfTopLevelItem(item);
		if (index == -1)
		{
			return;
		}

		QTreeWidgetItem* item = m_variablesTree->takeTopLevelItem(index);
		if (item != nullptr)
		{
			delete item;
		}
	}
}

//
// DialogSchemasReport
//
DialogSchemasReport::DialogSchemasReport(const QString& path,
										 const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
										 const std::vector<Builder::SchemaTypesParams>& defaultFileTypeParams,
										 const SchemasReportOptions& options,
										 DbController* db,
										 QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_reportPath(path),
	m_schemaTypesParams(schemaTypesParams),
	m_defaultFileTypeParams(defaultFileTypeParams),
	m_options(options),
	m_db(db)
{
	setWindowTitle(tr("Create Schemas Album"));
	setMinimumWidth(500);

	QVBoxLayout* mainLayout = new QVBoxLayout();

	{
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
		mainLayout->addLayout(reportPathLayout);
	}


	// Tab widget
	//
	{
		QTabWidget* tabWidget = new QTabWidget();

		// Schemas tab
		//
		{
			QWidget* schemasTab = new QWidget();

			QVBoxLayout* schemasLayout = new QVBoxLayout(schemasTab);

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
			schemasLayout->addWidget(m_schemaTypesTree);

			// Schema tags tree
			//
			m_schemaTagsTree = new QTreeWidget();
			m_schemaTagsTree->setHeaderLabels({tr("Schema tag")});
			m_schemaTagsTree->setRootIsDecorated(false);
			for (const auto& [tag, tagSelected] : options.schemaTags())
			{
				QTreeWidgetItem* item = new QTreeWidgetItem({tag});
				item->setCheckState(0, tagSelected ? Qt::Checked : Qt::Unchecked);
				m_schemaTagsTree->addTopLevelItem(item);
			}
			schemasLayout->addWidget(m_schemaTagsTree);

			tabWidget->addTab(schemasTab, tr("Schemas"));
		}


		// Options tab
		//
		{
			QWidget* optionsTab = new QWidget();

			QVBoxLayout* optionsLayout = new QVBoxLayout(optionsTab);

			// Options buttons
			//
			m_checkAddTableOfContents = new QCheckBox(tr("Add table of contents"));
			m_checkAddTableOfContents->setChecked(m_options.tableOfContents());
			optionsLayout->addWidget(m_checkAddTableOfContents);

			m_checkAddFolders = new QCheckBox(tr("Add folders names to table of contents"));
			m_checkAddFolders->setChecked(m_options.folders());
			optionsLayout->addWidget(m_checkAddFolders);

			m_checkAddFooters = new QCheckBox(tr("Add pages footers"));
			m_checkAddFooters->setChecked(m_options.footers());
			optionsLayout->addWidget(m_checkAddFooters);

			m_checkSignalsDetails = new QCheckBox(tr("Add schema signals pages"));
			m_checkSignalsDetails->setChecked(m_options.signalsDetails());
			optionsLayout->addWidget(m_checkSignalsDetails);

			m_checkItemsLabels = new QCheckBox(tr("Show schema items labels"));
			m_checkItemsLabels->setChecked(m_options.itemsLabels());
			optionsLayout->addWidget(m_checkItemsLabels);

			optionsLayout->addStretch();

			{
				QHBoxLayout* buttonLayout = new QHBoxLayout();
				QPushButton* pageSetupButton = new QPushButton(tr("Page Setup..."));
				connect(pageSetupButton, &QPushButton::clicked, this, &DialogSchemasReport::pageSetupClicked);
				buttonLayout->addWidget(pageSetupButton);
				buttonLayout->addStretch();
				optionsLayout->addLayout(buttonLayout);
			}

			tabWidget->addTab(optionsTab, tr("Options"));
		}

		// Variables tab
		//
		{
			m_userVariables = new VariablesWidget(m_options.userVariables());
			tabWidget->addTab(m_userVariables, tr("User Variables"));

			m_projectVariables = new VariablesWidget(m_options.projectVariables());
			tabWidget->addTab(m_projectVariables, tr("Project Variables"));
		}

		mainLayout->addWidget(tabWidget);
	}

	// Buttons layout
	//
	{
		QPushButton* applyButton = new QPushButton(tr("Apply"));
		connect(applyButton, &QPushButton::clicked, this, &DialogSchemasReport::applyClicked);

		QPushButton* okButton = new QPushButton(tr("Generate"));
		connect(okButton, &QPushButton::clicked, this, &DialogSchemasReport::okClicked);

		QPushButton* cancelButton = new QPushButton(tr("Cancel"));
		connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

		QHBoxLayout* buttonsLayout = new QHBoxLayout();
		buttonsLayout->addWidget(applyButton);
		buttonsLayout->addStretch();
		buttonsLayout->addWidget(okButton);
		buttonsLayout->addWidget(cancelButton);
		mainLayout->addLayout(buttonsLayout);
	}

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

QString DialogSchemasReport::path() const
{
	return m_reportPath;
}

void DialogSchemasReport::applyClicked()
{
	applyOptions();
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

	if (applyOptions() == false)
	{
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

bool DialogSchemasReport::applyOptions()
{
	// Save selected types
	//
	int selectedTypesCount = 0;
	for (int i = 0; i < m_schemaTypesTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_schemaTypesTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked)
		{
			selectedTypesCount++;
		}

		m_schemaTypesParams[i].setSelected(item->checkState(0) == Qt::Checked);
	}

	if (selectedTypesCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Please choose at least one file type!"));
		return false;
	}

	// Save tags
	//
	int selectedTagsCount = 0;
	for (int i = 0; i < m_schemaTagsTree->topLevelItemCount(); i++)
	{
		QTreeWidgetItem* item = m_schemaTagsTree->topLevelItem(i);
		if (item->checkState(0) == Qt::Checked)
		{
			selectedTagsCount++;
		}

		m_options.schemaTags()[item->text(0)] = item->checkState(0) == Qt::Checked;
	}

	if (selectedTagsCount == 0)
	{
		QMessageBox::critical(this, qAppName(), tr("Please choose at least one schema tag!"));
		return false;
	}

	// Save options
	//
	m_options.setFolders(m_checkAddFolders->isChecked());
	m_options.setFooters(m_checkAddFooters->isChecked());
	m_options.setTableOfContents(m_checkAddTableOfContents->isChecked());
	m_options.setSignalsDetails(m_checkSignalsDetails->isChecked());
	m_options.setItemsLabels(m_checkItemsLabels->isChecked());

	// Save variables
	//
	m_options.setUserVariables(m_userVariables->getVariables());
	m_options.setProjectVariables(m_projectVariables->getVariables());

	m_options.save(m_db);

	return true;

}