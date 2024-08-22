#include "TuningSignalListSet.h"
#include "Settings.h"


std::set<Hash> TuningSignalListSet::filtersSetHashes(const std::set<QString>& filtersSet) const
{
	std::set<Hash> result;

	// Make filters hashes intersection for this counter
	//
	bool first = true;
	for (const QString& uiFilters : filtersSet)
	{
		std::set<Hash> filterHashes;

		if (uiFilters.contains('+') == true)
		{
			// Filters union
			//
			QStringList uiFiltersUnion = uiFilters.split('+', Qt::SkipEmptyParts);

			for (const QString& id : uiFiltersUnion)
			{
				AppSignalLists::AppSignalList* pageList = get(id).get();
				if (pageList == nullptr)
				{
					Q_ASSERT(false);
					continue;
				}

				filterHashes.insert(pageList->tuningListHashesCache().begin(), pageList->tuningListHashesCache().end());
			}
		}
		else
		{
			// Separate filter
			//
			AppSignalLists::AppSignalList* pageList = get(uiFilters).get();
			if (pageList == nullptr)
			{
				Q_ASSERT(false);
				continue;
			}

			filterHashes = pageList->tuningListHashesCache();
		}

		if (first == true)
		{
			first = false;
			result = filterHashes;
		}
		else
		{
			std::vector<Hash> v_intersection;
			std::set_intersection(result.begin(),
								  result.end(),
								  filterHashes.begin(),
								  filterHashes.end(),
								  std::back_inserter(v_intersection));
			result.clear();
			result.insert(v_intersection.begin(), v_intersection.end());
		}
	}

	return result;
}





bool TuningSignalListSet::load(QString* errorMessage)
{
	QString path = QDir::toNativeSeparators(QString{"%1//TuningSignalLists//%2"}
												.arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
												.arg(TuningClientAppSettings::instance().instanceStrId()));

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
			*errorMessage = tr("Error loading TuningSignalList from file %1.").arg(filePath);
			return false;
		}

		QByteArray data = f.readAll();

		std::shared_ptr<AppSignalLists::AppSignalList> list = std::make_shared<AppSignalLists::AppSignalList>();

		Proto::Envelope envelope;
		if (envelope.ParseFromArray(data.constData(), static_cast<int>(data.size())) == false)
		{
			*errorMessage = tr("Error parsing TuningSignalList from envelope %1.").arg(filePath);
			return false;
		}

		bool ok = list->LoadData(envelope);
		if (ok == false)
		{
			*errorMessage = tr("Error loading TuningSignalList from envelope %1.").arg(filePath);
			return false;
		}

		add(list);
	}

	return true;
}

bool TuningSignalListSet::save(QString* errorMessage) const
{
	QString path = QDir::toNativeSeparators(QString{"%1//TuningSignalLists//%2"}
												.arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation))
												.arg(TuningClientAppSettings::instance().instanceStrId()));

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
		for (const QString& fileName: files) 
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
