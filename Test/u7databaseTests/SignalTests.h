#pragma once
#include <QTest>

class SignalTests : public QObject
{
	Q_OBJECT

public:
	SignalTests();

private slots:
	void initTestCase();
	void add_signalTest();
	void get_signal_IdsTest();
	void get_signal_countTest();
	void checkin_signalsTest();
	void checkout_signalsTest();
	void delete_signalTest();
	void get_latest_signalTest();
	void get_latest_signalsTest();
	void get_latest_signals_allTest();
	void undo_signal_changesTest();
	void set_signal_workcopyTest();

public:
	int m_firstUserForTest = -1;
	int m_secondUserForTest = -1;
	static const int maxValueId = 9999999;

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
		QString strId;
		QString extStrId;
		QString caption;
		int dataFormatId =0;
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
		QString deviceStrId;
		int outputRangeMode = 0;
		int filteringTime = 0;
		int maxDifference = 0;
		int byteOrder = 0;
		QString enableTuning;
	};
};
