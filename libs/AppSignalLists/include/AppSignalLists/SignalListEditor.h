#pragma once

#include "SignalList.h"
#include "../../../AppSignalLib/ISignalManager.h"
#include "../../../lib/PropertyEditor.h"

namespace AppSignalLists
{
	class SignalsModel : public QAbstractTableModel
	{
		Q_OBJECT

	public:
		SignalsModel(ISignalManager& signalManager);
		~SignalsModel();

		TuningValue defaultValue(const AppSignalParam& asp) const;
		void setDefaultValues(const std::vector<std::pair<Hash, TuningValue>>& values);

		std::vector<Hash> allHashes() const;
		void setHashes(std::vector<Hash>& allHashes);

		Hash hash(int row) const;

		QString columnText(int index) const;
		QString cellText(int column, int row) const;

		// Item count

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		// Sorting

		void sort(int column, Qt::SortOrder order) override;

	public:
		enum class Columns
		{
			CustomAppSignalID = 0,
			EquipmentID,
			AppSignalID,
			Caption,
			Units,
			Type,
			LowLimit,
			HighLimit,
			Default,
			Count
		};

	private:
		virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		ISignalManager& m_signalManager;
		std::vector<Hash> m_allHashes;
		std::map<Hash, TuningValue> m_defaultValues;
	};

	class SignalModelSorter
	{
	public:
		SignalModelSorter(ISignalManager& tuningSignalManager, SignalsModel::Columns column, Qt::SortOrder order);
		bool sortFunction(Hash hash1, Hash hash2) const;
		bool operator()(Hash hash1, Hash hash2) const
		{
			return sortFunction(hash1, hash2);
		}

	private:
		ISignalManager& m_signalManager;
		SignalsModel::Columns m_column = SignalsModel::Columns::AppSignalID;
		Qt::SortOrder m_order = Qt::AscendingOrder;
	};

	class AppSignalListModel : public QAbstractTableModel
	{
		Q_OBJECT

	public:
		AppSignalListModel(ISignalManager& signalManager);
		~AppSignalListModel();

		const AppSignalList* list() const;
		void setList(AppSignalList* list);

		bool itemExists(Hash hash) const;
		Hash itemHash(int row) const;

		[[nodiscard]] bool add(const AppSignalListItem& item);
		[[nodiscard]] bool remove(Hash hash);

		QString columnText(int index) const;
		QString cellText(int column, int row) const;

		// Item count

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		// Sorting

		void sort(int column, Qt::SortOrder order) override;

	public:
		enum class Columns
		{
			CustomAppSignalID,
			EquipmentID,
			AppSignalID,
			Caption,
			Units,
			Type,
			LowLimit,
			HighLimit,
			Value,
			Count
		};

	private:
		virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		ISignalManager& m_signalManager;
		const AppSignalList* m_appSignallist = nullptr;

		std::vector<Hash> m_allHashes;
	};

	class AppSignalListModelSorter
	{
	public:
		AppSignalListModelSorter(ISignalManager& tuningSignalManager, const AppSignalList* appSignallist, AppSignalListModel::Columns column, Qt::SortOrder order);
		bool sortFunction(Hash hash1, Hash hash2) const;
		bool operator()(Hash hash1, Hash hash2) const
		{
			return sortFunction(hash1, hash2);
		}

	private:
		ISignalManager& m_signalManager;
		const AppSignalList* m_appSignallist = nullptr;

		AppSignalListModel::Columns m_column = AppSignalListModel::Columns::AppSignalID;
		Qt::SortOrder m_order = Qt::AscendingOrder;
	};

	class DialogAppSignalListValue : public QDialog
	{
		Q_OBJECT

	public:
		explicit DialogAppSignalListValue(TuningValue value, TuningValue defaultValue, bool sameValue, bool sameDefaultValue,
			TuningValue lowLimit, TuningValue highLimit, E::AnalogFormat analogFormat, int decimalPlaces, QWidget* parent);
		~DialogAppSignalListValue();

	private:
		TuningValue m_value;
		TuningValue m_defaultValue;
		TuningValue m_lowLimit;
		TuningValue m_highLimit;

		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
		int m_decimalPlaces = 0;

		virtual void accept();

	private:
		QCheckBox* m_discreteCheck = nullptr;
		QLineEdit* m_analogEdit = nullptr;
		QPushButton* m_defaultButton = nullptr;
		QPushButton* m_okButton = nullptr;
		QPushButton* m_cancelButton = nullptr;

	public:
		TuningValue value() { return m_value; }

	private slots:
		void onValueCheckStateChanged(int state);
		void onValueDefaultClicked();
	};


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

		AppSignalList* m_appSignallist = nullptr;

		// Left side

		SignalsModel m_signalsModel;
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

		// Right side

		AppSignalListModel m_itemsModel;
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