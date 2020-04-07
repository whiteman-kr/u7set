#pragma once

#include "../../lib/IAppSignalManager.h"
#include "../TrendView/TrendSignal.h"

namespace Ui {
	class DialogChooseTrendSignals;
}

class DialogChooseTrendSignals : public QDialog
{
	Q_OBJECT

public:
	DialogChooseTrendSignals(IAppSignalManager* signalManager,
							 std::vector<TrendLib::TrendSignalParam>& trendSignals,
							 QWidget* parent);
	virtual ~DialogChooseTrendSignals();

	std::vector<AppSignalParam> acceptedSignals() const;

protected:
	virtual void resizeEvent(QResizeEvent* event) override;

	void fillSignalList();

	void addSignal(const AppSignalParam& signal);
	void removeSelectedSignal();

	bool trendSignalsHasSignalId(QString signalId);

	void disableControls();

private slots:
	void on_addSignalButton_clicked();
	void on_removeSignalButton_clicked();
	void on_removeAllSignalsButton_clicked();

	void on_filterEdit_textChanged(const QString &arg);
	void on_filterEdit_editingFinished();

	void on_tagsEdit_textChanged(const QString &arg);
	void on_tagsEdit_editingFinished();

	void on_filteredSignals_doubleClicked(const QModelIndex &index);
	void slot_filteredSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void on_trendSignals_doubleClicked(const QModelIndex &index);
	void slot_trendSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void on_buttonBox_accepted();

private:
	Ui::DialogChooseTrendSignals* ui;
	QCompleter* m_filterCompleter = nullptr;
	QCompleter* m_tagsCompleter = nullptr;

	const QString m_filterCompleterSettingsName = "/DialogChooseTrendSignals/trendSignalsDialogFilterCompleter";
	const QString m_tagsCompleterSettingsName = "/DialogChooseTrendSignals/trendSignalsDialogTagsCompleter";
	const QString m_sizeSettingsName = "/DialogChooseTrendSignals/size";

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

	void filterSignals(QString filter, QString tags);

	AppSignalParam signalByRow(int row) const;

private:
	std::vector<int> m_signalIndexes;
	std::vector<AppSignalParam> m_signals;
	std::map<QString, std::vector<int>> m_startWithArrays;	// Key is startWith, in lowercase. Values are indexes in m_signals for stratWith
};

