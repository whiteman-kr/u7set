#ifndef TUNINGMODEL_H
#define TUNINGMODEL_H

#include <cmath>

#include "TuningSignalState.h"
#include "TuningFilter.h"


class TuningModel;

enum class TuningModelColumns
{
	CustomAppSignalID = 0,
	EquipmentID,
	AppSignalID,
	Caption,
	Units,
	Type,

	ValueFirst = 100,
	ValueLast = ValueFirst + MAX_VALUES_COLUMN_COUNT - 1,

	LowLimit,
	HighLimit,
	Default,
	Valid,
	OutOfRange
};

struct TuningModelHashSet
{
	Hash firstHash() const;

	int hashCount() const;

	Hash hash[MAX_VALUES_COLUMN_COUNT];

};

class TuningModelSorter
{
public:
	TuningModelSorter(TuningModelColumns column, Qt::SortOrder order, const TuningModel* model, const TuningSignalManager* tuningSignalManager);

	bool operator()(const TuningModelHashSet& set1, const TuningModelHashSet& set2) const
	{
		return sortFunction(set1, set2, m_column, m_order);
	}

	bool sortFunction(const TuningModelHashSet& set1, const TuningModelHashSet& set2, TuningModelColumns column, Qt::SortOrder order) const;

private:
	TuningModelColumns m_column = TuningModelColumns::AppSignalID;

	Qt::SortOrder m_order = Qt::AscendingOrder;

	const TuningSignalManager* m_tuningSignalManager = nullptr;

	const TuningModel* m_model = nullptr;
};

class TuningModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	TuningModel(TuningSignalManager* tuningSignalManager, const std::vector<QString>& valueColumnsAppSignalIdSuffixes, QWidget* parent);
	~TuningModel();

	TuningValue defaultValue(const AppSignalParam& asp) const;
	void setDefaultValues(const std::vector<std::pair<Hash, TuningValue>>& values);

public:


public:
	std::vector<Hash> allHashes() const;
	void setHashes(std::vector<Hash>& allHashes);

	Hash hashByIndex(int row, int valueColumn) const;
	const TuningModelHashSet& hashSetByIndex(int row) const;

	TuningSignalManager* tuningSignalManager();

public:

	// Columns processing
	int valueColumnsCount() const;

	void addColumn(TuningModelColumns column);
	void removeColumn(TuningModelColumns column);
	TuningModelColumns columnType(int index) const;
	std::vector<TuningModelColumns> columnTypes();
	void setColumnTypes(std::vector<TuningModelColumns> columnTypes);

	// Font

	void setFont(const QFont& font);
	void setImportantFont(const QFont& font);

	// Item count

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;

	// Sorting

	void sort(int column, Qt::SortOrder order) override;

	E::AnalogFormat analogFormat() const;
	void setAnalogFormat(E::AnalogFormat format);

protected:
	virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	virtual QBrush backColor(const QModelIndex& index) const;
	virtual QBrush foregroundColor(const QModelIndex& index) const;

private:
	std::map<TuningModelColumns, QString> m_columnsNamesMap;

	QFont* m_font = nullptr;
	QFont* m_importantFont = nullptr;

protected:
	TuningSignalManager* m_tuningSignalManager = nullptr;

	std::vector<Hash> m_allHashes;

	std::vector<TuningModelHashSet> m_hashSets;

	std::map<Hash, int> m_hashToChannelMap;

	std::map<Hash, Hash> m_hashToGeneralHashMap;

	std::map<Hash, TuningModelHashSet> m_generalHashToHashSetMap;

	std::map<Hash, TuningValue> m_defaultValues;

	std::vector<TuningModelColumns> m_columnsTypes;

	std::vector<QStringList> m_valueColumnAppSignalIdSuffixes;

	E::AnalogFormat m_analogFormat = E::AnalogFormat::g_9_or_9e;
};

class DialogInputTuningValue : public QDialog
{
	Q_OBJECT

public:
	explicit DialogInputTuningValue(TuningValue value, TuningValue defaultValue, bool sameValue, TuningValue lowLimit, TuningValue highLimit, E::AnalogFormat analogFormat, int decimalPlaces, QWidget* parent);
	~DialogInputTuningValue();

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
	QPushButton* m_buttonDefault = nullptr;
	QPushButton* m_buttonOK = nullptr;
	QPushButton* m_buttonCancel = nullptr;

public:
	TuningValue value() { return m_value; }

private slots:
	void on_m_checkBox_stateChanged(int state);
	void on_m_buttonDefault_clicked();
};



#endif // TUNINGMODEL
