#include "DialogSchemasReport.h"
#include "DialogReportPageSetup.h"

using namespace Builder;
using namespace ReportLib;

VariablesWidget::VariablesWidget(const std::map<QString, QString>& variables, bool readOnly) :
	m_variables(variables),
	m_readOnly(readOnly)
{
	QVBoxLayout* variablesLayout = new QVBoxLayout(this);
	variablesLayout->setContentsMargins(0, 0, 0, 0);
	m_variablesTree = new QTreeWidget();
	m_variablesTree->setHeaderLabels(QStringList() << "Name"
												   << "Value");
	m_variablesTree->setRootIsDecorated(false);
	m_variablesTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(m_variablesTree, &QTreeWidget::customContextMenuRequested, this, &VariablesWidget::onCustomContextMenuRequested);

	for (const auto& [name, value] : m_variables)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << name << value);
		if (readOnly == false)
		{
			item->setFlags(item->flags() | Qt::ItemIsEditable);
		}
		m_variablesTree->addTopLevelItem(item);
	}
	variablesLayout->addWidget(m_variablesTree);
	m_variablesTree->resizeColumnToContents(0);
	m_variablesTree->resizeColumnToContents(1);

	if (readOnly == false)
	{
		QHBoxLayout* buttonLayout = new QHBoxLayout();
		variablesLayout->addLayout(buttonLayout);

		QPushButton* b = new QPushButton("Add");
		connect(b, &QPushButton::clicked, this, &VariablesWidget::onAddVariableClicked);
		buttonLayout->addWidget(b);

		b = new QPushButton("Remove");
		connect(b, &QPushButton::clicked, this, &VariablesWidget::onRemoveVariableClicked);
		buttonLayout->addWidget(b);

		buttonLayout->addStretch();
	}

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

void VariablesWidget::onCopyVariableClicked(int column)
{
	if (column < 0 || column >= m_variablesTree->columnCount())
	{
		return;
	}
	const auto items = m_variablesTree->selectedItems();

	QStringList text;
	for (const QTreeWidgetItem* item : items)
	{
		text.push_back(item->text(column));
	}
	if (text.isEmpty() == true)
	{
		return;
	}

	QApplication::clipboard()->setText(text.join('\n'));
}

void VariablesWidget::onEditVariableClicked(int column)
{
	if (column < 0 || column >= m_variablesTree->columnCount())
	{
		return;
	}
	const auto items = m_variablesTree->selectedItems();
	if (items.size() != 1)
	{
		return;
	}
	m_variablesTree->editItem(items[0], column);
}

void VariablesWidget::onAddVariableClicked()
{
	// Create unique variable name
	//
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

	// Add new item
	//
	m_variablesTree->clearSelection();

	QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << name << tr("Value"));
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	m_variablesTree->addTopLevelItem(item);
	m_variablesTree->scrollToItem(item);
	item->setSelected(true);
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

void VariablesWidget::onCustomContextMenuRequested(const QPoint& pos)
{
		int column = m_variablesTree->columnAt(pos.x());

		const auto items = m_variablesTree->selectedItems();
	
	QMenu menu(this);

	{
		QAction* a = new QAction(tr("Copy"), &menu);
		connect(a, &QAction::triggered, this, [this, column]()
				{
					onCopyVariableClicked(column);
				});

		a->setEnabled(items.size() > 0);
		menu.addAction(a);
	}

	if (m_readOnly == false)
	{
		menu.addSeparator();

		QAction* a = new QAction(tr("Add"), &menu);
		connect(a, &QAction::triggered, this, &VariablesWidget::onAddVariableClicked);
		menu.addAction(a);

		a = new QAction(tr("Edit"), &menu);
		connect(a, &QAction::triggered, this, [this, column]()
				{
					onEditVariableClicked(column);
				});
		a->setEnabled(items.size() == 1);
		menu.addAction(a);

		a = new QAction(tr("Remove"), &menu);
		a->setEnabled(items.size() > 0);
		connect(a, &QAction::triggered, this, &VariablesWidget::onRemoveVariableClicked);
		menu.addAction(a);
	}

	menu.exec(QCursor::pos());
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
	setMinimumWidth(400);

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
			m_schemasTabSplitter = new QSplitter(Qt::Vertical);

			// Schema types tree
			//
			m_schemaTypesTree = new QTreeWidget();
			m_schemaTypesTree->setHeaderLabels({tr("Schema type")});
			m_schemaTypesTree->setRootIsDecorated(false);
			for (const auto& stp : schemaTypesParams)
			{
				if (stp.hasFileId() == false)
				{	
					// This is global setting for single-file report
					continue;
				}

				QTreeWidgetItem* item = new QTreeWidgetItem({stp.caption()});
				item->setCheckState(0, stp.selected() ? Qt::Checked : Qt::Unchecked);
				item->setData(0, Qt::UserRole, stp.fileId());
				m_schemaTypesTree->addTopLevelItem(item);
			}
			m_schemasTabSplitter->addWidget(m_schemaTypesTree);

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
			m_schemasTabSplitter->addWidget(m_schemaTagsTree);

			QWidget* w = new QWidget();
			QVBoxLayout* l = new QVBoxLayout(w);
			l->addWidget(m_schemasTabSplitter);
			tabWidget->addTab(w, tr("Schemas"));

			m_schemasTabSplitter->restoreState(QSettings().value("DialogSchemasReport/schemasTabSplitterState", m_schemasTabSplitter->saveState()).toByteArray());
			m_schemasTabSplitter->setChildrenCollapsible(false);
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
		
			// Checkboxes
			//
			m_checkSignleFile = new QCheckBox(tr("Single file"));
			m_checkSignleFile->setChecked(m_options.singleFile());
			optionsLayout->addWidget(m_checkSignleFile, row++, col);

			m_checkAddTableOfContents = new QCheckBox(tr("Table of contents"));
			m_checkAddTableOfContents->setChecked(m_options.tableOfContents());
			optionsLayout->addWidget(m_checkAddTableOfContents, row++, col);

			m_checkAddFolders = new QCheckBox(tr("Folders to table of contents"));
			m_checkAddFolders->setChecked(m_options.folders());
			optionsLayout->addWidget(m_checkAddFolders, row++, col);

			m_checkAddFooters = new QCheckBox(tr("Header and footer"));
			m_checkAddFooters->setChecked(m_options.footers());
			optionsLayout->addWidget(m_checkAddFooters, row++, col);

			m_checkSignalsDetails = new QCheckBox(tr("Schema signals pages"));
			m_checkSignalsDetails->setChecked(m_options.signalsDetails());
			optionsLayout->addWidget(m_checkSignalsDetails, row++, col);

			m_checkItemsLabels = new QCheckBox(tr("Schema items labels"));
			m_checkItemsLabels->setChecked(m_options.itemsLabels());
			optionsLayout->addWidget(m_checkItemsLabels, row++, col);

			// Edit controls
			//
			optionsLayout->addWidget(new QLabel("Start page number"), row, col);
			m_editStartPageNumber = new QLineEdit();
			m_editStartPageNumber->setText(QString::number(m_options.startPageNumber()));
			optionsLayout->addWidget(m_editStartPageNumber, row, col + 1);
			optionsLayout->addWidget(new QWidget(), row++, col + 2);

			optionsLayout->addWidget(new QLabel("Table of contents header font size"), row, col);
			m_editContentsTextFontSize = new QLineEdit();
			m_editContentsTextFontSize->setText(QString::number(m_options.contentsTextFontSize()));
			optionsLayout->addWidget(m_editContentsTextFontSize, row, col + 1);
			optionsLayout->addWidget(new QWidget(), row++, col + 2);

			optionsLayout->addWidget(new QLabel("Table of contents table font size"), row, col);
			m_editContentsTableFontSize = new QLineEdit();
			m_editContentsTableFontSize->setText(QString::number(m_options.contentsTableFontSize()));
			optionsLayout->addWidget(m_editContentsTableFontSize, row, col + 1);
			optionsLayout->addWidget(new QWidget(), row++, col + 2);
			
			optionsLayout->addWidget(new QLabel("Text font size"), row, col);
			m_editTextFontSize = new QLineEdit();
			m_editTextFontSize->setText(QString::number(m_options.textFontSize()));
			optionsLayout->addWidget(m_editTextFontSize, row, col + 1);
			optionsLayout->addWidget(new QWidget(), row++, col + 2);

			optionsLayout->addWidget(new QLabel("Tables font size"), row, col);
			m_editTableFontSize = new QLineEdit();
			m_editTableFontSize->setText(QString::number(m_options.tableFontSize()));
			optionsLayout->addWidget(m_editTableFontSize, row, col + 1);
			optionsLayout->addWidget(new QWidget(), row++, col + 2);
	
			// Add spacer
			//
			optionsLayout->addWidget(new QWidget(), row, col);
			optionsLayout->setRowStretch(row++, 10);
			optionsLayout->setColumnStretch(2, 10);

			// Page setup button
			//
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
			m_variablesTabSplitter = new QSplitter(Qt::Vertical);

			{
				QWidget* w = new QWidget();
				QVBoxLayout* l = new QVBoxLayout(w);
				l->setContentsMargins(0, 0, 0, 0);
				l->addWidget(new QLabel(tr("Project Variables")));
				m_projectVariables = new VariablesWidget(m_options.projectVariables(), false /*readOnly*/);
				l->addWidget(m_projectVariables);
				m_variablesTabSplitter->addWidget(w);
			}

			{
				QWidget* w = new QWidget();
				QVBoxLayout* l = new QVBoxLayout(w);
				l->setContentsMargins(0, 0, 0, 0);
				l->addWidget(new QLabel(tr("User Variables")));
				m_userVariables = new VariablesWidget(m_options.userVariables(), false /*readOnly*/);
				l->addWidget(m_userVariables);
				m_variablesTabSplitter->addWidget(w);
			}

			{
				QWidget* w = new QWidget();
				QVBoxLayout* l = new QVBoxLayout(w);
				l->setContentsMargins(0, 0, 0, 0);
				l->addWidget(new QLabel(tr("Auto Variables")));
				std::map<QString, QString> autoVariablesMap;
				autoVariablesMap["REPORT_PAGE"] = "Current page";
				autoVariablesMap["REPORT_PAGE_<SCHEMAID>"] = "Page number of each schema";
				autoVariablesMap["REPORT_PAGE_COUNT"] = "Total number of pages";
				VariablesWidget* autoVariables = new VariablesWidget(autoVariablesMap, true /*readOnly*/);
				l->addWidget(autoVariables);
				m_variablesTabSplitter->addWidget(w);
			}

			m_variablesTabSplitter->restoreState(QSettings().value("DialogSchemasReport/variablesTabSplitterState", m_variablesTabSplitter->saveState()).toByteArray());
			m_variablesTabSplitter->setChildrenCollapsible(false);

			QWidget* w = new QWidget();
			QVBoxLayout* l = new QVBoxLayout(w);
			l->addWidget(m_variablesTabSplitter);
			tabWidget->addTab(w, tr("Variables"));
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

	// Restore size
	//
	QSettings settings;
	QByteArray ba = settings.value("DialogSchemasReport/Geometry").toByteArray();
	if (ba.isEmpty() == false)
	{
		restoreGeometry(ba);
	}
	else
	{
		resize(400, 600);
	}
}

DialogSchemasReport::~DialogSchemasReport()
{
	QSettings settings;

	settings.setValue("DialogSchemasReport/Geometry", saveGeometry());
	settings.setValue("DialogSchemasReport/schemasTabSplitterState", m_schemasTabSplitter->saveState());
	settings.setValue("DialogSchemasReport/variablesTabSplitterState", m_variablesTabSplitter->saveState());
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
	std::vector<Builder::SchemaTypesParams> editSchemaTypesParams;

	bool singleFile = m_checkSignleFile->isChecked();

	for (const auto& params : m_schemaTypesParams)
	{
		// Show single file param if single file option is set, otherwise show all others except single file
		//
		bool singleFileParam = params.hasFileId() == false;
		if (singleFileParam == singleFile)
		{
			editSchemaTypesParams.push_back(params);
		}
	}

	DialogReportPageSetup d(editSchemaTypesParams, m_defaultFileTypeParams, this);
	if (d.exec() == QDialog::Accepted)
	{
		editSchemaTypesParams = d.schemaTypesParams();

		for (auto& params : m_schemaTypesParams)
		{
			for (const auto& editParams : editSchemaTypesParams)
			{
				if (params.fileId() == editParams.fileId())
				{
					params = editParams;
				}
			}
		}
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

		int fileId = item->data(0, Qt::UserRole).toInt();

		for (auto& stp : m_schemaTypesParams)
		{
			if (fileId == stp.fileId())
			{
				stp.setSelected(item->checkState(0) == Qt::Checked);
			}
		}
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
	
	m_options.setSignleFile(m_checkSignleFile->isChecked());
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

	value = m_editContentsTextFontSize->text().toInt(&ok);
	if (ok == false)
	{
		m_editContentsTextFontSize->setFocus();
		return false;
	}
	m_options.setContentsTextFontSize(value);

	value = m_editContentsTableFontSize->text().toInt(&ok);
	if (ok == false)
	{
		m_editContentsTableFontSize->setFocus();
		return false;
	}
	m_options.setContentsTableFontSize(value);

	value = m_editTextFontSize->text().toInt(&ok);
	if (ok == false)
	{
		m_editTextFontSize->setFocus();
		return false;
	}
	m_options.setTextFontSize(value);

	value = m_editTableFontSize->text().toInt(&ok);
	if (ok == false)
	{
		m_editTableFontSize->setFocus();
		return false;
	}
	m_options.setTableFontSize(value);

	// Save variables
	//
	m_options.setUserVariables(m_userVariables->getVariables());
	m_options.setProjectVariables(m_projectVariables->getVariables());

	return true;

}