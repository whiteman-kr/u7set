#pragma once

#include "SignalList.h"
#include "../../../AppSignalLib/ISignalManager.h"

#include <vector>
#include <memory>

#include <QWidget>

namespace AppSignalLists
{
	class AppSignalListModel;
	class SignalsModel;

	class AppSignalListWidget : public QWidget
	{
		Q_OBJECT

	public:
		AppSignalListWidget(ISignalManager& signalManager, bool requestValuesEnabled, QWidget* parent);
		~AppSignalListWidget();

		bool readOnly() const;
		void setReadOnly(bool value);

		AppSignalList* list() const;
		void setList(AppSignalList* list);

	signals:
		void signalsChanged();
		//void getCurrentSignalValue(Hash appSignalHash, TuningValue* value, bool* ok); 	// Qt::DirectConnection!

	private:
		enum class FilterTextType
		{
			All,
			AppSignalID,
			CustomAppSignalID,
			EquipmentID,
			Caption,
			Tag
		};

		enum class FilterValueType
		{
			All,
			Zero,
			One
		};

		enum class SignalType
		{
			All,
			Analog,
			Discrete
		};

	private:
		void fillSignalsList();
		void fillItemsList();

		void enableSignalsListControls();
		void enableItemsListControls();

	private:
		ISignalManager& m_signalManager;
		std::vector<Hash> m_signalHashes;

		AppSignalList* m_appSignalList = nullptr;

		// Left side

		std::unique_ptr<SignalsModel> m_signalsModel;
		QTableView* m_signalsTable = nullptr;

		QComboBox* m_signalTypeCombo = nullptr;
		QComboBox* m_filterValueCombo = nullptr;
		QComboBox* m_filterTextTypeCombo = nullptr;
		QLineEdit* m_filterTextEdit = nullptr;
		QPushButton* m_applyFilterButton = nullptr;

		int m_signalsSortColumn = 0;
		Qt::SortOrder m_signalsSortOrder = Qt::AscendingOrder;

		// Middle

		QPushButton* m_addValueButton = nullptr;
		QPushButton* m_removeValueButton = nullptr;

		QSplitter* m_splitter = nullptr;

		// Right side

		std::unique_ptr<AppSignalListModel> m_itemsModel;
		QTableView* m_itemsTable = nullptr;

		int m_itemsSortColumn = 0;
		Qt::SortOrder m_itemsSortOrder = Qt::AscendingOrder;

		QPushButton* m_setValueButton = nullptr;
		QPushButton* m_setCurrentButton = nullptr;
		QPushButton* m_exportValuesButton = nullptr;
		QPushButton* m_importValuesButton = nullptr;

		//
		bool m_readOnly = false;

	private slots:
		void onSignalsSortIndicatorChanged(int column, Qt::SortOrder order);
		void onSignalsTableSelectionChanged(const QItemSelection&, const QItemSelection&);
		void onSignalsApplyFilterClicked();
		void onSignalsFilterTypeComboCurrentIndexChanged(int index);
		void onSignalsFilterValueComboCurrentIndexChanged(int index);
		void onSignalsFilterTextChanged();
		void onSignalsTypeComboCurrentIndexChanged(int index);
		void onSignalsTableDoubleClicked(const QModelIndex& index);
		void onSignalsHeaderColumnContextMenuRequested(const QPoint& pos);
		void onSignalsHeaderColumnToggled(bool checked);

		void onItemsSortIndicatorChanged(int column, Qt::SortOrder order);
		void onItemsTreeSelectionChanged();
		void onItemsTreeDoubleClicked(const QModelIndex& index);
		void onItemsHeaderColumnContextMenuRequested(const QPoint& pos);
		void onItemsHeaderColumnToggled(bool checked);

		void onAddClicked();
		void onRemoveClicked();

		void onSetValueClicked();
		void onSetCurrentClicked();
		void onExportValuesClicked();
		void onImportValuesClicked();
	};

}