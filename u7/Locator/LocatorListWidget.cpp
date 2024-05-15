#include "LocatorListWidget.h"
#include "SignalPropertiesDialog.h"
#include "../GlobalMessanger.h"
#include "../DialogConnections.h"

#include <QAbstractItemModelTester>

namespace Locator
{
	QModelIndex LocatorListModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
	{
		if (hasIndex(row, column, parent) == false)
		{
			return {};
		}

		return createIndex(row, column);
	}

	QModelIndex LocatorListModel::parent(const QModelIndex& /*index*/) const
	{
		return {};
	}

	int LocatorListModel::rowCount(const QModelIndex& parentIndex /*= QModelIndex()*/) const
	{
		if (parentIndex.column() > 0)
		{
			return 0;
		}

		int rows = 0;

		if (parentIndex.isValid() == false)
		{
			for (const auto&[_, items] : m_data)
			{
				rows += static_cast<int>(items.size());
			}
		}

		return rows;
	}

	int LocatorListModel::columnCount(const QModelIndex& /*parent = QModelIndex()*/) const
	{
		return 2;
	}

	QVariant LocatorListModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
	{
		QVariant result;

		if (role == Qt::DisplayRole)
		{
			const LocatedItem* item = locatedItem(index.row());
			Q_ASSERT(item);

			if (item != nullptr)
			{
				result = index.column() == 0 ? item->what : item->caption;
			}
		}

		return result;
	}

	QVariant LocatorListModel::headerData(int /*section*/, Qt::Orientation /*orientation*/, int /*role*/) const
	{
		return {};
	}

	void LocatorListModel::addData(QString text, QString providerName, const std::vector<LocatedItem>& items)
	{
		if (m_text != text)
		{
			m_text = text;

			beginResetModel();
			m_data.clear();
			endResetModel();
		}

		if (items.empty() == true)
		{
			return;
		}

		beginResetModel();
		m_data[providerName] = items;
		endResetModel();
		return;
	}

	const LocatedItem* LocatorListModel::locatedItem(size_t index) const
	{
		const LocatedItem* result = nullptr;

		int currentDataRow = 0;
		for (const auto&[_, items] : m_data)
		{
			if (index >= currentDataRow && index < currentDataRow + items.size())
			{
				result = &(items[index - currentDataRow]);
				break;
			}

			currentDataRow += static_cast<int>(items.size());
		}

		return result;
	}

	QString LocatorListModel::providerName(size_t index) const
	{
		QString result;

		int currentDataRow = 0;
		for (const auto&[provider, items] : m_data)
		{
			if (index >= currentDataRow && index < currentDataRow + items.size())
			{
				result = provider;
				break;
			}

			currentDataRow += static_cast<int>(items.size());
		}

		return result;
	}


	LocatorListWidget::LocatorListWidget(DbController* dbc, QWidget* parentWidget) :
		QTreeView{parentWidget},
		m_dbc(dbc)
	{
		Q_ASSERT(m_dbc);

		header()->hide();
		header()->setSectionResizeMode(QHeaderView::Stretch);

		setFocusPolicy(Qt::FocusPolicy::NoFocus);
		setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
		setExpandsOnDoubleClick(false);
		setItemsExpandable(false);
		setAllColumnsShowFocus(true);
		setUniformRowHeights(true);

		QScreen* screen = QGuiApplication::screenAt(parentWidget->mapToGlobal(QPoint{0, 0}));
		resize(screen->geometry().width() / 2, screen->geometry().height() / 3);

		hide();

		// --
		//
#ifdef QT_DEBUG
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
		[[maybe_unused]] QAbstractItemModelTester* modelTester = new QAbstractItemModelTester(&m_model,
																							 QAbstractItemModelTester::FailureReportingMode::Fatal,
																							 this);
		modelTester->setUseFetchMore(false);
#endif
#endif
		setModel(&m_model);

		connect(this, &QTreeView::doubleClicked, this, &LocatorListWidget::slot_doubleClicked);

		return;
	}

	void LocatorListWidget::addData(QString text, QString providerName, const std::vector<LocatedItem>& items)
	{
		m_model.addData(text, providerName, items);
		return;
	}

	void LocatorListWidget::showList()
	{
		show();
	}

	void LocatorListWidget::hideList()
	{
		if (m_doubleCklickShowsModalDialog == false)
		{
			hide();
		}
	}

	void LocatorListWidget::slot_doubleClicked(const QModelIndex& index)
	{
		if (index.isValid() == false)
		{
			return;
		}

		const LocatedItem* item = m_model.locatedItem(index.row());
		QString providerName = m_model.providerName(index.row());

		if (item == nullptr)
		{
			Q_ASSERT(item);
			return;
		}

		if (providerName == "AppSignal" && item->what.startsWith(QChar{'#'}) == true)
		{
			// LocatedItem::what is AppSignalID
			//
			QStringList appSignalList;
			appSignalList.push_back(item->what);

			m_doubleCklickShowsModalDialog = true;
			SignalPropertiesDialog::editApplicationSignals(appSignalList, m_dbc, this);
			m_doubleCklickShowsModalDialog = false;
			return;
		}

		if (providerName == "Schema" && item->what.isEmpty() == false && item->what != "...")
		{
			DbFileInfo fileInfo = item->data.value<DbFileInfo>();

			// fileInfo can be outdated, as LocatorProviders can cache data.
			// Update fileInfo.
			//
			DbFileInfo updatedFileInfo;
			bool ok = m_dbc->getFileInfo(fileInfo.fileId(), &updatedFileInfo, nullptr);

			if (ok == true)
			{
				if (updatedFileInfo.state() == E::VcsState::CheckedOut)
				{
					GlobalMessanger::instance().fireOpenSchema(updatedFileInfo);
				}
				else
				{
					m_doubleCklickShowsModalDialog = true;		// Changeset select dialog is shown here, do not save its focus in LocatorLineEdit.
					GlobalMessanger::instance().fireViewSchema(updatedFileInfo);
					m_doubleCklickShowsModalDialog = false;
				}
			}

			emit clearFocusFromInput();
			return;
		}

		if (providerName == "Equipment" && item->what.isEmpty() == false)
		{
			// LocatedItem::what is EquipmentID
			//
			QString equpmnetId = item->what;
			GlobalMessanger::instance().fireFindDeviceObject(equpmnetId);

			emit clearFocusFromInput();
			return;
		}

		if (providerName == "Connection" && item->what.isEmpty() == false)
		{
			QString connectionId = item->data.toString();

			if (theDialogConnections == nullptr)
			{
				theDialogConnections = new DialogConnections(m_dbc, parentWidget());
				theDialogConnections->show();
			}
			else
			{
				theDialogConnections->activateWindow();
			}

			theDialogConnections->setFilter(connectionId);

			emit clearFocusFromInput();
			return;
		}

		return;
	}
}
