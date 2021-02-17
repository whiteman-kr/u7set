#include "EquipmentVcsDialog.h"
#include "ui_EquipmentVcsDialog.h"
#include "../lib/DbController.h"

const wchar_t* EquipmentVcsDialog::ColumnName[] = {L"StrID", L"Caption", L"Place", L"Type", L"User", L"Action"};

EquipmentVcsDialog::EquipmentVcsDialog(DbController* db, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::EquipmentVcsDialog),
	m_db(db)
{
	ui->setupUi(this);
	assert(m_db);

	// Set splitter sizes, big for list and small for comment
	//
	QList<int> list= ui->splitter->sizes();

	list.replace(0, static_cast<int>(static_cast<double>(ui->splitter->height()) / 0.1));
	list.replace(1, static_cast<int>(static_cast<double>(ui->splitter->height()) / 0.9));

	ui->splitter->setSizes(list);

	// Set header for the tree
	//
	QStringList headerLabels;

	for (const wchar_t* cn : ColumnName)
	{
		headerLabels << QString::fromStdWString(std::wstring(cn));
	}

	ui->m_treeWidget->setHeaderLabels(headerLabels);

	// Get data from project database
	//
	on_m_refresh_clicked();

	return;
}

EquipmentVcsDialog::~EquipmentVcsDialog()
{
	delete ui;
}

void EquipmentVcsDialog::on_m_checkIn_clicked()
{

}

void EquipmentVcsDialog::on_m_undo_clicked()
{

}

void EquipmentVcsDialog::on_m_refresh_clicked()
{
	ui->m_treeWidget->clear();

	if (m_db->isProjectOpened() == false || m_db->hcFileId() == -1)
	{
		return;
	}

	// Get checked out files
	//
	//assert(false);
//	std::list<std::shared_ptr<DbFile>> files;

//	DbFileInfo parentFile = m_db->systemFileInfo(m_db->hcFileId());
//	assert(parentFile.fileId() != -1);

//	bool ok = m_db->getCheckedOutFiles(parentFile, &files, this);

//	if (ok == false || files.empty() == true)
//	{
//		return;
//	}

//	// Read files to DeviceObject
//	//
//	std::map<int, std::shared_ptr<Hardware::DeviceObject>> objectsMap;	// key is fileId

//	for (const std::shared_ptr<DbFile>& f : files)
//	{
//		std::shared_ptr<Hardware::DeviceObject> object(Hardware::DeviceObject::fromDbFile(*f));
//		objectsMap[object->fileInfo().fileId()] = object;
//	}


	//---------------------_
	//---------------------_
	//---------------------_

	// Set list of trees
	//
//	bool rootWasFound = false;
//	for (const auto& pair : objectsMap)
//	{
//		// Get parentId
//		//
//		int fileId = pair.second->fileInfo().fileId();
//		int parentId = pair.second->fileInfo().parentId();

//		auto parentIterator = objectsMap.find(parentId);
//		if (parentIterator == objectsMap.end())
//		{
//			// Apparently it is the root item, so, we have to check it and set flag that we already found it
//			//
//			assert(rootWasFound == false);
//			assert(file.fileId() == fileId);

//			*out = objectsMap[fileId];	// !!! HOW TO USE parentIterator->second; ?????
//			rootWasFound = true;

//			continue;
//		}

//		(*parentIterator).second->addChild(pair.second);
//	}

//	if (rootWasFound == false)
//	{
//		assert(rootWasFound == true);
//		return false;
//	}


	return;
}

void EquipmentVcsDialog::on_m_ok_clicked()
{
	accept();
}
