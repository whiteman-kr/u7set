#pragma once

#include <QTest>
#include <QtSql>
#include "../../lib/Signal.h"
#include "../../lib/WUtils.h"

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
	void get_signal_historyTest();
	void get_specific_signalTest();

private:
	bool readSignalFromQuery(const QSqlQuery& q, Signal& s, quint64 excludeMask);

	void verifyQueryAndSignal(const QSqlQuery& q, Signal& s, quint64 excludeMask);

private:
	static QString SF_APP_SIGNAL_ID;
	static QString SF_CUSTOM_APP_SIGNAL_ID;
	static QString SF_CAPTION;
	static QString SF_EQUIPMENT_ID;
	static QString SF_BUS_TYPE_ID;
	static QString SF_CHANNEL;

	static QString SF_SIGNAL_TYPE;
	static QString SF_IN_OUT_TYPE;

	static QString SF_DATA_SIZE;
	static QString SF_BYTE_ORDER;

	static QString SF_ANALOG_SIGNAL_FORMAT;
	static QString SF_UNIT;

	static QString SF_LOW_ADC;
	static QString SF_HIGH_ADC;
	static QString SF_LOW_ENGENEERING_UNITS;
	static QString SF_HIGH_ENGENEERING_UNITS;
	static QString SF_LOW_VALID_RANGE;
	static QString SF_HIGH_VALID_RANGE;
	static QString SF_FILTERING_TIME;
	static QString SF_SPREAD_TOLERANCE;

	static QString SF_ELECTRIC_LOW_LIMIT;
	static QString SF_ELECTRIC_HIGH_LIMIT;
	static QString SF_ELECTRIC_UNIT;
	static QString SF_SENSOR_TYPE;
	static QString SF_OUTPUT_MODE;

	static QString SF_ENABLE_TUNING;
	static QString SF_TUNING_DEFAULT_VALUE;
	static QString SF_TUNING_LOW_BOUND;
	static QString SF_TUNING_HIGH_BOUND;

	static QString SF_ACQUIRE;
	static QString SF_DECIMAL_PLACES;
	static QString SF_COARSE_APERTURE;
	static QString SF_FINE_APERTURE;
	static QString SF_ADAPTIVE_APERTURE;

	static QString SF_SIGNAL_ID;
	static QString SF_ID;
	static QString SF_SIGNAL_GROUP_ID;
	static QString SF_SIGNAL_INSTANCE_ID;
	static QString SF_CHANGESET_ID;
	static QString SF_CHECKED_OUT;
	static QString SF_USER_ID;
	static QString SF_CREATED;
	static QString SF_DELETED;
	static QString SF_INSTANCE_CREATED;
	static QString SF_INSTANCE_ACTION;
	static QString SF_ACTION;

	static QString SF_CHECKED_IN_INSTANCE_ID;
	static QString SF_CHECKED_OUT_INSTANCE_ID;

	const quint64 EXM_CHANNEL = 0x0000000000000001ll;
	const quint64 EXM_SIGNAL_ID = 0x0000000000000002ll;
	const quint64 EXM_SIGNAL_GROUP_ID = 0x0000000000000004ll;
	const quint64 EXM_SIGNAL_TYPE = 0x0000000000000008ll;
	const quint64 EXM_CHECKED_OUT = 0x0000000000000010ll;
	const quint64 EXM_USER_ID = 0x0000000000000020ll;
	const quint64 EXM_CREATED = 0x0000000000000040ll;
	const quint64 EXM_DELETED = 0x0000000000000080ll;
	const quint64 EXM_INSTANCE_CREATED = 0x00000000000000100ll;
	const quint64 EXM_INSTANCE_ACTION = 0x00000000000000200ll;

	const quint64 EXM_SIGNAL_TABLE_FIELDS = EXM_CHANNEL | EXM_SIGNAL_ID | EXM_SIGNAL_GROUP_ID |
											EXM_SIGNAL_TYPE | EXM_CHECKED_OUT | EXM_USER_ID | EXM_CREATED |
											EXM_DELETED | EXM_INSTANCE_CREATED | EXM_INSTANCE_ACTION;

	//

	int m_firstUserForTest = -1;
	int m_secondUserForTest = -1;
	static const int maxValueId = 9999999;

	QString m_adminPassword = "123412341234";
};
