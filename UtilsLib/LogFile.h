#pragma once

#include "../UtilsLib/SimpleThread.h"
#include "../UtilsLib/ILogFile.h"

namespace Log
{
	inline static const std::array<QString, 6> messageTypeTextShort{"ERR", "WRN", "MSG", "ALERT", "TXT", "DATA"};
	inline static const std::array<char, 6> messageTypeFirstLetter{'E', 'W', 'M', 'A', 'T', 'D'};
	inline static const QString messageTimeFormat{"dd.MM.yyyy hh:mm:ss.zzz"};

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
		qint64 time{}; // msecs from 1970-01-01
		MessageType type = MessageType::Error;
		std::string text;

		static QString toString(const LogFileRecord& r, const QString& sessionHashString);
		static bool fromString(const char* buf, const qint64 bufSize, const quint64 currentSessionHash, LogFileRecord& r);

	private:
		static void replaceStringInPlace(std::string& subject, const std::string& search, const std::string& replace);
	};

	using LogFileChunk = std::vector<LogFileRecord>;

	class LogFileWorker;

	class LogFile : public QObject,
					public ILogFile
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

		void resetAckCounters();
		[[nodiscard]] int alertAckCounter() const;
		[[nodiscard]] int errorAckCounter() const;
		[[nodiscard]] int warningAckCounter() const;

		QString logName() const;
		QString getCurrentFileName() const;
		QString getLogPath() const;

		void load(bool currentSessionOnly);
		void loadFromFile(const QString& fileName);
		void cancelLoad();

		bool loadInProgress() const;
		void getLoadedChunks(std::vector<std::shared_ptr<LogFileChunk>>* result);
		
		quint64 sessionHash() const;
		QString sessionHashString() const;
		
		bool noDiskLog() const;
		void setNoDiskLog(bool value);

		std::vector<LogFileRecord> queue() const;
		
	signals:
		void loadChunkComplete();
		void loadComplete();
		void recordArrived(LogFileRecord record);

		void writeFailure(QString errorString);
		void alertArrived(QString text);

	private slots:
		void onFlushFailure(QString errorString);

	private:
		LogFileWorker* m_worker = nullptr;
		SimpleThread m_logThread;

		int m_alertAckCounter = 0;
		int m_errorAckCounter = 0;
		int m_warningAckCounter = 0;
	};
}

Q_DECLARE_METATYPE(Log::LogFileRecord)
