#include "MonitorAppSignalListSet.h"
#include "MonitorAppSettings.h"

bool MonitorAppSignalListSet::load(QString* errorMessage)
{
	QString path = QDir::toNativeSeparators(QString{"%1//AppSignalLists//%2"}
												.arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
												.arg(MonitorAppSettings::instance().equipmentId()));

	QDir dir(path);
	if (dir.exists() == false)
	{
		return true;
	}

	QStringList files = dir.entryList(QStringList() << "*.aslist");

	for (QString& fileName : files)
	{
		QString filePath = path + QDir().separator() + fileName;

		QFile f(filePath);
		if (f.open(QFile::ReadOnly) == false)
		{
			*errorMessage = tr("Error loading AppSignalList from file %1.").arg(filePath);
			return false;
		}

		QByteArray data = f.readAll();

		std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();

		Proto::Envelope envelope;
		if (envelope.ParseFromArray(data.constData(), static_cast<int>(data.size())) == false)
		{
			*errorMessage = tr("Error parsing AppSignalList from envelope %1.").arg(filePath);
			return false;
		}

		bool ok = list->LoadData(envelope);
		if (ok == false)
		{
			*errorMessage = tr("Error loading AppSignalList from envelope %1.").arg(filePath);
			return false;
		}

		add(list);
	}

	return true;
}

bool MonitorAppSignalListSet::save(QString* errorMessage) const
{
	QString path = QDir::toNativeSeparators(QString{"%1//AppSignalLists//%2"}
												.arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
												.arg(MonitorAppSettings::instance().equipmentId()));

	QDir dir(path);
	if (dir.exists() == false)
	{
		if (dir.mkpath(path) == false)
		{
			*errorMessage = tr("Error creating directory: %1").arg(path);
			return false;
		}
	}
	else
	{
		QStringList files = dir.entryList(QStringList() << "*.aslist");
		for (const QString& fileName : files)
		{
			QString filePath = path + QDir().separator() + fileName;
			QFile f(filePath);
			bool ok = f.remove();
			Q_ASSERT(ok);
		}
	}

	for (const auto& list : lists())
	{
		if (list->systemTagsList().contains(AppSignalLists::AppSignalList::tagIde) == true)
		{
			continue;
		}

		Proto::Envelope message;
		list->SaveData(&message);

		QByteArray data;
		data.resize(static_cast<int>(message.ByteSizeLong()));

		bool result = message.SerializeToArray(data.data(), static_cast<int>(message.ByteSizeLong()));
		if (result == false)
		{
			Q_ASSERT(result);
			return false;
		}

		QString filePath = QDir::toNativeSeparators(QString{"%1//%2.%3"}.arg(path).arg(list->id()).arg("aslist"));

		QFile f(filePath);
		if (f.open(QFile::WriteOnly) == false)
		{
			*errorMessage = tr("Error opening file for writing: %1").arg(filePath);
			return false;
		}

		if (f.write(data) != data.size())
		{
			*errorMessage = tr("Error writing data to file: %1").arg(filePath);
			return false;
		}
	}

	return true;
}
