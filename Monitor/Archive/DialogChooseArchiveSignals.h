#pragma once

#include "ArchiveData.h"
#include "../../OnlineLib/SoftwareSettings.h"

class MonitorSignalManager;

namespace Ui {
	class DialogChooseArchiveSignals;
}


class DialogChooseArchiveSignals : public QDialog
{
	Q_OBJECT

public:
	DialogChooseArchiveSignals(const MonitorSignalManager* signalManager,
							   const std::vector<SoftwareEndpoint::ArchiveService>& archiveServices,
							   const ArchiveSource& init,
							   QWidget* parent);
	virtual ~DialogChooseArchiveSignals();

	[[nodiscard]] ArchiveSource accpetedResult() const;

protected:
	void fillServerCombo();
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

	// Variable to restore last UI state
	//
	static ArchiveSignalType s_lastSignalType;
	static QString s_lastServer;

	inline static const QString s_allServers{"All Servers"};
};

