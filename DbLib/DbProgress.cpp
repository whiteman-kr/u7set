#ifndef DB_LIB_DOMAIN
#error Don't include this file in the project! Link DbLib instead.
#endif

#include "DbProgress.h"
#include "DbProgressDialog.h"

#include <QThread>
#include <QGuiApplication>
#include <QDebug>
#include <QCoreApplication>
#include <QMessageBox>
#include <QProgressDialog>

//
//
//	DbProgress
//
//

DbProgress::DbProgress() :
	m_completed(false),
	m_cancel(false),
	m_progressEnabled(false),
	m_value(0)
{
}

bool DbProgress::init()
{
	m_completed = false;
	m_cancel = false;
	m_value = 0;

	{
		QMutexLocker l(&m_mutex);

		m_currentOperation.clear();
		m_errorMessage.clear();
		m_completeMessage.clear();
	}

	return true;
}

bool DbProgress::run(QWidget* parentWidget, const QString& description)
{
	bool isGuiThread = false;
	if (QCoreApplication::instance() != nullptr)
	{
		isGuiThread = QThread::currentThread() == QCoreApplication::instance()->thread();
	}

	const bool isConsoleApp = (dynamic_cast<const QGuiApplication*>(QCoreApplication::instance()) == nullptr);

	if (isProgressEnabled() == true)
	{
		assert(isGuiThread == true);
		ProgressDialog::showProgressDialog(parentWidget, description, this);
	}
	else
	{
		while (completed() == false)
		{
			QThread::yieldCurrentThread();
		}
	}

	if (hasError() == true)
	{
		if (isGuiThread == true)
		{
			qDebug() << errorMessage();

			if (isConsoleApp == false)
			{
				QMessageBox mb(parentWidget);
				mb.setText(errorMessage());
				mb.exec();
			}
		}

		return false;
	}

	if (completeMessage().isEmpty() == false &&
		isGuiThread == true)
	{
		qDebug() << completeMessage();

		if (isConsoleApp == false)
		{
			QMessageBox mb(parentWidget);
			mb.setText(completeMessage());
			mb.exec();
		}
	}

	return true;
}

bool DbProgress::completed() const
{
	return m_completed.load(std::memory_order::relaxed);
}

void DbProgress::setCompleted(bool value)
{
	m_completed = value;
}

bool DbProgress::wasCanceled() const
{
	return m_cancel.load(std::memory_order::relaxed);
}

void DbProgress::setCancel(bool value)
{
	m_cancel = value;
}

QString DbProgress::currentOperation() const
{
	QMutexLocker l(&m_mutex);
	return m_currentOperation;
}

void DbProgress::setCurrentOperation(const QString& value)
{
	QMutexLocker l(&m_mutex);
	m_currentOperation = value;
}

int DbProgress::value() const
{
	return m_value.load(std::memory_order::relaxed);
}

void DbProgress::setValue(int value)
{
	m_value = value;
}

QString DbProgress::errorMessage() const
{
	QMutexLocker l(&m_mutex);
	return m_errorMessage;
}

void DbProgress::setErrorMessage(const QString& value)
{
	QMutexLocker l(&m_mutex);

	if (m_errorMessage.isEmpty() == false)
	{
		m_errorMessage += "\n";
	}

	m_errorMessage += value;
}

bool DbProgress::hasError() const
{
	QMutexLocker l(&m_mutex);
	return !m_errorMessage.isEmpty();
}

QString DbProgress::completeMessage() const
{
	QMutexLocker l(&m_mutex);
	return m_completeMessage;
}

void DbProgress::setCompleteMessage(const QString& value)
{
	QMutexLocker l(&m_mutex);
	m_completeMessage = value;
}

void DbProgress::enableProgress()
{
	m_progressEnabled = true;
}

void DbProgress::disableProgress()
{
	m_progressEnabled = false;
}

bool DbProgress::isProgressEnabled() const
{
	return m_progressEnabled.load(std::memory_order::relaxed);
}

