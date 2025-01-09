#pragma once

#include <QObject>

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

	void afbComponentConstruct();
	void afbComponentParamTest();

	void mulFloatingPointTest();
	void divFloatingPointTest();
	void addFloatingPointTest();
	void subFloatingPointTest();

	void addSignedIntegerTest();
	void subSignedIntegerTest();
	void mulSignedIntegerTest();
	void divSignedIntegerTest();

	void absFloatingPointTest();
	void absSignedIntTest();

	void sinFloatingPointTest();
	void cosFloatingPointTest();

	void convertSInt32ToSInt64();
	void convertSInt64ToSInt32();
	void convertSignedIntToFloatTest();
	void convertWordToFloatTest();
	void convertWordToSignedIntTest();

	void afbComponentInstanceConstruct();

	void modelComponent();
	void afbComponentSet();
};

