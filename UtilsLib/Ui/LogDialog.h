#pragma once

#include <QDialog>
#include <QTableView>
#include <QStyledItemDelegate>
#include <QSortFilterProxyModel>

#include "../../UtilsLib/LogFile.h"

namespace Log
{
	class LogRecordModel;
	class LogRecordProxyModel;
	class LogTableView;

	class LogFileDialog : public QDialog
	{
		Q_OBJECT

	public:
		static void view(LogFile& log,
						 QWidget* parent,
						 bool showType = true,
						 bool headerVisible = false,
						 const QStringList& headerTitles = {});

	private:
		LogFileDialog(LogFile& log, QWidget* parent, bool useMessageType, bool headerVisible, const QStringList& headerTitles);
		virtual ~LogFileDialog();

	private:
		void enableControls(bool enable);
		void searchIssue(bool forward);

        int searchIssue(int startRow, bool forward) const;
        int searchRecord(int startRow, const std::string& text) const;

    private slots:
		void onDialogFinished(int result);
		void onTypeComboIndexChanged(int index);
		void onTimeFilter();
		void onFilter();
		void onFilterEdited(const QString& text);
		void onSearch();

		void onAllSessionsClicked();

        void onLoadChunkComplete();
		void onLoadComplete();

		void onRecordArrived(Log::LogFileRecord record);

		void onCellCopyKeyPressed();

		void turnOffAutoscroll();
		void onLoad();
		void onClear();
		void onExport();
		void onPrevIssue();
		void onNextIssue();

		void onLinkActivated(const QString& link);

	private:
		LogFile& m_log;

		QComboBox* m_recordTypeCombo = nullptr;

		QPushButton* m_timeFilterButton = nullptr;

		QLineEdit* m_filterLineEdit = nullptr;

		QPushButton* m_allSessions = nullptr;
		QPushButton* m_autoScroll = nullptr;
		QLabel* m_counterLabel = nullptr;

		QPushButton* m_prevIssue = nullptr;
		QPushButton* m_nextIssue = nullptr;

		QPushButton* m_load = nullptr;
		QPushButton* m_clear = nullptr;
		QPushButton* m_export = nullptr;
		QPushButton* m_filter = nullptr;
		QPushButton* m_search = nullptr;

		std::unique_ptr<LogRecordModel> m_model;
		std::unique_ptr<LogRecordProxyModel> m_proxyModel;
		LogTableView* m_table = nullptr;

		QLabel* m_logPathLabel = nullptr;

        std::vector<LogFileRecord> m_arrivedRecords;

        bool m_loadedFromFile = false;
        QString m_loadedFileName;

		inline static std::map<QString, LogFileDialog*> m_logDialogs;
	};
}
