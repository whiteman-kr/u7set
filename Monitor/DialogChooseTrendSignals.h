#ifndef DIALOGCHOOSETRENDSIGNALS_H
#define DIALOGCHOOSETRENDSIGNALS_H

#include <QDialog>
#include "../lib/AppSignalManager.h"

namespace Ui {
	class DialogChooseTrendSignals;
}

class DialogChooseTrendSignals : public QDialog
{
	Q_OBJECT

public:
	explicit DialogChooseTrendSignals(QWidget* parent);
	virtual ~DialogChooseTrendSignals();

	std::vector<AppSignalParam> acceptedSignals() const;

protected:
	void fillSignalList();

	void addSignal(const AppSignalParam& signal);
	void removeSelectedSignal();

	bool trendSignalsHasSignalId(QString signalId);

	void disableControls();

private slots:
	void on_addSignalButton_clicked();
	void on_removeSignalButton_clicked();
	void on_removeAllSignalsButton_clicked();

	void on_filterEdit_textChanged(const QString &arg1);

	void on_filteredSignals_doubleClicked(const QModelIndex &index);
	void slot_filteredSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void on_trendSignals_doubleClicked(const QModelIndex &index);
	void slot_trendSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void on_buttonBox_accepted();

private:
	Ui::DialogChooseTrendSignals* ui;

	std::vector<AppSignalParam> m_acceptedSignals;
};

class FilteredTrendSignalsModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	FilteredTrendSignalsModel(const std::vector<AppSignalParam>& signalss, QObject* parent);

public:
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void filterSignals(QString filter);

	AppSignalParam signalByRow(int row) const;

private:
	std::vector<int> m_signalIndexes;
	std::vector<AppSignalParam> m_signals;
	std::map<QString, std::vector<int>> m_startWithArrays;	// Key is startWith, in lowercase. Valus is indexes in m_signals for stratWith
};

#endif // DIALOGCHOOSETRENDSIGNALS_H
