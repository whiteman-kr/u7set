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
