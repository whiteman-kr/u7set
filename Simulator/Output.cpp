#include "Output.h"
#include <cassert>
#include <QTime>

namespace Sim
{

	Output::Output(QTextStream* outputStream, QString scope, std::shared_ptr<QMutex> mutex) :
		m_scope(scope),
		m_textStream(outputStream),
		m_mutex(mutex),
#ifdef Q_OS_WIN
		m_nullDevice(fopen("NUL:", "w")),
#endif
#ifdef Q_OS_UNIX
		m_nullDevice(fopen("/dev/null", "w")),
#endif
		m_nullTextStream(m_nullDevice)
	{
		if (m_textStream != nullptr)
		{
			m_currentStream = m_textStream;
		}
		else
		{
			m_currentStream = &m_nullTextStream;
		}

		if (m_scope.isEmpty() == false)
		{
			writeMessage("Instance created.");
		}

		assert(m_currentStream);
		assert(m_mutex);

		return;
	}

	Output::Output(QTextStream* outputStream, QString scope) :
		Output(outputStream, scope, std::make_shared<QMutex>())
	{
		assert(m_currentStream);
		assert(m_mutex);
		return;
	}

	Output::Output(const Output& src) :
		Output(src.m_textStream, src.m_scope, src.m_mutex)
	{
		assert(m_currentStream);
		assert(m_mutex);
	}

	Output::Output(const Output& src, QString scope) :
		Output(src.m_textStream, scope, src.m_mutex)
	{
		assert(m_currentStream);
		assert(m_mutex);
	}

	Output::~Output()
	{
		if (m_scope.isEmpty() == false)
		{
			writeMessage("Instance destroyed.");
		}

		if (m_nullDevice != nullptr)
		{
			fclose(m_nullDevice);
		}

		return;
	}

	QTextStream& Output::output()
	{
		return *m_currentStream;
	}

	QTextStream& Output::output() const
	{
		return *m_currentStream;
	}

	QString Output::curentTime() const
	{
		return QTime::currentTime().toString(QLatin1String("hh:mm:ss.zzz"));
	}

	void Output::writeMessage(const QString& text)
	{
		QString str;
		if (m_scope.isEmpty() == false)
		{
			str = QString("%1 %2 msg: %3")
					.arg(curentTime())
					.arg(m_scope)
					.arg(text);
		}
		else
		{
			str = QString("%1 msg: %2")
					.arg(curentTime())
					.arg(text);
		}

		// No need to use mutex as outpud done via single operation
		//
		m_mutex->lock();
		output() << str << endl;
		m_mutex->unlock();
		return;
	}

	void Output::writeWaning(const QString& text)
	{
		QString str;
		if (m_scope.isEmpty() == false)
		{
			str = QString("%1 %2 wrn: %3")
					.arg(curentTime())
					.arg(m_scope)
					.arg(text);
		}
		else
		{
			str = QString("%1 wrn: %2")
					.arg(curentTime())
					.arg(text);
		}

		// No need to use mutex as outpud done via single operation
		//
		m_mutex->lock();
		output() << str << endl;
		m_mutex->unlock();
		return;
	}

	void Output::writeError(const QString& text)
	{
		QString str;
		if (m_scope.isEmpty() == false)
		{
			str = QString("%1 %2 err: %3")
					.arg(curentTime())
					.arg(m_scope)
					.arg(text);
		}
		else
		{
			str = QString("%1 err: %2")
					.arg(curentTime())
					.arg(text);
		}

		// No need to use mutex as outpud done via single operation
		//
		m_mutex->lock();
		output() << str << endl;
		m_mutex->unlock();
		return;
	}

	const QString& Output::outputScope() const
	{
		return m_scope;
	}

	void Output::setOutputScope(QString value)
	{
		m_scope = value;
	}

}
