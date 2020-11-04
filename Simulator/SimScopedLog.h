#pragma once
#include <QLoggingCategory>
#include "../lib/ILogFile.h"


Q_DECLARE_LOGGING_CATEGORY(u7sim)


namespace Sim
{

	class ScopedLog : public QObject
	{
		Q_OBJECT

	public:
		ScopedLog(const ScopedLog& src);
		explicit ScopedLog(ILogFile* log, QString outputScope);
		explicit ScopedLog(ScopedLog src, QString outputScope);

		~ScopedLog();

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

	private:
		mutable ILogFile* m_log = nullptr;
		QString m_scope;
	};

}




