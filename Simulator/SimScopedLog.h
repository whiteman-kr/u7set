#pragma once
#include "../UtilsLib/ILogFile.h"


namespace Sim
{
	class ScopedLog : public QObject
	{
		Q_OBJECT

	public:
		ScopedLog(const ScopedLog& src);
		explicit ScopedLog(ILogFile* log, bool allowDebugMessages, QString outputScope);
		explicit ScopedLog(const ScopedLog& src, QString outputScope);

		virtual ~ScopedLog();

	public slots:
		bool writeAlert(QString text);
		bool writeError(QString text);
		bool writeWarning(QString text);
		bool writeMessage(QString text);
		bool writeText(QString text);
		bool writeDebug(QString text);

		// --
		//
	public:
		const QString& outputScope() const;
		void setOutputScope(QString value);

		bool debugMessagesEnabled() const;
		void setDebugMessagesEnabled(bool value);

		ILogFile* logInterface();

	private:
		mutable ILogFile* m_log = nullptr;
		QString m_scope;
		std::atomic<bool> m_debugMessagesEnabled = true;
	};
}




