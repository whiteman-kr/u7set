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

	TuningLog::TuningLog(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount)
		: Log::LogFile(logName, path, maxFileSize, maxFilesCount, false/*addAppInfoOnStart*/)
	{
	}

	TuningLog::~TuningLog()
	{
	}

	bool TuningLog::write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue, const QString& userName)
	{
		QStringList l;

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

	bool TuningLog::write(const QString& message, const QString& userName)
	{
		QStringList l;

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
}

