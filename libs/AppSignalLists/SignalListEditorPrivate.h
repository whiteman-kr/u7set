#pragma once

#include "../../../AppSignalLib/ISignalManager.h"
#include <AppSignalLists/SignalList.h>


class AppSignalParam;

namespace AppSignalLists
{
	class SignalsModel : public QAbstractTableModel
	{
		Q_OBJECT

	public:
		SignalsModel(ISignalManager& signalManager);

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
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		ISignalManager& m_signalManager;
		std::vector<Hash> m_allHashes;
		std::map<Hash, TuningValue> m_defaultValues;
	};

	class AppSignalListModel : public QAbstractTableModel
	{
		Q_OBJECT

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

	public:
		AppSignalListModel(ISignalManager& signalManager, bool hasValueColumn);

		const AppSignalList* list() const;
		void setList(AppSignalList* list);

		bool itemExists(Hash hash) const;
		Hash itemHash(int row) const;

		[[nodiscard]] bool add(const AppSignalListItem& item);
		[[nodiscard]] bool remove(Hash hash);

		Columns column(int index) const;
		QString columnName(int index) const;	// Untranslated
		QString columnText(int index) const;	// Translated
		QString cellText(int column, int row) const;

		// Item count

		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;

		// Sorting

		void sort(int column, Qt::SortOrder order) override;

	private:
		virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		ISignalManager& m_signalManager;
		const AppSignalList* m_appSignalList = nullptr;

		std::vector<Hash> m_allHashes;

		std::vector <Columns> m_columns;
	};

	class DialogAppSignalListValue : public QDialog
	{
		Q_OBJECT

	public:
		explicit DialogAppSignalListValue(TuningValue value,
										  TuningValue defaultValue,
										  bool sameValue,
										  bool sameDefaultValue,
										  TuningValue lowLimit,
										  TuningValue highLimit,
										  E::AnalogFormat analogFormat,
										  int decimalPlaces,
										  QWidget* parent);

	private:
		TuningValue m_value;
		TuningValue m_defaultValue;
		TuningValue m_lowLimit;
		TuningValue m_highLimit;

		E::AnalogFormat m_analogFormat = E::AnalogFormat::f_9;
		int m_decimalPlaces = 0;

		virtual void accept() override;

	private:
		QCheckBox* m_discreteCheck = nullptr;
		QLineEdit* m_analogEdit = nullptr;
		QPushButton* m_defaultButton = nullptr;
		QPushButton* m_okButton = nullptr;
		QPushButton* m_cancelButton = nullptr;

	public:
		TuningValue value() { return m_value; }

	private slots:
		void onValueCheckStateChanged(Qt::CheckState state);
		void onValueDefaultClicked();
	};
} // namespace AppSignalLists
