#include <QSettings>
#include <QResizeEvent>
#include <QMessageBox>
#include <QStringListModel>
#include <QClipboard>
#include <QMenu>

#include <AppSignalLists/SignalList.h>
#include <TrendView/DialogChooseTrendSignals.h>

#include <CommonLib/PropertyObject.h>
#include "ui_DialogChooseTrendSignals.h"

namespace TrendLibInternal
{
	//
	//		FilteredTrendSignalsModel
	//
	FilteredTrendSignalsModel::FilteredTrendSignalsModel(const TrendLib::ISignalHasTag* signalHasTag,
														 const std::vector<TrendLib::TrendSignalParam>& signalss,
														 QObject* parent) :
		QAbstractTableModel(parent),
		m_signalHasTag(signalHasTag),
		m_signals(signalss)
	{
		Q_ASSERT(m_signalHasTag);

		// Sort signal by signalId
		//
		std::sort(m_signals.begin(), m_signals.end(),
				  [](const auto& s1, const auto& s2) -> bool
		{
			return s1.signalId() < s2.signalId();
		});

		m_signalIndexes.reserve(m_signals.size());

		// Init m_startWithArrays.
		// m_startWithArrays keeps vector of indexes for "StartWith"
		//
		size_t siganlCount = m_signals.size();

		for (size_t index = 0; index < siganlCount; index ++)
		{
			const auto& trendSignal = m_signals[index];
			QString customSignalId = trendSignal.signalId().toLower();

			if (customSignalId.isEmpty() == true)
			{
				Q_ASSERT(customSignalId.isEmpty() == false);
				continue;
			}

			QString firstLetter = customSignalId.at(0);

			auto foundStartWithIt = m_startWithArrays.find(firstLetter);
			if (foundStartWithIt == m_startWithArrays.end())
			{
				std::vector<size_t> signalIndexes;
				signalIndexes.reserve(8192);

				signalIndexes.push_back(static_cast<int>(index));

				m_startWithArrays[firstLetter] = signalIndexes;
			}
			else
			{
				std::vector<size_t>& signalIndexes = foundStartWithIt->second;
				signalIndexes.push_back(static_cast<int>(index));
			}
		}

		return;
	}

	int FilteredTrendSignalsModel::rowCount(const QModelIndex& /*parent*/) const
	{
		return static_cast<int>(m_signalIndexes.size());
	}

	int FilteredTrendSignalsModel::columnCount(const QModelIndex& /*parent*/) const
	{
		return 4;	// Columns: SignalID, Type, Caption, Server
	}

	QVariant FilteredTrendSignalsModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
	{
		if (role == Qt::DisplayRole)
		{
			if (orientation == Qt::Horizontal)
			{
				switch (section)
				{
				case 0:
					return tr("SignalID");
				case 1:
					return tr("Type");
				case 2:
					return tr("Caption");
				case 3:
					return tr("Server");
				}
			}
		}

		return {};
	}

	QVariant FilteredTrendSignalsModel::data(const QModelIndex& index, int role) const
	{
		int row = index.row();
		int col = index.column();

		switch (role)
		{
		case Qt::DisplayRole:
			{
				if (row < 0 || row >= static_cast<int>(m_signalIndexes.size()))
				{
					Q_ASSERT(row >= 0 && row < static_cast<int>(m_signalIndexes.size()));
					return {};
				}

				size_t signalIndex = m_signalIndexes[row];

				if (signalIndex >= m_signals.size())
				{
					Q_ASSERT(signalIndex < m_signals.size());
					return {};
				}

				const auto& trendSignal = m_signals[signalIndex];

				switch (col)
				{
				case 0:
					return trendSignal.signalId();
				case 1:
					switch (trendSignal.type())
					{
					case E::SignalType::Analog:		return QString("A");
					case E::SignalType::Discrete:	return QString("D");
					case E::SignalType::Bus:		return QString("B");
					default:
						Q_ASSERT(false);
						return {};
					}
				case 2:
					return trendSignal.caption();
				case 3:
					return trendSignal.archiveServerShortId();
				default:
					Q_ASSERT(false);
					return {};
				}
			}
			break;
		case Qt::ToolTipRole:
			{
				if (row < 0 || row >= std::ssize(m_signalIndexes))
				{
					Q_ASSERT(row >= 0 && row < std::ssize(m_signalIndexes));
					return {};
				}

				size_t signalIndex = m_signalIndexes[row];

				if (signalIndex >= m_signals.size())
				{
					Q_ASSERT(signalIndex < m_signals.size());
					return {};
				}

				const auto& trendSignal = m_signals[signalIndex];

				QString toolTip = QString("%1\n%2\n%3\n%4")
								  .arg(trendSignal.signalId())
								  .arg(trendSignal.appSignalId())
								  .arg(trendSignal.caption())
								  .arg(trendSignal.archiveServerShortId());

				return toolTip;

			}
		default:
			return {};
		}
	}

	void FilteredTrendSignalsModel::filterSignals(QString server, std::optional<AppSignalLists::AppSignalList*> appSignalList, QString filter, QStringList tagList)
	{
		// Get hashes list filtered by signal list
		//
		std::set<Hash> appSignalListHashes;
		if (appSignalList.has_value() == true)
		{
			Q_ASSERT(appSignalList.value());
			appSignalListHashes = appSignalList.value()->appListHashesCache();
		}

		beginResetModel();

		server = server.trimmed();
		QString filterText = filter.trimmed().toLower();

		if (filterText.isEmpty() == true)
		{
			// No filter, add all signals
			//
			m_signalIndexes.clear();

			for (size_t i = 0, signalCount = m_signals.size(); i < signalCount; i++)
			{
				const auto& s = m_signals[i];

				if (appSignalList.has_value() == true)
				{
					if (appSignalListHashes.contains(s.appSignalHash()) == false)
					{
						continue;
					}
				}

				if (server.isEmpty() == false &&
					s.archiveServerId() != server)
				{
					continue;
				}

				if (tagList.isEmpty() == true)
				{
					m_signalIndexes.push_back(i);
				}
				else
				{
					for (const QString& t : tagList)
					{
						if (m_signalHasTag->signalHasTag(s.appSignalId(), t) == true)
						{
							m_signalIndexes.push_back(i);
							break;	// Breaks tags loop
						}
					}
				}
			}

			endResetModel();
			return;
		}

		// filterText is not emty
		//
		m_signalIndexes.clear();

		auto foundStartWithIt = m_startWithArrays.find(filterText.left(1));		// m_startWithArrays keeps only firts letter
		if (foundStartWithIt == m_startWithArrays.end())
		{
		}
		else
		{
			const std::vector<size_t>& signalIndexes = foundStartWithIt->second;

			for (size_t index : signalIndexes)
			{
				if (index >= m_signals.size())
				{
					Q_ASSERT(index < m_signals.size());
					continue;
				}

				const auto& signal = m_signals[index];

				if (appSignalList.has_value() == true)
				{
					if (appSignalListHashes.contains(signal.appSignalHash()) == false)
					{
						continue;
					}
				}

				// if filterText.size() == 1 then we already filrtered it by getting data from m_startWithArrays
				//
				if (filterText.size() == 1 ||
					signal.signalId().startsWith(filterText, Qt::CaseInsensitive) == true)
				{
					// Filter server
					//
					if (server.isEmpty() == false &&
						signal.archiveServerId() != server)
					{
						continue;
					}

					// Filter tags
					//
					if (tagList.isEmpty() == true)
					{
						m_signalIndexes.push_back(index);
					}
					else
					{
						for (const QString& t : tagList)
						{
							if (m_signalHasTag->signalHasTag(signal.appSignalId(), t) == true)
							{
								m_signalIndexes.push_back(index);
								break;
							}
						}
					}
				}
			}
		}

		endResetModel();

		return;
	}

	const TrendLib::TrendSignalParam& FilteredTrendSignalsModel::signalByRow(int row) const
	{
		size_t indexInIndex = static_cast<size_t>(row);
		Q_ASSERT(indexInIndex < m_signalIndexes.size());

		size_t signalIndex = m_signalIndexes[indexInIndex];
		Q_ASSERT(signalIndex < m_signals.size());

		return m_signals[signalIndex];
	}
}

namespace TrendLib
{
	using namespace TrendLibInternal;

	DialogChooseTrendSignals::DialogChooseTrendSignals(const TrendLib::ISignalHasTag* signalHasTag,
													   std::vector<TrendLib::TrendSignalParam> trendSignals,
													   const std::vector<TrendLib::TrendSignalParam>& acceptedSignals,
													   const std::vector<TrendLib::ArchiveServer>& archiveServers,
													   const AppSignalLists::AppSignalListSet& appSignalLists,
													   QWidget* parent) :
		QDialog(parent),
		s_allServers(tr("All Servers")),
		m_appSignalListSet(appSignalLists)
	{
		init(signalHasTag, std::move(trendSignals), acceptedSignals, archiveServers);

		QObject::connect(ui->trendSignals, &QTreeWidget::itemSelectionChanged, [this]()
			{
				QList<QTreeWidgetItem*> selectedItems = ui->trendSignals->selectedItems();
				if (selectedItems.isEmpty() == false)
				{
					QTreeWidgetItem* selectedItem = selectedItems.first();
					ui->trendSignals->setCurrentItem(selectedItem);
				}
			});

		return;
	}


	DialogChooseTrendSignals::~DialogChooseTrendSignals()
	{
		delete ui;
	}

	void DialogChooseTrendSignals::init(const TrendLib::ISignalHasTag* signalHasTag,
										std::vector<TrendLib::TrendSignalParam> signalss,
										const std::vector<TrendLib::TrendSignalParam>& acceptedSignals,
										const std::vector<TrendLib::ArchiveServer>& archiveServers)
	{
		m_signalHasTag = signalHasTag;
		m_archiveServers = archiveServers;

		Q_ASSERT(m_signalHasTag);

		ui = new Ui::DialogChooseTrendSignals;
		ui->setupUi(this);

		setWindowFlag(Qt::WindowContextHelpButtonHint, false);
		setWindowFlag(Qt::WindowMaximizeButtonHint, true);

		// Set filter completer
		//
		QSettings s{};
		QStringList trendSignalsDialogFilterCompleter = s.value(m_filterCompleterSettingsName).toStringList();
		QStringList trendSignalsDialogTagsCompleter = s.value(m_tagsCompleterSettingsName).toStringList();
		QSize widgetSize = s.value(m_sizeSettingsName).toSize();

		m_filterCompleter = new QCompleter(trendSignalsDialogFilterCompleter, this);
		m_filterCompleter->setCaseSensitivity(Qt::CaseInsensitive);
		m_filterCompleter->setFilterMode(Qt::MatchContains);
		m_filterCompleter->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
		ui->filterEdit->setCompleter(m_filterCompleter);

		m_tagsCompleter = new QCompleter(trendSignalsDialogTagsCompleter, this);
		m_tagsCompleter->setCaseSensitivity(Qt::CaseInsensitive);
		ui->tagsEdit->setCompleter(m_tagsCompleter);

		// Archive servers combo
		//
		std::ranges::sort(m_archiveServers, [](const auto& a, const auto& b) {return a.shortEquipmentId < b.shortEquipmentId;});

		fillServerCombo();

		connect(ui->serverCombo, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &DialogChooseTrendSignals::serverCurrentIndexChanged);

		// AppSignalLists
		//
		connect(&m_appSignalListSet,
				&AppSignalLists::AppSignalListSet::updatePerformed,
				this,
				&DialogChooseTrendSignals::fillAppSignalLists);
		fillAppSignalLists();

		// --
		//
		QStringList headerLabels;
		headerLabels << tr("SignalID");
		headerLabels << tr("Type");
		headerLabels << tr("Caption");
		headerLabels << tr("Server");

		ui->trendSignals->setHeaderLabels(headerLabels);

		FilteredTrendSignalsModel* model = new FilteredTrendSignalsModel(m_signalHasTag, std::move(signalss), ui->filteredSignals);
		ui->filteredSignals->setModel(model);

		// --
		//
		fillSignalList();

		// --
		//
		connect(ui->filteredSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseTrendSignals::slot_filteredSignalsSelectionChanged);
		connect(ui->trendSignals->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DialogChooseTrendSignals::slot_trendSignalsSelectionChanged);
		connect(ui->listCombo,
				static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
				this,
				&DialogChooseTrendSignals::listComboIndexChanged);

		// --
		// --
		disableControls();

		ui->filteredSignals->header()->resizeSection(1, ui->filteredSignals->header()->sectionSizeHint(1));		// 1 is TypeColumn (A/D)
		ui->trendSignals->header()->resizeSection(1 , ui->trendSignals->header()->sectionSizeHint(1));			// 1 is TypeColumn (A/D)

		// Fill added signals
		//
		for (const TrendLib::TrendSignalParam& trendSignal : acceptedSignals)
		{
			addSignal(trendSignal);
		}

		if (widgetSize.isNull() == false)
		{
			resize(widgetSize);
		}

		return;
	}

	std::vector<TrendLib::TrendSignalParam> DialogChooseTrendSignals::acceptedSignals() const
	{
		return m_acceptedSignals;
	}

	void DialogChooseTrendSignals::resizeEvent(QResizeEvent* event)
	{
		QSettings{}.setValue(m_sizeSettingsName, event->size());
	}

	void DialogChooseTrendSignals::fillServerCombo()
	{
		Q_ASSERT(ui->serverCombo);

		ui->serverCombo->clear();

		// Fill combo box
		//
		ui->serverCombo->addItem(s_allServers, {});

		for (const auto& server : m_archiveServers)
		{
			ui->serverCombo->addItem(server.shortEquipmentId, QVariant{server.equipmentId});
		}

		// Set current item in combo box
		//
		if (int serverIndex = ui->serverCombo->findData(QVariant{s_lastServer});
			serverIndex != -1)
		{
			ui->serverCombo->setCurrentIndex(serverIndex);
		}
	}

	void DialogChooseTrendSignals::fillAppSignalLists()
	{
		// Refresh AppSignalLists combo
		//
		QString selectedList = ui->listCombo->currentData().toString();

		ui->listCombo->blockSignals(true);

		ui->listCombo->clear();
		ui->listCombo->addItem(tr("Not selected"), QString());

		// Remove previously set filter
		//
		if (selectedList.isEmpty() == false)
		{
			fillSignalList();
		}

		// Fill lists combo
		//
		const auto lists = m_appSignalListSet.lists();

		for (const auto& list : lists)
		{
			ui->listCombo->addItem(tr("[%1] %2").arg(list->id()).arg(list->caption()), list->id());
		}
		if (lists.empty() == true)
		{
			ui->listCombo->setEnabled(false);
		}

		ui->listCombo->blockSignals(false);
	}

	void DialogChooseTrendSignals::fillSignalList()
	{
		// Get ArchiveServiceId
		//
		QString server = ui->serverCombo->currentData().toString();

		// appSignalList
		//
		std::optional<AppSignalLists::AppSignalList*> appSignalList;

		QString selectedList = ui->listCombo->currentData().toString();
		if (selectedList.isEmpty() == false)
		{
			std::shared_ptr<AppSignalLists::AppSignalList> list = m_appSignalListSet.get(selectedList);
			if (list != nullptr)
			{
				appSignalList = list.get();
			}
		}

		// Filter text
		//
		QString filterText = ui->filterEdit->text().trimmed();

		// Tags
		//
		QString tagText = ui->tagsEdit->text().trimmed().toLower();
		QStringList tagList = tagText.split(' ', Qt::SkipEmptyParts);

		// --
		//
		FilteredTrendSignalsModel* model = dynamic_cast<FilteredTrendSignalsModel*>(ui->filteredSignals->model());
		Q_ASSERT(model);

		model->filterSignals(server, appSignalList, filterText, tagList);

		return;
	}

	void DialogChooseTrendSignals::addSignal(const TrendSignalParam& signal)
	{
		if (trendSignalsHasSignalId(signal.signalId(), signal.archiveServerShortId()) == true)
		{
			// SignaID already presnt in TrenSignals
			//
			return;
		}

		if (ui->trendSignals->topLevelItemCount() >= 16)
		{
			QMessageBox::critical(this, qAppName(), tr("The maximum number of signals reached."));
			return;
		}

		QString signalType;
		switch (signal.type())
		{
		case E::SignalType::Analog:		signalType = "A";	break;
		case E::SignalType::Discrete:	signalType = "D";	break;
		case E::SignalType::Bus:		signalType = "B";	break;
		default:
			Q_ASSERT(false);
		}

		QStringList itemData;
		itemData << signal.signalId();
		itemData << signalType;
		itemData << signal.caption();
		itemData << signal.archiveServerShortId();

		QString toolTip = QString("%1\n%2\n%3\n%4")
						  .arg(signal.signalId())
						  .arg(signal.appSignalId())
						  .arg(signal.caption())
						  .arg(signal.archiveServerShortId());

		QTreeWidgetItem* item = new QTreeWidgetItem(ui->trendSignals, itemData);
		item->setData(0, Qt::UserRole, QVariant::fromValue(signal));

		item->setToolTip(0, toolTip);
		item->setToolTip(1, toolTip);
		item->setToolTip(2, toolTip);
		item->setToolTip(3, toolTip);

		ui->trendSignals->addTopLevelItem(item);
		ui->trendSignals->setCurrentItem(item, QItemSelectionModel::SelectCurrent);

		disableControls();

		return;
	}

	void DialogChooseTrendSignals::removeSelectedSignal()
	{
		Q_ASSERT(ui->trendSignals);

		QModelIndex currentIndex = ui->trendSignals->currentIndex();

		if (currentIndex.isValid() == true)
		{
			QTreeWidgetItem* takenItem = ui->trendSignals->takeTopLevelItem(currentIndex.row());
			delete takenItem;
		}

		disableControls();
		return;
	}

	bool DialogChooseTrendSignals::trendSignalsHasSignalId(QString signalId, QString archiveServerShortId)
	{
		int itemCount = ui->trendSignals->topLevelItemCount();

		for (int i = 0; i < itemCount; i++)
		{
			QTreeWidgetItem* item = ui->trendSignals->topLevelItem(i);
			Q_ASSERT(item);

			// 0 is signalid, 3 is a archive service short id
			//
			if (item->text(0) == signalId && item->text(3) == archiveServerShortId)
			{
				return true;
			}
		}

		return false;
	}

	void DialogChooseTrendSignals::disableControls()
	{
		Q_ASSERT(ui->filteredSignals);
		Q_ASSERT(ui->trendSignals);

		const FilteredTrendSignalsModel* fileterModel = dynamic_cast<const FilteredTrendSignalsModel*>(ui->filteredSignals->model());
		if (fileterModel == nullptr)
		{
			Q_ASSERT(fileterModel != nullptr);
			return;
		}

		// --
		//
		bool enableAddButton = true;
		bool enableRemoveButton = true;
		bool enableRemoveAll = true;

		bool enableMoveUp = false;
		bool enableMoveDown = false;

		// Add Signal Button
		//
		{
			QModelIndex index = ui->filteredSignals->currentIndex();

			if (index.isValid() == true)
			{
				const TrendLib::TrendSignalParam& signal = fileterModel->signalByRow(index.row());
				enableAddButton = !trendSignalsHasSignalId(signal.signalId(), signal.archiveServerShortId());
			}
			else
			{
				enableAddButton = false;
			}
		}

		// Remove Signal Button
		//
		{
			QModelIndex index = ui->trendSignals->currentIndex();

			if (index.isValid() == false ||
				index.row() < 0)
			{
				enableRemoveButton = false;
			}
			else
			{
				enableRemoveButton = true;
			}
		}

		// Remove All Signals Button
		//
		{
			enableRemoveAll = ui->trendSignals->topLevelItemCount() > 0;
		}

		// Move up/down.
		//
		{
			QModelIndex index = ui->trendSignals->currentIndex();

			if (index.isValid() == true)
			{
				enableMoveUp = index.row() > 0;
				enableMoveDown = (index.row() + 1) < ui->trendSignals->topLevelItemCount();
			}
		}

		// --
		//
		ui->addSignalButton->setEnabled(enableAddButton);
		ui->removeSignalButton->setEnabled(enableRemoveButton);
		ui->removeAllSignalsButton->setEnabled(enableRemoveAll);

		ui->upSignalButton->setEnabled(enableMoveUp);
		ui->downSignalButton->setEnabled(enableMoveDown);

		return;
	}

	void DialogChooseTrendSignals::serverCurrentIndexChanged(int /*index*/)
	{
		fillSignalList();
	}

	void DialogChooseTrendSignals::on_addSignalButton_clicked()
	{
		QModelIndex index = ui->filteredSignals->currentIndex();
		if (index.isValid() == false)
		{
			return;
		}

		const FilteredTrendSignalsModel* model = dynamic_cast<const FilteredTrendSignalsModel*>(index.model());

		if (model == nullptr)
		{
			Q_ASSERT(dynamic_cast<const FilteredTrendSignalsModel*>(index.model()) != nullptr);
			return;
		}

		const auto& signal = model->signalByRow(index.row());
		addSignal(signal);

		return;
	}

	void DialogChooseTrendSignals::on_removeSignalButton_clicked()
	{
		removeSelectedSignal();
	}

	void DialogChooseTrendSignals::on_removeAllSignalsButton_clicked()
	{
		ui->trendSignals->clear();

		disableControls();

		return;
	}

	void DialogChooseTrendSignals::on_filterEdit_textChanged(const QString& /*arg*/)
	{
		fillSignalList();
	}


	void DialogChooseTrendSignals::on_filterEdit_editingFinished()
	{
		QString arg = ui->filterEdit->text();

		QSettings s;
		QStringList trendSignalsDialogFilterCompleter = s.value(m_filterCompleterSettingsName).toStringList();

		if (trendSignalsDialogFilterCompleter.contains(arg) == false)
		{
			trendSignalsDialogFilterCompleter << arg;

			while (trendSignalsDialogFilterCompleter.size() > 1000)
			{
				trendSignalsDialogFilterCompleter.pop_front();
			}

			QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_filterCompleter->model());
			Q_ASSERT(completerModel);

			if (completerModel != nullptr)
			{
				completerModel->setStringList(trendSignalsDialogFilterCompleter);
			}

			s.setValue(m_filterCompleterSettingsName, trendSignalsDialogFilterCompleter);
		}

		return;
	}

	void DialogChooseTrendSignals::on_tagsEdit_textChanged(const QString& /*arg*/)
	{
		fillSignalList();
	}

	void DialogChooseTrendSignals::on_tagsEdit_editingFinished()
	{
		QString arg = ui->tagsEdit->text();

		QSettings s;
		QStringList trendSignalsDialogTagsCompleter = s.value(m_tagsCompleterSettingsName).toStringList();

		if (trendSignalsDialogTagsCompleter.contains(arg) == false)
		{
			trendSignalsDialogTagsCompleter << arg;

			while (trendSignalsDialogTagsCompleter.size() > 1000)
			{
				trendSignalsDialogTagsCompleter.pop_front();
			}

			QStringListModel* completerModel = dynamic_cast<QStringListModel*>(m_tagsCompleter->model());
			Q_ASSERT(completerModel);

			if (completerModel != nullptr)
			{
				completerModel->setStringList(trendSignalsDialogTagsCompleter);
			}

			s.setValue(m_tagsCompleterSettingsName, trendSignalsDialogTagsCompleter);
		}

		return;
	}

	void DialogChooseTrendSignals::on_filteredSignals_doubleClicked(const QModelIndex& index)
	{
		if (index.isValid() == false)
		{
			return;
		}

		const FilteredTrendSignalsModel* model = dynamic_cast<const FilteredTrendSignalsModel*>(index.model());

		if (model == nullptr)
		{
			Q_ASSERT(dynamic_cast<const FilteredTrendSignalsModel*>(index.model()) != nullptr);
			return;
		}

		const auto signal = model->signalByRow(index.row());
		addSignal(signal);

		return;
	}

	void DialogChooseTrendSignals::slot_filteredSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
	{
		disableControls();
	}

	void DialogChooseTrendSignals::on_trendSignals_doubleClicked(const QModelIndex& /*index*/)
	{
		removeSelectedSignal();
	}

	void DialogChooseTrendSignals::slot_trendSignalsSelectionChanged(const QItemSelection& /*selected*/, const QItemSelection& /*deselected*/)
	{
		disableControls();
	}

	void DialogChooseTrendSignals::listComboIndexChanged(int /*index*/)
	{
		fillSignalList();
	}

	void DialogChooseTrendSignals::on_buttonBox_accepted()
	{
		m_acceptedSignals.reserve(ui->trendSignals->topLevelItemCount());

		for (int i = 0; i < ui->trendSignals->topLevelItemCount(); i++)
		{
			QTreeWidgetItem* treeItem = ui->trendSignals->topLevelItem(i);
			Q_ASSERT(treeItem);

			QVariant signalVariant = treeItem->data(0, Qt::UserRole);

			Q_ASSERT(signalVariant.isNull() == false);
			Q_ASSERT(signalVariant.isValid() == true);

			auto signalParam = signalVariant.value<TrendLib::TrendSignalParam>();
			Q_ASSERT(signalParam.signalId() == treeItem->text(0));

			m_acceptedSignals.push_back(signalParam);
		}

		return;
	}


	void DialogChooseTrendSignals::on_trendSignals_customContextMenuRequested(const QPoint& pos)
	{
		Q_ASSERT(ui->trendSignals);

		QModelIndex index = ui->trendSignals->indexAt(pos);
		if (index.isValid() == false)
		{
			return;
		}

		QModelIndex signalIdIndex = index.siblingAtColumn(0);
		if (signalIdIndex.isValid() == false)
		{
			return;
		}

		QString appSignalId = ui->trendSignals->model()->data(signalIdIndex).toString();
		if (appSignalId.isEmpty() == true)
		{
			return;
		}

		QAction action{tr("Copy SignalID")};
		QObject::connect(&action, &QAction::triggered,
			[appSignalId]()
			{
				QClipboard* clipboard = QGuiApplication::clipboard();
				clipboard->setText(appSignalId);
			});

		QMenu menu;
		menu.insertAction(nullptr, &action);
		menu.exec(ui->trendSignals->mapToGlobal(pos));

		return;
	}


	void DialogChooseTrendSignals::on_upSignalButton_clicked()
	{
		Q_ASSERT(ui->trendSignals);

		QModelIndex currentIndex = ui->trendSignals->currentIndex();

		if (currentIndex.isValid() == true && currentIndex.row() > 0)
		{
			// Reorder item.
			//
			QTreeWidgetItem* itemToMove = ui->trendSignals->takeTopLevelItem(currentIndex.row());
			ui->trendSignals->insertTopLevelItem(currentIndex.row() - 1, itemToMove);

			// Select moved item.
			//
			ui->trendSignals->setCurrentItem(itemToMove);
		}

		disableControls();
		return;
	}


	void DialogChooseTrendSignals::on_downSignalButton_clicked()
	{
		Q_ASSERT(ui->trendSignals);

		QModelIndex currentIndex = ui->trendSignals->currentIndex();

		if (currentIndex.isValid() == true && (currentIndex.row() + 1) < ui->trendSignals->topLevelItemCount())
		{
			// Reorder item.
			//
			QTreeWidgetItem* itemToMove = ui->trendSignals->takeTopLevelItem(currentIndex.row());
			ui->trendSignals->insertTopLevelItem(currentIndex.row() + 1, itemToMove);

			// Select moved item.
			//
			ui->trendSignals->setCurrentItem(itemToMove);
		}

		disableControls();
		return;
	}

}
