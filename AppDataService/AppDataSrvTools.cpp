#include "AppDataSrvTools.h"

namespace AppDataSrvTools
{
	bool readAppDataSources(const QByteArray& fileData,
							const QString& profile,
							AppDataSources& appDataSources,
							CircularLoggerShared log)
	{
		TEST_PTR_RETURN_FALSE(log);

		appDataSources.clear();

		QVector<OnlineLib::DataSource> dataSources;

		bool result = OnlineLib::DataSourcesXML<OnlineLib::DataSource>::readFromXml(fileData, &dataSources);

		if (result == false)
		{
			DEBUG_LOG_ERR(log, QString("Error reading AppDataSources from XML-file"));
			return false;
		}

		result = appDataSources.init(profile, dataSources, log);

		if (result == true)
		{
			DEBUG_LOG_MSG(log, QString("AppDataSources successfully loaded"));
		}
		else
		{
			DEBUG_LOG_ERR(log, QString("AppDataSources loading error!"));
		}

		return result;
	}

	bool readAppSignals(const QByteArray& fileData, AppSignals& appSignals)
	{
		appSignals.clear();

		::Proto::AppSignalSet signalSet;

		bool result = signalSet.ParseFromArray(fileData.constData(), static_cast<int>(fileData.size()));

		if (result == false)
		{
			return false;
		}

		int signalCount = signalSet.appsignal_size();

		for(int i = 0; i < signalCount; i++)
		{
			const ::Proto::AppSignal& appSignal = signalSet.appsignal(i);

			appSignals.insert(appSignal);
		}

		return true;
	}

	void createAndInitSignalStates(	const AppSignals& appSignals,
									DynamicAppSignalStates& appSignalStates,
									int autoArchivingGroupsCount)
	{
		appSignalStates.clear();

		if (appSignals.isEmpty())
		{
			return;
		}

		int signalCount = 0;

		for(AppSignal* signal : appSignals)
		{
			TEST_PTR_CONTINUE(signal);

			if (signal->isBus() == true)
			{
				continue;
			}

			signalCount++;
		}

		appSignalStates.setSize(signalCount);

		int index = 0;

		for(AppSignal* signal : appSignals)
		{
			TEST_PTR_CONTINUE(signal);

			if (signal->isBus() == true)
			{
				continue;
			}

			DynamicAppSignalState* signalState = appSignalStates[index];

			signalState->setSignalParams(signal, appSignals);

			index++;
		}

		appSignalStates.buidlHash2State();

		appSignalStates.setAutoArchivingGroups(autoArchivingGroupsCount);
	}
}
