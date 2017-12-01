#ifndef LOGFILE_H
#define LOGFILE_H

#include "SimpleThread.h"

#include <QTimer>
#include <QDateTime>
#include <QMutex>

namespace Log
{
	enum class MessageType
	{
		All = 0,
		Error,
		Warning,
		Message,
		Text
	};

	struct LogFileRecord
	{
		QDateTime time;
		MessageType type;
		quint64 sessionHash;
		QString text;

		QString toString(const QString& sessionHashString);

		bool loadFromString(const QString& source, quint64 currentSessionHash);

	};

	class LogFileWorker : public SimpleThreadWorker
	{
		Q_OBJECT
	public:

		LogFileWorker(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount, quint64 sessionHash);
		virtual ~LogFileWorker();

		bool write(MessageType type, const QString& text);

		// Loading funtcions

		void read(bool currentSessionOnly);

		void getLoadedData(std::vector<LogFileRecord> *result);

	protected:
		virtual void onThreadStarted();
		virtual void onThreadFinished();

	private:

		QString getLogFileName(int index) const;

		bool readLogFileInfo(const QString& fileName, QDateTime& startTime, QDateTime& endTime, int& recordsCount);

		bool writeLogFileInfo(QFile& file, const QDateTime& startTime, const QDateTime& endTime, int recordsCount);

		bool flush();

		bool switchToNextLogFile();

		bool readFileRecords(const QString& fileName, bool currentSessionOnly, std::vector<LogFileRecord>* result);

	private slots:

		void slot_onTimer();

		void slot_load(bool currentSessionOnly);

	signals:
		void flushFailure();

		void readStart(bool currentSessionOnly);

		void readComplete();

		void recordArrived(LogFileRecord record);

	private:

		QTimer *m_timer = nullptr;

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

		QMutex m_readMutex;
		std::vector<LogFileRecord> m_readResult;
	};

	class LogRecordModel : public QAbstractItemModel
	{
		Q_OBJECT

	public:
		LogRecordModel();
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

		void setFilter(MessageType messageType, const QString& text);

	private:
		void fillRecords();

		bool processRecordFilter(const LogFileRecord& record) const;

	public:
		double columnWidthPercent(int index);

		virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
		virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;


	protected:
		virtual QModelIndex parent(const QModelIndex& index) const override;
		virtual	QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
		virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	private:
		QStringList m_columnsNames;
		std::vector<double> m_columnsWidthPercent;

		std::vector<int> m_filteredRecordsIndex;
		std::vector<LogFileRecord> m_records;

		MessageType m_filterMessageType = MessageType::All;
		QString m_filterText;

	protected:
	};

	class LogFileDialog : public QDialog
	{
		Q_OBJECT

	public:

		LogFileDialog(LogFileWorker* worker, QWidget* parent);
		virtual ~LogFileDialog();

	private:
		virtual void resizeEvent(QResizeEvent *event);

		void enableControls(bool enable);

	private:

		LogFileWorker* m_worker = nullptr;

		QComboBox* m_typeCombo = nullptr;

		QLineEdit* m_filterLineEdit = nullptr;

		QPushButton* m_allSessions = nullptr;

		QPushButton* m_autoScroll = nullptr;

		LogRecordModel m_model;

		QTableView* m_table = nullptr;

		QLabel* m_counterLabel = nullptr;

	private slots:
		void onTypeComboIndexChanged(int index);
		void onFilter();

		void onAllSessionsClicked();

		void onReadComplete();
		void onRecordArrived(LogFileRecord record);

	};

	class LogFile : public QObject
	{
		Q_OBJECT
	public:

		LogFile(const QString& logName, const QString& path = QString(), int maxFileSize = 1048576, int maxFilesCount = 3);
		virtual ~LogFile();

		bool writeMessage(const QString& text);
		bool writeError(const QString& text);
		bool writeWarning(const QString& text);
		bool writeText(const QString& text);

		bool write(MessageType type, const QString& text);

		void view(QWidget* parent);
	signals:
		void writeFailure();

	private slots:
		void onFlushFailure();

		void onDialogFinished(int result);

	private:

		LogFileWorker* m_logFileWorker = nullptr;
		SimpleThread m_logThread;

		LogFileDialog* m_logDialog = nullptr;

		quint64 m_sessionHash;
	};
}

#endif // LOGFILE_H

