#ifndef DIALOGCHOOSEARCHIVESIGNALS_H
#define DIALOGCHOOSEARCHIVESIGNALS_H

#include "../lib/AppSignalManager.h"
#include "../VFrame30/Schema.h"
#include "ArchiveData.h"


namespace Ui {
	class DialogChooseArchiveSignals;
}

class DialogChooseArchiveSignals : public QDialog
{
	Q_OBJECT
public:
	struct Result;

public:
	DialogChooseArchiveSignals(const std::vector<VFrame30::SchemaDetails>& schemaDetails,
							   const ArchiveSource& init,
							   QWidget* parent);
	virtual ~DialogChooseArchiveSignals();

	ArchiveSource accpetedResult() const;

protected:
	void fillSignalList();
	void filterSignals();

	void addSignal(const AppSignalParam& signal);
	void removeSelectedSignal();

	bool archiveSignalsHasSignalId(QString signalId);

	void disableControls();

private slots:
	void signalTypeCurrentIndexChanged(int index);
	void schemaCurrentIndexChanged(int index);

	void on_addSignalButton_clicked();
	void on_removeSignalButton_clicked();
	void on_removeAllSignalsButton_clicked();

	void on_filterEdit_textChanged(const QString &arg);
	void on_filterEdit_editingFinished();

	void on_filteredSignals_doubleClicked(const QModelIndex &index);
	void slot_filteredSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void on_archiveSignals_doubleClicked(const QModelIndex &index);
	void slot_archiveSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void on_buttonBox_accepted();

	// Types
	//
public:
	enum class ArchiveSignalType
	{
		AllSignals,
		AnalogSignals,
		DiscreteSignals
	};
	Q_ENUM(ArchiveSignalType);

private:
	Ui::DialogChooseArchiveSignals* ui;
	QCompleter* m_filterCompleter = nullptr;

	const std::vector<VFrame30::SchemaDetails>& m_schemasDetails;

	// Variable to restore last UI state
	//
	static ArchiveSignalType m_lastSignalType;
	static QString m_lastSchemaId;

	ArchiveSource m_result;
};

class FilteredArchiveSignalsModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	FilteredArchiveSignalsModel(const std::vector<AppSignalParam>& signalss,
								const std::vector<VFrame30::SchemaDetails>& schemasDetails,
								QObject* parent);

public:
	int rowCount(const QModelIndex& parent = QModelIndex()) const override;
	int columnCount(const QModelIndex& parent = QModelIndex()) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	void filterSignals(DialogChooseArchiveSignals::ArchiveSignalType signalType, QString signalIdFilter, QString schemaId);

	AppSignalParam signalByRow(int row) const;

private:
	std::vector<int> m_signalIndexes;
	std::vector<AppSignalParam> m_signals;
	const std::vector<VFrame30::SchemaDetails>& m_schemasDetails;
	std::map<QString, std::vector<int>> m_startWithArrays;	// Key is startWith, in lowercase. Values is indexes in m_signals for stratWith
};

#endif // DIALOGCHOOSEARCHIVESIGNALS_H
