#ifndef DIALOGSIGNALSEARCH_H
#define DIALOGSIGNALSEARCH_H

#include "../lib/AppSignal.h"
#include <QDialog>

namespace Ui {
class DialogSignalSearch;
}

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
