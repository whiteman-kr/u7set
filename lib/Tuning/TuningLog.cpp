#include "TuningLog.h"
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QTextStream>
#include <QDateTime>
#include <QAbstractItemModel>
#include <QComboBox>
#include <QUuid>
#include "../CommonLib/Hash.h"

namespace TuningLog
{
	//
	// TuningLog
	//

	TuningLog::TuningLog(const QString& logName, const QString& path, int maxFileSize, int maxFilesCount)
		: Log::LogFile(logName, path, maxFileSize, maxFilesCount)
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
		std::vector<std::pair<QString, double>> headers;

		headers.push_back(std::make_pair(tr("User"), 0.1));
		headers.push_back(std::make_pair(tr("EquipmentID/Message"), 0.3));
		headers.push_back(std::make_pair(tr("CustomAppSignalID"), 0.2));
		headers.push_back(std::make_pair(tr("Old Value"), 0.2));
		headers.push_back(std::make_pair(tr("New Value"), 0.2));

		LogFile::view(parent, false, headers);
	}
}

