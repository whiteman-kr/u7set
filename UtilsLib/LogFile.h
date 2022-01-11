#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "../UtilsLib/ILogFile.h"

#include <QTimer>
#include <QDateTime>
#include <QMutex>
#include <QDialog>
#include <QComboBox>
#include <QTableView>
#include <QLabel>
#include <QPushButton>
#include <QDateTimeEdit>
#include <string>

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
		quint64 sessionHash;
		qint64 time;			// msecs from 1970-01-01
		MessageType type;
		std::string text;

		QString toString(const QString& sessionHashString) const;
		bool loadFromString(const char* buf, const qint64 bufSize, const quint64 currentSessionHash, int* errorCount, int* warningCount);
	};

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
		void read(bool currentSessionOnly);

		void loadFromFile(const QString& fileName);

		void getLoadedData(std::vector<LogFileRecord>* result);

		// Information functions
		//
		QString logName() const;

		QString getCurrentFileName() const;
		QString getLogPath() const;

		bool loadedFromFile() const;
		QString getLoadedFileName() const;

		quint64 sessionHash() const;
		const QString& sessionHashString() const;

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
		bool readFileRecords(const QString& fileName, bool currentSessionOnly, std::vector<LogFileRecord>* result, int* errorCount, int* warningCount);

	private slots:
		void slot_onTimer();
		void slot_load(bool currentSessionOnly);
		void slot_loadFromFile(QString fileName);

	signals:
		void writeFailure(QString errorString);
		void readStart(bool currentSessionOnly);
		void readFromFile(QString fileName);
		void readInProgress(QString fileName, int recordsRead);
		void readComplete();
		void recordArrived(LogFileRecord record);

	private:
		QTimer* m_timer = nullptr;

		QMutex m_queueMutex;
		std::vector<LogFileRecord> m_queue;

		QString m_logName;
		QString m_path;
		int m_maxFileSize;
		int m_maxFilesCount;

		int m_currentFileNumber = 0;

		quint64 m_sessionHash = 0;
		QString m_sessionHashString;

		const int m_serviceStringLength = 80;

		QMutex m_readLogMutex;						// Locks m_readResult for reading data from log files
		std::vector<LogFileRecord> m_readResult;
		int m_errorCount = 0;
		int m_warningCount = 0;

		bool m_loadedFromFile = false;
		QString m_loadedFileName;

		std::unique_ptr<QSharedMemory> m_sharedMemory;
	};

	class LogRecordModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		LogRecordModel(bool showTypeColumn, std::vector<std::pair<QString, double>> headerTitles);
		~LogRecordModel();

	public:

		enum class Columns
		{
			Time = 0,
			Type,
			Text
		};


	public:
		void setRecords(std::vector<LogFileRecord>* records);
		void addRecord(const LogFileRecord& record);

		int recordsCount() const;

		int filterRecordTypeMask() const;
		void setFilterRecordTypeMask(int value);

		QString filterText() const;
		void setFilterText(const QString& value);

		qint64 filterTimeFrom() const;
		void setFilterTimeFrom(qint64 value);

		qint64 filterTimeTo() const;
		void setFilterTimeTo(qint64 value);

		void fillRecords();

		void getFilteredRecords(std::vector<LogFileRecord>* result) const;

	private:
		bool processRecordFilter(const LogFileRecord& record) const;

	public:
		double columnWidthPercent(int index) const;

		int errorCount() const;
		int warningCount() const;

		int searchIssue(int startRow, bool forward) const;
		int searchRecord(int startRow, const std::string& text) const;

		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;

		virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

	protected:
		virtual QModelIndex parent(const QModelIndex& index) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		QStringList m_columnsNames;
		std::vector<double> m_columnsWidthPercent;

		std::vector<int> m_filteredRecordsIndex;
		std::vector<LogFileRecord> m_records;

		int m_errorCount = 0;
		int m_warningCount = 0;

		int m_filterRecordTypeMask = MessageType::All;
		std::string m_filterText;

		qint64 m_filterTimeFrom = -1;
		qint64 m_filterTimeTo = -1;

		bool m_showTypeColumn = true;

		int m_columnTime = -1;
		int m_columnType = -1;
		int m_columnText = -1;

	protected:
	};

	class LogFileProgressDialog : public QDialog
	{
		Q_OBJECT
	public:
		LogFileProgressDialog(QWidget* parent);
		virtual ~LogFileProgressDialog();

	public slots:
		void readInProgress(QString fileName, int recordsRead);
		void readComplete();

	private:
		virtual void accept(){}
		virtual void reject(){}

	private:
		QLabel* m_label = nullptr;
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
		LogFileDialog(LogFileWorker* worker, QWidget* parent, bool useMessageType, bool headerVisible, const std::vector<std::pair<QString, double>>& headerTitles);
		virtual ~LogFileDialog();

	private:

		virtual void showEvent(QShowEvent* event);
		virtual void resizeEvent(QResizeEvent *event);

		void enableControls(bool enable);
		void adjustColumnsWidth();

		void searchIssue(bool forward);

	private slots:
		void onTypeComboIndexChanged(int index);
		void onTimeFilter();
		void onFilter();
		void onSearch();

		void onAllSessionsClicked();

		void onReadComplete();
		void onRecordArrived(LogFileRecord record);

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
		LogTableView* m_table = nullptr;

		QLabel* m_logPathLabel = nullptr;

		LogFileProgressDialog m_progressDialog;

		bool m_firstShow = true;
	};

	class LogFile : public QObject, public ILogFile
	{
		Q_OBJECT

	public:
		LogFile(const QString& logName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 64, bool addAppInfoOnStart = true);
		virtual ~LogFile();

		virtual bool writeMessage(const QString& text) override;
		virtual bool writeAlert(const QString& text) override;
		virtual bool writeError(const QString& text) override;
		virtual bool writeWarning(const QString& text) override;
		virtual bool writeText(const QString& text) override;

		bool writeArray(const QStringList& textArray);
		bool write(MessageType type, const QString& text);

		void view(QWidget* parent, bool showType = true, bool headerVisible = false, std::vector<std::pair<QString, double>> headerTitles = {});

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



