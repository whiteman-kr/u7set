#pragma once
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(u7sim)

#define OUTPUT_WRITE_ERROR(text) \
    if (outputScope().isEmpty() == true)\
    { \
        qCCritical(u7sim).noquote() << text; \
    } \
    else \
    { \
        qCCritical(u7sim).noquote() << outputScope() << text; \
    }

namespace Sim
{
	class Output
	{
	public:
        explicit Output(QString outputScope = "");
		virtual ~Output();

	protected:
		void writeDebug(const QString& text);
		void writeDebug(const QString& text) const;

		void writeMessage(const QString& text);
		void writeMessage(const QString& text) const;

		void writeWaning(const QString& text);
		void writeWaning(const QString& text) const;

		void writeError(const QString& text);
		void writeError(const QString& text) const;

		const QString& outputScope() const;
		void setOutputScope(QString value);

	private:
        QString m_scope;
	};

}

