#include "TuningLog.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>
#include <QAbstractItemModel>
#include <QComboBox>
#include <QUuid>

namespace TuningLog
{
	//
	// TuningLog
	//

	TuningLog::TuningLog(ClientLib::TuningUserManager& userManager, const QString& logName, const QString& path, int maxFileSize, int maxFilesCount)
		: Log::LogFile(logName, path, maxFileSize, maxFilesCount, false/*addAppInfoOnStart*/),
		  m_userManager(userManager)
	{
	}

	bool TuningLog::write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue)
	{
		QStringList l;

		QString userName = m_userManager.loggedInUser();

		if (userName.isEmpty() == true)
		{
			l << tr("UnknownUser");
		}
		else
		{
			l << userName;
		}

		l << asp.lmEquipmentId();
		l << asp.customSignalId();
		l << oldValue.toString();
		l << newValue.toString();

		return LogFile::writeArray(l);
	}

	bool TuningLog::write(const QString& message)
	{
		QStringList l;

		QString userName = m_userManager.loggedInUser();

		if (userName.isEmpty() == true)
		{
			l << tr("UnknownUser");
		}
		else
		{
			l << userName;
		}

		l << message;

		return LogFile::writeArray(l);
	}

	void TuningLog::viewSignalsLog(QWidget* parent)
	{
        QStringList headers;
        headers.reserve(6);

        headers.emplace_back(tr("User"));
        headers.emplace_back(tr("EquipmentID/Message"));
        headers.emplace_back(tr("CustomAppSignalID"));
        headers.emplace_back(tr("Old Value"));
        headers.emplace_back(tr("New Value"));
        headers.emplace_back(QString());

		LogFile::view(parent, false/*showType*/, true/*headerVisible*/, headers);
	}

	TuningLogStub::TuningLogStub(ClientLib::TuningUserManager& userManager, const QString& logName, const QString& path, int maxFileSize, int maxFilesCount) :
		TuningLog(userManager, logName, path, maxFileSize, maxFilesCount)
	{
	}

	bool TuningLogStub::write(const AppSignalParam& /*asp*/, const TuningValue& /*oldValue*/, const TuningValue& /*newValue*/)
	{
		return true;
	}

	bool TuningLogStub::write(const QString& /*message*/)
	{
		return true;
	}

	void TuningLogStub::viewSignalsLog(QWidget* /*parent*/)
	{
	}
}

