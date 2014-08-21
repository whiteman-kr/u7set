#include "Stable.h"
#include "ConfigurationsTabPage.h"
#include "../include/DbController.h"
#include "Settings.h"
#include "../include/ConfigData.h"
#include "ChangesetDialog.h"
#include "DialogValueEdit.h"

//
//
//	ConfigurationFileView
//
//
ConfigurationFileView::ConfigurationFileView(DbStore* dbstore) :
	FileView(dbstore)
{
	filesModel().setFilter("cdf");

	m_openFileAction->setEnabled(true);

	return;
}

ConfigurationFileView::~ConfigurationFileView()
{
}

void ConfigurationFileView::openFile(std::vector<DbFileInfo> files)
{
	emit openFileSignal(files);
	return;
}

void ConfigurationFileView::viewFile(std::vector<DbFileInfo> files)
{
	emit viewFileSignal(files);
	return;
}

void ConfigurationFileView::addFile()
{
/*	// Choose Configuration file name
	//
    bool ok = false;

	QString confName = QInputDialog::getText(this, tr("Choose configuration name"), tr("Configuration name:"), QLineEdit::Normal, "conf_blockname", &ok);
	if (ok == false)
	{
		return;
	}

	if (confName.isEmpty() == true)
	{
		QMessageBox msg(this);
		msg.setText(tr("Configuration name must not be empty."));
		msg.exec();
		return;
	}

    if (confName.endsWith(".cdf") == false)
	{
		confName += ".cdf";
	}

	// Create new CDF file and add it to the vcs
	//
	ConfigData configData;

    std::shared_ptr<DbFile> newConfFile = std::make_shared<DbFile>();
    configData.save(newConfFile.get());
	newConfFile->setFileName(confName);

	std::vector<std::shared_ptr<DbFile>> addFilesList;
	addFilesList.push_back(newConfFile);

	dbStore()->addFiles(&addFilesList, this);

	// Add file to the FileModel and select them
	//
	std::shared_ptr<DbFileInfo> file = std::make_shared<DbFileInfo>(*newConfFile.get());

	if (file->fileId() != -1)
	{
		selectionModel()->clear();

		filesModel().addFile(file);

		int fileRow = filesModel().getFileRow(file->fileId());

		if (fileRow != -1)
		{
			QModelIndex md = filesModel().index(fileRow, 0);		// m_filesModel.columnCount()
			selectionModel()->select(md, QItemSelectionModel::Select | QItemSelectionModel::Rows);
		}
	}

	filesViewSelectionChanged(QItemSelection(), QItemSelection());
	return;*/
}

//
//
//	ConfigurationsTabPage
//
//
ConfigurationsTabPage::ConfigurationsTabPage(DbController* dbcontroller, QWidget* parent) :
	MainTabPage(dbcontroller, parent)
{
/*	assert(dbcontroller != nullptr);

	// Create Actions
	//
	CreateActions();

	//
	// Controls
	//
	m_filesView = new ConfigurationFileView(dbcontroller);
	m_tabWidget = new QTabWidget();

	m_splitter = new QSplitter();

	m_splitter->addWidget(m_filesView);
	m_splitter->addWidget(m_tabWidget);

	m_splitter->setStretchFactor(0, 0);
	m_splitter->setStretchFactor(1, 2);

	m_splitter->restoreState(theSettings.m_configurationTabPageSplitterState);

	//
	// Layouts
	//

	QHBoxLayout* pMainLayout = new QHBoxLayout();

	pMainLayout->addWidget(m_splitter);

	setLayout(pMainLayout);

	// --
	//
	connect(dbController(), &DbStore::projectOpened, this, &ConfigurationsTabPage::projectOpened);
	connect(dbController(), &DbStore::projectClosed, this, &ConfigurationsTabPage::projectClosed);

	connect(m_filesView, &ConfigurationFileView::openFileSignal, this, &ConfigurationsTabPage::openFiles);
	connect(m_filesView, &ConfigurationFileView::viewFileSignal, this, &ConfigurationsTabPage::viewFiles);

	// Evidently, project is not opened yet
	//
	this->setEnabled(false);*/
}

ConfigurationsTabPage::~ConfigurationsTabPage()
{
	theSettings.m_configurationTabPageSplitterState = m_splitter->saveState();
	theSettings.writeUserScope();
}

void ConfigurationsTabPage::CreateActions()
{
	return;
}

void ConfigurationsTabPage::closeEvent(QCloseEvent* e)
{
	//	theSettings.m_configurationTabPageSplitterState = m_splitter->saveState();
	e->accept();
}

void ConfigurationsTabPage::projectOpened()
{
	this->setEnabled(true);
	return;
}

void ConfigurationsTabPage::projectClosed()
{
	this->setEnabled(false);
	return;
}

void ConfigurationsTabPage::openFiles(std::vector<DbFileInfo> files)
{
/*	if (files.empty() == true || files.size() != 1)
	{
		assert(files.empty() == false);
		return;
	}

	const DbFileInfo file = files[0];

	if (file.state() != VcsState::CheckedOut)
	{
		QMessageBox mb(this);
		mb.setText(tr("Check out file for edit first."));
		mb.exec();
		return;
	}

	if (file.state() == VcsState::CheckedOut && file.user() != dbController()->currentUser())
	{
		QMessageBox mb(this);
		mb.setText(tr("File %1 already checked out by user %2.").arg(file.fileName()).arg(file.user().username()));
		mb.exec();
		return;
	}

	assert(file.state() == VcsState::CheckedOut && file.user() == dbController()->currentUser());

	// Check if file already open, and activate file tab if it is
	//
	for (int i = 0; i < m_tabWidget->count(); i++)
	{
		ModuleConfigurationTabPage* mtb = dynamic_cast<ModuleConfigurationTabPage*>(m_tabWidget->widget(i));

		if (mtb == nullptr)
		{
			assert(mtb);
			continue;
		}

		if (mtb->configData().fileInfo().fileName() == file.fileName() && mtb->readOnly() == false)
		{
			m_tabWidget->setCurrentIndex(i);
			return;	 // !!!
		}
	}


	// Get file
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool result = dbController()->getWorkcopy(files, &out, this);
	if (result == false || out.size() != files.size())
	{
		return;
	}

	// Load configuration
	//
	ModuleConfigurationTabPage* mtb = new ModuleConfigurationTabPage(dbController());

	mtb->setReadOnly(false);

	int tabIndex = m_tabWidget->addTab(mtb, mtb->windowTitle());
	m_tabWidget->setCurrentWidget(mtb);

	mtb->load(*out[0].get());

	m_tabWidget->setTabText(tabIndex, mtb->windowTitle());

	return;*/
}

void ConfigurationsTabPage::viewFiles(std::vector<DbFileInfo> files)
{
/*	if (files.empty() == true || files.size() != 1)
	{
		assert(files.empty() == false);
		return;
	}

	const DbFileInfo file = files[0];

	// Get file history
	//
	std::vector<DbChangesetInfo> fileHistory;

	dbController()->getFileHistory(file, &fileHistory, this);

	// Show chageset dialog
	//
	int changesetId = ChangesetDialog::getChangeset(fileHistory, this);

	if (changesetId == -1)
	{
		return;
	}

	// Get file with choosen changeset
	//
	std::vector<std::shared_ptr<DbFile>> out;

	bool result = dbController()->getSpecificCopy(changesetId, files, &out, this);

	if (result == false || out.size() != files.size())
	{
		return;
	}

	// Load configuration
	//
    ModuleConfigurationTabPage* mtb = new ModuleConfigurationTabPage(dbController());

	mtb->setReadOnly(true);

	int tabIndex = m_tabWidget->addTab(mtb, mtb->windowTitle());
	m_tabWidget->setCurrentWidget(mtb);

	mtb->load(*out[0].get());

	m_tabWidget->setTabText(tabIndex, mtb->windowTitle());

	return;*/
}

//
//
// ModuleConfigurationTabPage
//
//

ModuleConfigurationTabPage::ModuleConfigurationTabPage()
{
    assert(false);
}

ModuleConfigurationTabPage::ModuleConfigurationTabPage(DbController* pDbController) :
	QWidget(nullptr),
    m_readOnly(false),
	m_dbController(pDbController)
{
	assert(pDbController != nullptr);

    // tree
    //
    model = new ConfigDataModel;
    m_tree = new QTreeView();
    m_tree->setModel(model);
    m_tree->setColumnWidth(0, 200);
    m_tree->setColumnWidth(1, 200);
    connect(m_tree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(on_tree_doubleClicked(QModelIndex)));

    //buttons
    //
    m_undo = new QPushButton(tr("Undo"));
    m_save = new QPushButton(tr("Save"));
    //m_loadData = new QPushButton(tr("Load Data"));
    m_compile = new QPushButton(tr("Compile"));
    connect(m_save, SIGNAL(clicked()), this, SLOT(saveFile()));
    //connect(m_loadData, SIGNAL(clicked()), this, SLOT(loadData()));
    connect(m_compile, SIGNAL(clicked()), this, SLOT(compileFile()));

    // layouts
    //
    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->addWidget(m_undo);
    hlayout->addWidget(m_save);
    //hlayout->addWidget(m_loadData);
    hlayout->addStretch();
    hlayout->addWidget(m_compile);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(m_tree);
    vlayout->addLayout(hlayout);
    setLayout(vlayout);

	return;
}

ModuleConfigurationTabPage::~ModuleConfigurationTabPage()
{
}

bool ModuleConfigurationTabPage::load(const DbFile& file)
{
	bool result = m_configData.load(file);

	if (result == true)
	{
		if (m_readOnly == false)
		{
            setWindowTitle(file.fileName());
		}
		else
		{
			setWindowTitle(QString("%1, ReadOnly").arg(file.fileName()));
		}

        fillTree();
	}

    m_save->setEnabled(readOnly() == false);
    m_compile->setEnabled(readOnly() == true);

	return result;
}

void ModuleConfigurationTabPage::save(DbFile* file)
{
	assert(file != nullptr);
    m_configData.save(file);

    return;
}

void ModuleConfigurationTabPage::on_tree_doubleClicked(QModelIndex index)
{
    if (readOnly() == true)
        return;

    if (index.isValid() == false)
        return;

    ConfigDataModelNode* editNode = model->nodeFromIndex(index);
    if (editNode == nullptr)
        return;

    if (editNode->type != ConfigDataModelNode::Value)
        return;

    ConfigValue* pValue = static_cast<ConfigValue *>(editNode->object.get());

    if (pValue == nullptr)
        return;

   if (pValue->pData() != nullptr)
       return;

    DialogValueEdit dlg(pValue);
    if (dlg.exec() == QDialog::Accepted)
    {
    }
}

void ModuleConfigurationTabPage::saveFile()
{
/*    std::shared_ptr<DbFile> sp = std::make_shared<DbFile>();
    save(sp.get());

    std::vector<std::shared_ptr<DbFile>> files;
    files.push_back(sp);

	dbController()->setWorkcopy(files, this);
	return;*/
}

void ModuleConfigurationTabPage::compileFile()
{
    QString defaultFileName = "Untitled.cdb";
    if (m_configData.configurations().size() != 0)
    {
        defaultFileName = m_configData.configurations()[0].name() + ".cdb";
    }

    QString fileName = QFileDialog::getSaveFileName(this, tr("Compile"), defaultFileName, tr("Binary Files(*.cdb)"));
    if (fileName.isEmpty())
        return;

    QByteArray data;
    if (m_configData.compile(data) == false)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error compiling file:") + fileName);
        return;
    }


    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)  == false || file.write(data) == -1)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error saving file:") + fileName);
        return;
    }
    file.close();

}

/*void ModuleConfigurationTabPage::loadData()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Load data"), "", tr("Data Files(*.cdd)"));
    if (fileName.isEmpty())
        return;


    if (m_configData.loadData(fileName) == true)
    {
        //fillTree();
        QMessageBox::critical(this, tr("Done"), tr("Done:") + fileName);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Error loading file:") + fileName);
    }

}*/

DbController* ModuleConfigurationTabPage::dbController()
{
	assert(m_dbController != nullptr);
	return m_dbController;
}

const ConfigData& ModuleConfigurationTabPage::configData() const
{
	return m_configData;
}

bool ModuleConfigurationTabPage::readOnly() const
{
	return m_readOnly;
}

void ModuleConfigurationTabPage::setReadOnly(bool value)
{
	m_readOnly = value;
}

void ModuleConfigurationTabPage::fillTree()
{
    ConfigDataModelNode* rootNode = new ConfigDataModelNode();

    for (int c = 0; c < m_configData.configurations().size(); c++)
    {
        ConfigConfiguration& config = m_configData.configurations()[c];

        std::shared_ptr<ConfigConfiguration> sp(&config);
        ConfigDataModelNode* configNode = new ConfigDataModelNode(sp, rootNode);
        rootNode->children.append(configNode);

        for (int v = 0; v < config.variables().size(); v++)
        {
            ConfigVariable& var = config.variables()[v];

            if (var.pData() != nullptr)
            {
                std::shared_ptr<ConfigVariable> sv(&var);
                ConfigDataModelNode* variableNode = new ConfigDataModelNode(sv, configNode);
                configNode->children.append(variableNode);

                appendStructItems(variableNode, config, var.pData());
            }
        }
    }

    model->setRootNode(rootNode);
}

void ModuleConfigurationTabPage::appendStructItems(ConfigDataModelNode *parentNode, const ConfigConfiguration& config, const std::shared_ptr<ConfigStruct> &pData)
{
    for (int m = 0; m < pData->values().size(); m++)
    {
        ConfigValue& val = pData->values()[m];

        std::shared_ptr<ConfigValue> sval(&val);
        ConfigDataModelNode* valNode = new ConfigDataModelNode(sval, parentNode);
        parentNode->children.append(valNode);

        if (val.pData() != nullptr)
        {
            appendStructItems(valNode, config, val.pData());
        }
    }
}
