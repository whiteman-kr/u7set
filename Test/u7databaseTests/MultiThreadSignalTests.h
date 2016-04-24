#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include <QThread>

class MultiThreadGetSignalTest : public QThread
{
public:
	MultiThreadGetSignalTest(const char* dbHost,
								const char* dbUser,
								const char* dbUserPassword,
								const char* name,
								std::vector<int>& signalIds);

	virtual ~MultiThreadGetSignalTest();

	virtual void run();

	std::vector<int> m_signalIds;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;

	int m_currentSignalId = 0;

	QMutex mutex;
};

class MultiThreadSignalCheckInTest : public QThread
{
public:
	MultiThreadSignalCheckInTest(const char* dbHost,
								const char* dbUser,
								const char* dbUserPassword,
								const char* name,
								const int userIdSignalCreator,
								std::vector<int> &signalIds,
								 MultiThreadGetSignalTest* thread);

	virtual ~MultiThreadSignalCheckInTest();

	virtual void run();

	std::vector<int> m_signalIds;

	MultiThreadGetSignalTest *m_getSignalThread = nullptr;

	QMutex mutex;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;

	int m_userIdSignalCreator = 0;
};

class MultiThreadSignalTest : public QThread
{
public:
	MultiThreadSignalTest(int number,
						  const char* dbHost,
						  const char* dbUser,
						  const char* dbUserPassword,
						  const char* name,
						  int amountOfFiles);

	virtual ~MultiThreadSignalTest();

	static int fillSignalIdsVector(std::vector<int>& signalIds,
								   int userId,
								   int signalAmount,
								   const char* dbHost,
								   const char* dbUser,
								   const char* dbUserPassword,
								   const char* name);
	static int create_user(const char* dbHost,
						   const char* dbUser,
						   const char* dbUserPassword,
						   const char* name);

	virtual void run();

	int m_threadNumber = 0;
	int m_amountOfSignalIds = 0;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;

	struct signalData
	{
		int signalId = 0;
		int signalGroupId = 0;
		int signalInstanceId = 0;
		int changeSetId = 0;
		QString checkedOut;
		int userId = 0;
		int channel = 0;
		int type = 0;
		QString created;
		QString deleted;
		QString instanceCreated;
		int action = 0;
		QString appSignalID;
		QString customAppSignalID;
		QString caption;
		int dataFormatId = 0;
		int dataSize = 0;
		int lowAdc = 0;
		int highAdc = 0;
		int lowLimit = 0;
		int highLimit = 0;
		int unitId = 0;
		int adjustment = 0;
		int dropLimit = 0;
		int excessLimit = 0;
		int unbalanceLimit = 0;
		int inputLowLimit = 0;
		int inputHighLimit = 0;
		int inputUnitId = 0;
		int inputSensorId = 0;
		int outputLowLimit = 0;
		int outputHighLimit = 0;
		int outputUnitId = 0;
		int outputSensorId = 0;
		QString acquire;
		QString calculated;
		int normalState = 0;
		int decimalPlaces = 0;
		int aperture = 0;
		int inOutType = 0;
		QString equipmentID;
		int outputRangeMode = 0;
		int filteringTime = 0;
		int maxDifference = 0;
		int byteOrder = 0;
		QString enableTuning;
	};
};
