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
	void get_signal_Ids_with_appsignalIdTest();
	void get_signal_Ids_with_customAppSignalIdTest();
	void get_signal_Ids_with_equipmentIdTest();
	void delete_signal_by_equipmentidTest();
	void is_signal_with_equipmentid_existsTest();
	void get_latest_signals_by_appsignalIds();

public:
	int m_firstUserForTest = -1;
	int m_secondUserForTest = -1;
	static const int maxValueId = 9999999;

	struct SignalData
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
		int dataFormatId =0;
		int dataSize = 0;
		int lowAdc = 0;
		int highAdc = 0;
		double lowengeneeringunits = 0;
		double highengeneeringunits = 0;
		int unitId = 0;
		double adjustment = 0;
		double lowvalidrange = 0;
		double highvalidrange = 0;
		double unbalanceLimit = 0;
		double inputLowLimit = 0;
		int inputHighLimit = 0;
		int inputUnitId = 0;
		int inputSensorId = 0;
		double outputLowLimit = 0;
		double outputHighLimit = 0;
		int outputUnitId = 0;
		int outputSensorId = 0;
		QString acquire;
		QString calculated;
		int normalState = 0;
		int decimalPlaces = 0;
		double aperture = 0;
		int inOutType = 0;
		QString equipmentID;
		int outputRangeMode = 0;
		double filteringTime = 0;
		double spreadtolerance = 0;
		int byteOrder = 0;
		QString enableTuning;
		double tuningDefaultValue = 0;
	};
};
