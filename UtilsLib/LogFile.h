#pragma once

#include <string>
#include <queue>

#include <QTimer>
#include <QFile>
#include <QDateTime>
#include <QMutex>
#include <QDialog>
#include <QComboBox>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QDateTimeEdit>
#include <QStyledItemDelegate>
#include <QSharedMemory>
#include <QSortFilterProxyModel>

#include "../UtilsLib/SimpleThread.h"
#include "../UtilsLib/ILogFile.h"


namespace Log
{
	enum MessageType
	{
		All = -1,
		Error,
		Warning,
		Message,
		Alert,
		Text,
		Data,
	};

	struct LogFileRecord
	{
		quint64 sessionHash{};
		qint64 time{};			// msecs from 1970-01-01
		MessageType type = MessageType::Error;
		std::string text;

		QString toString(const QString& sessionHashString) const;
        bool loadFromString(const char* buf, const qint64 bufSize, const quint64 currentSessionHash);
	};

    using LogFileChunk = std::vector<LogFileRecord>;

	class LogFileWorker : public SimpleThreadWorker
	{
		Q_OBJECT

	public:
		LogFileWorker(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount, quint64 sessionHash);
		virtual ~LogFileWorker();

		// Writing functions
		//
		bool write(MessageType type, const QString& text);
		bool writeArray(const QStringList& textArray);

		// Loading funtcions
		//
        void load(bool currentSessionOnly);
        void loadFromFile(const QString& fileName);

        void cancelLoad();

        void getLoadedChunks(std::vector<std::shared_ptr<LogFileChunk>>* result);

		// Information functions
		//
		QString logName() const;

		QString getCurrentFileName() const;
		QString getLogPath() const;

		quint64 sessionHash() const;
		const QString& sessionHashString() const;

        bool loadInProgress() const;

	protected:
		virtual void onThreadStarted();
		virtual void onThreadFinished();

	private:
		QString getLogFileName(int index) const;

		bool readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount);
		bool writeLogFileInfo(QFile& file, const QDateTime& startTime, const QDateTime& endTime, int recordsCount);
		bool lockShared(bool lock, bool* alreadyLocked = nullptr);
		bool flush(QString* errorString);
		bool switchToNextLogFile(QString* errorString);
        bool readFileRecords(const QString& fileName, bool currentSessionOnly);

	private slots:
		void slot_onTimer();
		void slot_load(bool currentSessionOnly);
		void slot_loadFromFile(QString fileName);

	signals:
		void writeFailure(QString errorString);
        void loadStart(bool currentSessionOnly);
        void loadLogFromFile(QString fileName);
        void loadChunkComplete();
        void loadComplete();
        void recordArrived(Log::LogFileRecord record);

	private:
		QTimer* m_timer = nullptr;

		QMutex m_queueMutex;
		std::vector<LogFileRecord> m_queue;

		QString m_logName;
		QString m_path;
		int m_maxFileSize;
		int m_maxFilesCount;

        bool m_loadInProgress = false;
        bool m_cancelLoad = false;

		int m_currentFileNumber = 0;

		quint64 m_sessionHash = 0;
		QString m_sessionHashString;

		const int m_serviceStringLength = 80;

        QMutex m_loadLogMutex;						// Locks m_loadedChunks for reading data from log files
        std::queue<std::shared_ptr<LogFileChunk>> m_loadedChunks;   // A queue contains chunks loaded from log files

		std::unique_ptr<QSharedMemory> m_sharedMemory;
	};

    class LogRecordModel;

    class LogRecordProxyModel : public QSortFilterProxyModel
    {
    public:
        LogRecordProxyModel(LogRecordModel* sourceModel);

        int recordCount() const;

        int sourceRow(int row) const;

        int errorCount() const;
        int warningCount() const;

        int filterRecordTypeMask() const;
        void setFilterRecordTypeMask(int value);

        QString filterText() const;
        void setFilterText(const QString& value);

        qint64 filterTimeFrom() const;
        qint64 filterTimeTo() const;
        void setFilterTime(qint64 from, quint64 to);

    private:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        LogRecordModel* m_sourceModel = nullptr;

        mutable int m_errorCount = 0;
        mutable int m_warningCount = 0;

        int m_filterRecordTypeMask = MessageType::All;
        std::string m_filterText;

        qint64 m_filterTimeFrom = -1;
        qint64 m_filterTimeTo = -1;
    };

	class LogRecordModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
        LogRecordModel(bool showTypeColumn, const QStringList& headerTitles);
		~LogRecordModel();

	public:
		enum class Columns
		{
			Time = 0,
			Type,
			Text
		};

	public:
        void clear();

        void appendChunk(const std::shared_ptr<LogFileChunk>& chunk);
        void appendRecord(const LogFileRecord& record);

	public:
        const LogFileRecord& record(int row) const;
        QBrush color(const QModelIndex& index, bool selected) const;

    protected:
        virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;

        virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
		virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

		virtual QModelIndex parent(const QModelIndex& index) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		QStringList m_columnsNames;

        std::vector<std::shared_ptr<LogFileChunk>> m_chunks;

		bool m_showTypeColumn = true;

		int m_columnTime = -1;
		int m_columnType = -1;
		int m_columnText = -1;

	protected:
	};

	class SelectionControlDelegate : public QStyledItemDelegate
	{
		public:
            SelectionControlDelegate(QObject* parent, LogRecordModel* model, LogRecordProxyModel* proxyModel);
			void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;

	private:
			LogRecordModel* m_model = nullptr;
            LogRecordProxyModel* m_proxyModel = nullptr;
	};

	class DialogTimeFilter : public QDialog
	{
		Q_OBJECT
	public:
		DialogTimeFilter(qint64 filterTimeFrom, qint64 filterTimeTo, QWidget* parent);
		virtual ~DialogTimeFilter();

		qint64 filterTimeFrom() const;
		qint64 filterTimeTo() const;

	private:
		virtual void accept();

		qint64 m_filterTimeFrom = -1;
		qint64 m_filterTimeTo = -1;

		QDateTimeEdit* m_timeFromEdit = nullptr;
		QDateTimeEdit* m_timeToEdit = nullptr;
	};

	class LogTableView : public QTableView
	{
		Q_OBJECT
	protected:
		virtual void keyPressEvent(QKeyEvent *event) override;
		virtual void wheelEvent(QWheelEvent* event) override;

	signals:
		void copyKeyPressed();
		void turnOffAutoscroll();
	};

	class LogFileDialog : public QDialog
	{
		Q_OBJECT

	public:
        LogFileDialog(LogFileWorker* worker, QWidget* parent, bool useMessageType, bool headerVisible, const QStringList& headerTitles);
		virtual ~LogFileDialog();

	private:
		void enableControls(bool enable);
		void searchIssue(bool forward);

        int searchIssue(int startRow, bool forward) const;
        int searchRecord(int startRow, const std::string& text) const;

    private slots:
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
		LogFileWorker* m_worker = nullptr;

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

		LogRecordModel m_model;
        LogRecordProxyModel m_proxyModel;
		LogTableView* m_table = nullptr;

		QLabel* m_logPathLabel = nullptr;

        std::vector<LogFileRecord> m_arrivedRecords;

        bool m_loadedFromFile = false;
        QString m_loadedFileName;
	};

	class LogFile : public QObject, public ILogFile
	{
		Q_OBJECT

	public:
		LogFile(const QString& logName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 64, bool addAppInfoOnStart = true);
		virtual ~LogFile();

		virtual bool writeMessage(const QString& text, const QString& tag = {}) override;
		virtual bool writeAlert(const QString& text, const QString& tag = {}) override;
		virtual bool writeError(const QString& text, const QString& tag = {}) override;
		virtual bool writeWarning(const QString& text, const QString& tag = {}) override;
		virtual bool writeText(const QString& text, const QString& tag = {}) override;

		bool writeArray(const QStringList& textArray);
		bool write(MessageType type, const QString& text);

        void view(QWidget* parent, bool showType = true, bool headerVisible = false, const QStringList &headerTitles = {});

		[[nodiscard]] int alertAckCounter() const;
		[[nodiscard]] int errorAckCounter() const;
		[[nodiscard]] int warningAckCounter() const;

		QString getCurrentFileName() const;
		QString getLogPath() const;

	signals:
		void writeFailure(QString errorString);
		void alertArrived(QString text);

	private slots:
		void onFlushFailure(QString errorString);
		void onDialogFinished(int result);

	private:
		LogFileWorker* m_logFileWorker = nullptr;
		SimpleThread m_logThread;

		LogFileDialog* m_logDialog = nullptr;

		quint64 m_sessionHash;

		int m_alertAckCounter = 0;
		int m_errorAckCounter = 0;
		int m_warningAckCounter = 0;
	};
}

Q_DECLARE_METATYPE(Log::LogFileRecord)



