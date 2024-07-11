#include "TreeFilterWidget.h"
#include "Settings.h"
#include "TuningSourcesHelper.h"
#include <ClientLib/TuningConnection.h>
#include <ClientLib/TuningUserManager.h>
#include <TuningLib/TuningUiItem.h>
#include <AppSignalLists/SignalList.h>
#include "TuningCounters.h"

TreeFilterWidget::TreeFilterWidget(TuningConfigController& configController,
								   TuningLib::TuningUiStorage& tuningUi,
								   AppSignalLists::AppSignalListSet& appSignalLists,
								   ClientLib::TuningUserManager& userManager,
								   ClientLib::TuningConnection& tuningConnection,
								   TuningCountersManager& tuningCounters,
								   QWidget* parent) :
	QWidget(parent),
	m_configController(configController),
	m_tuningUi(tuningUi),
	m_appSignalLists(appSignalLists),
	m_userManager(userManager),
	m_tuningConnection(tuningConnection),
	m_tuningCounters(tuningCounters)
{
	createFilterTree();
	fillFiltersTree();

}

TreeFilterWidget::~TreeFilterWidget()
{
	if (isVisible() == true)
	{
		QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

		if (m_columnNameIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnNameIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnIndex", width);

		}
		if (m_columnAccessIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnAccessIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnsAccess", width);
		}

		for (int i = 0; i < static_cast<int>(m_columnDiscreteCountIndexes.size()); i++)
		{
			int width = m_filterTree->columnWidth(m_columnDiscreteCountIndexes[i]);
			settings.setValue(tr("TuningWorkspace/FilterTreeColumnCounter%1").arg(i), width);
		}

		if (m_columnSorIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnSorIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnSor", width);
		}


		if (m_columnStatusIndex != -1)
		{
			int width = m_filterTree->columnWidth(m_columnStatusIndex);
			settings.setValue("TuningWorkspace/FilterTreeColumnStatus", width);
		}

		// Save masks
		//
		if (m_treeMaskCombo != nullptr)
		{
			TuningClientAppSettings::instance().user().m_tuningWorkspaceMasks.clear();
			for (int i = 0; i < m_treeMaskCombo->count(); i++)
			{
				TuningClientAppSettings::instance().user().m_tuningWorkspaceMasks.push_back(m_treeMaskCombo->itemText(i));
			}
		}
	}
}

void TreeFilterWidget::fillFiltersTree()
{
	// Remember previously chosen filter
	//
	QUuid selectedFilterUuid;
	
	{
		QList <QTreeWidgetItem*> selectedItems = m_filterTree->selectedItems();
		if (selectedItems.size() == 1)
		{
			if (selectedItems[0] != nullptr)
			{
				selectedFilterUuid = selectedItems[0]->data(0, Qt::UserRole).toUuid();
			}
		}
	}

	m_filterTree->clear();

	QString mask;
	if (m_treeMaskCombo != nullptr)
	{
		mask = m_treeMaskCombo->currentText();
		if (mask.isEmpty() == false)
		{
			if (m_treeMaskCombo->findText(mask) == -1)
			{
				m_treeMaskCombo->addItem(mask);
			}
			while (m_treeMaskCombo->count() > 10)
			{
				m_treeMaskCombo->removeItem(0);
			}
			m_treeMaskCombo->setCurrentText(mask);
		}
	}

	QTreeWidgetItem* rootItem = new QTreeWidgetItem({tr("All Signals")});
	m_filterTree->addTopLevelItem(rootItem);

	// Schemas
	//
	QTreeWidgetItem* schemasItem = new QTreeWidgetItem({tr("Schemas")});
	addTreeObjects(schemasItem, mask, {AppSignalLists::AppSignalList::tagSchema}, {});
	if (schemasItem->childCount() == 0)
	{
		delete schemasItem;
	}
	else
	{
		rootItem->addChild(schemasItem);
		schemasItem->setExpanded(true);
	}

	// Equipment
	//
	QTreeWidgetItem* equipmentItem = new QTreeWidgetItem({tr("Equipment")});
	addTreeObjects(equipmentItem, mask, {AppSignalLists::AppSignalList::tagEquipment}, {});
	if (equipmentItem->childCount() == 0)
	{
		delete equipmentItem;
	}
	else
	{
		rootItem->addChild(equipmentItem);
		equipmentItem->setExpanded(true);
	}

	// Auto-created
	//
	QTreeWidgetItem* autoItem = new QTreeWidgetItem({tr("Auto-created filters")});
	addTreeObjects(autoItem, mask, {AppSignalLists::AppSignalList::tagTcAuto}, {});
	if (autoItem->childCount() == 0)
	{
		delete autoItem;
	}
	else
	{
		rootItem->addChild(autoItem);
		autoItem->setExpanded(true);
	}

	// Project lists
	//
	QTreeWidgetItem* globalItem = new QTreeWidgetItem({tr("Project Lists")});
	addTreeObjects(globalItem,
				   mask,
				   {AppSignalLists::AppSignalList::tagIde},
				   {AppSignalLists::AppSignalList::tagSchema,
					AppSignalLists::AppSignalList::tagEquipment,
					AppSignalLists::AppSignalList::tagTcAuto,
					AppSignalLists::AppSignalList::tagUi});
	rootItem->addChild(globalItem);
	globalItem->setExpanded(true);

	// Local lists
	//
	QTreeWidgetItem* localItem = new QTreeWidgetItem({tr("Local Lists")});
	addTreeObjects(localItem,
				   mask,
				   {},
				   {AppSignalLists::AppSignalList::tagSchema,
					AppSignalLists::AppSignalList::tagEquipment,
					AppSignalLists::AppSignalList::tagTcAuto,
					AppSignalLists::AppSignalList::tagUi,
					AppSignalLists::AppSignalList::tagIde});
	rootItem->addChild(localItem);
	localItem->setExpanded(true);

	rootItem->setExpanded(true);

	// Restore selection
	//
	if (selectedFilterUuid.isNull() == false)
	{
		// Find a tree item for restored selected filter and select it
		for (int i = 0; i < m_filterTree->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* treeFilterWidgetItem = findFilterWidget(selectedFilterUuid, m_filterTree->topLevelItem(i));
			if (treeFilterWidgetItem != nullptr)
			{
				treeFilterWidgetItem->setSelected(true);

				// Expand all parents

				QTreeWidgetItem* parent = treeFilterWidgetItem->parent();
				while (parent != nullptr)
				{
					parent->setExpanded(true);
					parent = parent->parent();
				}
			}
		}
	}

	m_filterTree->sortItems(0, Qt::AscendingOrder);

	if (mask.isEmpty() == false)
	{
		m_filterTree->expandAll();
	}
}

void TreeFilterWidget::updateFiltersTree()
{
	if (m_filterTree == nullptr || m_filterTree->isVisible() == false)
	{
		return;
	}

	for (int i = 0; i < m_filterTree->topLevelItemCount(); i++)
	{
		updateTreeItemStatus(m_filterTree->topLevelItem(i));
	}
}

bool TreeFilterWidget::isEmpty() const
{
	return m_filterTree->topLevelItemCount() == 0;
}

QTreeWidget* TreeFilterWidget::treeWidget()
{
	return m_filterTree;
}

void TreeFilterWidget::createFilterTree()
{
	if (m_filterTree != nullptr)
	{
		Q_ASSERT(m_filterTree == nullptr);
		return;
	}

	m_filterTree = new QTreeWidget();
	m_filterTree->setSortingEnabled(true);
	m_filterTree->setObjectName("FilterTreeWidget");

	m_filterTree->viewport()->installEventFilter(this);
	m_filterTree->installEventFilter(this);

	m_filterTree->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(m_filterTree, &QTreeWidget::itemSelectionChanged, this, &TreeFilterWidget::slot_treeSelectionChanged);
	connect(m_filterTree, &QWidget::customContextMenuRequested, this, &TreeFilterWidget::slot_treeContextMenuRequested);
	connect(m_filterTree, &QTreeWidget::itemDoubleClicked, this, &TreeFilterWidget::slot_treeItemDoubleClicked);

	int columnIndex = m_columnNameIndex;

	QStringList headerLabels;

	headerLabels << tr("Caption");
	columnIndex++;

	// Access (?)

	if (m_configController.configuration().lmStatusFlagMode() == TuningClientSettings::LmStatusFlagMode::AccessKey)
	{
		headerLabels << tr("Access");
		m_columnAccessIndex = columnIndex;
		columnIndex++;
	}

	// SOR (?)

	if (m_configController.configuration().lmStatusFlagMode() == TuningClientSettings::LmStatusFlagMode::SOR)
	{
		headerLabels << tr("SOR");
		m_columnSorIndex = columnIndex;
		columnIndex++;
	}

	// Counters ()

	QStringList counterColumnsNames;

	{
		for (int i = 0; i < m_tuningUi.root()->childCount(); i++)
		{
			auto child = m_tuningUi.root()->child(i);
			if (child->isCounter() && child->counterType() == TuningLib::TuningUiItem::CounterType::FilterTree)
			{
				counterColumnsNames.push_back(child->caption());
			}
		}
	}

	for (int i = 0; i < counterColumnsNames.size(); i++)
	{
		headerLabels << counterColumnsNames.at(i);
		m_columnDiscreteCountIndexes.push_back(columnIndex++);
	}

	// Status

	headerLabels << tr("Status");
	m_columnStatusIndex = columnIndex;
	columnIndex++;

	//

	headerLabels << tr("");

	m_filterTree->setColumnCount(static_cast<int>(headerLabels.size()));
	m_filterTree->setHeaderLabels(headerLabels);

	// Set column width

	const int columnMaxWidth = 500;

	QSettings settings(QSettings::UserScope, qApp->organizationName(), qApp->applicationName());

	if (m_columnNameIndex != -1)
	{
		const int defaultWidth = 200;

		int width = settings.value("TuningWorkspace/FilterTreeColumnIndex", defaultWidth).toInt();
		if (width < defaultWidth || width > columnMaxWidth)
		{
			width = defaultWidth;
		}

		m_filterTree->setColumnWidth(m_columnNameIndex, width);
	}

	if (m_columnAccessIndex != -1)
	{
		const int defaultWidth = 50;

		int width = settings.value("TuningWorkspace/FilterTreeColumnsAccess", defaultWidth).toInt();
		if (width < defaultWidth || width > columnMaxWidth)
		{
			width = defaultWidth;
		}

		m_filterTree->setColumnWidth(m_columnAccessIndex, width);

	}

	if (m_columnSorIndex != -1)
	{
		const int defaultWidth = 80;

		int width = settings.value("TuningWorkspace/FilterTreeColumnSor", defaultWidth).toInt();
		if (width < defaultWidth || width > columnMaxWidth)
		{
			width = defaultWidth;
		}

		m_filterTree->setColumnWidth(m_columnSorIndex, width);
	}

	for (int i = 0; i < counterColumnsNames.size(); i++)
	{
		const int defaultWidth = 40;

		int width = settings.value(QString("TuningWorkspace/FilterTreeColumnCounter%1").arg(i), defaultWidth).toInt();
		if (width < defaultWidth || width > columnMaxWidth)
		{
			width = defaultWidth;
		}

		m_filterTree->setColumnWidth(m_columnDiscreteCountIndexes[i], width);
	}


	if (m_columnStatusIndex != -1)
	{
		const int defaultWidth = 80;

		int width = settings.value("TuningWorkspace/FilterTreeColumnStatus", defaultWidth).toInt();
		if (width < defaultWidth || width > columnMaxWidth)
		{
			width = defaultWidth;
		}

		m_filterTree->setColumnWidth(m_columnStatusIndex, width);
	}

	//

	m_treeMaskCombo = new QComboBox();
	m_treeMaskCombo->setEditable(true);
	m_treeMaskCombo->setInsertPolicy(QComboBox::NoInsert);

	// Load masks
	//
	m_treeMaskCombo->addItems(TuningClientAppSettings::instance().user().m_tuningWorkspaceMasks);
	m_treeMaskCombo->setEditText(QString());

	QLineEdit* filterLineEdit = m_treeMaskCombo->lineEdit();
	if (filterLineEdit == nullptr)
	{
		Q_ASSERT(filterLineEdit);
	}
	else
	{
		connect(filterLineEdit, &QLineEdit::returnPressed, this, &TreeFilterWidget::slot_maskApply);
	}

	// Mask Apply

	m_treeMaskApply = new QPushButton(tr("Filter"));
	connect(m_treeMaskApply, &QPushButton::clicked, this, &TreeFilterWidget::slot_maskApply);

	QHBoxLayout* searchLayout = new QHBoxLayout();
	searchLayout->addWidget(m_treeMaskCombo, 3);
	searchLayout->addWidget(m_treeMaskApply, 1);

	QVBoxLayout* treeLayout = new QVBoxLayout(this);

	treeLayout->addWidget(m_filterTree);
	treeLayout->addLayout(searchLayout);
}

void TreeFilterWidget::addTreeObjects(QTreeWidgetItem* parentItem, const QString& mask, const QStringList& includeSystemTags, const QStringList& excludeSystemTags)
{
	/*QStringList specialCaptions;
	specialCaptions.push_back("All Signals");
	specialCaptions.push_back("Equipment");
	specialCaptions.push_back("Schemas");

	QStringList specialCaptionsTranslated;
	specialCaptionsTranslated.push_back(tr("All Signals"));
	specialCaptionsTranslated.push_back(tr("Equipment"));
	specialCaptionsTranslated.push_back(tr("Schemas"));*/

	for (int i = 0; i < m_appSignalLists.count(); i++)
	{
		const AppSignalLists::AppSignalList* list = m_appSignalLists.get(i).get();
		if (list == nullptr)
		{
			assert(list);
			continue;
		}

		bool includeOk = true;

		if (includeSystemTags.isEmpty() == false)
		{
			includeOk = false;
			for (const QString& includeTag : includeSystemTags)
			{
				if (list->systemTagsList().contains(includeTag) == true)
				{
					includeOk = true;
					break;
				}
			}
		}

		bool excludeOk = true;

		for (const QString& excludeTag: excludeSystemTags)
		{
			if (list->systemTagsList().contains(excludeTag) == true)
			{
				excludeOk = false;
				break;
			}
		}

		if (includeOk == false || excludeOk == false) 
		{
			continue;
		}

		if (mask.isEmpty() == false)
		{
			if  (list->caption().contains(mask, Qt::CaseInsensitive) == false)
			{
				continue;
			}
		}

		QTreeWidgetItem* item = new QTreeWidgetItem({QString("%1").arg(list->caption())});
		item->setData(0, Qt::UserRole, list->uuid());

		if (parentItem != nullptr)
		{
			parentItem->addChild(item);
		}
		else 
		{
			m_filterTree->addTopLevelItem(item);
		}
	}
}

void TreeFilterWidget::updateTreeItemStatus(QTreeWidgetItem* treeItem)
{
	if (treeItem == nullptr)
	{
		Q_ASSERT(treeItem);
		return;
	}

	QUuid listUuid = treeItem->data(0, Qt::UserRole).toUuid();
	
	if (listUuid.isNull() == false)
	{
		AppSignalLists::AppSignalList* list = m_appSignalLists.get(listUuid).get();
		if (list == nullptr)
		{
			assert(list);
			return;
		}

		// Counters columns

		updateTreeItemCounters(treeItem, list);

		// Status column

		TuningCounters counters = m_tuningCounters.counters(list->id());

		if (list->systemTagsList().contains(AppSignalLists::AppSignalList::tagEquipment))
		{
			updateTuningSourceTreeItem(treeItem, list);
		}
		else
		{
			assert(m_columnStatusIndex != -1);

			QColor backColor;
			QColor textColor;
			QString text;

			if (counters.errorCounter == 0)
			{
				backColor = Qt::white;
				textColor = Qt::black;
			}
			else
			{
				text = QString("E: %1").arg(counters.errorCounter);
				backColor = redColor;
				textColor = Qt::white;
			}

			if (treeItem->text(m_columnStatusIndex) != text)
			{
				treeItem->setText(m_columnStatusIndex, text);
			}

			if (treeItem->background(m_columnStatusIndex) != backColor)
			{
				treeItem->setBackground(m_columnStatusIndex, backColor);
			}

			if (treeItem->foreground(m_columnStatusIndex) != textColor)
			{
				treeItem->setForeground(m_columnStatusIndex, textColor);
			}
		}

		// SOR Column

		if (m_columnSorIndex != -1 && m_configController.configuration().lmStatusFlagMode() == TuningClientSettings::LmStatusFlagMode::SOR)
		{
			QColor backColor;
			QColor textColor;
			QString text;

			if (counters.sorActive == false)
			{
				// Inactive
				backColor = Qt::white;
				textColor = Qt::black;
			}
			else
			{
				if (counters.sorValid == false)
				{
					text = "?";
					backColor = redColor;
					textColor = Qt::white;
				}
				else
				{
					if (counters.sorCounter == 0)
					{
						// Sor NO
						backColor = Qt::white;
						textColor = Qt::black;
					}
					else
					{
						if (counters.sorCounter == 1)
						{
							text = QString("SOR");
						}
						else
						{
							text = QString("SOR [%1]").arg(counters.sorCounter);
						}
						backColor = redColor;
						textColor = Qt::white;
					}
				}
			}

			if (treeItem->text(m_columnSorIndex) != text)
			{
				treeItem->setText(m_columnSorIndex, text);
			}

			if (treeItem->background(m_columnSorIndex) != backColor)
			{
				treeItem->setBackground(m_columnSorIndex, backColor);
			}

			if (treeItem->foreground(m_columnSorIndex) != textColor)
			{
				treeItem->setForeground(m_columnSorIndex, textColor);
			}
		}
	}

	int count = treeItem->childCount();
	for (int i = 0; i < count; i++)
	{
		updateTreeItemStatus(treeItem->child(i));
	}
}

void TreeFilterWidget::updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, const AppSignalLists::AppSignalList* list)
{
	if (list == nullptr)
	{
		assert(list);
		return;
	}

	Hash hash = ::calcHash(list->id()); // Assume that list id is source EquipmentId

	assert(m_columnStatusIndex != -1);

	TuningCounters counters = m_tuningCounters.counters(list->id());

	int errorCounter = counters.errorCounter;

	QStringList statusStrings;

	int validCount = 0;
	int controlIsEnabledCount = 0;
	int isReplyCount = 0;
	int hasUnappliedParamsCount = 0;

	QStringList replyCounts;

	int statesCount = 0;
	int accessCount = 0;

	std::vector<ClientLib::TuningSource> sourceInfo = m_tuningConnection.tuningSourceInfo(hash);

	if (sourceInfo.empty() == true)
	{
		statesCount++;
		statusStrings.push_back(tr("Unknown"));
	}

	for (const ClientLib::TuningSource& ts : sourceInfo)
	{
		QString sourceStatus;

		if (ts.valid() == false)
		{
			statesCount++;
			sourceStatus = tr("Non-Valid");

			if (statusStrings.empty() == true || statusStrings.last() != sourceStatus)
			{
				statusStrings.push_back(sourceStatus);
			}
		}
		else
		{
			for (int c = 0; c < ts.statesCount(); c++)
			{
				statesCount++;

				const ::Network::TuningSourceState& state = ts.state(c);

				if (state.controlisactive() == false)
				{
					sourceStatus = tr("Inactive");

					if (statusStrings.empty() == true || statusStrings.last() != sourceStatus)
					{
						statusStrings.push_back(sourceStatus);
					}
				}
				else
				{
					if (state.isreply() == false)
					{
						sourceStatus = tr("No Reply");

						if (statusStrings.empty() == true || statusStrings.last() != sourceStatus)
						{
							statusStrings.push_back(sourceStatus);
						}
					}
					else
					{
						if (m_configController.configuration().clientSettings.applyMode == TuningClientSettings::ApplyMode::Manual &&
							state.hasunappliedparams() == true)
						{
							statusStrings.push_back(tr("Unapplied [%1]").arg(state.replycount()));
						}
						else
						{
							statusStrings.push_back(tr("Active [%1]").arg(state.replycount()));
						}

						replyCounts.push_back(tr("%1").arg(static_cast<int>(state.replycount())));
					}
				}

				// Increment counters
				//
				if (ts.valid() == true) validCount++;
				if (state.controlisactive() == true) controlIsEnabledCount++;
				if (state.isreply() == true) isReplyCount++;
				
				if (m_configController.configuration().clientSettings.applyMode == TuningClientSettings::ApplyMode::Manual &&
					state.hasunappliedparams() == true)
					hasUnappliedParamsCount++;

				if (m_configController.configuration().lmStatusFlagMode() == TuningClientSettings::LmStatusFlagMode::AccessKey &&
					ts.valid() == true &&
					state.controlisactive() == true &&
					state.isreply() == true)
				{
					if (state.writingdisabled() == false) accessCount++;
				}
			}
		}
	}	// Loop through states

	QString statusText = statusStrings.join(" / ");

	if (statusText.isEmpty() == true)
	{
		statusText = tr("Unknown");
	}

	if (statesCount > 0 && validCount == statesCount)
	{
		if (hasUnappliedParamsCount > 0)
		{
			// All are unappplied
			//
			statusText = tr("Unapplied [%1]").arg(replyCounts.join(" / "));
		}
		else
		{
			if (isReplyCount == statesCount)
			{
				// All are active
				//
				statusText = tr("Active [%1]").arg(replyCounts.join(" / "));
			}
		}
	}

	if (errorCounter > 0)
	{
		statusText += tr(", E: %1").arg(errorCounter);
	}

	// Access column
	//
	if (m_columnAccessIndex != -1)
	{
		QColor accessBackColor = Qt::white;
		QColor accessTextColor = Qt::black;

		if (accessCount > 0)
		{
			accessBackColor = QColor(0, 128, 0);
			accessTextColor = Qt::white;
		}

		QString accessText;

		if (accessCount == 0)
		{
			accessText = tr("No");
		}
		else
		{
			accessText = accessCount == statesCount ? tr("Yes") : tr("Yes (%1/%2)").arg(statesCount - accessCount).arg(statesCount);
		}

		if (treeItem->text(m_columnAccessIndex) != accessText)
		{
			treeItem->setText(m_columnAccessIndex, accessText);
		}

		if (treeItem->background(m_columnAccessIndex) != accessBackColor)
		{
			treeItem->setBackground(m_columnAccessIndex, accessBackColor);
		}

		if (treeItem->foreground(m_columnAccessIndex) != accessTextColor)
		{
			treeItem->setForeground(m_columnAccessIndex, accessTextColor);
		}

	}

	// Status column text and color
	//
	if (treeItem->text(m_columnStatusIndex) != statusText)
	{
		treeItem->setText(m_columnStatusIndex, statusText);
	}

	QColor stateBackColor = Qt::white;
	QColor stateTextColor = Qt::black;

	if (validCount == 0)
	{
		// All are non-valid
		//
		stateBackColor = redColor;
		stateTextColor = Qt::white;
	}
	else
	{
		if (controlIsEnabledCount == 0)
		{
			// Control is not enabled for all
			//
			stateBackColor = Qt::gray;
			stateTextColor = Qt::white;
		}
		else
		{
			if (isReplyCount == 0 || errorCounter > 0)
			{
				// All are No Reply or some errors present
				//
				stateBackColor = redColor;
				stateTextColor = Qt::white;
			}
			else
			{
				if ((validCount < statesCount) || (isReplyCount < statesCount) || (controlIsEnabledCount < statesCount))
				{
					// Some are non-valid no-reply or control is not enabled
					//
					stateBackColor = QColor(0xF87217);
					stateTextColor = Qt::white;
				}
				else
				{
					// Unapplied params present
					//
					if (hasUnappliedParamsCount > 0)
					{
						stateBackColor = Qt::yellow;
						stateTextColor = Qt::black;
					}
				}
			}
		}
	}


	if (treeItem->background(m_columnStatusIndex) != stateBackColor)
	{
		treeItem->setBackground(m_columnStatusIndex, stateBackColor);
	}

	if (treeItem->foreground(m_columnStatusIndex) != stateTextColor)
	{
		treeItem->setForeground(m_columnStatusIndex, stateTextColor);
	}
}

void TreeFilterWidget::updateTreeItemCounters(QTreeWidgetItem* treeItem, const AppSignalLists::AppSignalList* list)
{
	if (list == nullptr)
	{
		Q_ASSERT(list);
		return;
	}

	int counterIndex = 0;

	for (int i = 0; i < m_tuningUi.root()->childCount(); i++)
	{
		const TuningLib::TuningUiItem* counterItem = m_tuningUi.root()->child(i).get();
		if (counterItem == nullptr) 
		{
			Q_ASSERT(false);
			return;
		}

		if (counterItem->isCounter() == false || counterItem->counterType() != TuningLib::TuningUiItem::CounterType::FilterTree)
		{
			continue;
		}

		AppSignalLists::AppSignalList* counterList = m_appSignalLists.get(counterItem->filters()).get();
		if (counterList == nullptr)
		{
			Q_ASSERT(counterList);
			return;
		}

		// Set column text and color

		if (counterIndex >= static_cast<int>(m_columnDiscreteCountIndexes.size()))
		{
			//Q_ASSERT(false);	// Possibly, workspace has not been recreated yet, just skip
			continue;
		}

		int columnIndex = m_columnDiscreteCountIndexes[counterIndex];

		// Counters for columns are calculated by following algorithm:
		// We take counter Ui item and get its filtres (for example, two "BLOCKS_ANALOG;BLOCKS_DISCRETE" filters).
		// Then we take a filter for the tree item (e. g. for schema or user list) and add it's Id to the request.
		// We get "BLOCKS_ANALOG;BLOCKS_DISCRETE;USER_LIST_000" counters request.
		// We filter signals using all these filters and get counters.
		QStringList columnFilterIds = counterItem->filtersList();
		columnFilterIds.push_back(counterList->id());

		TuningCounters tc = m_tuningCounters.counters(columnFilterIds.join(';'));

		QColor backColor = tc.discreteCounter == 0 ? Qt::white : counterItem->backAlertedColor();
		QColor textColor = tc.discreteCounter == 0 ? Qt::black : counterItem->textAlertedColor();

		//QString text = QString("%1 %2").arg(childFilter->caption()) .arg(tc.discreteCounter);
		QString text = QString("%1").arg(tc.discreteCounter);

		if (treeItem->text(columnIndex) != text)
		{
			treeItem->setText(columnIndex, text);
		}

		if (treeItem->background(columnIndex) != backColor)
		{
			treeItem->setBackground(columnIndex, backColor);
		}

		if (treeItem->foreground(columnIndex) != textColor)
		{
			treeItem->setForeground(columnIndex, textColor);
		}

		//

		counterIndex++;
	}
}

void TreeFilterWidget::activateControl(const QString& equipmentId, bool enable)
{
	if (m_userManager.login(this) == false)
	{
		return;
	}

	ClientLib::TuningSourcesHelper::activateTuningSource(m_tuningConnection, equipmentId, enable, this);
}

QTreeWidgetItem* TreeFilterWidget::findFilterWidget(const QUuid& uuid, QTreeWidgetItem* treeItem)
{
	for (int i = 0; i < treeItem->childCount(); i++)
	{
		QTreeWidgetItem* childItem = treeItem->child(i);
		if (childItem == nullptr)
		{
			assert(childItem);
			return nullptr;
		}

		if (uuid == childItem->data(0, Qt::UserRole).toUuid())
		{
			return childItem;
		}

		// Recursive search

		QTreeWidgetItem* result = findFilterWidget(uuid, childItem);

		if (result != nullptr)
		{
			return result;
		}
	}

	return nullptr;
}

void TreeFilterWidget::slot_treeSelectionChanged()
{
	QList <QTreeWidgetItem*> selectedItems = m_filterTree->selectedItems();
	if (selectedItems.size() != 1)
	{
		return;
	}

	QTreeWidgetItem* selected = selectedItems[0];
	if (selected == nullptr)
	{
		return;
	}

	emit treeFilterSelectionChanged(selected->data(0, Qt::UserRole).toUuid());
}

void TreeFilterWidget::slot_treeContextMenuRequested(const QPoint& pos)
{
	QTreeWidgetItem* item = m_filterTree->itemAt(pos);
	if (item == nullptr)
	{
		return;
	}

	QUuid uuid = item->data(0, Qt::UserRole).toUuid();
	if (uuid.isNull()) 
	{
		return;
	}

	AppSignalLists::AppSignalList* list = m_appSignalLists.get(uuid).get();
	if (list == nullptr)
	{
		assert(list);
		return;
	}

	if (list->systemTagsList().contains(AppSignalLists::AppSignalList::tagEquipment) == false)
	{
		return;
	}

	Hash sourceHash = ::calcHash(list->caption());

	int sourceStatesCount = m_tuningConnection.tuningSourceStatesCount(sourceHash);
	int activeStatesCount = m_tuningConnection.activatedTuningSourceStatesCount(sourceHash);
	bool activateEnabled = activeStatesCount < sourceStatesCount;
	bool deactivateEnabled = activeStatesCount != 0 && activeStatesCount == sourceStatesCount;

	QMenu menu(this);

	// EnableControl

	QAction* actionEnable = new QAction(tr("Activate Control"), &menu);

	auto fEnableControl = [this, list]() -> void
	{
		activateControl(list->caption(), true);
	};
	actionEnable->setEnabled(activateEnabled);
	connect(actionEnable, &QAction::triggered, this, fEnableControl);

	menu.addAction(actionEnable);

	// Disable Control

	QAction* actionDisable = new QAction(tr("Deactivate Control"), &menu);

	auto fDisableControl = [this, list]() -> void
	{
		activateControl(list->caption(), false);
	};
	actionDisable->setEnabled(deactivateEnabled);
	connect(actionDisable, &QAction::triggered, this, fDisableControl);

	menu.addAction(actionDisable);

	// Run the menu

	if (actionEnable->isEnabled() == true || actionDisable->isEnabled() == true)
	{
		menu.exec(QCursor::pos());
	}
}

void TreeFilterWidget::slot_treeItemDoubleClicked(QTreeWidgetItem* item, int /*column*/)
{
	if (item == nullptr)
	{
		return;
	}

	QUuid uuid = item->data(0, Qt::UserRole).toUuid();
	if (uuid.isNull()) 
	{
		return;
	}

	AppSignalLists::AppSignalList* list = m_appSignalLists.get(uuid).get();
	if (list == nullptr)
	{
		assert(list);
		return;
	}

	if (list->systemTagsList().contains(AppSignalLists::AppSignalList::tagEquipment) == false)
	{
		return;
	}

	Hash sourceHash = ::calcHash(list->caption());

	int sourceStatesCount = m_tuningConnection.tuningSourceStatesCount(sourceHash);
	int activeStatesCount = m_tuningConnection.activatedTuningSourceStatesCount(sourceHash);
	bool activateEnabled = activeStatesCount < sourceStatesCount;
	bool deactivateEnabled = activeStatesCount != 0 && activeStatesCount == sourceStatesCount;

	if (activateEnabled == deactivateEnabled)
	{
		return;
	}

	activateControl(list->caption(), activateEnabled == true);
}

void TreeFilterWidget::slot_maskReturnPressed()
{
	slot_maskApply();
}

void TreeFilterWidget::slot_maskApply()
{
	if (m_filterTree->topLevelItemCount() != 1)
	{
		return;
	}

	QTreeWidgetItem* rootItem = m_filterTree->topLevelItem(0);
	if (rootItem == nullptr)
	{
		assert(false);
		return;
	}

	if (m_treeMaskCombo->currentText().isEmpty() == false)
	{
		m_treeMaskCombo->setStyleSheet("QComboBox { color: red }");
		m_treeMaskApply->setStyleSheet("QPushButton { color: red }");
	}
	else
	{
		m_treeMaskCombo->setStyleSheet(QString());
		m_treeMaskApply->setStyleSheet(QString());
	}

	fillFiltersTree();
}
