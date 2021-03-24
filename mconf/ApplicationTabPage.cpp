#include "Stable.h"
#include "ApplicationTabPage.h"
#include "../lib/OutputLog.h"

#include "../lib/Configurator.h"

ApplicationTabPage::ApplicationTabPage(bool expertMode, QWidget *parent)
	: QWidget(parent),
	  m_expertMode(expertMode)
{

	QVBoxLayout* pLeftLayout = new QVBoxLayout();

	//
	pLeftLayout->addWidget(new QLabel(tr("Bitstream File Name:")));

	QHBoxLayout* bl = new QHBoxLayout();

	m_fileNameEdit = new QLineEdit();
	m_fileNameEdit->setReadOnly(true);

	QPushButton* b = new QPushButton(tr("Browse..."));
	bl->addWidget(m_fileNameEdit);
	bl->addWidget(b);

	connect(b, &QPushButton::clicked, this, &ApplicationTabPage::openFileClicked);

	pLeftLayout->addLayout(bl);

	// Create Subsystems list widget

	pLeftLayout->addWidget(new QLabel(tr("Choose the Subsystem:")));
	m_subsystemsListTree = new QTreeWidget();
	m_subsystemsListTree->setRootIsDecorated(false);
	connect(m_subsystemsListTree, &QTreeWidget::currentItemChanged, this, &ApplicationTabPage::subsystemChanged);
	pLeftLayout->addWidget(m_subsystemsListTree, 3);

	QStringList l;
	l << tr("Subsystem");

	m_subsystemsListTree->setColumnCount(l.size());
	m_subsystemsListTree->setHeaderLabels(l);

	int il = 0;
	m_subsystemsListTree->setColumnWidth(il++, 100);

	// Detect Subsystem

	bl = new QHBoxLayout();
	bl->addStretch();

	b = new QPushButton(tr("Detect Subsystem"));
	connect(b, &QPushButton::clicked, this, &ApplicationTabPage::detectSubsystemsClicked);
	bl->addWidget(b);

	pLeftLayout->addLayout(bl);

	// BTS UART list widget

	pLeftLayout->addWidget(new QLabel(tr("Bitstream Firmware Types:")));
	m_bitstreamUartListTree = new QTreeWidget();
	m_bitstreamUartListTree->setRootIsDecorated(false);
	pLeftLayout->addWidget(m_bitstreamUartListTree, 1);

	l.clear();
	l << tr("UartID");
	l << tr("Type");
	l << tr("Upload Count");
	l << tr("Status");

	m_bitstreamUartListTree->setColumnCount(l.size());
	m_bitstreamUartListTree->setHeaderLabels(l);

	il = 0;
	m_bitstreamUartListTree->setColumnWidth(il++, 80);
	m_bitstreamUartListTree->setColumnWidth(il++, 100);
	m_bitstreamUartListTree->setColumnWidth(il++, 100);
	m_bitstreamUartListTree->setColumnWidth(il++, 60);

	// Reset Counters

	bl = new QHBoxLayout();
	bl->addStretch();

	b = new QPushButton(tr("Reset Upload Counters"));
	bl->addWidget(b);
	connect(b, &QPushButton::clicked, this, &ApplicationTabPage::resetCountersClicked);

	pLeftLayout->addLayout(bl);

	//pLeftLayout->addStretch();

	if (expertMode == true)
	{
		// UART List Widget

		m_uartIdTypes[0x101] = tr("AppLogic");
		m_uartIdTypes[0x102] = tr("Configuration");
		m_uartIdTypes[0x103] = tr("Service");
		m_uartIdTypes[0x104] = tr("Tuning");

		pLeftLayout->addWidget(new QLabel(tr("Module UART List:")));
		m_pUartsListTree = new QTreeWidget();
		m_pUartsListTree->setRootIsDecorated(false);
		pLeftLayout->addWidget(m_pUartsListTree, 1);

		l.clear();
		l << tr("UartID");
		l << tr("Type");
		l << tr("Process");

		m_pUartsListTree->setColumnCount(l.size());
		m_pUartsListTree->setHeaderLabels(l);

		il = 0;
		m_pUartsListTree->setColumnWidth(il++, 80);
		m_pUartsListTree->setColumnWidth(il++, 100);
		m_pUartsListTree->setColumnWidth(il++, 80);

		// Add an empty item
		//
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(static_cast<int>(UartListColumn::Id), tr("Default"));
		item->setData(static_cast<int>(UartListColumn::Id), Qt::UserRole, -1);
		m_pUartsListTree->addTopLevelItem(item);

		// Reset Counters

		bl = new QHBoxLayout();
		bl->addStretch();

		b = new QPushButton(tr("Detect UART List"));
		bl->addWidget(b);
		connect(b, &QPushButton::clicked, this, &ApplicationTabPage::detectUartsClicked);

		pLeftLayout->addLayout(bl);
	}

	//
	setLayout(pLeftLayout);


}

ApplicationTabPage::~ApplicationTabPage()
{

}

bool ApplicationTabPage::isFileLoaded() const
{
	return m_firmware.isEmpty() == false;
}

ModuleFirmwareStorage *ApplicationTabPage::configuration()
{
	return &m_firmware;
}

QString ApplicationTabPage::selectedSubsystem() const
{
	for (QTreeWidgetItem* item : m_subsystemsListTree->selectedItems())
	{
		QVariant vData = item->data(columnSubsysId, Qt::UserRole);

		if (vData.isValid())
		{
			return vData.toString();
		}
	}

	return QString();
}

void ApplicationTabPage::selectSubsystem(const QString& id)
{
	m_subsystemsListTree->clearSelection();

	int count = m_subsystemsListTree->topLevelItemCount();

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_subsystemsListTree->topLevelItem(i);

		QVariant vData = item->data(columnSubsysId, Qt::UserRole);

		if (vData.isValid() && vData.toString() == id)
		{
			m_subsystemsListTree->setCurrentItem(item);
			return;
		}
	}
}

std::optional<std::vector<int>> ApplicationTabPage::selectedUarts() const
{
	if (m_expertMode == false)
	{
		return {};
	}

	std::vector<int> selectedUarts;

	if ( m_pUartsListTree == nullptr)
	{
		Q_ASSERT(m_pUartsListTree);
		return {};
	}

	int count = m_pUartsListTree->topLevelItemCount();
	if (count == 0)
	{
		// List is empty, no result is returned
		return {};
	}

	bool uartIdFound = false;

	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item =	m_pUartsListTree->topLevelItem(i);

		bool ok = false;

		int uartId = item->data(static_cast<int>(UartListColumn::Id), Qt::UserRole).toInt(&ok);
		if (ok == false || uartId <= 0)
		{
			continue;
		}

		uartIdFound = true;

		if (item->checkState(static_cast<int>(UartListColumn::Process)) == Qt::Checked)
		{
			selectedUarts.push_back(uartId);
		}
	}

	if (uartIdFound == false)
	{
		return {};
	}

	return selectedUarts;
}

void ApplicationTabPage::subsystemChanged(QTreeWidgetItem* item1, QTreeWidgetItem* item2)
{
	Q_UNUSED(item1);
	Q_UNUSED(item2);


	QTreeWidgetItem* subsystemItem = m_subsystemsListTree->currentItem();
	if (subsystemItem == nullptr)
	{
		return;
	}

	QString subsystemId = subsystemItem->data(columnSubsysId, Qt::UserRole).toString();

	bool ok = false;
	const ModuleFirmware& mf = m_firmware.firmware(subsystemId, &ok);
	if (ok == false)
	{
		assert(false);
		return;
	}

	const std::vector<UartPair>& uartList = mf.uartList();

	m_bitstreamUartListTree->clear();

	for (auto it : uartList)
	{
		int uartID = it.first;
		QString uartType = it.second;

		QStringList l;
		l << tr("%1h").arg(QString::number(uartID, 16));
		l << uartType;
		l << "0";

		QTreeWidgetItem* uartItem = new QTreeWidgetItem(l);
		uartItem->setData(columnUartId, Qt::UserRole, uartID);
		uartItem->setData(columnUploadCount, Qt::UserRole, 0);

		m_bitstreamUartListTree->addTopLevelItem(uartItem);
	}
}

void ApplicationTabPage::openFileClicked()
{
	QFileDialog fd(this);

	fd.setAcceptMode(QFileDialog::AcceptOpen);
	fd.setFileMode(QFileDialog::ExistingFile);

	QStringList filters;
	filters << "Bitstream files (*.bts)"
			<< "All files (*.*)";

	fd.setNameFilters(filters);

	if (fd.exec() == QDialog::Rejected)
	{
		return;
	}

	QStringList fileList = fd.selectedFiles();

	if (fileList.size() != 1)
	{
		return;
	}

	QString fileName = fileList[0];

	if (QFile::exists(fileName) == false)
	{
		QMessageBox::critical(this, qApp->applicationName(), tr("File %1 doesn't exists.").arg(fileName));
		return;
	}

	m_fileNameEdit->setText(fileName);

	clearSubsystemsUartData();

	emit loadBinaryFile(fileName, &m_firmware);

	return;
}

void ApplicationTabPage::loadBinaryFileHeaderComplete()
{
	clearSubsystemsUartData();

	QStringList subsystemsList = m_firmware.subsystems();

	for (const QString& subsystemId : subsystemsList)
	{
		QTreeWidgetItem* subsystemItem = new QTreeWidgetItem(QStringList() << subsystemId);
		subsystemItem->setData(columnSubsysId, Qt::UserRole, subsystemId);

		m_subsystemsListTree->sortByColumn(columnUartId, Qt::AscendingOrder);
		m_subsystemsListTree->addTopLevelItem(subsystemItem);
	}

	// User must think
	//
	//if (m_pSubsystemsListWidget->topLevelItemCount() > 0)
	//{
		//m_pSubsystemsListWidget->setCurrentItem(m_pSubsystemsListWidget->topLevelItem(0));
	//}
}

void ApplicationTabPage::resetCountersClicked()
{
	int count = m_bitstreamUartListTree->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_bitstreamUartListTree->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		item->setData(columnUploadCount, Qt::UserRole, 0);
		item->setText(columnUploadCount, "0");
	}
}

void ApplicationTabPage::detectSubsystemsClicked()
{
	theLog.writeMessage("");
	theLog.writeMessage(tr("Detecting Subsystem..."));

	emit detectSubsystem();
}

void ApplicationTabPage::detectUartsClicked()
{
	theLog.writeMessage("");
	theLog.writeMessage(tr("Detecting UARTs supported by module..."));

	emit detectUarts();
}

void ApplicationTabPage::uartOperationStart(int uartID, QString operation)
{
	int count = m_bitstreamUartListTree->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_bitstreamUartListTree->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		bool ok = false;
		int itemUartId = item->data(columnUartId, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(ok);
			continue;
		}

		if (uartID == itemUartId)
		{
			item->setText(columnUartStatus, operation);
			return;
		}
	}
	return;
}

void ApplicationTabPage::uploadComplete(int uartID)
{
	int count = m_bitstreamUartListTree->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_bitstreamUartListTree->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}

		bool ok = false;
		int itemUartId = item->data(columnUartId, Qt::UserRole).toInt(&ok);
		if (ok == false)
		{
			Q_ASSERT(ok);
			continue;
		}

		if (uartID == itemUartId)
		{
			int itemUploadCount = item->data(columnUploadCount, Qt::UserRole).toInt(&ok);
			if (ok == false)
			{
				Q_ASSERT(ok);
				continue;
			}

			itemUploadCount++;

			item->setData(columnUploadCount, Qt::UserRole, itemUploadCount);
			item->setText(columnUploadCount, QString::number(itemUploadCount));

			return;
		}
	}
	return;
}

void ApplicationTabPage::detectSubsystemComplete(int subsystemId)
{
	theLog.writeMessage(tr("Subsystem Key is %1").arg(subsystemId));

	bool subsystemFound = false;

	QStringList subsystems = m_firmware.subsystems();
	for (auto s : subsystems)
	{
		bool ok = false;
		const ModuleFirmware& fw = m_firmware.firmware(s, &ok);
		if (ok == false)
		{
			assert(false);
			return;
		}

		if (subsystemId == fw.ssKey())
		{
			// Select the subsystem
			//
			theLog.writeMessage(tr("Subsystem ID is %1").arg(fw.subsysId()));
			selectSubsystem(fw.subsysId());
			subsystemFound = true;
			break;
		}
	}

	if (subsystemFound == false)
	{
		m_subsystemsListTree->clearSelection();
		theLog.writeMessage(tr("Subsystem ID is unknown"));
	}

	theLog.writeSuccess(tr("Successful."));
}

void ApplicationTabPage::detectUartsComplete(std::vector<int> uartIds)
{
	if (m_expertMode == false)
	{
		Q_ASSERT(false);
		return;
	}

	m_uartIds = uartIds;

	fillUartsList();
}

void ApplicationTabPage::enableControls()
{
	// Clear status of all Uarts
	//
	int count = m_bitstreamUartListTree->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item = m_bitstreamUartListTree->topLevelItem(i);
		if (item == nullptr)
		{
			assert(item);
			return;
		}
		item->setText(columnUartStatus, QString());
	}
}

void ApplicationTabPage::clearSubsystemsUartData()
{
	m_subsystemsListTree->clear();
	m_bitstreamUartListTree->clear();
}

void ApplicationTabPage::fillUartsList()
{
	if ( m_pUartsListTree == nullptr)
	{
		Q_ASSERT(m_pUartsListTree);
		return;
	}

	// Remember previous checked uarts

	std::map<int, bool> previousUarts;	// key is uartId, value is Process

	int count = m_pUartsListTree->topLevelItemCount();
	for (int i = 0; i < count; i++)
	{
		QTreeWidgetItem* item =	m_pUartsListTree->topLevelItem(i);
		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		bool ok = false;
		int uartId = item->data(static_cast<int>(UartListColumn::Id), Qt::UserRole).toInt(&ok);
		if (ok == false || uartId <= 0)
		{
			continue;
		}

		previousUarts[uartId] = item->checkState(static_cast<int>(UartListColumn::Process)) == Qt::Checked;
	}

	// Fill new list

	m_pUartsListTree->clear();

	if (m_uartIds.empty() == false)
	{
		for (int uartId : m_uartIds)
		{

			QString numberStr = QString::number(uartId, 16) + "h";

			QString typeStr = tr("Custom");

			auto typeIt = m_uartIdTypes.find(uartId);
			if (typeIt != m_uartIdTypes.end())
			{
				typeStr = typeIt->second;
			}

			QTreeWidgetItem* item = new QTreeWidgetItem();
			item->setText(static_cast<int>(UartListColumn::Id), numberStr);
			item->setText(static_cast<int>(UartListColumn::Type), typeStr);
			item->setData(static_cast<int>(UartListColumn::Id), Qt::UserRole, uartId);

			if (uartId == ConfigurationUartId)
			{
				item->setText(static_cast<int>(UartListColumn::Process), "N/A");
			}
			else
			{
				item->setText(static_cast<int>(UartListColumn::Process), QString());
				item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

				// Check previously checked or new items

				bool checked = true;

				auto it = previousUarts.find(uartId);
				if (it != previousUarts.end())
				{
					checked = it->second;
				}

				item->setCheckState(static_cast<int>(UartListColumn::Process), checked ? Qt::Checked : Qt::Unchecked);
			}

			m_pUartsListTree->addTopLevelItem(item);
		}
	}

	return;
}
