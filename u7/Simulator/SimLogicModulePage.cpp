#include "SimLogicModulePage.h"
#include "SimSignalSnapshot.h"
#include "SimWidget.h"

SimLogicModulePage::SimLogicModulePage(SimIdeSimulator* simulator, VFrame30::AppSignalController* appSignalController,
									   QString equipmentId,
									   QWidget* parent)
	: SimBasePage(simulator, parent),
	  m_lmEquipmentId(equipmentId),
	  m_appSignalController(appSignalController)
{
	assert(m_simulator);
	assert(m_appSignalController);
	assert(m_lmEquipmentId.isEmpty() == false);

	// --
	//
#if defined(Q_OS_WIN)
		QFont font = QFont("Consolas");
#else
		QFont font = QFont("Courier");
#endif
	m_subsystemIdLabel->setFont(font);
	m_equipmentIdLabel->setFont(font);
	m_channelLabel->setFont(font);

	m_subsystemIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_equipmentIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_channelLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

	m_disableButton->setCheckable(true);
	m_disableButton->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	QStringList schemaListHeader;
	schemaListHeader << tr("SchemaID");
	schemaListHeader << tr("Caption");
	m_schemasList->setHeaderLabels(schemaListHeader);

	m_schemasList->setItemsExpandable(false);
	m_schemasList->setRootIsDecorated(false);
	m_schemasList->setUniformRowHeights(true);
	m_schemasList->setShortcutEnabled(false);
	m_schemasList->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

	m_schemasList->setSortingEnabled(true);
	m_schemasList->sortByColumn(0, Qt::SortOrder::AscendingOrder);


	m_schemaFilterEdit->setPlaceholderText(tr("Schema Filter: Start typing IDs, Labels, Signals' IDS etc"));
	m_schemaFilterEdit->setClearButtonEnabled(true);

	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_schemaFilterEdit->setCompleter(m_completer);

	m_splitter->setChildrenCollapsible(false);

	// Left side
	//
	{
		QWidget* leftWidget = new QWidget(m_splitter);

		QGridLayout* layout = new QGridLayout;
		leftWidget->setLayout(layout);

		int row = 0;

		layout->addWidget(m_subsystemIdLabel, row++, 0, 1, 3);
		layout->addWidget(m_equipmentIdLabel, row++, 0, 1, 3);
		layout->addWidget(m_channelLabel, row++, 0, 1, 3);

		layout->addWidget(m_disableButton, row, 0, 1, 1);
		layout->addWidget(m_stateLabel, row++, 1, 1, 1);

		// Add spacer
		//
		QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
		layout->addItem(spacer, row++, 0, 1, 3);

		// Buttons  Signals, Memory, Code
		//
		layout->addWidget(m_signalsButton, row, 0, 1, 1);
		layout->addWidget(m_memoryButton, row, 1, 1, 1);
		layout->addWidget(m_codeButton, row, 2, 1, 1);

		m_splitter->addWidget(leftWidget);
		m_splitter->setStretchFactor(0, 1);
	}

	// Right side
	//
	{
		QWidget* rightWidget = new QWidget(m_splitter);

		QGridLayout* layout = new QGridLayout;
		rightWidget->setLayout(layout);

		layout->addWidget(m_schemasLabel, 0, 0, 1, 3);
		layout->addWidget(m_schemasList, 1, 0, 1, 3);
		layout->addWidget(m_schemaFilterEdit, 2, 0, 1, 3);

		m_splitter->addWidget(rightWidget);
		m_splitter->setStretchFactor(1, 3);
	}

	QGridLayout* layout = new QGridLayout;
	this->setLayout(layout);
	layout->addWidget(m_splitter, 0, 0, 1, 1);

	// --
	//
	updateLogicModuleInfoInfo();
	m_schemasList->resizeColumnToContents(0);

	// --
	//
	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimLogicModulePage::projectUpdated);

	connect(m_disableButton, &QPushButton::toggled, this, &SimLogicModulePage::powerOff);

	connect(m_signalsButton, &QPushButton::clicked, this, &SimLogicModulePage::signalsButtonClicked);
	connect(m_codeButton, &QPushButton::clicked, this, &SimLogicModulePage::codeButtonClicked);
	connect(m_memoryButton, &QPushButton::clicked, this, &SimLogicModulePage::memoryButtonClicked);

	connect(m_schemaFilterEdit, &QLineEdit::textChanged, this, &SimLogicModulePage::schemaFilterChanged);

	connect(m_schemasList, &QTreeWidget::customContextMenuRequested, this, &SimLogicModulePage::schemaContextMenuRequested);
	connect(m_schemasList, &QTreeWidget::itemDoubleClicked, this, &SimLogicModulePage::schemaItemDoubleClicked);

	connect(&(m_simulator->control()), &Sim::Control::statusUpdate, this, &SimLogicModulePage::updateModuleStates);

	// --
	//
	QSettings s;

	if (QByteArray ba = s.value("Simulator/SimLogicModulePage/m_schemasList").toByteArray();
		ba.isEmpty() == false)
	{
		m_schemasList->header()->restoreState(ba);
	}

	if (QByteArray ba = s.value("Simulator/SimLogicModulePage/m_splitter").toByteArray();
		ba.isEmpty() == false)
	{
		m_splitter->restoreState(ba);
	}

	return;
}

SimLogicModulePage::~SimLogicModulePage()
{
	QSettings s;

	s.setValue("Simulator/SimLogicModulePage/m_schemasList", m_schemasList->header()->saveState());
	s.setValue("Simulator/SimLogicModulePage/m_splitter", m_splitter->saveState());

	return;
}

void SimLogicModulePage::updateLogicModuleInfoInfo()
{
	auto lm = logicModule();
	if (lm == nullptr)
	{
		return;
	}

	const Hardware::LogicModuleInfo& lmInfo = lm->logicModuleInfo();

	m_equipmentIdLabel->setText(QString("EquipmnetID: %1").arg(lmInfo.equipmentId));
	m_channelLabel->setText(QString("LmNumber: %1, Channel: %2")
								.arg(lmInfo.lmNumber)
								.arg(QChar('A' + static_cast<char>(lmInfo.channel))));

	// Fill schema list
	//
	fillSchemaList();

	updateFilterCompleter();

	return;
}

void SimLogicModulePage::fillSchemaList()
{
	Q_ASSERT(m_schemasList);
	m_schemasList->clear();

	// Get all schemas fro LM
	//
	std::vector<VFrame30::SchemaDetails> schemas = m_simulator->schemasForLm(equipmentId());

	// Filter data
	//
	QString filter = m_schemaFilterEdit->text().trimmed();

	if (filter.isEmpty() == false)
	{
		schemas.erase(std::remove_if(schemas.begin(),
									 schemas.end(),
									 [&filter](const auto& s) {	return s.searchForString(filter) == false; }),
					  schemas.end());
	}

	// Fill UI list
	//
	QList<QTreeWidgetItem*> treeItems;
	for (const VFrame30::SchemaDetails& s : schemas)
	{
		QStringList sl;
		sl << s.m_schemaId;
		sl << s.m_caption;

		QTreeWidgetItem* item = new QTreeWidgetItem(m_schemasList, sl);
		item->setData(0, Qt::UserRole, s.m_schemaId);

		treeItems.push_back(item);
	}

	m_schemasList->addTopLevelItems(treeItems);

	return;
}

void SimLogicModulePage::projectUpdated()
{
	updateLogicModuleInfoInfo();
	return;
}

void SimLogicModulePage::powerOff(bool toPowerOff)
{
	if (auto lm = logicModule();
		lm != nullptr && lm->isPowerOff() != toPowerOff)
	{
		lm->setPowerOff(toPowerOff);
	}

	return;
}

void SimLogicModulePage::signalsButtonClicked()
{
	// Show snapshot with applied filter lmEquipmnetId()
	//

	SimWidget* simWidget = nullptr;

	QWidget* pw = parentWidget();
	while (pw != nullptr)
	{
		if (dynamic_cast<SimWidget*>(pw) != nullptr)
		{
			simWidget = dynamic_cast<SimWidget*>(pw);
			break;
		}
		pw = pw->parentWidget();
	}

	if (simWidget == nullptr)
	{
		Q_ASSERT(simWidget);
		return;
	}

	SimDialogSignalSnapshot::showDialog(m_simulator, m_appSignalController, equipmentId(), simWidget);

	return;
}

void SimLogicModulePage::codeButtonClicked()
{
	emit openCodePageRequest(equipmentId());
	return;
}

void SimLogicModulePage::memoryButtonClicked()
{
	QString memoryDump = m_simulator->appSignalManager().ramDump(m_lmEquipmentId);

	QString fileNameTemplate = QDir::tempPath() + "/" + "ram_dump_" + m_lmEquipmentId + "_XXXXXX.txt";

	QTemporaryFile* file = new QTemporaryFile{fileNameTemplate, this};

	// The end of life of file is this object, so it will be deleted rigt after tab page is closed
	// thi is done to give time for QDesktopServices::openUrl(url) to open file
	//

	if (file->open() == false)
	{
		return;
	}

	QTextStream out(file);
	out << memoryDump;

	file->close();

	QUrl url = QUrl::fromLocalFile(file->fileName());
	QDesktopServices::openUrl(url);

	return;
}

void SimLogicModulePage::schemaFilterChanged()
{
	fillSchemaList();
	return;
}

void SimLogicModulePage::schemaContextMenuRequested(const QPoint& pos)
{
	Q_ASSERT(m_schemasList);

	QTreeWidgetItem* currentItem = m_schemasList->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QMenu menu;
	QAction* openAction = menu.addAction(tr("Open"));

	QAction* menuResult = menu.exec(m_schemasList->mapToGlobal(pos));

	if (menuResult == openAction)
	{
		openSelectedSchema();
	}

	return;
}

void SimLogicModulePage::schemaItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	if (item == nullptr)
	{
		return;
	}

	openSelectedSchema();
	return;
}

void SimLogicModulePage::openSelectedSchema()
{
	Q_ASSERT(m_schemasList);

	QTreeWidgetItem* currentItem = m_schemasList->currentItem();
	if (currentItem == nullptr)
	{
		return;
	}

	QString schemaId = currentItem->data(0, Qt::UserRole).toString();

	if (schemaId.isEmpty() == true)
	{
		assert(schemaId.isEmpty() == false);
		return;
	}

	emit openSchemaRequest(schemaId, {});

	return;
}

void SimLogicModulePage::updateFilterCompleter()
{
	// Get all schemas fro LM
	//
	std::vector<VFrame30::SchemaDetails> schemas = m_simulator->schemasForLm(equipmentId());

	QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_completer->model());

	if (completerModel == nullptr)
	{
		assert(completerModel);
		return;
	}

	QStringList completerList;

	for (const VFrame30::SchemaDetails& sd : schemas)
	{
		completerList.push_back(sd.m_schemaId);
		completerList.push_back(sd.m_equipmentId);

		for (const auto& s : sd.m_signals)
		{
			completerList.push_back(s);
		}

		for (const auto& l : sd.m_labels)
		{
			completerList.push_back(l);
		}

		for (const auto& c : sd.m_connections)
		{
			completerList.push_back(c);
		}

		for (const auto& l : sd.m_loopbacks)
		{
			completerList.push_back(l);
		}

		for (const auto& t : sd.m_tags)
		{
			completerList.push_back(t);
		}
	}

	// --
	//
	completerModel->setStringList(completerList);

	return;
}

void SimLogicModulePage::updateModuleStates(Sim::ControlStatus state)
{
	auto it = std::find_if(std::begin(state.m_lmDeviceModes),
						   std::end(state.m_lmDeviceModes),
						   [this](const Sim::ControlStatus::LmMode& p)
						   {
								return p.lmEquipmentId == this->equipmentId();
						   });

	if (it == std::end(state.m_lmDeviceModes))
	{
		return;
	}

	const Sim::ControlStatus::LmMode& lmState = *it;

	QColor color{Qt::black};

	QString text;
	text.reserve(64);

	if (state.m_state == Sim::SimControlState::Pause)
	{
		text = QObject::tr("Pause - ");
	}

	if (state.m_state != Sim::SimControlState::Stop)
	{
		switch (lmState.deviceMode)
		{
		case Sim::DeviceMode::Off:
			text += QStringLiteral("Off");
			break;
		case Sim::DeviceMode::Start:
			text += QStringLiteral("Start");
			break;
		case Sim::DeviceMode::Fault:
			text += QStringLiteral("Fault");
			color = qRgb(0xD0, 0x00, 0x00);
			break;
		case Sim::DeviceMode::Operate:
			text += QStringLiteral("Operate");
			break;
		default:
			Q_ASSERT(false);
			text += QStringLiteral("Unknown");
			color = qRgb(0xD0, 0x00, 0x00);
		}
	}

	bool disabled = false;

	if (auto lm = logicModule();
		lm != nullptr)
	{
		disabled = lm->isPowerOff();
	}

	if (m_disableButton->isChecked() != disabled)
	{
		// Update push button state
		//
		m_disableButton->setChecked(disabled);
	}

	// State
	//
	QString stateText;
	if (m_simulator->isLoaded() == true)
	{
		if (m_simulator->isStopped() == true)
		{
			stateText = tr("State: Stopped");
		}
		else
		{
			stateText = text;
		}
	}

	m_stateLabel->setText(stateText);

	return;
}

QString SimLogicModulePage::equipmentId() const
{
	return m_lmEquipmentId;
}

std::shared_ptr<Sim::LogicModule> SimLogicModulePage::logicModule()
{
	return m_simulator->logicModule(m_lmEquipmentId);
}

std::shared_ptr<Sim::LogicModule> SimLogicModulePage::logicModule() const
{
	return m_simulator->logicModule(m_lmEquipmentId);
}
