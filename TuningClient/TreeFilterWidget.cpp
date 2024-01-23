#include "TreeFilterWidget.h"
#include "Settings.h"
#include "TuningSourcesHelper.h"

TreeFilterWidget::TreeFilterWidget(TuningConfigController& configController,
								   TuningClientFilterStorage& tuningFilterStorage,
								   ClientLib::TuningUserManager& userManager,
								   ClientLib::TuningConnection& tuningConnection,
								   QWidget* parent):
	QWidget(parent),
	m_configController(configController),
	m_tuningFilterStorage(tuningFilterStorage),
	m_userManager(userManager),
	m_tuningConnection(tuningConnection)
{
	createFilterTree();
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

void TreeFilterWidget::fillFiltersTree(std::shared_ptr<TuningFilter> rootFilter)
{
	// Remember previously chosen filter
	//
	std::shared_ptr<TuningFilter> selectedFilter;
	{
		QList <QTreeWidgetItem*> selectedItems = m_filterTree->selectedItems();
		if (selectedItems.size() == 1)
		{
			if (selectedItems[0] != nullptr)
			{
				selectedFilter = selectedItems[0]->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
			}
		}
	}

	m_filterTree->clear();

	// Fill the filter tree
	//
	if (rootFilter == nullptr)
	{
		assert(rootFilter);
		return;
	}

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

	QStringList l;
	l << tr(rootFilter->caption().toUtf8());

	QTreeWidgetItem* rootItem = new QTreeWidgetItem(l);
	rootItem->setData(0, Qt::UserRole, QVariant::fromValue(rootFilter));

	addChildTreeObjects(rootFilter, rootItem, mask);

	if (rootItem->childCount() == 0)
	{
		delete rootItem;
		return;
	}

	// Fill filters control
	//

	m_filterTree->addTopLevelItem(rootItem);

	// Restore selection

	if (selectedFilter == nullptr)
	{
		rootItem->setSelected(true);
	}
	else
	{
		// Find a pointer to previously selected tree filter (remember we are working with shared_ptrs)

		selectedFilter = rootFilter->findFilterById(selectedFilter->ID());

		if (selectedFilter == nullptr)
		{
			// No such filter - select root

			rootItem->setSelected(true);
		}
		else
		{
			// Find a tree item for restored selected filter and select it

			QTreeWidgetItem* treeFilterWidgetItem = findFilterWidget(selectedFilter->ID(), rootItem);

			if (treeFilterWidgetItem == nullptr)
			{
				// No such filter - select root

				rootItem->setSelected(true);
			}
			else
			{
				treeFilterWidgetItem->setSelected(true);

				// Expand all parents

				QTreeWidgetItem* parent = treeFilterWidgetItem->parent();
				while (parent != nullptr && parent != rootItem)
				{
					parent->setExpanded(true);
					parent = parent->parent();
				}
			}
		}
	}

	// Expand root item

	rootItem->setExpanded(true);

	// Expand "Equipment" item

	for (int i = 0; i < rootItem->childCount(); i++)
	{
		QTreeWidgetItem* rootChildItem = rootItem->child(i);

		std::shared_ptr<TuningFilter> filter = rootChildItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return;
		}

		if (filter->isEmpty() == true && filter->isSourceEquipment())
		{
			rootChildItem->setExpanded(true);
			break;
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
	updateTreeItemStatus();
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
		int childCount = m_tuningFilterStorage.root()->childFiltersCount();
		for (int i = 0; i < childCount; i++)
		{
			auto child = m_tuningFilterStorage.root()->childFilter(i);
			if (child->isCounter() && child->counterType() == TuningFilter::CounterType::FilterTree)
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

void TreeFilterWidget::addChildTreeObjects(const std::shared_ptr<TuningFilter> filter, QTreeWidgetItem* parent, const QString& mask)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (parent == nullptr)
	{
		assert(parent);
		return;
	}

	QStringList specialCaptions;
	specialCaptions.push_back("All Signals");
	specialCaptions.push_back("Equipment");
	specialCaptions.push_back("Schemas");

	QStringList specialCaptionsTranslated;
	specialCaptionsTranslated.push_back(tr("All Signals"));
	specialCaptionsTranslated.push_back(tr("Equipment"));
	specialCaptionsTranslated.push_back(tr("Schemas"));

	for (int i = 0; i < filter->childFiltersCount(); i++)
	{
		std::shared_ptr<TuningFilter> f = filter->childFilter(i);
		if (f == nullptr)
		{
			assert(f);
			continue;
		}

		if (f->isTree() == false)
		{
			continue;
		}

		QString caption = f->caption();
		if (specialCaptions.contains(caption) == true)
		{
			caption = tr(caption.toUtf8());
		}

		if (mask.isEmpty() == false)
		{
			// Check if filter has child filters EXCEPT counters
			//
			bool hasChildFilters = false;

			int childFiltersCount = f->childFiltersCount();
			for (int j = 0; j < childFiltersCount; j++)
			{
				TuningFilter* const cf = f->childFilter(j).get();
				if (cf->isCounter() == false)
				{
					hasChildFilters = true;
					break;
				}
			}

			if (hasChildFilters == false &&
				caption.contains(mask, Qt::CaseInsensitive) == false)
			{
				continue;
			}
		}


		//if (f->isSourceSchema() == true || f->isSourceEquipment() == true)
		//{
		//caption += QString(" [+%1 DEBUG counters]").arg(f->childFiltersCount());
		//}

		static QString equipmentString = tr("Equipment");
		static QString schemasString = tr("Schemas");
		Q_UNUSED(equipmentString);
		Q_UNUSED(schemasString);

		QStringList l;
		l << tr(caption.toUtf8().data());	// Try to translate filter name!

		QTreeWidgetItem* item = new QTreeWidgetItem(l);
		item->setData(0, Qt::UserRole, QVariant::fromValue(f));

		parent->addChild(item);

		addChildTreeObjects(f, item, mask);
	}
}

void TreeFilterWidget::updateTreeItemStatus(QTreeWidgetItem* treeItem)
{
	if (m_filterTree == nullptr || m_filterTree->isVisible() == false)
	{
		return;
	}

	if (treeItem == nullptr)
	{
		if (m_filterTree->topLevelItemCount() == 0)
		{
			return;
		}

		treeItem = m_filterTree->topLevelItem(0);
	}

	std::shared_ptr<TuningFilter> filter = treeItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (filter->isEmpty() == false)
	{
		updateTreeItemCounters(treeItem, filter.get());

		// Counters column

		TuningCounters counters = filter->counters();

		// Status column

		if (filter->isSourceEquipment() == true)
		{
			updateTuningSourceTreeItem(treeItem, filter.get());
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

void TreeFilterWidget::updateTuningSourceTreeItem(QTreeWidgetItem* treeItem, TuningFilter* filter)
{
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	std::vector<Hash> equipmentHashes = filter->equipmentHashes();

	if (equipmentHashes.size() != 1)
	{
		Q_ASSERT(filter);
		return;
	}

	Hash hash = equipmentHashes[0];

	assert(m_columnStatusIndex != -1);

	int errorCounter = filter->counters().errorCounter;

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
						if (state.hasunappliedparams() == true)
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
				if (state.hasunappliedparams() == true) hasUnappliedParamsCount++;

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

void TreeFilterWidget::updateTreeItemCounters(QTreeWidgetItem* treeItem, TuningFilter* filter)
{
	if (filter == nullptr)
	{
		Q_ASSERT(filter);
		return;
	}

	int childCount = filter->childFiltersCount();

	int counterIndex = 0;

	for (int i = 0; i < childCount; i++)
	{
		TuningFilter* childFilter = filter->childFilter(i).get();
		if (childFilter == nullptr)
		{
			Q_ASSERT(childFilter);
			return;
		}

		if (childFilter->isCounter() == false || childFilter->counterType() != TuningFilter::CounterType::FilterTree)
		{
			continue;
		}

		// Set column text and color

		if (counterIndex >= static_cast<int>(m_columnDiscreteCountIndexes.size()))
		{
			Q_ASSERT(false);
			return;
		}

		int columnIndex = m_columnDiscreteCountIndexes[counterIndex];

		TuningCounters tc = childFilter->counters();

		QColor backColor = tc.discreteCounter == 0 ? Qt::white : childFilter->backAlertedColor();
		QColor textColor = tc.discreteCounter == 0 ? Qt::black : childFilter->textAlertedColor();

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

QTreeWidgetItem* TreeFilterWidget::findFilterWidget(const QString& id, QTreeWidgetItem* treeItem)
{
	for (int i = 0; i < treeItem->childCount(); i++)
	{
		QTreeWidgetItem* childItem = treeItem->child(i);
		if (childItem == nullptr)
		{
			assert(childItem);
			return nullptr;
		}

		std::shared_ptr<TuningFilter> filter = childItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
		if (filter == nullptr)
		{
			assert(filter);
			return nullptr;
		}

		if (filter->ID() == id)
		{
			return childItem;
		}

		// Recursive search

		QTreeWidgetItem* result = findFilterWidget(id, childItem);

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

	std::shared_ptr<TuningFilter> selectedFilter = selected->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	emit treeFilterSelectionChanged(selectedFilter);
}

void TreeFilterWidget::slot_treeContextMenuRequested(const QPoint& pos)
{
	QTreeWidgetItem* item = m_filterTree->itemAt(pos);
	if (item == nullptr)
	{
		return;
	}

	std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (filter->isEmpty() == true)
	{
		return;
	}

	if (filter->isSourceEquipment() == false)
	{
		return;
	}

	Hash sourceHash = ::calcHash(filter->caption());

	int sourceStatesCount = m_tuningConnection.tuningSourceStatesCount(sourceHash);
	int activeStatesCount = m_tuningConnection.activatedTuningSourceStatesCount(sourceHash);
	bool activateEnabled = activeStatesCount < sourceStatesCount;
	bool deactivateEnabled = activeStatesCount != 0 && activeStatesCount == sourceStatesCount;

	QMenu menu(this);

	// EnableControl

	QAction* actionEnable = new QAction(tr("Activate Control"), &menu);

	auto fEnableControl = [this, filter]() -> void
	{
		activateControl(filter->caption(), true);
	};
	actionEnable->setEnabled(activateEnabled);
	connect(actionEnable, &QAction::triggered, this, fEnableControl);

	menu.addAction(actionEnable);

	// Disable Control

	QAction* actionDisable = new QAction(tr("Deactivate Control"), &menu);

	auto fDisableControl = [this, filter]() -> void
	{
		activateControl(filter->caption(), false);
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

void TreeFilterWidget::slot_treeItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if (item == nullptr)
	{
		return;
	}

	std::shared_ptr<TuningFilter> filter = item->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (filter == nullptr)
	{
		assert(filter);
		return;
	}

	if (filter->isEmpty() == true)
	{
		return;
	}

	if (filter->isSourceEquipment() == false)
	{
		return;
	}

	Hash sourceHash = ::calcHash(filter->caption());

	int sourceStatesCount = m_tuningConnection.tuningSourceStatesCount(sourceHash);
	int activeStatesCount = m_tuningConnection.activatedTuningSourceStatesCount(sourceHash);
	bool activateEnabled = activeStatesCount < sourceStatesCount;
	bool deactivateEnabled = activeStatesCount != 0 && activeStatesCount == sourceStatesCount;

	if (activateEnabled == deactivateEnabled)
	{
		return;
	}

	activateControl(filter->caption(), activateEnabled == true);
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

	std::shared_ptr<TuningFilter> rootFilter = rootItem->data(0, Qt::UserRole).value<std::shared_ptr<TuningFilter>>();
	if (rootFilter == nullptr)
	{
		assert(rootFilter);
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

	fillFiltersTree(rootFilter);
}
