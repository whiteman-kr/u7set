#pragma once
#include <QDebug>

class ILogFile
{
public:
	virtual ~ILogFile() = default;
	virtual bool writeAlert(const QString& text,
							const QString& tag = {}) = 0; // tags are used by ReportLib, so they can be ignored in many cases.
	virtual bool writeError(const QString& text, const QString& tag = {}) = 0;
	virtual bool writeWarning(const QString& text, const QString& tag = {}) = 0;
	virtual bool writeMessage(const QString& text, const QString& tag = {}) = 0;
	virtual bool writeText(const QString& text, const QString& tag = {}) = 0;
};

class ILogFileStub : public ILogFile
{
public:
	virtual bool writeAlert(const QString&, const QString&) override { return true; }
	virtual bool writeError(const QString&, const QString&) override { return true; }
	virtual bool writeWarning(const QString&, const QString&) override { return true; }
	virtual bool writeMessage(const QString&, const QString&) override { return true; }
	virtual bool writeText(const QString&, const QString&) override { return true; }
};

// Logs everything to qDebug()
//
class ILogFileConsole : public ILogFile
{
public:
	bool writeAlert(const QString& text, const QString& tag = {}) override { return writeError("Alert", text, tag); }
	bool writeError(const QString& text, const QString& tag = {}) override { return writeError("Error", text, tag); }
	bool writeWarning(const QString& text, const QString& tag = {}) override { return writeError("Warning", text, tag); }
	bool writeMessage(const QString& text, const QString& tag = {}) override { return writeError("Message", text, tag); }
	bool writeText(const QString& text, const QString& tag = {}) override { return writeError("Text", text, tag); }

private:
	bool writeError(const QString& cat, const QString& text, const QString& tag = {})
	{
		if (tag.isEmpty() == true)
		{
			qDebug().noquote() << cat << ": " << text;
		}
		else
		{
			qDebug().noquote() << cat << ": " << text << " [" << tag << "]";
		}
		return true;
	}
};


class HasLogFile
{
public:
	HasLogFile() = delete;
	HasLogFile(ILogFile* logFile, QString context) :
		m_logFile(logFile),
		m_context(std::move(context))
	{
		Q_ASSERT(m_logFile);

		if (m_context.isEmpty() == false)
		{
			writeMessage("Started");
		}

		return;
	}

	virtual ~HasLogFile()
	{
		if (m_context.isEmpty() == false)
		{
			writeMessage("Finished");
		}

		return;
	}

public:
	bool writeAlert(const QString& text)
	{
		return m_context.isEmpty() == true ? m_logFile->writeAlert(text) : m_logFile->writeAlert(QString("%1: %2").arg(m_context, text));
	}

	bool writeError(const QString& text)
	{
		QString message = m_context.isEmpty() == true ? text : QString("%1: %2").arg(m_context, text);

		qDebug() << message;

		return m_logFile->writeError(message);
	}

	bool writeWarning(const QString& text)
	{
		return m_context.isEmpty() == true ? m_logFile->writeWarning(text) :
											 m_logFile->writeWarning(QString("%1: %2").arg(m_context, text));
	}

	bool writeMessage(const QString& text)
	{
		return m_context.isEmpty() == true ? m_logFile->writeMessage(text) :
											 m_logFile->writeMessage(QString("%1: %2").arg(m_context, text));
	}

	bool writeText(const QString& text)
	{
		return m_context.isEmpty() == true ? m_logFile->writeText(text) : m_logFile->writeText(QString("%1: %2").arg(m_context, text));
	}

	[[nodiscard]] ILogFile* logFile() { return m_logFile; }

	[[nodiscard]] ILogFile* logFile() const { return m_logFile; }

	operator ILogFile*() { return m_logFile; }

private:
	mutable ILogFile* m_logFile = nullptr;
	QString m_context;
};
