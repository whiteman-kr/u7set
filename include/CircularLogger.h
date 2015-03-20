#ifndef CIRCULARLOGGER_H
#define CIRCULARLOGGER_H

#include <QObject>
#include <QQueue>
#include <QMap>

class QFile;
class QTextStream;

const int	MT_USER_ACTION = 0,
			MT_NET = 1,
			MT_APPLICATION = 2;

const int	MC_ERROR = 0,
			MC_WARNING = 1,
			MC_MESSAGE = 2,
			MC_CONFIG = 3;


class CircularLoggerWorker : public QObject
{
	Q_OBJECT
public:
	CircularLoggerWorker(QString logName, int fileCount, int fileSizeInMB, QString placementPath = "");
	~CircularLoggerWorker();

	void close();

public slots:
	void writeRecord(const QString record);
	void flushStream();

private:
	QTextStream* m_stream = nullptr;
	QFile* m_file = nullptr;

	void detectFiles();
	void removeOldFiles();
	void checkFileSize();
	int getFileID(int index);
	QString fileName(int index);
	void openFile(int index);
	void writeFirstRecord();
	void writeLastRecord();

	QString m_logName;
	QString m_path;
	QString m_logFileName;
	int m_beginFileNumber = -1;
	int m_endFileNumber = -1;
	int m_fileCount;
	int m_fileSizeLimit;	// in megabytes
	qint64 m_beginFileID = -1;
	qint64 m_endFileID = -1;
};

class CircularLogger : public QObject
{
	Q_OBJECT
public:
	CircularLogger(QObject *parent = 0);
	~CircularLogger();

	void initLog(QString logName, int fileCount, int fileSizeInMB, QString placementPath = "");
	void initLog(int fileCount, int fileSizeInMB, QString placementPath = "");

signals:
	void writeRecord(const QString record);

public slots:

	QString appErr(const QString& function, const QString& message)
	{
		return write(MT_APPLICATION, MC_ERROR, function, message);
	}

	QString appWrn(const QString& function, const QString& message)
	{
		return write(MT_APPLICATION, MC_WARNING, function, message);
	}

	QString appMsg(const QString& function, const QString& message)
	{
		return write(MT_APPLICATION, MC_MESSAGE, function, message);
	}

	/*QString netErr(char* function, QHostAddress& IP, RequestHeader& header, char* message, ...);
	QString netWrn(char* function, QHostAddress& IP, RequestHeader& header, char* message, ...);
	QString netMsg(char* function, QHostAddress& IP, RequestHeader& header, char* message, ...);*/

	QString userErr(const QString& function, const QString& message)
	{
		return write(MT_USER_ACTION, MC_ERROR, function, message);
	}

	QString userWrn(const QString& function, const QString& message)
	{
		return write(MT_USER_ACTION, MC_WARNING, function, message);
	}

	QString userMsg(const QString& function, const QString& message)
	{
		return write(MT_USER_ACTION, MC_MESSAGE, function, message);
	}

	QString write(int type, int category, QString function, const QString& message/*ip, header*/)
	{
		QString record = composeRecord(type, category, function, message);
		emit writeRecord(record);
		return record;
	}

private:

	CircularLoggerWorker* m_circularLoggerWorker = nullptr;
	QThread* m_thread = nullptr;

	QString composeRecord(int type, int category, const QString& function, const QString& message/*ip, header*/);
};

#define MESSAGE_POSITION QString("%1\" \"POS=%2:%3").arg(Q_FUNC_INFO).arg(__FILE__).arg(__LINE__)

#define APP_ERR(log,str) (log).appErr(MESSAGE_POSITION,str);
#define APP_WRN(log,str) (log).appWrn(MESSAGE_POSITION,str);
#define APP_MSG(log,str) (log).appMsg(MESSAGE_POSITION,str);

/*#define NET_ERR(log,ip,header,str) (log).netErr(Q_FUNC_INFO,ip,header,str,__VA_ARGS__);
 *#define NET_WRN(log,ip,header,str) (log).netWrn(Q_FUNC_INFO,ip,header,str,__VA_ARGS__);
 *#define NET_MSG(log,ip,header,str) (log).netMsg(Q_FUNC_INFO,ip,header,str,__VA_ARGS__);
 */

#define USR_ERR(log,str) (log).userErr(MESSAGE_POSITION,str);
#define USR_WRN(log,str) (log).userWrn(MESSAGE_POSITION,str);
#define USR_MSG(log,str) (log).userMsg(MESSAGE_POSITION,str);

#endif // CIRCULARLOGGER_H
