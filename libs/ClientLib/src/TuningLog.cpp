#include <ClientLib/TuningLog.h>

namespace ClientLib
{
	//
	// TuningLog
	//

	TuningLog::TuningLog(ITuningAuthorization& tuningAuthorization,
						 const QString& logName,
						 const QString& path,
						 int maxFileSize,
						 int maxFilesCount) :
		m_logFile(logName, path, maxFileSize, maxFilesCount, false /*addAppInfoOnStart*/),
		m_tuningAuthorization(tuningAuthorization)
	{
	}

	bool TuningLog::write(const AppSignalParam& asp, const TuningValue& oldValue, const TuningValue& newValue)
	{
		QStringList l;

		QString userName = m_tuningAuthorization.userName();

		if (userName.isEmpty() == true)
		{
			l << QObject::tr("UnknownUser");
		}
		else
		{
			l << userName;
		}

		l << asp.lmEquipmentId();
		l << asp.customSignalId();
		l << oldValue.toString();
		l << newValue.toString();

		return m_logFile.writeArray(l);
	}

	bool TuningLog::write(const QString& message)
	{
		QStringList l;

		QString userName = m_tuningAuthorization.userName();

		if (userName.isEmpty() == true)
		{
			l << QObject::tr("UnknownUser");
		}
		else
		{
			l << userName;
		}

		l << message;

		return m_logFile.writeArray(l);
	}

	void TuningLog::viewTuningLog(QWidget* parent)
	{
		QStringList headers;
		headers.reserve(6);

		headers.emplace_back(QObject::tr("User"));
		headers.emplace_back(QObject::tr("EquipmentID/Message"));
		headers.emplace_back(QObject::tr("CustomAppSignalID"));
		headers.emplace_back(QObject::tr("Old Value"));
		headers.emplace_back(QObject::tr("New Value"));
		headers.emplace_back(QString());

		m_logFile.view(parent, false /*showType*/, true /*headerVisible*/, headers);
	}

	bool TuningLogStub::write(const AppSignalParam& /*asp*/, const TuningValue& /*oldValue*/, const TuningValue& /*newValue*/)
	{
		return true;
	}

	bool TuningLogStub::write(const QString& /*message*/)
	{
		return true;
	}

	void TuningLogStub::viewTuningLog(QWidget* /*parent*/) {}
} // namespace ClientLib
