#include "ComparatorsStorage.h"
#include "../../UtilsLib/CsvFile.h"

#include <vector>
#include <QSettings>
#include <QFile>
#include <QByteArray>
#include <QStringList>
#include <QMessageBox>


ComparatorsStorage::ComparatorsStorage()
{
	m_csvHeaders << "Version" << "CaseID" << "SchemaID"
				 << "InputAppSignalID" << "InputCustomAppSignalID" << "InputAppSignalCaption"
				 << "OutputAppSignalID" << "OutputCustomAppSignalID" << "OutputAppSignalCaption"
				 << "DefaultCriteria" << "Criteria" << "Type"
				 << "SetpointAppSignalID" << "SetpointCustomAppSignalID" << "SetpointAppSignalCaption" << "Precision";
}

ComparatorsStorage::~ComparatorsStorage() 
{
}

void ComparatorsStorage::clear()
{
	m_comparatorData.clear();
	m_comparatorDataMap.clear();
	return;
}

bool ComparatorsStorage::loadFromFile(const QString& fileName, QString* errorMsg)
{
	clear();

	QFile file(fileName);
	if (file.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		return false;
	}

	const int csvColumnsCount = m_csvHeaders.size();

	QTextStream in(&file);
	QString header = in.readLine();
	QStringList headerFields = header.split(";", Qt::KeepEmptyParts);
	if (headerFields.size() != csvColumnsCount || m_csvHeaders.size() != csvColumnsCount)
	{
		*errorMsg = QObject::tr("Expected %1 columns in the header.").arg(csvColumnsCount);
		return false;
	}

	for (int i = 0; i < csvColumnsCount; i++) 
	{
		const QString& csvColHeader = headerFields.at(i);
		const QString& expectedColHeader = m_csvHeaders.at(i);
		if (csvColHeader != expectedColHeader) 
		{
			*errorMsg = QObject::tr("CSV column %1: header is %2, expected %3.").arg(i).arg(csvColHeader).arg(expectedColHeader);
			return false;
		}
	}

	int lineIndex = 0;

	CsvFile toCsv;

	while (in.atEnd() == false)
	{
		lineIndex++;
		QString line = in.readLine();
		QStringList fields = toCsv.csvToStrings(line);

		int c = 0;
		QString versionStr = fields[c++];

		if (versionStr != "1") 
		{
			*errorMsg = QObject::tr("Expected version 1 of CSV record!");
			return false;
		}

		ComparatorData data;
		data.caseID = fields[c++];
		data.schemaID = fields[c++];
		data.inputAppSignalID = fields[c++];
		data.inputCustomAppSignalID = fields[c++];
		data.inputSignalCaption = fields[c++];
		data.outputAppSignalID = fields[c++];
		data.outputCustomAppSignalID = fields[c++];
		data.outputSignalCaption = fields[c++];
		data.defaultValue = fields[c++].toDouble();
		data.currentValue = fields[c++].toDouble();
		data.type = fields[c++];
		data.setpointAppSignalID = fields[c++];
		data.setpointCustomAppSignalID = fields[c++];
		data.setpointAppSignalCaption = fields[c++];
		data.precision = fields[c++].toInt();
		
		Hash hash = ::calcHash(data.outputAppSignalID);
		data.hash = hash;

		if (m_comparatorDataMap.find(hash) != m_comparatorDataMap.end()) 
		{
			*errorMsg = QObject::tr("\nDuplicated output signal %1 in the file, record number: %2.").arg(data.outputAppSignalID).arg(lineIndex);
			return false;
		}

		m_comparatorData.push_back(hash);
		m_comparatorDataMap[hash] = data;
	}
	file.close();

	return true;
}

static QStringList getComparatorDataAsStringList(const ComparatorData& data)
{
	QStringList fields;
	fields << "1" << data.caseID << data.schemaID
		<< data.inputAppSignalID << data.inputCustomAppSignalID << data.inputSignalCaption
		<< data.outputAppSignalID << data.outputCustomAppSignalID << data.outputSignalCaption
		<< QString::number(data.defaultValue) << QString::number(data.currentValue) << data.type
		<< data.setpointAppSignalID << data.setpointCustomAppSignalID << data.setpointAppSignalCaption << QString::number(data.precision);
	return fields;
}

bool ComparatorsStorage::saveToFile(const QString& fileName)
{
	QFile file(fileName);
	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		return false;
	}

	QTextStream out(&file);

	CsvFile toCsv;
	out << toCsv.stringsToCSV(m_csvHeaders, false);
	
	for (const Hash hash : m_comparatorData)
	{
		const auto& cmp = m_comparatorDataMap.at(hash);

		out << "\n" << toCsv.stringsToCSV(getComparatorDataAsStringList(cmp), false);
	}
	file.close();
	
	return true;
}

static bool updateComparatorData(const QString& lmId,
								 ComparatorData& data,
								 const Comparator& comparator,
								 const AppSignalSet& appSignals,
								 QString* errorMsg)
{
	data.schemaID = comparator.schemaID();

	const AppSignal* inputSignal = appSignals.getSignal(comparator.input().appSignalID());
	if (inputSignal != nullptr)
	{
		data.inputAppSignalID = inputSignal->appSignalID();
		data.inputCustomAppSignalID = inputSignal->customAppSignalID();
		data.inputSignalCaption = inputSignal->caption();
		data.precision = inputSignal->decimalPlaces();
	}
	else 
	{
		*errorMsg =
			QObject::tr("Input signal %1 of the comparator was not found in AppSignals file. Check if files are from the same build.")
				.arg(comparator.input().appSignalID());
		return false;
	}

	QStringList lmIdList = lmId.split("_", Qt::SkipEmptyParts);
	if (lmIdList.size() > 1)
	{
		data.caseID = lmIdList[1];
	}
	else
	{
		data.caseID = lmId;
	}


	const AppSignal* outputSignal = appSignals.getSignal(comparator.output().appSignalID());
	if (outputSignal != nullptr)
	{
		data.outputAppSignalID = outputSignal->appSignalID();
		data.outputCustomAppSignalID = outputSignal->customAppSignalID();
		data.outputSignalCaption = outputSignal->caption();
	}
	else
	{
		*errorMsg =
			QObject::tr("Output signal %1 of the comparator was not found in AppSignals file. Check if files are from the same build.")
				.arg(comparator.output().appSignalID());
		return false;
	}


	data.isDynamic = comparator.compare().isConst() == false;

	if (data.isDynamic == false)
	{
		data.type = "S";
		data.defaultValue = comparator.compare().constValue();
		data.setpointAppSignalID = "";
		data.setpointCustomAppSignalID = "";
		data.setpointAppSignalCaption = "";
	}
	else
	{
		data.type = "D";

		const AppSignal* setpointSignal = appSignals.getSignal(comparator.compare().appSignalID());
		if (setpointSignal != nullptr)
		{
			data.setpointAppSignalID = setpointSignal->appSignalID();
			data.setpointCustomAppSignalID = setpointSignal->customAppSignalID();
			data.setpointAppSignalCaption = setpointSignal->caption();
			data.defaultValue = setpointSignal->tuningDefaultValue().toDouble();
		}
		else
		{
			*errorMsg =
				QObject::tr("Setpoint signal %1 of the comparator was not found in AppSignals file. Check if files are from the same build.")
					.arg(comparator.compare().appSignalID());
			return false;
		}
	}
	
	return true;
}

std::pair<bool, ImportResult> ComparatorsStorage::importComparatorsSet(const QString& comparatorsFile,
																	   const QString& appSignalsFile,
																	   QString* errorMsg)
{
	if (QFileInfo(comparatorsFile).absolutePath() != QFileInfo(appSignalsFile).absolutePath())
	{
		*errorMsg = QObject::tr("Comparators file and AppSignals file must be in the same directory.");
		return {false, {}};
	}


	ComparatorSet comparatorsSet;
	AppSignalSet appSignals;
	ImportResult ir;

	bool ok = loadComparators(comparatorsFile, comparatorsSet);
	if (ok == false) 
	{
		*errorMsg = QObject::tr("Failed to load comparators from file: %1").arg(comparatorsFile);
		return {false, {}};
	}


	ok = loadAppSignals(appSignalsFile, appSignals);
	if (ok == false)
	{
		*errorMsg = QObject::tr("Failed to load app signals from file: %1").arg(appSignalsFile);
		return {false, {}};
	}

	std::set<Hash> loadedKeys; // Keys loaded from the imported file

	// Add or update imported comparators to the database
	//
	{
		QStringList lmIDs = comparatorsSet.lmIDs();

		for (const QString& lmID : lmIDs)
		{
			std::vector<std::shared_ptr<Comparator>> lmComparators = comparatorsSet.getByLmID(lmID);

			for (const std::shared_ptr<Comparator>& comparator : lmComparators)
			{
				if (comparator == nullptr)
				{
					Q_ASSERT(comparator);
					continue;
				}

				Hash key = ::calcHash(comparator->output().appSignalID());
				loadedKeys.insert(key);

				auto it = m_comparatorDataMap.find(key);
				if (it != m_comparatorDataMap.end())
				{
					// This is an exising comparator, update its contents
					//
					ComparatorData& data = it->second;
					data.hash = key;

					if (updateComparatorData(lmID, data, *comparator, appSignals, errorMsg) == false)
					{
						clear();
						return {false, {}};
					}

					if (data.isValueOverriden()) 
					{
						ir.overriddenValuesComparatorCount++;
					}
					
				}
				else
				{
					// This is a new comparator, add it

					ComparatorData data;
					data.hash = key;

					if (updateComparatorData(lmID, data, *comparator, appSignals, errorMsg) == false)
					{
						clear();
						return {false, {}};
					}
					
					data.currentValue = data.defaultValue;

					m_comparatorDataMap[key] = data;
					m_comparatorData.push_back(key);
					ir.newComparatorCount++;
				}
			}
		}
	}

	// Remove non-deleted comparators from the database
	//
	{
		std::vector<Hash> oldComparatorData = std::move(m_comparatorData);
		m_comparatorData = {};
		m_comparatorData.reserve(oldComparatorData.size());
		
		for (Hash cmpHash : oldComparatorData) 
		{
			if (loadedKeys.contains(cmpHash) == true) 
			{
				m_comparatorData.push_back(cmpHash);
			}
			else
			{
				ir.removedComparatorCount++;
				m_comparatorDataMap.erase(cmpHash);
			}
		}
	}

	return {true, ir};
}

std::vector<Hash> ComparatorsStorage::comparatorsHashes() const
{ 
	return m_comparatorData; 
}

std::vector<Hash> ComparatorsStorage::filteredComparatorsHashes(QList<QString> id, FilterType filterType) const
{ 
	std::vector<Hash> comparatorDataByCaseID;

	if (filterType == FilterType::BySchemaID)
	{
		for (const Hash hash : m_comparatorData)
		{
			const auto& data = m_comparatorDataMap.at(hash);

			if (id.isEmpty() == true || id.contains(data.schemaID))
			{
				comparatorDataByCaseID.push_back(hash);
			}
		}
	}
	else if (filterType == FilterType::ByCaseID)
	{
		for (const Hash hash : m_comparatorData)
		{
			const auto& data = m_comparatorDataMap.at(hash);

			if (id.isEmpty() == true || id.contains(data.caseID))
			{
				comparatorDataByCaseID.push_back(hash);
			}
		}
	}

	return comparatorDataByCaseID;
}

const ComparatorData& ComparatorsStorage::comparator(Hash hash, bool* ok) const
{ 
	static ComparatorData err;

	if (ok == nullptr) 
	{
		Q_ASSERT(ok);
		return err;
	}

	*ok = true;
	auto it = m_comparatorDataMap.find(hash); 
	if (it == m_comparatorDataMap.end()) 
	{
		*ok = false;
		return err;
	}

	return it->second;
}

QList<QString> ComparatorsStorage::schemaIds() const
{
	QList<QString> schemaIDsSet;
	for (const auto& [hash, data]: m_comparatorDataMap)
	{
		if (schemaIDsSet.contains(data.schemaID) == false)
		{
			schemaIDsSet.append(data.schemaID);
		}
	}
	std::sort(schemaIDsSet.begin(), schemaIDsSet.end());
		
	return schemaIDsSet;
};

QList<QString> ComparatorsStorage::caseIds() const
{
	QList<QString> caseIDsSet;
	for (const auto& [hash, data] : m_comparatorDataMap)
	{
		if (caseIDsSet.contains(data.caseID) == false)
		{
			caseIDsSet.append(data.caseID);
		}
	}
	std::sort(caseIDsSet.begin(), caseIDsSet.end());

	return caseIDsSet;
}

void ComparatorsStorage::setComparatorCriteria(Hash hash, double criteria)
{
	auto it = m_comparatorDataMap.find(hash);
	if (it == m_comparatorDataMap.end())
	{
		Q_ASSERT(false);
		return;
	}
	it->second.currentValue = criteria;
}

bool ComparatorsStorage::loadComparators(const QString& comparatorsFile, ComparatorSet& comparatorSet) const
{
	QFile file(comparatorsFile);

	qint64 fileSize = file.size();
	QByteArray fileData;

	if (file.open(QIODevice::ReadOnly | QIODeviceBase::ExistingOnly) == true)
	{
		fileData = file.readAll();
		// file.close();
	}
	else
	{
		return false;
	}

	if (fileData.size() != fileSize)
	{
		return false;
	}

	bool result = comparatorSet.serializeFrom(fileData);
	if (result == false)
	{
		fileData = qUncompress(fileData);

		result = comparatorSet.serializeFrom(fileData);
		if (result == false)
		{
			return false;
		}
	}

	return true;
}

bool ComparatorsStorage::loadAppSignals(const QString& appSignalsFile, AppSignalSet& appSignalSet) const
{
	bool result = appSignalSet.serializeFromProtoFile(appSignalsFile);
	if (result == false)
	{
		return false;
	}

	return true;
}
