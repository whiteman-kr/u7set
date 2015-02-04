#pragma once

#include "MainTabPage.h"
#include "../include/DbStruct.h"
#include "FileListView.h"
#include "../include/ConfigData.h"

class ConfigurationFileView : public FileListView
{
	Q_OBJECT
public:
	ConfigurationFileView(DbController* dbcontroller);
	virtual ~ConfigurationFileView();

	// Methods
	//
public:
	virtual void openFile(std::vector<DbFileInfo> files) override;
	virtual void viewFile(std::vector<DbFileInfo> files) override;
	virtual void addFile() override;

signals:
	void openFileSignal(std::vector<DbFileInfo> files);
	void viewFileSignal(std::vector<DbFileInfo> files);

	// Data
	//
protected:
};


class ConfigurationsTabPage : public MainTabPage
{
	Q_OBJECT
public:
	ConfigurationsTabPage(DbController* dbcontroller, QWidget* parent);
	virtual ~ConfigurationsTabPage();

protected:
	void CreateActions();

	// Events
	//
protected:
	virtual void closeEvent(QCloseEvent*) override;

public slots:
	void projectOpened();
	void projectClosed();

	void openFiles(std::vector<DbFileInfo> files);
	void viewFiles(std::vector<DbFileInfo> files);

	// Data
	//
private:
	ConfigurationFileView* m_filesView;
	QTabWidget* m_tabWidget;
	QSplitter* m_splitter;
};


class ModuleConfigurationTabPage : public QWidget
{
	Q_OBJECT
private:
	ModuleConfigurationTabPage();

public:
	ModuleConfigurationTabPage(DbController* pDbController);
    virtual ~ModuleConfigurationTabPage();

	// Methods
	//
public:
	bool load(const DbFile& file);
    void save(DbFile* file);

private:
    void fillTree();
    void appendStructItems(ConfigDataModelNode *parentNode, const ConfigConfiguration& config, const std::shared_ptr<ConfigStruct> &pData);

private slots:
    void on_tree_doubleClicked(QModelIndex index);
    void saveFile();
    //void loadData();
    void compileFile();

    // Properties
	//
protected:
	DbController* dbController();

public:
    const ConfigData& configData() const;

	bool readOnly() const;
	void setReadOnly(bool value);

	// Data
	//
private:
    QTreeView* m_tree;
    ConfigDataModel* model;
    QPushButton* m_undo;
    QPushButton* m_save;
    //QPushButton* m_loadData;
    QPushButton* m_compile;

protected:
	ConfigData m_configData;
	bool m_readOnly;

	DbController* m_dbController;
};
