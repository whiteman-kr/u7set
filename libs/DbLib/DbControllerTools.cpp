#ifndef DB_LIB_DOMAIN
#error Do not include this file in the project! Link DbLib instead.
#endif

#include <DbLib/DbControllerTools.h>
#include <DbLib/DbController.h>
#include <HardwareLib/DeviceAppSignal.h>
#include "../AppSignalLib/AppSignal.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QTreeWidget>
#include <QVBoxLayout>


std::pair<int, std::vector<int>> DbControllerTools::showSelectFolderDialog(DbController* db, int parentFileId, int currentSelectionFileId, bool showRootFile, QWidget* parentWidget)
{
	// Show dialog with file tree to select file, can be used as parent.
	// function returns selected file id or -1 if operation canceled
	//
	QDialog d(parentWidget, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint);
	d.setWindowTitle(QObject::tr("Select parent"));

	DbFileTree files;

	if (bool ok = db->getFileListTree(&files, parentFileId, true, parentWidget);
		ok == false)
	{
		return {-1, {}};
	}

	files.removeIf([](const DbFileInfo& fi)
		{
			return fi.directoryAttribute() == false;
		});

	std::shared_ptr<DbFileInfo> schemaFile = files.rootFile();		// SchemaFile
	Q_ASSERT(schemaFile->directoryAttribute() == true);

	// --
	//
	QLabel* textLabel = new QLabel(QObject::tr("Select parent for new file"));

	QTreeWidget* treeWidget = new QTreeWidget;
	treeWidget->setSortingEnabled(true);
	treeWidget->sortItems(0, Qt::SortOrder::AscendingOrder);
	treeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	treeWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	treeWidget->setHeaderLabel("File");

	static QIcon staticFolderIcon(":/Images/Images/SchemaFolder.svg");
	const QIcon* const ptrToIcon = &staticFolderIcon;
	QTreeWidgetItem* treeItemToSelect = nullptr;

	std::function<void(std::shared_ptr<DbFileInfo>, QTreeWidgetItem*)> addChilderenFilesFunc =
		[&addChilderenFilesFunc, &files, treeWidget, currentSelectionFileId, &treeItemToSelect, ptrToIcon](std::shared_ptr<DbFileInfo> parent, QTreeWidgetItem* parentTreeItem)
		{
			Q_ASSERT(parent->isNull() == false);

			const auto& childeren = files.children(parent->fileId());

			for (auto file : childeren)
			{
				if (file->isNull() == true ||
					file->directoryAttribute() == false)
				{
					Q_ASSERT(file->isNull() == false);
					Q_ASSERT(file->directoryAttribute() == true);
					return;
				}

				QTreeWidgetItem* treeItem = nullptr;

				if (parentTreeItem == nullptr)
				{
					treeItem = new QTreeWidgetItem(treeWidget, {file->fileName()}, file->fileId()) ;
					treeWidget->addTopLevelItem(treeItem);
				}
				else
				{
					treeItem = new QTreeWidgetItem(parentTreeItem, {file->fileName()}, file->fileId()) ;
				}
				treeItem->setIcon(0, *ptrToIcon);

				addChilderenFilesFunc(file, treeItem);

				if (file->fileId() == currentSelectionFileId)
				{
					treeItem->setSelected(true);
					treeItemToSelect = treeItem;
				}
			}
		};

	QTreeWidgetItem* rootTreeItem = nullptr;
	if (showRootFile == true)
	{
		rootTreeItem = new QTreeWidgetItem(treeWidget, {schemaFile->fileName()}, schemaFile->fileId()) ;
		rootTreeItem->setIcon(0, staticFolderIcon);
		treeWidget->addTopLevelItem(rootTreeItem);

		if (schemaFile->fileId() == currentSelectionFileId)
		{
			rootTreeItem->setSelected(true);
			treeItemToSelect = rootTreeItem;
		}
	}

	addChilderenFilesFunc(schemaFile, rootTreeItem);

	if (treeItemToSelect != nullptr)
	{
		treeWidget->scrollToItem(treeItemToSelect);
	}

	treeWidget->expandRecursively(QModelIndex(), 1);

	QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	// --
	//
	QVBoxLayout* layout = new QVBoxLayout;

	layout->addWidget(textLabel);
	layout->addWidget(treeWidget);
	layout->addWidget(buttonBox);

	d.setLayout(layout);

	QObject::connect(buttonBox, &QDialogButtonBox::accepted, &d, &QDialog::accept);
	QObject::connect(buttonBox, &QDialogButtonBox::rejected, &d, &QDialog::reject);

	int result = d.exec();
	if (result == QDialog::Accepted)
	{
		QList<QTreeWidgetItem*> selected = treeWidget->selectedItems();
		if (selected.size() != 1)
		{
			return {-1, {}};
		}

		std::vector<int> selectedParents;	// FileID of all parent items of selected item

		QTreeWidgetItem* selectedParent = selected.front()->parent();
		while (selectedParent != nullptr)
		{
			selectedParents.push_back(selectedParent->type());

			selectedParent = selectedParent->parent();
		}

		return {selected.front()->type(), selectedParents};
	}

	return {-1, {}};
}

std::shared_ptr<Hardware::DeviceObject> DbControllerTools::deviceObjectFromDbFile(const DbFile& file)
{
	std::shared_ptr<Hardware::DeviceObject> object = Hardware::DeviceObject::Create(file.data());
	Q_ASSERT(object != nullptr);

	if (object != nullptr)
	{
		auto fileInfo = std::make_shared<DbFileInfo>(file, object->details());
		object->setData(fileInfo);
	}

	return object;
}

std::vector<std::shared_ptr<Hardware::DeviceObject>> DbControllerTools::deviceObjectFromDbFiles(const std::vector<std::shared_ptr<DbFile>>& files)
{
	std::vector<std::shared_ptr<Hardware::DeviceObject>> result;
	result.reserve(files.size());

	for (const std::shared_ptr<DbFile>& f : files)
	{
		std::shared_ptr<Hardware::DeviceObject> object = deviceObjectFromDbFile(*f.get());
		result.push_back(object);
	}

	return result;
}

QString DbControllerTools::initAppSignalFromDeviceAppSignal(const Hardware::DeviceAppSignal& deviceSignal, AppSignal* appSignal)
{
	if (appSignal == nullptr)
	{
		Q_ASSERT(false);
		return QString("Null pointer");
	}

	QString errMsg;

	//
	// Identificators intialization
	//

	QString deviceSignalEquipmentID = deviceSignal.equipmentIdTemplate();

	QString appSignalID;
	QString customAppSignalID;
	QString appSignalCaption;

	if (deviceSignal.propertyExists(EquipmentPropNames::APP_SIGNAL_ID_TEMPLATE) == true)
	{
		appSignalID = Hardware::expandDeviceSignalTemplate(deviceSignal,
														   deviceSignal.propertyValue((EquipmentPropNames::APP_SIGNAL_ID_TEMPLATE)).toString(),
														   &errMsg);

		if (errMsg.isEmpty() == false)
		{
			return errMsg;
		}
	}
	else
	{
		appSignalID = QString("#%1").arg(deviceSignalEquipmentID);
	}

	if (deviceSignal.propertyExists(EquipmentPropNames::CUSTOM_APP_SIGNAL_ID_TEMPLATE) == true)
	{
		// customSignalIDTemplate will be expand in compile time
		//
		customAppSignalID = deviceSignal.propertyValue(EquipmentPropNames::CUSTOM_APP_SIGNAL_ID_TEMPLATE).toString();
	}
	else
	{
		customAppSignalID = deviceSignalEquipmentID;
	}

	if (deviceSignal.propertyExists(EquipmentPropNames::APP_SIGNAL_CAPTION_TEMPLATE) == true)
	{
		appSignalCaption = deviceSignal.propertyValue(EquipmentPropNames::APP_SIGNAL_CAPTION_TEMPLATE).toString();
	}
	else
	{
		appSignalCaption = deviceSignal.caption();
	}

	//
	// Tuning settings checking
	//

	bool enableTuning = false;

	QVariant tuningLowBound;
	QVariant tuningHighBound;
	QVariant tuningDefaultValue;

	if (deviceSignal.propertyExists(EquipmentPropNames::ENABLE_TUNING) == true)
	{
		if (deviceSignal.propertyExists(EquipmentPropNames::TUNING_DEFAULT_VALUE) == false ||
			deviceSignal.propertyExists(EquipmentPropNames::TUNING_LOW_BOUND) == false ||
			deviceSignal.propertyExists(EquipmentPropNames::TUNING_HIGH_BOUND) == false)
		{
			return QString("Not all required properties for tuning settings initialization is exists in device signal %1").
				arg(deviceSignalEquipmentID);
		}

		switch (deviceSignal.signalType())
		{
		case E::SignalType::Analog:
		case E::SignalType::Discrete:
			break;

		default:
			return QString("Device signal %1 is not Analog or Discrete. Tuning is no allowed.").
				arg(deviceSignalEquipmentID);
		}

		switch (deviceSignal.function())
		{
		case E::SignalFunction::Output:
			break;

		default:
			return QString("Device signal %1 is not Output. Tuning is no allowed.").
				arg(deviceSignalEquipmentID);
		}

		enableTuning = deviceSignal.propertyValue(EquipmentPropNames::ENABLE_TUNING).toBool();

		tuningLowBound = deviceSignal.propertyValue(EquipmentPropNames::TUNING_LOW_BOUND);
		tuningHighBound = deviceSignal.propertyValue(EquipmentPropNames::TUNING_HIGH_BOUND);
		tuningDefaultValue = deviceSignal.propertyValue(EquipmentPropNames::TUNING_DEFAULT_VALUE);
	}

	errMsg = appSignal->initFromDeviceSignal(deviceSignalEquipmentID,
											 deviceSignal.signalType(),
											 deviceSignal.function(),
											 appSignalID,
											 customAppSignalID,
											 appSignalCaption,
											 deviceSignal.appSignalBusTypeId(),
											 deviceSignal.appSignalDataFormat(),
											 deviceSignal.signalSpecPropsStruct(),
											 enableTuning,
											 tuningLowBound,
											 tuningHighBound,
											 tuningDefaultValue);
	return errMsg;
}
