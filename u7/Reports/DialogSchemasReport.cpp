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
	l << tr("No Margins (1:1)");
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
		item->setCheckState(4, ft.noMargins() ? Qt::Checked : Qt::Unchecked);
	}

	for (int i = 0; i < m_treeWidget->columnCount(); i++)
	{
		m_treeWidget->resizeColumnToContents(i);
	}

	return;
}

void DialogSchemasReportTypePageSetup::accept()
{
	int count = m_treeWidget->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		SchemaTypesParams& ft = m_schemaTypesParams[i];
		ft.setNoMargins(m_treeWidget->topLevelItem(i)->checkState(4) == Qt::Checked);
	}

	QDialog::accept();
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

		QTreeWidgetItem* itemToDelete = m_variablesTree->takeTopLevelItem(index);
		if (itemToDelete != nullptr)
		{
			delete itemToDelete;
		}
	}
}

//
// DialogSchemasReport
//
DialogSchemasReport::DialogSchemasReport(const QString& path,
										 const std::vector<Builder::SchemaTypesParams>& schemaTypesParams,
										 const SchemasReportOptions& options,
										 DbController* db,
										 QWidget* parent) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	m_reportPath(path),
	m_schemaTypesParams(schemaTypesParams),
	m_defaultFileTypeParams(Builder::SchemasReportGenerator::defaultFileTypesParams(db)),
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

			QGridLayout* optionsLayout = new QGridLayout(optionsTab);

			// Options buttons
			//
			int row = 0;
			int col = 0;
		
			/* m_checkSignleFile = new QCheckBox(tr("Generate single report file"));
			m_checkSignleFile->setChecked(m_options.singleFile());
			optionsLayout->addWidget(m_checkSignleFile, row++, col);*/

			m_checkAddTableOfContents = new QCheckBox(tr("Generate table of contents"));
			m_checkAddTableOfContents->setChecked(m_options.tableOfContents());
			optionsLayout->addWidget(m_checkAddTableOfContents, row++, col);

			m_checkAddFolders = new QCheckBox(tr("Add folders names to table of contents"));
			m_checkAddFolders->setChecked(m_options.folders());
			optionsLayout->addWidget(m_checkAddFolders, row++, col);

			m_checkAddFooters = new QCheckBox(tr("Add header and footer"));
			m_checkAddFooters->setChecked(m_options.footers());
			optionsLayout->addWidget(m_checkAddFooters, row++, col);

			m_checkSignalsDetails = new QCheckBox(tr("Add schema signals pages"));
			m_checkSignalsDetails->setChecked(m_options.signalsDetails());
			optionsLayout->addWidget(m_checkSignalsDetails, row++, col);

			m_checkItemsLabels = new QCheckBox(tr("Show schema items labels"));
			m_checkItemsLabels->setChecked(m_options.itemsLabels());
			optionsLayout->addWidget(m_checkItemsLabels, row++, col);

			optionsLayout->addWidget(new QLabel("Start page number"), row, col);
			m_editStartPageNumber = new QLineEdit();
			m_editStartPageNumber->setText(QString::number(m_options.startPageNumber()));
			optionsLayout->addWidget(m_editStartPageNumber, row++, col + 1);

			optionsLayout->addWidget(new QLabel("Table of contents font size"), row, col);
			m_editTableOfContentsFontSize = new QLineEdit();
			m_editTableOfContentsFontSize->setText(QString::number(m_options.tableOfContentsFontSize()));
			optionsLayout->addWidget(m_editTableOfContentsFontSize, row++, col + 1);
			
			optionsLayout->addWidget(new QLabel("Tables font size"), row, col);
			m_editTableFontSize = new QLineEdit();
			m_editTableFontSize->setText(QString::number(m_options.tableFontSize()));
			optionsLayout->addWidget(m_editTableFontSize, row++, col + 1);

			optionsLayout->addWidget(new QLabel("Text font size"), row, col);
			m_editNormalFontSize = new QLineEdit();
			m_editNormalFontSize->setText(QString::number(m_options.normalFontSize()));
			optionsLayout->addWidget(m_editNormalFontSize, row++, col + 1);

			{
				QPushButton* pageSetupButton = new QPushButton(tr("Page Setup..."));
				connect(pageSetupButton, &QPushButton::clicked, this, &DialogSchemasReport::pageSetupClicked);
				optionsLayout->addWidget(pageSetupButton, row++, col);
			}

			tabWidget->addTab(optionsTab, tr("Options"));
		}

		// Variables tab
		//
		{
			QWidget* variablesWidget = new QWidget();
			QVBoxLayout* variablesLayout = new QVBoxLayout(variablesWidget);

			variablesLayout->addWidget(new QLabel(tr("Project Variables")));
			m_projectVariables = new VariablesWidget(m_options.projectVariables());
			variablesLayout->addWidget(m_projectVariables);

			variablesLayout->addWidget(new QLabel(tr("User Variables")));
			m_userVariables = new VariablesWidget(m_options.userVariables());
			variablesLayout->addWidget(m_userVariables);
		
			tabWidget->addTab(variablesWidget, tr("Variables"));
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

bool DialogSchemasReport::optionsApplied() const
{
	return m_optionsApplied;
}

void DialogSchemasReport::applyClicked()
{
	m_optionsApplied = applyOptions();
}

void DialogSchemasReport::okClicked()
{
	m_optionsApplied = applyOptions();

	if (m_optionsApplied == false)
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
	// Save path
	//
	QString text = m_editReportPath->text();
	if (text.isEmpty() == true)
	{
		QMessageBox::critical(this, qAppName(), tr("Please enter the file name!"));
		m_editReportPath->setFocus();
		return false;
	}
	m_reportPath = text;

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
	
	//m_options.setSignleFile(m_checkSignleFile->isChecked());
	m_options.setFolders(m_checkAddFolders->isChecked());
	m_options.setFooters(m_checkAddFooters->isChecked());
	m_options.setTableOfContents(m_checkAddTableOfContents->isChecked());
	m_options.setSignalsDetails(m_checkSignalsDetails->isChecked());
	m_options.setItemsLabels(m_checkItemsLabels->isChecked());

	bool ok = false;
	int value = m_editStartPageNumber->text().toInt(&ok);
	if (ok == false)
	{
		m_editStartPageNumber->setFocus();
		return false;
	}
	m_options.setStartPageNumber(value);

	value = m_editTableOfContentsFontSize->text().toInt(&ok);
	if (ok == false)
	{
		m_editTableOfContentsFontSize->setFocus();
		return false;
	}
	m_options.setTableOfContentsFontSize(value);

	value = m_editTableFontSize->text().toInt(&ok);
	if (ok == false)
	{
		m_editTableFontSize->setFocus();
		return false;
	}
	m_options.setTableFontSize(value);

	value = m_editNormalFontSize->text().toInt(&ok);
	if (ok == false)
	{
		m_editNormalFontSize->setFocus();
		return false;
	}
	m_options.setNormalFontSize(value);

	// Save variables
	//
	m_options.setUserVariables(m_userVariables->getVariables());
	m_options.setProjectVariables(m_projectVariables->getVariables());

	return true;

}