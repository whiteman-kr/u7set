#pragma once
#include <QLoggingCategory>
#include "../lib/ILogFile.h"


Q_DECLARE_LOGGING_CATEGORY(u7sim)

//#define OUTPUT_WRITE_ERROR(text) \
//	if (outputScope().isEmpty() == true)\
//	{ \
//		qCCritical(u7sim).noquote() << text; \
//	} \
//	else \
//	{ \
//		qCCritical(u7sim).noquote() << outputScope() << text; \
//	}

namespace Sim
{

	class ScopedLog
	{
	public:
		ScopedLog(const ScopedLog& src) = default;
		explicit ScopedLog(ILogFile* log, QString outputScope);
		explicit ScopedLog(ScopedLog src, QString outputScope);

		~ScopedLog();

	public:
		bool writeAlert(const QString& text);
		bool writeError(const QString& text);
		bool writeWarning(const QString& text);
		bool writeMessage(const QString& text);
		bool writeText(const QString& text);
		bool writeDebug(const QString& text);

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




