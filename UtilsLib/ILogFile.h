#pragma once


class ILogFile
{
public:
	virtual bool writeAlert(const QString& text) = 0;
	virtual bool writeError(const QString& text) = 0;
	virtual bool writeWarning(const QString& text) = 0;
	virtual bool writeMessage(const QString& text) = 0;
	virtual bool writeText(const QString& text) = 0;
};


class ILogFileStub : public ILogFile
{
public:
	virtual bool writeAlert(const QString&) override { return true; };
	virtual bool writeError(const QString&)  override { return true; };
	virtual bool writeWarning(const QString&)  override { return true; };
	virtual bool writeMessage(const QString&)  override { return true; };
	virtual bool writeText(const QString&)  override { return true; };
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
		return m_context.isEmpty() == true ?
					m_logFile->writeAlert(text) :
					m_logFile->writeAlert(QString("%1: %2").arg(m_context, text));
	}

	bool writeError(const QString& text)
	{
		QString message = m_context.isEmpty() == true ?
							  text :
							  QString("%1: %2").arg(m_context, text);

		qDebug() << message;

		return m_logFile->writeError(message);
	}

	bool writeWarning(const QString& text)
	{
		return m_context.isEmpty() == true ?
					m_logFile->writeWarning(text) :
					m_logFile->writeWarning(QString("%1: %2").arg(m_context, text));
	}

	bool writeMessage(const QString& text)
	{
		return m_context.isEmpty() == true ?
					m_logFile->writeMessage(text) :
					m_logFile->writeMessage(QString("%1: %2").arg(m_context, text));
	}

	bool writeText(const QString& text)
	{
		return m_context.isEmpty() == true ?
					m_logFile->writeText(text) :
					m_logFile->writeText(QString("%1: %2").arg(m_context, text));
	}

	[[nodiscard]] ILogFile* logFile()
	{
		return m_logFile;
	}

	[[nodiscard]] const ILogFile* logFile() const
	{
		return m_logFile;
	}

private:
	ILogFile* m_logFile = nullptr;
	QString m_context;
};
