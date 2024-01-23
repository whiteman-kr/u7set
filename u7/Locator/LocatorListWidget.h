#pragma once
#include "LocatorProvider.h"

namespace Locator
{
	class LocatorListModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		virtual QModelIndex parent(const QModelIndex& index) const override;

		virtual int rowCount(const QModelIndex& parentIndex = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

	public:
		void addData(QString text, QString providerName, const std::vector<LocatedItem>& items);

		const LocatedItem* locatedItem(size_t index) const;
		QString providerName(size_t index) const;

	private:
		QString m_text;
		std::map<QString, std::vector<LocatedItem>> m_data;	// key is locator name, ad map is sorted, then data types
															// by locator always will be in the same order
	};

	class LocatorListWidget : public QTreeView
	{
		Q_OBJECT

	public:
		explicit LocatorListWidget(DbController* dbc, QWidget* parentWidget);

		void addData(QString text, QString providerName, const std::vector<LocatedItem>& items);

		// Use these functions indead of QWidget::show/hide
		//
		void showList();
		void hideList();

	protected slots:
		void slot_doubleClicked(const QModelIndex& index);

	signals:
		void clearFocusFromInput();

	private:
		DbController* m_dbc = nullptr;
		LocatorListModel m_model;

		bool m_doubleCklickShowsModalDialog = false;
	};
}

