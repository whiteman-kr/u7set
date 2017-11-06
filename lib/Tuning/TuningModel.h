#ifndef TUNINGMODEL_H
#define TUNINGMODEL_H

#include <cmath>

#include "TuningSignalState.h"
#include "TuningFilter.h"


class TuningModel;


class TuningModelSorter
{
public:
	TuningModelSorter(int column, Qt::SortOrder order, TuningSignalManager* tuningSignalManager);

	bool operator()(Hash hash1, Hash hash2) const
	{
		return sortFunction(hash1, hash2, m_column, m_order);
	}

	bool sortFunction(Hash hash1, Hash hash2, int column, Qt::SortOrder order) const;

private:
	int m_column = -1;

	Qt::SortOrder m_order = Qt::AscendingOrder;

	TuningSignalManager* m_tuningSignalManager = nullptr;
};

class TuningModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TuningModel(TuningSignalManager* tuningSignalManager, QWidget* parent);
	~TuningModel();

public:

	enum class Columns
	{
		CustomAppSignalID = 0,
		EquipmentID,
		AppSignalID,
		Caption,
		Units,
		Type,

		Value,
		LowLimit,
		HighLimit,
		Default,
		Valid,
		OutOfRange
	};


public:
	std::vector<Hash> hashes() const;
	void setHashes(std::vector<Hash>& hashes);

	Hash hashByIndex(int index) const;

	TuningSignalManager* tuningSignalManager();

public:

	void addColumn(Columns column);
	void removeColumn(Columns column);
	int columnIndex(int index) const;
	std::vector<int> columnsIndexes();
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	void setFont(const QString& fontName, int fontSize, bool fontBold);
	void setImportantFont(const QString& fontName, int fontSize, bool fontBold);

	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

	void sort(int column, Qt::SortOrder order) override;

	bool limitsUnbalance(const AppSignalParam& asp, const TuningSignalState& tss) const;

protected:
	QModelIndex parent(const QModelIndex& index) const override;
	virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	virtual QBrush backColor(const QModelIndex& index) const;
	virtual QBrush foregroundColor(const QModelIndex& index) const;

private:
	QStringList m_columnsNames;

	QFont* m_font = nullptr;
	QFont* m_importantFont = nullptr;

protected:
	TuningSignalManager* m_tuningSignalManager = nullptr;

	std::vector<Hash> m_hashes;

	std::vector<int> m_columnsIndexes;

	bool m_blink = false;

};

class DialogInputTuningValue : public QDialog
{
	Q_OBJECT

public:
	explicit DialogInputTuningValue(bool analog, TuningValue value, double defaultValue, bool sameValue, double lowLimit, double highLimit, int decimalPlaces, QWidget* parent);
	~DialogInputTuningValue();

private:

	TuningValue m_value;
	double m_defaultValue = 0;
	double m_lowLimit = 0;
	double m_highLimit = 0;
	int m_decimalPlaces = 0;
	bool m_analog = true;

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
	void on_m_checkBox_clicked(bool checked);
	void on_m_buttonDefault_clicked();
};



#endif // TUNINGMODEL
