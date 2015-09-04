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

public:
	int m_firstUserForTest = -1;
	int m_secondUserForTest = -1;
	static const int maxValueId = 9999999;
};
