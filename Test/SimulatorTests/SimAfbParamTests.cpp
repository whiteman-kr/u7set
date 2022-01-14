#include "SimAfbParamTests.h"
#include "../../UtilsLib/WUtils.h"
#include <cmath>

SimAfbParamTests::SimAfbParamTests()
{
}

void SimAfbParamTests::initTestCase()
{
}

void SimAfbParamTests::cleanupTestCase()
{
}

void SimAfbParamTests::init()
{
}

void SimAfbParamTests::cleanup()
{
}

void SimAfbParamTests::afbComponentParamTest()
{
	const int OpIndex = 12;

	Sim::AfbComponentParam p{OpIndex};
	QVERIFY(p.opIndex() == OpIndex);

	p.setWordValue(0xFFFF);
	QVERIFY(p.wordValue() == 0xFFFF);

	p.setDwordValue(0xFFFFFFFF);
	QVERIFY(p.dwordValue() == 0xFFFFFFFF);
	QVERIFY(p.signedIntValue() == -1);

	p.setDwordValue(0);
	QVERIFY(p.dwordValue() == 0);
	QVERIFY(p.signedIntValue() == 0);
	QVERIFY(p.floatValue() == 0);

	p.setFloatValue(0);
	QVERIFY(p.dwordValue() == 0);

	p.setFloatValue(400);
	QVERIFY(p.floatValue() == 400);

	p.setSignedInt64Value(std::numeric_limits<qint64>::max());
	QVERIFY(p.signedInt64Value() == std::numeric_limits<qint64>::max());

	p.setSignedInt64Value(std::numeric_limits<qint64>::lowest());
	QVERIFY(p.signedInt64Value() == std::numeric_limits<qint64>::lowest());

	QVERIFY(p.opIndex() == OpIndex);

	p.setOpIndex(25);
	QVERIFY(p.opIndex() == 25);

	return;
}

void SimAfbParamTests::addSignedIntegerTest()
{
	Sim::AfbComponentParam p1;
	Sim::AfbComponentParam p2;

	// Test regular +
	//
	{
		const qint32 value1 = 100;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);

		p1.addSignedInteger(p2);
		QVERIFY(p1.signedIntValue() == value1 + value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with 0 result
	//
	{
		const qint32 value1 = 200;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);

		p1.addSignedInteger(p2);
		QVERIFY(p1.signedIntValue() == value1 + value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::max() - 1;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);

		p1.addSignedInteger(p2);
		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::lowest();
		const qint32 value2 = -1;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);

		p1.addSignedInteger(p2);
		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::lowest());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::subSignedIntegerTest()
{
	Sim::AfbComponentParam p1;
	Sim::AfbComponentParam p2;

	// Test regular +
	//
	{
		const qint32 value1 = 100;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.subSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == value1 - value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with 0 result
	//
	{
		const qint32 value1 = -200;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.subSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == value1 - value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::max() - 1;
		const qint32 value2 = -2;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.subSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::lowest();
		const qint32 value2 = 1;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.subSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::lowest());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::mulSignedIntegerTest()
{
	Sim::AfbComponentParam p1;
	Sim::AfbComponentParam p2;

	// Test regular +
	//
	{
		const qint32 value1 = 64000;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.mulSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == value1 * value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with 0 result
	//
	{
		const qint32 value1 = -200;
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.mulSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == value1 * value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::max()  / 2;
		const qint32 value2 = 3;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.mulSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::lowest() / 2;
		const qint32 value2 = 3;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.mulSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::lowest());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::divSignedIntegerTest()
{
	Sim::AfbComponentParam p1;
	Sim::AfbComponentParam p2;

	// Test regular +
	//
	{
		const qint32 value1 = 64000;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.divSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == value1 / value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular 0 result
	//
	{
		const qint32 value1 = 10;
		const qint32 value2 = 100;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.divSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == value1 / value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test div by zero
	//
	{
		const qint32 value1 = 100;		//  X / 0 = -1
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.divSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == -1);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 1);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test div by zero
	//
	{
		const qint32 value1 = -100;		//  -X / 0 = 1
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.divSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == 1);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 1);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test div by zero
	//
	{
		const qint32 value1 = 0;		//  0 / 0 = -1
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p2.setSignedIntValue(value2);
		p1.divSignedInteger(p2);

		QVERIFY(p1.signedIntValue() == -1);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 1);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::addSignedIntegerNumberTest()
{
	Sim::AfbComponentParam p1;

	// Test regular +
	//
	{
		const qint32 value1 = 100;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p1.addSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 + value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with 0 result
	//
	{
		const qint32 value1 = 200;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p1.addSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 + value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::max() - 1;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p1.addSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::lowest();
		const qint32 value2 = -1;

		p1.setSignedIntValue(value1);
		p1.addSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::lowest());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::subSignedIntegerNumberTest()
{
	Sim::AfbComponentParam p1;

	// Test regular +
	//
	{
		const qint32 value1 = 100;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p1.subSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 - value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with 0 result
	//
	{
		const qint32 value1 = -200;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p1.subSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 - value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::max() - 1;
		const qint32 value2 = -2;

		p1.setSignedIntValue(value1);
		p1.subSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::lowest();
		const qint32 value2 = 1;

		p1.setSignedIntValue(value1);
		p1.subSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::lowest());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::mulSignedIntegerNumberTest()
{
	Sim::AfbComponentParam p1;

	// Test regular +
	//
	{
		const qint32 value1 = 64000;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p1.mulSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 * value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with 0 result
	//
	{
		const qint32 value1 = -200;
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p1.mulSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 * value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::max()  / 2;
		const qint32 value2 = 3;

		p1.setSignedIntValue(value1);
		p1.mulSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		const qint32 value1 = std::numeric_limits<qint32>::lowest() / 2;
		const qint32 value2 = 3;

		p1.setSignedIntValue(value1);
		p1.mulSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::lowest());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::divSignedIntegerNumberTest()
{
	Sim::AfbComponentParam p1;

	// Test regular +
	//
	{
		const qint32 value1 = 64000;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p1.divSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 / value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular 0 result
	//
	{
		const qint32 value1 = 10;
		const qint32 value2 = 100;

		p1.setSignedIntValue(value1);
		p1.divSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == value1 / value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test div by zero
	//
	{
		const qint32 value1 = 100;		//  X / 0 = -1
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p1.divSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == -1);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 1);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test div by zero
	//
	{
		const qint32 value1 = -100;		//  -X / 0 = 1
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p1.divSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == 1);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 1);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test div by zero
	//
	{
		const qint32 value1 = 0;		//  0 / 0 = -1
		const qint32 value2 = 0;

		p1.setSignedIntValue(value1);
		p1.divSignedIntegerNumber(value2);

		QVERIFY(p1.signedIntValue() == -1);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 1);
		QVERIFY(p1.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::addFloatingPointTest()
{
	Sim::AfbComponentParam p1;
	Sim::AfbComponentParam p2;

	// Test regular +
	//
	{
		const float value1 = 100;
		const float value2 = -200;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.addFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), value1 + value2));

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular + with 0 result
	//
	{
		const float value1 = 200;
		const float value2 = -200;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.addFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), value1 + value2));

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular + with overflow
	//
	{
		const float value1 = std::numeric_limits<float>::max();
		const float value2 = std::numeric_limits<float>::max();

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.addFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), std::numeric_limits<float>::max()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// -overflow
	//
	{
		const float value1 = std::numeric_limits<float>::lowest();
		const float value2 = std::numeric_limits<float>::lowest();

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.addFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), std::numeric_limits<float>::lowest()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// NaN
	//
	{
		const float value1 = std::numeric_limits<float>::quiet_NaN();
		const float value2 = 10;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.addFloatingPoint(p2);

		QVERIFY(std::isnan(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 1);
	}

	// +inf
	//
	{
		const float value1 = std::numeric_limits<float>::infinity();
		const float value2 = 1;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.addFloatingPoint(p2);

		QVERIFY(std::isinf(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	return;
}

void SimAfbParamTests::subFloatingPointTest()
{
	Sim::AfbComponentParam p1;
	Sim::AfbComponentParam p2;

	// Test regular -
	//
	{
		const float value1 = 100;
		const float value2 = -200;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.subFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), value1 - value2));

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular - with 0 result
	//
	{
		const float value1 = 200;
		const float value2 = 200;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.subFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), value1 - value2));

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular - with overflow
	//
	{
		const float value1 = std::numeric_limits<float>::lowest();
		const float value2 = std::numeric_limits<float>::max();

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.subFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), std::numeric_limits<float>::lowest()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// -overflow
	//
	{
		const float value1 = std::numeric_limits<float>::lowest();
		const float value2 = std::numeric_limits<float>::max();

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.subFloatingPoint(p2);

		QVERIFY(isFloatEquals(p1.floatValue(), std::numeric_limits<float>::lowest()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// NaN
	//
	{
		const float value1 = std::numeric_limits<float>::quiet_NaN();
		const float value2 = 10;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.subFloatingPoint(p2);

		QVERIFY(std::isnan(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 1);
	}

	// +inf
	//
	{
		const float value1 = std::numeric_limits<float>::infinity();
		const float value2 = 1;

		p1.setFloatValue(value1);
		p2.setFloatValue(value2);
		p1.subFloatingPoint(p2);

		QVERIFY(std::isinf(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	return;
}

void SimAfbParamTests::absFloatingPointTest()
{
	{
		Sim::AfbComponentParam p;

		p.setFloatValue(0.f);
		p.absFloatingPoint();

		QVERIFY(p.floatValue() == 0.f);
		QVERIFY(std::signbit(p.floatValue()) == false);
		QVERIFY(p.mathZero() == 1);
	}

	{
		Sim::AfbComponentParam p;

		p.setFloatValue(-std::numeric_limits<float>::infinity());
		p.absFloatingPoint();

		QVERIFY(std::isinf(p.floatValue()) == true);
		QVERIFY(std::signbit(p.floatValue()) == false);
	}

	{
		Sim::AfbComponentParam p;

		p.setFloatValue(-123.0f);
		p.absFloatingPoint();

		QVERIFY(isFloatEquals<float>(p.floatValue(), 123.0f));
	}

	{
		Sim::AfbComponentParam p;

		p.setFloatValue(123.0f);
		p.absFloatingPoint();

		QVERIFY(isFloatEquals<float>(p.floatValue(), 123.0f));
	}

	{
		Sim::AfbComponentParam p;

		p.setFloatValue(std::numeric_limits<float>::quiet_NaN());
		p.absFloatingPoint();

		QVERIFY(std::isnan(p.floatValue()) == true);
		QVERIFY(p.mathNan() == 1);
	}
}

void SimAfbParamTests::absSignedIntTest()
{
	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(0);
		p.absSignedInt();

		QVERIFY(p.signedIntValue() == 0);
		QVERIFY(p.mathZero() == 1);
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(std::numeric_limits<int32_t>::lowest());
		p.absSignedInt();

		QVERIFY(p.signedIntValue() == std::numeric_limits<int32_t>::max());
		QVERIFY(p.mathOverflow() == 1);
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(-123);
		p.absSignedInt();

		QVERIFY(p.signedIntValue() == 123);
		QVERIFY(p.mathOverflow() == 0);
		QVERIFY(p.mathZero() == 0);
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(std::numeric_limits<qint32>::max());
		p.absSignedInt();

		QVERIFY(p.signedIntValue() == std::numeric_limits<qint32>::max());
		QVERIFY(p.mathOverflow() == 0);
		QVERIFY(p.mathZero() == 0);
	}

	return;
}

void SimAfbParamTests::convertSInt32ToSInt64()
{
	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(0);
		p.convertSInt32ToSInt64();

		QVERIFY(p.signedInt64Value() == 0);
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(std::numeric_limits<qint32>::max());
		p.convertSInt32ToSInt64();

		QVERIFY(p.signedInt64Value() == std::numeric_limits<qint32>::max());
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(std::numeric_limits<qint32>::min());
		p.convertSInt32ToSInt64();

		QVERIFY(p.signedInt64Value() == std::numeric_limits<qint32>::min());
	}

	return;
}

void SimAfbParamTests::convertSInt64ToSInt32()
{
	{
		Sim::AfbComponentParam p;

		p.setSignedInt64Value(0);
		p.convertSInt64ToSInt32();

		QVERIFY(p.signedIntValue() == 0);
		QVERIFY(p.mathOverflow() == false);
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedInt64Value(std::numeric_limits<qint32>::max());
		p.convertSInt64ToSInt32();

		QVERIFY(p.signedIntValue() == std::numeric_limits<qint32>::max());
		QVERIFY(p.mathOverflow() == false);
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedInt64Value(std::numeric_limits<qint32>::max());
		p.setSignedInt64Value(p.signedInt64Value() + 1);
		p.convertSInt64ToSInt32();

		QVERIFY(p.signedIntValue() == std::numeric_limits<qint32>::max());
		QVERIFY(p.mathOverflow() != 0);
	}

	{
		Sim::AfbComponentParam p;

		p.setSignedInt64Value(std::numeric_limits<qint32>::min());
		p.setSignedInt64Value(p.signedInt64Value() - 1);
		p.convertSInt64ToSInt32();

		QVERIFY(p.signedIntValue() == std::numeric_limits<qint32>::min());
		QVERIFY(p.mathOverflow() != 0);
	}

	return;
}

void SimAfbParamTests::convertSignedIntToFloatTest()
{
	// 0x00 -> 0.0
	//
	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(0);
		p.convertSignedIntToFloat();

		QVERIFY(p.floatValue() == 0);
	}

	// 123 -> 123.0
	//
	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(123);
		p.convertSignedIntToFloat();

		QVERIFY(isFloatEquals(p.floatValue(), 123.0f));
	}

	// -123 -> -123.0
	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(-123);
		p.convertSignedIntToFloat();

		QVERIFY(isFloatEquals(p.floatValue(), -123.0f));
	}

	return;
}

void SimAfbParamTests::convertWordToFloatTest()
{
	// 0x00 -> 0.0
	//
	{
		Sim::AfbComponentParam p;

		p.setWordValue(0);
		p.convertWordToFloat();

		QVERIFY(p.floatValue() == 0);
	}

	// 123 -> 123.0
	//
	{
		Sim::AfbComponentParam p;

		p.setWordValue(123);
		p.convertWordToFloat();

		QVERIFY(isFloatEquals(p.floatValue(), 123.0f));
	}

	// 65535 -> 65535.0
	{
		Sim::AfbComponentParam p;

		p.setWordValue(0xFFFF);
		p.convertWordToFloat();

		QVERIFY(isFloatEquals(p.floatValue(), 65535.0f));
	}

	return;
}

void SimAfbParamTests::convertWordToSignedIntTest()
{
	// 0x0 -> 0
	//
	{
		Sim::AfbComponentParam p;

		p.setWordValue(0);
		p.convertWordToSignedInt();

		QVERIFY(p.signedIntValue() == 0);
	}

	// 123 -> 123
	//
	{
		Sim::AfbComponentParam p;

		p.setWordValue(123);
		p.convertWordToSignedInt();

		QVERIFY(p.signedIntValue() == 123);
	}

	// 65535 -> 65535
	{
		Sim::AfbComponentParam p;

		p.setWordValue(0xFFFF);
		p.convertWordToSignedInt();

		QVERIFY(p.signedIntValue() == 65535);
	}

	return;
}

