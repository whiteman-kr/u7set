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
	m_value(0),
	m_progressEnabled(false)
{
}

bool DbProgress::init()
{
	QMutexLocker l(&m_mutex);

	m_completed = false;
	m_cancel = false;
	m_value = 0;

	m_errorMessage.clear();
	m_completeMessage.clear();

	return true;
}

bool DbProgress::run(QWidget* parentWidget, const QString& description)
{
	const bool isGuiThread = QThread::currentThread() == QCoreApplication::instance()->thread();

	if (m_progressEnabled == true)
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
	QMutexLocker l(&m_mutex);
	return m_completed;
}

void DbProgress::setCompleted(bool value)
{
	QMutexLocker l(&m_mutex);
	m_completed = value;
}

bool DbProgress::wasCanceled() const
{
	QMutexLocker l(&m_mutex);
	return m_cancel;
}

void DbProgress::setCancel(bool value)
{
	QMutexLocker l(&m_mutex);
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
	QMutexLocker l(&m_mutex);
	return m_value;
}

void DbProgress::setValue(int value)
{
	QMutexLocker l(&m_mutex);
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
	QMutexLocker l(&m_mutex);
	m_progressEnabled = true;
}

void DbProgress::disableProgress()
{
	QMutexLocker l(&m_mutex);
	m_progressEnabled = false;
}

bool DbProgress::isProgressEnabled()
{
	QMutexLocker l(&m_mutex);
	return m_progressEnabled;
}

