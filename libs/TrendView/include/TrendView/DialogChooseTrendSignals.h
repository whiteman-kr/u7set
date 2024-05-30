#pragma once

#include <map>
#include <vector>

#include "../lib/ISignalHasTag.h"
#include <QCompleter>
#include <QDialog>

#include "TrendSignal.h"

class QItemSelection;


namespace Ui
{
	class DialogChooseTrendSignals;
}

namespace AppSignalLists
{
	class AppSignalList;
	class AppSignalListSet;
}

namespace TrendLibInternal
{
	class FilteredTrendSignalsModel : public QAbstractTableModel
	{
		Q_OBJECT
	public:
		FilteredTrendSignalsModel(const ISignalHasTag* signalHasTag,
								  const std::vector<TrendLib::TrendSignalParam>& signalss,
								  QObject* parent);

	public:
		int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		int columnCount(const QModelIndex& parent = QModelIndex()) const override;
		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
		QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		void filterSignals(QString server, std::optional<AppSignalLists::AppSignalList*> appSignalList, QString filter, QStringList tagList);

		const TrendLib::TrendSignalParam& signalByRow(int row) const;

	private:
		const ISignalHasTag* m_signalHasTag = nullptr;
		std::vector<size_t> m_signalIndexes;
		std::vector<TrendLib::TrendSignalParam> m_signals;
		std::map<QString, std::vector<size_t>>
			m_startWithArrays; // Key is startWith, in lowercase. Values are indexes in m_signals for stratWith
	};
} // namespace TrendLibInternal

namespace TrendLib
{
	class DialogChooseTrendSignals : public QDialog
	{
		Q_OBJECT

	public:
		// Constructor for TrendLib::TrendSignalParam
		//
		DialogChooseTrendSignals(const ISignalHasTag* signalHasTag,
								 std::vector<TrendLib::TrendSignalParam> trendSignals,
								 const std::vector<TrendLib::TrendSignalParam>& acceptedSignals,
								 const std::vector<TrendLib::ArchiveServer>& archiveServers,
								 const AppSignalLists::AppSignalListSet& appSignalLists,
								 QWidget* parent);

		virtual ~DialogChooseTrendSignals();

	protected:
		void init(const ISignalHasTag* signalHasTag,
				  std::vector<TrendLib::TrendSignalParam> signalss,
				  const std::vector<TrendLib::TrendSignalParam>& acceptedSignals,
				  const std::vector<TrendLib::ArchiveServer>& archiveServers);

	public:
		std::vector<TrendLib::TrendSignalParam> acceptedSignals() const;

	protected:
		virtual void resizeEvent(QResizeEvent* event) override;

		void fillServerCombo();
		void fillAppSignalLists();
		void fillSignalList();

		void addSignal(const TrendSignalParam& signal);
		void removeSelectedSignal();

		bool trendSignalsHasSignalId(QString signalId, QString archiveServerShortId);

		void disableControls();

	private slots:
		void serverCurrentIndexChanged(int index);

		void on_addSignalButton_clicked();
		void on_removeSignalButton_clicked();
		void on_removeAllSignalsButton_clicked();

		void on_filterEdit_textChanged(const QString& arg);
		void on_filterEdit_editingFinished();

		void on_tagsEdit_textChanged(const QString& arg);
		void on_tagsEdit_editingFinished();

		void on_filteredSignals_doubleClicked(const QModelIndex& index);
		void slot_filteredSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		void on_trendSignals_doubleClicked(const QModelIndex& index);
		void slot_trendSignalsSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

		void listComboIndexChanged(int index);

		void on_buttonBox_accepted();

		void on_trendSignals_customContextMenuRequested(const QPoint& pos);

		void on_upSignalButton_clicked();
		void on_downSignalButton_clicked();

	private:
		Ui::DialogChooseTrendSignals* ui = nullptr;

		const ISignalHasTag* m_signalHasTag = nullptr;

		std::vector<TrendLib::TrendSignalParam> m_acceptedSignals;
		std::vector<TrendLib::ArchiveServer> m_archiveServers;
		const AppSignalLists::AppSignalListSet& m_appSignalListSet;

		// --
		//
		QCompleter* m_filterCompleter = nullptr;
		QCompleter* m_tagsCompleter = nullptr;

		const QString m_filterCompleterSettingsName = "DialogChooseTrendSignals/trendSignalsDialogFilterCompleter";
		const QString m_tagsCompleterSettingsName = "DialogChooseTrendSignals/trendSignalsDialogTagsCompleter";
		const QString m_sizeSettingsName = "DialogChooseTrendSignals/size";

		QString s_allServers;
		inline static QString s_lastServer;
	};
} // namespace TrendLib
