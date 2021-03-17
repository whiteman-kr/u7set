#include "DialogChoosePreset.h"
#include "ui_DialogChoosePreset.h"
#include "../../lib/DbController.h"


int DialogChoosePreset::m_lastSortColumn = 0;
QString DialogChoosePreset::m_lastSelectedPreset;
Qt::SortOrder DialogChoosePreset::m_lastSortOrder = Qt::SortOrder::AscendingOrder;


DialogChoosePreset::DialogChoosePreset(QWidget* parent, DbController* db, Hardware::DeviceType selectedType, bool limitToSelectedType) :
	QDialog(parent, Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint),
	ui(new Ui::DialogChoosePreset),
	m_db(db)
{
	ui->setupUi(this);

	Q_ASSERT(m_db);

	QStringList columnNames{tr("Caption"), tr("Certification"), tr("PresetName"), tr("Type")};

	ui->m_presetTree->setColumnCount(columnNames.size());
	ui->m_presetTree->setHeaderLabels(columnNames);
	int il = 0;
	ui->m_presetTree->setColumnWidth(il++, 250);
	ui->m_presetTree->setColumnWidth(il++, 150);
	ui->m_presetTree->setColumnWidth(il++, 150);
	ui->m_presetTree->setColumnWidth(il++, 150);

	// Get file list
	//
	std::vector<DbFileInfo> fileList;

	bool ok = m_db->getFileList(&fileList, DbDir::HardwarePresetsDir, true, this);

	if (ok == false || fileList.empty() == true)
	{
		return;
	}

	// Read files from DB
	//
	std::vector<std::shared_ptr<DbFile>> files;
	files.reserve(fileList.size());

	ok = m_db->getLatestVersion(fileList, &files, this);

	if (ok == false || files.empty() == true)
	{
		return;
	}

	// Read DeviceObjects from raw data
	//
	m_presets.clear();
	m_presets.reserve(files.size());

	for (std::shared_ptr<DbFile>& f : files)
	{
		std::shared_ptr<Hardware::DeviceObject> object(Hardware::DeviceObject::fromDbFile(*f));
		Q_ASSERT(object != nullptr);

		m_presets.push_back(object);
	}

	// Choose preset
	//
	std::map<QString, QTreeWidgetItem*> categoryTreeItem;

	for (std::shared_ptr<Hardware::DeviceObject>& preset : m_presets)
	{
		Q_ASSERT(preset);

		Hardware::DeviceType presetType = preset->deviceType();

		if (static_cast<int>(presetType) >= static_cast<int>(Hardware::DeviceType::DeviceTypeCount))
		{
			Q_ASSERT(false);
			continue;
		}

		bool showPreset = false;

		if (limitToSelectedType == true)
		{
			showPreset = (selectedType == presetType);
		}
		else
		{
			switch (selectedType)
			{
			case Hardware::DeviceType::System:
				if (presetType == Hardware::DeviceType::Rack ||
					presetType == Hardware::DeviceType::Chassis ||
					presetType == Hardware::DeviceType::Module ||
					presetType == Hardware::DeviceType::Controller ||
					presetType == Hardware::DeviceType::Workstation)
				{
					showPreset = true;
				}
				break;

			case Hardware::DeviceType::Rack:
				if (presetType == Hardware::DeviceType::Chassis ||
					presetType == Hardware::DeviceType::Module ||
					presetType == Hardware::DeviceType::Controller ||
					presetType == Hardware::DeviceType::Workstation)
				{
					showPreset = true;
				}
				break;

			case Hardware::DeviceType::Chassis:
				if (presetType == Hardware::DeviceType::Module ||
					presetType == Hardware::DeviceType::Controller ||
					presetType == Hardware::DeviceType::Workstation)
				{
					showPreset = true;
				}
				break;

			case Hardware::DeviceType::Module:
				if (presetType == Hardware::DeviceType::Controller)
				{
					showPreset = true;
				}
				break;

			case Hardware::DeviceType::Workstation:
				if (presetType == Hardware::DeviceType::Controller ||
					presetType == Hardware::DeviceType::Software)
				{
					showPreset = true;
				}
				break;
			}
		}

		if (showPreset == false)
		{
			continue;
		}

		QStringList l;
		l << preset->caption();
		l << (preset->propertyExists("Certification") ? preset->propertyByCaption("Certification")->value().toString() : QString{});
		l << preset->caption();
		l << preset->presetName();
		l << Hardware::DeviceTypeNames[static_cast<int>(presetType)];

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		item->setData(0, Qt::UserRole, preset->presetName());

		ui->m_presetTree->addTopLevelItem(item);

		if (m_lastSelectedPreset == preset->presetName())
		{
			ui->m_presetTree->setCurrentItem(item);
		}
	}

	ui->m_presetTree->sortItems(m_lastSortColumn, m_lastSortOrder);
	ui->m_presetTree->setSortingEnabled(true);

	return;
}

DialogChoosePreset::~DialogChoosePreset()
{
	delete ui;
}

void DialogChoosePreset::showEvent(QShowEvent*)
{
	// Resize depends on monitor size, DPI, resolution
	//
	QRect screen = QDesktopWidget().availableGeometry(parentWidget());

	resize(static_cast<int>(screen.width() * 0.35),
		   static_cast<int>(screen.height() * 0.45));

	move(screen.center() - rect().center());

	return;
}

void DialogChoosePreset::on_DialogChoosePreset_accepted()
{
	QList<QTreeWidgetItem*> list = ui->m_presetTree->selectedItems();
	if (list.size() != 1)
	{
		return;
	}

	QString data = list[0]->data(0, Qt::UserRole).toString();

	for (std::shared_ptr<Hardware::DeviceObject>& p : m_presets)
	{
		if (data == p->presetName())
		{
			selectedPreset = p;
			m_lastSelectedPreset = p->presetName();
			return;
		}
	}

	Q_ASSERT(false);
	return;
}

void DialogChoosePreset::on_DialogChoosePreset_finished(int result)
{
	Q_UNUSED(result);

	m_lastSortColumn = ui->m_presetTree->header()->sortIndicatorSection();
	m_lastSortOrder = ui->m_presetTree->header()->sortIndicatorOrder();

	return;
}

void DialogChoosePreset::on_m_presetTree_doubleClicked(const QModelIndex& index)
{
	Q_UNUSED(index);

	on_DialogChoosePreset_accepted();
	accept();

	return;
}
