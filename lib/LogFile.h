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
		Error,
		Warning,
		Message,
		Text
	};

	struct LogFileRecord
	{
		static const int typeCont;

		static const char* typeTextShort[];

		QDateTime time;
		MessageType type;
		QString text;

		QString toString();

	};

	class LogFileDialog;

	class LogFileWorker : public SimpleThreadWorker
	{
		Q_OBJECT
	public:

		LogFileWorker(const QString& fileName, const QString& path, int maxFileSize, int maxFilesCount);
		virtual ~LogFileWorker();

		bool write(MessageType type, const QString& text);

		// Loading funtcions

		void read();

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

		bool readFileRecords(const QString& fileName, std::vector<LogFileRecord>* result);

	private slots:

		void slot_onTimer();

		void slot_load();

	signals:
		void flushFailure();

		void readStart();

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

		const int m_serviceStringLength = 80;

		QMutex m_readMutex;
		std::vector<LogFileRecord> m_readResult;
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

		double columnWidthPercent(int index);

	public:

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

		std::vector<LogFileRecord> m_records;

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

	private:

		LogFileWorker* m_worker = nullptr;

		QComboBox* m_typeCombo = nullptr;

		QLineEdit* m_filter = nullptr;

		QPushButton* m_autoScroll = nullptr;

		LogRecordModel m_model;

		QTableView* m_table = nullptr;

		QLabel* m_counterLabel = nullptr;

	private slots:
		void onChoosePeriod();
		void onFilter();
		void onAutoScroll();

		void onReadComplete();
		void onRecordArrived(LogFileRecord record);

	};

}

#endif // LOGFILE_H

