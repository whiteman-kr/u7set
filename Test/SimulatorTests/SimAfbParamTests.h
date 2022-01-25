#pragma once

#include <QObject>
#include "../../Simulator/SimAfb.h"


class SimAfbParamTests : public QObject
{
	Q_OBJECT

public:
	SimAfbParamTests() = default;

private slots:
	void initTestCase();
	void cleanupTestCase();
	void init();
	void cleanup();

	void afbComponentParamTest();

	void addSignedIntegerTest();
	void subSignedIntegerTest();
	void mulSignedIntegerTest();
	void divSignedIntegerTest();

	void addSignedIntegerNumberTest();
	void subSignedIntegerNumberTest();
	void mulSignedIntegerNumberTest();
	void divSignedIntegerNumberTest();

	void addFloatingPointTest();
	void subFloatingPointTest();

	void absFloatingPointTest();
	void absSignedIntTest();

	void convertSInt32ToSInt64();
	void convertSInt64ToSInt32();
	void convertSignedIntToFloatTest();
	void convertWordToFloatTest();
	void convertWordToSignedIntTest();
};

