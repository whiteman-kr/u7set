#pragma once

class IAppSignalManager;

namespace ClientLib
{
	class ISignalDataServer;
}

namespace SchemaClientLib
{
	class SignalSnapshotModel;

	enum class SnapshotColumns
	{
		SignalID = 0,		// Signal Param Columns
		EquipmentID,
		LmEquipmentID,
		AppSignalID,
		Caption,
		Type,
		Tags,

		SystemTime,			// Signal State Columns
		LocalTime,
		PlantTime,
		Value,
		Units,
		Valid,
		StateAvailable,
		Simulated,
		Blocked,
		Mismatch,
		OutOfLimits,

		ColumnCount
	};
} // namespace SchemaClientLib

namespace AppSignalLists
{
	class AppSignalListSet;
}

Q_DECLARE_METATYPE(SchemaClientLib::SnapshotColumns);

namespace SchemaClientLib
{
	enum class SnapshotSignalType
	{
		Any = 0,
		Analog,
		Discrete,
		Count
	};

	enum class SnapshotSignalRole
	{
		Any = 0,
		Input,
		Output,
		Internal,
		Tunable,
		Count
	};

	enum class SnapshotMaskType
	{
		All = 0,
		AppSignalId,
		CustomAppSignalId,
		EquipmentId,
		LmEquipmentId,
		Count
	};

	//
	// SignalSnapshotSorter
	//
	class SignalSnapshotSorter
	{
	public:
		SignalSnapshotSorter(int column, SignalSnapshotModel* model);

		bool operator()(int index1, int index2) const
		{
			return sortFunction(index1, index2);
		}

		bool sortFunction(int index1, int index2) const;

	private:
		int m_column = -1;

		SignalSnapshotModel* m_model = nullptr;
	};

	//
	// SnapshotItemModel
	//
	class SignalSnapshotModel : public QAbstractItemModel
	{
		friend class SignalSnapshotSorter;

	public:

	public:
		SignalSnapshotModel(IAppSignalManager* appSignalManager, ClientLib::ISignalDataServer* signalDataServer, AppSignalLists::AppSignalListSet* appSignalListSet, QObject* parent);

		void setSignals(std::vector<AppSignalParam>& signalList);

	public:
		// Properties

		QStringList columnsNames() const;

		// Overrides

		QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;

		// Operations

		void setSignalType(SnapshotSignalType type);
		void setSignalRole(SnapshotSignalRole role);

		void setMaskType(SnapshotMaskType type);
		void setMasks(const QStringList& masks);

		void setTags(const QStringList& tags);

		void setDataServiceId(const QString& dataServiceId);

		void setSchemaAppSignals(std::set<QString> schemaAppSignals);

		void setAppSignalList(const QString& listId);
		QString appSignalList() const;

		void fillSignals();

		void updateStates(int from, int to);

		void sort(int column, Qt::SortOrder order) override;

		AppSignalParam signalParam(int rowIndex, bool* found);
		AppSignalState signalState(int rowIndex, bool* found);

		E::AnalogFormat analogFormat() const;
		void setAnalogFormat(E::AnalogFormat format);

		int analogPrecision() const;
		void setAnalogPrecision(int precision);

	protected:
		QModelIndex parent(const QModelIndex& index) const override;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		IAppSignalManager* m_appSignalManager = nullptr;
		ClientLib::ISignalDataServer* m_signalDataServer = nullptr;
		AppSignalLists::AppSignalListSet* m_appSignalListSet = nullptr;

		QStringList m_columnsNames;

		// Model data

		std::vector<AppSignalParam> m_allSignals;
		std::vector<AppSignalState> m_allStates;
		std::vector<int> m_filteredSignals;

		// Filtering parameters

		SnapshotSignalType m_signalType = SnapshotSignalType::Any;
		SnapshotSignalRole m_signalRole = SnapshotSignalRole::Any;
		SnapshotMaskType m_maskType = SnapshotMaskType::CustomAppSignalId;
		QStringList m_masks;
		QStringList m_tags;
		QString m_dataServiceId;
		QString m_listId;
		std::set<QString> m_schemaAppSignals;

		// View params

		E::AnalogFormat m_analogFormat = E::AnalogFormat::g_9_or_9e;
		int m_analogPrecision = -1;
	};
} // namespace SchemaClientLib