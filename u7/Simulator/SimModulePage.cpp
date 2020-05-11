#include "SimModulePage.h"


SimModulePage::SimModulePage(SimIdeSimulator* simulator,
							   QString equipmentId,
							   QWidget* parent)
	: SimBasePage(simulator, parent),
	m_lmEquipmentId(equipmentId)
{
	assert(m_simulator);
	assert(m_lmEquipmentId.isEmpty() == false);

	// --
	//
	m_subsystemIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_equipmentIdLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);
	m_channelLabel->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard);

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

	m_splitter->setChildrenCollapsible(false);

	// Left side
	//
	{
		QWidget* leftWidget = new QWidget(m_splitter);

		QGridLayout* layout = new QGridLayout;
		leftWidget->setLayout(layout);

		layout->addWidget(m_subsystemIdLabel, 0, 0, 1, 3);
		layout->addWidget(m_equipmentIdLabel, 1, 0, 1, 3);
		layout->addWidget(m_channelLabel, 2, 0, 1, 3);

		// Add spacer
		//
		QSpacerItem* spacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
		layout->addItem(spacer, 3, 0, 1, 3);

		// Buttons  Signals, Memory, Code
		//
		layout->addWidget(m_signalsButton, 4, 0, 1, 1);
		layout->addWidget(m_memoryButton, 4, 1, 1, 1);
		layout->addWidget(m_codeButton, 4, 2, 1, 1);

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
	connect(m_simulator, &Sim::Simulator::projectUpdated, this, &SimModulePage::projectUpdated);

	connect(m_codeButton, &QPushButton::clicked, this, &SimModulePage::codeButtonClicked);

	connect(m_schemaFilterEdit, &QLineEdit::textChanged, this, &SimModulePage::schemaFilterChanged);

	connect(m_schemasList, &QTreeWidget::customContextMenuRequested, this, &SimModulePage::schemaContextMenuRequested);
	connect(m_schemasList, &QTreeWidget::itemDoubleClicked, this, &SimModulePage::schemaItemDoubleClicked);

	// --
	//
	QSettings s;

	if (QByteArray ba = s.value("Simulator/SimModulePage/m_schemasList").toByteArray();
		ba.isEmpty() == false)
	{
		m_schemasList->header()->restoreState(ba);
	}

	if (QByteArray ba = s.value("Simulator/SimModulePage/m_splitter").toByteArray();
		ba.isEmpty() == false)
	{
		m_splitter->restoreState(ba);
	}

	return;
}

SimModulePage::~SimModulePage()
{
	QSettings s;

	s.setValue("Simulator/SimModulePage/m_schemasList", m_schemasList->header()->saveState());
	s.setValue("Simulator/SimModulePage/m_splitter", m_splitter->saveState());

	return;
}

void SimModulePage::updateLogicModuleInfoInfo()
{
	auto lm = logicModule();
	if (lm == nullptr)
	{
		return;
	}

	const Hardware::LogicModuleInfo& lmInfo = lm->logicModuleInfo();

	m_subsystemIdLabel->setText(QString("Subsystem: %1").arg("TODO"));
	m_equipmentIdLabel->setText(QString("EquipmnetID: %1").arg(lmInfo.equipmentId));
	m_channelLabel->setText(QString("LmNumber: %1, Channel: %2")
								.arg(lmInfo.lmNumber)
								.arg(QChar('A' + static_cast<char>(lmInfo.channel))));

	// Fill schema list
	//
	fillSchemaList();

	return;
}

void SimModulePage::fillSchemaList()
{
	Q_ASSERT(m_schemasList);
	m_schemasList->clear();

	// Get all schemas fro LM
	//
	std::vector<VFrame30::SchemaDetails> schemas = m_simulator->schemasForLm(equipmnetId());

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

void SimModulePage::projectUpdated()
{
	updateLogicModuleInfoInfo();
	return;
}

void SimModulePage::codeButtonClicked()
{
	emit openCodePageRequest(equipmnetId());
	return;
}

void SimModulePage::schemaFilterChanged()
{
	fillSchemaList();
}

void SimModulePage::schemaContextMenuRequested(const QPoint& pos)
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

void SimModulePage::schemaItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	if (item == nullptr)
	{
		return;
	}

	openSelectedSchema();
	return;
}

void SimModulePage::openSelectedSchema()
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

	emit openSchemaRequest(schemaId);

	return;
}

QString SimModulePage::equipmnetId() const
{
	return m_lmEquipmentId;
}

std::shared_ptr<Sim::LogicModule> SimModulePage::logicModule()
{
	return m_simulator->logicModule(m_lmEquipmentId);
}

std::shared_ptr<Sim::LogicModule> SimModulePage::logicModule() const
{
	return m_simulator->logicModule(m_lmEquipmentId);
}
