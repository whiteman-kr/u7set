#ifndef DIALOGSIGNALSEARCH_H
#define DIALOGSIGNALSEARCH_H

#include "../lib/AppSignal.h"
#include <QDialog>

namespace Ui {
	class DialogSignalSearch;
}

class SignalSearchSorter	// later move this class to some library file, it can be used in other cases
{
public:

	enum class Columns
	{
		SignalID = 0,
		AppSignalID,
		Caption,
		EquipmentID,

		Channel,
		Type,
		Units,

		LowValidRange,
		HighValidRange,

		LowEngineeringUnits,
		HighEngineeringUnits,

		EnableTuning,
		TuningDefaultValue
	};

	SignalSearchSorter(std::vector<AppSignalParam>* appSignalParamVec, Columns sortColumn = Columns::SignalID, Qt::SortOrder sortOrder = Qt::AscendingOrder);

	bool operator()(int index1, int index2)
	{
		return sortFunction(index1, index2, this);
	}

	bool sortFunction(int index1, int index2, const SignalSearchSorter* pThis);

private:
	Columns m_sortColumn = Columns::SignalID;

	Qt::SortOrder m_sortOrder = Qt::AscendingOrder;

	std::vector<AppSignalParam>* m_appSignalParamVec = nullptr;

	QVariant v1;
	QVariant v2;
};

class SignalSearchItemModel : public QAbstractItemModel
{
	Q_OBJECT

public:
	SignalSearchItemModel(QObject *parent);

public:
	AppSignalParam getSignal(const QModelIndex& index) const;
	void setSignals(std::vector<AppSignalParam>* signalsVector);

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

public:

	enum class Columns
	{
		SignalID = 0,
		Caption,
	};

	enum class TypeFilter
	{
		All = 0,
		AnalogInput,
		AnalogOutput,
		DiscreteInput,
		DiscreteOutput
	};

protected:
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;

	QModelIndex parent(const QModelIndex &index) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:

	std::vector<AppSignalParam> m_signals;
	QStringList m_columnsNames;
};


class DialogSignalSearch : public QDialog
{
	Q_OBJECT

public:
	explicit DialogSignalSearch(QWidget* parent);
	virtual ~DialogSignalSearch();

private slots:
	void on_editSignalID_textEdited(const QString &arg1);
	void on_DialogSignalSearch_finished(int result);
	void prepareContextMenu(const QPoint& pos);

	void on_tableView_doubleClicked(const QModelIndex &index);

protected:

private:
	void search();

private:
	Ui::DialogSignalSearch* ui;
	static QString m_signalId;

	SignalSearchItemModel m_model;

	std::vector<AppSignalParam> m_signals;
};

#endif // DIALOGSIGNALSEARCH_H
