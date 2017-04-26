#ifndef TUNINGMODEL_H
#define TUNINGMODEL_H

#include "TuningSignalState.h"
#include "TuningFilter.h"


class TuningModel;

struct TuningModelRecord
{
	AppSignalParam param;
	TuningSignalState state;

	bool limitsUnbalance() const
	{
		if (state.valid() == true && (param.lowEngeneeringUnits() != state.readLowLimit() || param.highEngeneeringUnits() != state.readHighLimit()))
		{
			return true;
		}
		return false;

	}
};

class TuningModelRecordSorter
{
public:
	  TuningModelRecordSorter(int column, Qt::SortOrder order);

	  bool operator()(const TuningModelRecord& o1, const TuningModelRecord& o2) const
	  {
		  return sortFunction(o1, o2, m_column, m_order);
	  }

	  bool sortFunction(const TuningModelRecord& o1, const TuningModelRecord& o2, int column, Qt::SortOrder order) const;

private:
	  int m_column = -1;

	  Qt::SortOrder m_order = Qt::AscendingOrder;
};

class TuningModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	TuningModel(QWidget *parent);
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
		Underflow,
		Overflow,
	};


public:
	void setSignals(std::vector<TuningModelRecord> &signalsList);

	AppSignalParam* param(int index);
	TuningSignalState* state(int index);

	void addColumn(Columns column);
    void removeColumn(Columns column);
    int columnIndex(int index) const;
	std::vector<int> columnsIndexes();
	void setColumnsIndexes(std::vector<int> columnsIndexes);

	void setFont(const QString& fontName, int fontSize, bool fontBold);
    void setImportantFont(const QString& fontName, int fontSize, bool fontBold);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

	void sort(int column, Qt::SortOrder order) override;

protected:
	QModelIndex parent(const QModelIndex &index) const override;
	virtual	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	virtual QBrush backColor(const QModelIndex& index) const;
	virtual QBrush foregroundColor(const QModelIndex& index) const;

private:

	QStringList m_columnsNames;

	QFont* m_font = nullptr;
    QFont* m_importantFont = nullptr;

protected:
	std::vector<int> m_columnsIndexes;

	std::vector<TuningModelRecord> m_items;

	bool m_blink = false;

	QWidget* m_parent = nullptr;
};

class DialogInputTuningValue : public QDialog
{
    Q_OBJECT

public:
    explicit DialogInputTuningValue(bool analog, float value, float defaultValue, bool sameValue, float lowLimit, float highLimit, int decimalPlaces, QWidget *parent);
    ~DialogInputTuningValue();

private:

    float m_value = 0;
    float m_defaultValue = 0;
    float m_lowLimit = 0;
    float m_highLimit = 0;
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
    float value() { return m_value; }

private slots:
    void on_m_checkBox_clicked(bool checked);
    void on_m_buttonDefault_clicked();
};



#endif // TUNINGMODEL
