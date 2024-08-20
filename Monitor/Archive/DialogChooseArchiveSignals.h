#pragma once

#include "ArchiveData.h"
#include "../../OnlineLib/SoftwareSettings.h"


namespace ClientLib
{
	class AppSignalManager;
}

namespace AppSignalLists
{
	class AppSignalList;
	class AppSignalListSet;
}

namespace Ui {
	class DialogChooseArchiveSignals;
}

class DialogChooseArchiveSignals : public QDialog
{
	Q_OBJECT

public:
	DialogChooseArchiveSignals(const ClientLib::AppSignalManager& signalManager,
							   const std::vector<SoftwareEndpoint::ArchiveService>& archiveServices,
							   const ArchiveSource& init,
							   const AppSignalLists::AppSignalListSet& lists,
							   QWidget* parent);
	virtual ~DialogChooseArchiveSignals();

	[[nodiscard]] ArchiveSource accpetedResult() const;

private:
	void fillServerCombo();
	void fillAppSignalLists();
	void fillSignalList();
	void filterSignals();


	void addSignal(const ArchiveSignal& archiveSignal);
	void removeSelectedSignal();

	[[nodiscard]] bool signalAlreadyPresent(const QString& customSignalId, const QString& archiveServiceId);

	void updateControls();

private slots:
	void signalTypeCurrentIndexChanged(int index);
	void serverCurrentIndexChanged(int index);

	void on_addSignalButton_clicked();
	void on_removeSignalButton_clicked();
	void on_removeAllSignalsButton_clicked();

	void on_filterEdit_textChanged(const QString &arg);
	void on_filterEdit_editingFinished();

	void on_filteredSignals_doubleClicked(const QModelIndex &index);
	void slot_filteredSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

	void on_archiveSignals_doubleClicked(const QModelIndex &index);
	void slot_archiveSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
	
	void listComboIndexChanged(int index);

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

	std::vector<SoftwareEndpoint::ArchiveService> m_archiveServices;

	// --
	//
	ArchiveSource m_result;
	const AppSignalLists::AppSignalListSet& m_appSignalListSet;

	// Variable to restore last UI state
	//
	static ArchiveSignalType s_lastSignalType;
	static QString s_lastServer;

	QString s_allServers;
};

namespace MonitorInternal// Anonymous namespace, as this class is used just in this translation unit
{
	class FilteredArchiveSignalsModel : public QAbstractTableModel
	{
		 Q_OBJECT

	public:
		FilteredArchiveSignalsModel(std::vector<ArchiveSignal>&& signalss, QObject* parent);

	public:
		int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
		int columnCount(const QModelIndex& parent = QModelIndex{}) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		void filterSignals(QString server, DialogChooseArchiveSignals::ArchiveSignalType signalType, std::optional<AppSignalLists::AppSignalList*> appSignalList, const QString& signalIdFilter);

		[[nodiscard]] ArchiveSignal signalByRow(int row) const;		// can throw std::out_of_range()

	private:
		std::vector<size_t> m_signalIndexes;
		std::vector<ArchiveSignal> m_signals;
		std::map<QString, std::vector<size_t>> m_startWithArrays;	// Key is startWith, in lowercase. Values is indexes in m_signals for startWith

		enum class ColumnType
		{
			SignalId,
			Type,
			Caption,
			Server
		};
	};
}
