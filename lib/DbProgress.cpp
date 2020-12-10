#include "../lib/DbProgress.h"
#include "../lib/DbProgressDialog.h"

#include <QThread>
#include <QDebug>
#include <QCoreApplication>
#include <assert.h>
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

#ifndef Q_CONSOLE_APP
			QMessageBox mb(parentWidget);
			mb.setText(errorMessage());
			mb.exec();
#endif
		}

		return false;
	}

	if (completeMessage().isEmpty() == false &&
		isGuiThread == true)
	{
		qDebug() << completeMessage();

#ifndef Q_CONSOLE_APP
		QMessageBox mb(parentWidget);
		mb.setText(completeMessage());
		mb.exec();
#endif
	}

	return true;
}

DbProgress::~DbProgress()
{
}

bool DbProgress::completed() const
{
	return m_completed.load(std::memory_order::memory_order_relaxed);
}

void DbProgress::setCompleted(bool value)
{
	m_completed = value;
}

bool DbProgress::wasCanceled() const
{
	return m_cancel.load(std::memory_order::memory_order_relaxed);
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
	return m_value.load(std::memory_order::memory_order_relaxed);
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
	return m_progressEnabled.load(std::memory_order::memory_order_relaxed);
}

