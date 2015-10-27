#pragma once
#include <QString>
#include <QTest>
#include <QtSql>
#include <QThread>

class MultiThreadSignalStressTest : public QThread
{
public:
	MultiThreadSignalStressTest(const char* dbHost,
								const char* dbUser,
								const char* dbUserPassword,
								const char* name,
								const int mode,
								const int userIdSignalCreator,
								std::vector<int>& signalIds);

	virtual ~MultiThreadSignalStressTest();

	virtual void run();
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

	std::vector<int> m_signalIds;
	int m_mode = 0;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;
	int m_userIdSignalCreator;
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

	virtual void run();

	int m_threadNumber;
	int m_amountOfSignalIds;

	QString m_databaseHost;
	QString m_databaseUser;
	QString m_databaseUserPassword;
	QString m_projectName;

	struct signalData
	{
		int signalId;
		int signalGroupId;
		int signalInstanceId;
		int changeSetId;
		QString checkedOut;
		int userId;
		int channel;
		int type;
		QString created;
		QString deleted;
		QString instanceCreated;
		int action;
		QString strId;
		QString extStrId;
		QString name;
		int dataFormatId;
		int dataSize;
		int lowAdc;
		int highAdc;
		int lowLimit;
		int highLimit;
		int unitId;
		int adjustment;
		int dropLimit;
		int excessLimit;
		int unbalanceLimit;
		int inputLowLimit;
		int inputHighLimit;
		int inputUnitId;
		int inputSensorId;
		int outputLowLimit;
		int outputHighLimit;
		int outputUnitId;
		int outputSensorId;
		QString acquire;
		QString calculated;
		int normalState;
		int decimalPlaces;
		int aperture;
		int inOutType;
		QString deviceStrId;
		int outputRangeMode;
		int filteringTime;
		int maxDifference;
		int byteOrder;
	};
};
