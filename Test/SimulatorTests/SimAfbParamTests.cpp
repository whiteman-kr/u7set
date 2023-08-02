#include <memory>
#include <limits>
#include <cmath>
#include <numbers>
#include <QTest>
#include "SimAfbParamTests.h"
#include <SimAfb.h>
#include <SimException.h>

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

void SimAfbParamTests::afbComponentConstruct()
{
	{
		std::shared_ptr<Afb::AfbComponent> sp;

		Sim::AfbComponent afbcomp(std::move(sp));
		QCOMPARE(afbcomp.isNull(), true);

		QCOMPARE(afbcomp.opCode(), -1);
		QCOMPARE(afbcomp.caption(), "");
		QCOMPARE(afbcomp.maxInstCount(), -1);
		QCOMPARE(afbcomp.simulationFunc(), "");

		QCOMPARE(afbcomp.pinExists(0), false);
		QCOMPARE(afbcomp.pinCaption(0), "");
	}

	{
		auto afb = std::make_shared<Afb::AfbComponent>();
		afb->setOpCode(12);
		afb->setHasRam(true);
		afb->setCaption("AFB_COMP");
		afb->setMaxInstCount(256);
		afb->setSimulationFunc("SimFunc");

		Sim::AfbComponent simAfb(afb);

		QCOMPARE(simAfb.isNull(), false);
		QCOMPARE(simAfb.opCode(), afb->opCode());
		QCOMPARE(simAfb.caption(), afb->caption());
		QCOMPARE(simAfb.maxInstCount(), afb->maxInstCount());
		QCOMPARE(simAfb.simulationFunc(), afb->simulationFunc());

		QCOMPARE(simAfb.pinExists(0), false);
		QCOMPARE(simAfb.pinCaption(0), "[UnknownPin 0]");
	}

	return;
}

void SimAfbParamTests::afbComponentParamTest()
{
	const int OpIndex = 12;

	Sim::AfbComponentParam p{OpIndex};
	QCOMPARE(p.opIndex(), OpIndex);

	p.setWordValue(0xFFFF);
	QCOMPARE(p.wordValue(), 0xFFFF);

	p.setDwordValue(0xFFFFFFFF);
	QCOMPARE(p.dwordValue(), 0xFFFFFFFF);
	QCOMPARE(p.signedIntValue(), -1);

	p.setDwordValue(0);
	QCOMPARE(p.dwordValue(), 0u);
	QCOMPARE(p.signedIntValue(), 0);
	QCOMPARE(p.floatValue(), 0);

	p.setFloatValue(0);
	QCOMPARE(p.dwordValue(), .0f);

	p.setFloatValue(400);
	QCOMPARE(p.floatValue(), 400);

	p.setDoubleValue(900.0);
	QCOMPARE(p.doubleValue(), 900);

	p.setSignedInt64Value(std::numeric_limits<qint64>::max());
	QCOMPARE(p.signedInt64Value(), std::numeric_limits<qint64>::max());

	p.setSignedInt64Value(std::numeric_limits<qint64>::lowest());
	QCOMPARE(p.signedInt64Value(), std::numeric_limits<qint64>::lowest());

	QCOMPARE(p.opIndex(), OpIndex);

	p.setOpIndex(25);
	QCOMPARE(p.opIndex(), 25);

	return;
}

void SimAfbParamTests::mulFloatingPointTest()
{
	Sim::AfbComponentParam p1;

	// Test regular +
	//
	{
		const float value1 = 2.0;
		const float value2 = 3.0;

		p1.setFloatValue(value1);
		p1.mulFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), value1 * value2);

		QCOMPARE(p1.mathOverflow(), 0);
		QCOMPARE(p1.mathUnderflow(), 0);
		QCOMPARE(p1.mathZero(), 0);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	// Test regular + with 0 result
	//
	{
		const float value1 = -2.0;
		const float value2 = 3.0;

		p1.setFloatValue(value1);
		p1.mulFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), value1 * value2);

		QCOMPARE(p1.mathOverflow(), 0);
		QCOMPARE(p1.mathUnderflow(), 0);
		QCOMPARE(p1.mathZero(), 0);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	// inf
	{
		constexpr float value1 = std::numeric_limits<float>::max();
		constexpr float value2 = 3;

		p1.setFloatValue(value1);
		p1.mulFloatingPoint(value2);

		QVERIFY(std::isinf(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathZero() == 0);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	// -inf
	//
	{
		constexpr float value1 = std::numeric_limits<float>::max();
		constexpr float value2 = -3;

		p1.setFloatValue(value1);
		p1.mulFloatingPoint(value2);

		QVERIFY(std::isinf(p1.floatValue()));
		QVERIFY(std::signbit(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathZero() == 0);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	// nan
	{
		constexpr float value1 = std::numeric_limits<float>::quiet_NaN();
		constexpr float value2 = 3;

		p1.setFloatValue(value1);
		p1.mulFloatingPoint(value2);

		QVERIFY(std::isnan(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathZero() == 0);
		QCOMPARE(p1.mathNan(), 1);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	// underflow
	{
		constexpr float value1 = std::numeric_limits<float>::min();
		constexpr float value2 = std::numeric_limits<float>::min();

		p1.setFloatValue(value1);
		p1.mulFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), 0.0);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 1);
		QVERIFY(p1.mathZero() == 1);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	return;
}

void SimAfbParamTests::divFloatingPointTest()
{
		Sim::AfbComponentParam p1;

	// Regular
	//
	{
		constexpr float value1 = 2.0;
		constexpr float value2 = 3.0;

		p1.setFloatValue(value1);
		p1.divFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), value1 / value2);

		QCOMPARE(p1.mathOverflow(), 0);
		QCOMPARE(p1.mathUnderflow(), 0);
		QCOMPARE(p1.mathZero(), 0);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	// div by zero
	//
	{
		constexpr float value1 = 2.0;
		constexpr float value2 = 0;

		p1.setFloatValue(value1);
		p1.divFloatingPoint(value2);

		QCOMPARE(std::isinf(p1.floatValue()), true);

		QCOMPARE(p1.mathOverflow(), 0);
		QCOMPARE(p1.mathUnderflow(), 0);
		QCOMPARE(p1.mathZero(), 0);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 1);
	}

	// 0 / normal
	//
	{
		constexpr float value1 = 0;
		constexpr float value2 = 2;

		p1.setFloatValue(value1);
		p1.divFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), 0.0);

		QCOMPARE(p1.mathOverflow(), 0);
		QCOMPARE(p1.mathUnderflow(), 0);
		QCOMPARE(p1.mathZero(), 1);
		QCOMPARE(p1.mathNan(), 0);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	// nan
	//
	{
		constexpr float value1 = std::numeric_limits<float>::quiet_NaN();
		constexpr float value2 = 0;

		p1.setFloatValue(value1);
		p1.divFloatingPoint(value2);

		QCOMPARE(std::isnan(p1.floatValue()), true);

		QCOMPARE(p1.mathOverflow(), 0);
		QCOMPARE(p1.mathUnderflow(), 0);
		QCOMPARE(p1.mathZero(), 0);
		QCOMPARE(p1.mathNan(), 1);
		QCOMPARE(p1.mathDivByZero(), 0);
	}

	return;
}

void SimAfbParamTests::addFloatingPointTest()
{
	Sim::AfbComponentParam p1;

	// Test regular +
	//
	{
		constexpr float value1 = 100;
		constexpr float value2 = -200;

		p1.setFloatValue(value1);
		p1.addFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), value1 + value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular + with 0 result
	//
	{
		constexpr float value1 = 200;
		constexpr float value2 = -200;

		p1.setFloatValue(value1);
		p1.addFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), value1 + value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular + with overflow
	//
	{
		constexpr float value1 = std::numeric_limits<float>::max();
		constexpr float value2 = std::numeric_limits<float>::max();

		p1.setFloatValue(value1);
		p1.addFloatingPoint(value2);

		QCOMPARE(std::isinf(p1.floatValue()), true);

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// -overflow
	//
	{
		constexpr float value1 = std::numeric_limits<float>::lowest();
		constexpr float value2 = std::numeric_limits<float>::lowest();

		p1.setFloatValue(value1);
		p1.addFloatingPoint(value2);

		QVERIFY(std::isinf(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// NaN
	//
	{
		constexpr float value1 = std::numeric_limits<float>::quiet_NaN();
		constexpr float value2 = 10;

		p1.setFloatValue(value1);
		p1.addFloatingPoint(value2);

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
		constexpr float value1 = std::numeric_limits<float>::infinity();
		constexpr float value2 = 1;

		p1.setFloatValue(value1);
		p1.addFloatingPoint(value2);

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

	// Test regular -
	//
	{
		constexpr float value1 = 100;
		constexpr float value2 = -200;

		p1.setFloatValue(value1);
		p1.subFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), value1 - value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular - with 0 result
	//
	{
		constexpr float value1 = 200;
		constexpr float value2 = 200;

		p1.setFloatValue(value1);
		p1.subFloatingPoint(value2);

		QCOMPARE(p1.floatValue(), value1 - value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
		QVERIFY(p1.mathNan() == 0);
	}

	// Test regular - with overflow
	//
	{
		constexpr float value1 = std::numeric_limits<float>::lowest();
		constexpr float value2 = std::numeric_limits<float>::max();

		p1.setFloatValue(value1);
		p1.subFloatingPoint(value2);

		QCOMPARE(std::isinf(p1.floatValue()), true);

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// -overflow
	//
	{
		constexpr float value1 = std::numeric_limits<float>::lowest();
		constexpr float value2 = std::numeric_limits<float>::max();

		p1.setFloatValue(value1);
		p1.subFloatingPoint(value2);

		QCOMPARE(std::isinf(p1.floatValue()), true);

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	// NaN
	//
	{
		constexpr float value1 = std::numeric_limits<float>::quiet_NaN();
		constexpr float value2 = 10;

		p1.setFloatValue(value1);
		p1.subFloatingPoint(value2);

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
		constexpr float value1 = std::numeric_limits<float>::infinity();
		constexpr float value2 = 1;

		p1.setFloatValue(value1);
		p1.subFloatingPoint(value2);

		QVERIFY(std::isinf(p1.floatValue()));

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathUnderflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
		QVERIFY(p1.mathNan() == 0);
	}

	return;
}

void SimAfbParamTests::addSignedIntegerTest()
{
	Sim::AfbComponentParam p1;

	// Test regular +
	//
	{
		const qint32 value1 = 100;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p1.addSignedInteger(value2);
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

		p1.addSignedInteger(value2);
		QVERIFY(p1.signedIntValue() == value1 + value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		constexpr qint32 value1 = std::numeric_limits<qint32>::max() - 1;
		constexpr qint32 value2 = 2;

		p1.setSignedIntValue(value1);

		p1.addSignedInteger(value2);
		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		constexpr qint32 value1 = std::numeric_limits<qint32>::lowest();
		constexpr qint32 value2 = -1;

		p1.setSignedIntValue(value1);

		p1.addSignedInteger(value2);
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

	// Test regular +
	//
	{
		const qint32 value1 = 100;
		const qint32 value2 = -200;

		p1.setSignedIntValue(value1);
		p1.subSignedInteger(value2);

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
		p1.subSignedInteger(value2);

		QVERIFY(p1.signedIntValue() == value1 - value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		constexpr qint32 value1 = std::numeric_limits<qint32>::max() - 1;
		constexpr qint32 value2 = -2;

		p1.setSignedIntValue(value1);
		p1.subSignedInteger(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		constexpr qint32 value1 = std::numeric_limits<qint32>::lowest();
		constexpr qint32 value2 = 1;

		p1.setSignedIntValue(value1);
		p1.subSignedInteger(value2);

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

	// Test regular +
	//
	{
		const qint32 value1 = 64000;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p1.mulSignedInteger(value2);

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
		p1.mulSignedInteger(value2);

		QVERIFY(p1.signedIntValue() == value1 * value2);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 1);
	}

	// Test regular + with overflow
	//
	{
		constexpr qint32 value1 = std::numeric_limits<qint32>::max()  / 2;
		constexpr qint32 value2 = 3;

		p1.setSignedIntValue(value1);
		p1.mulSignedInteger(value2);

		QVERIFY(p1.signedIntValue() == std::numeric_limits<qint32>::max());

		QVERIFY(p1.mathOverflow() == 1);
		QVERIFY(p1.mathDivByZero() == 0);
		QVERIFY(p1.mathZero() == 0);
	}

	// Test regular + with -overflow
	//
	{
		constexpr qint32 value1 = std::numeric_limits<qint32>::lowest() / 2;
		constexpr qint32 value2 = 3;

		p1.setSignedIntValue(value1);
		p1.mulSignedInteger(value2);

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

	// Test regular +
	//
	{
		const qint32 value1 = 64000;
		const qint32 value2 = 2;

		p1.setSignedIntValue(value1);
		p1.divSignedInteger(value2);

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
		p1.divSignedInteger(value2);

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
		p1.divSignedInteger(value2);

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
		p1.divSignedInteger(value2);

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
		p1.divSignedInteger(value2);

		QVERIFY(p1.signedIntValue() == -1);

		QVERIFY(p1.mathOverflow() == 0);
		QVERIFY(p1.mathDivByZero() == 1);
		QVERIFY(p1.mathZero() == 0);
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

		QCOMPARE(p.floatValue(), 123.0f);
	}

	{
		Sim::AfbComponentParam p;

		p.setFloatValue(123.0f);
		p.absFloatingPoint();

		QCOMPARE(p.floatValue(), 123.0f);
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

void SimAfbParamTests::sinFloatingPointTest()
{
	{
		Sim::AfbComponentParam p;

		p.setFloatValue(0);
		p.sinFloatingPoint();

		QCOMPARE(p.floatValue(), 0);
	}

	return;
}

void SimAfbParamTests::cosFloatingPointTest()
{
	{
		Sim::AfbComponentParam p;

		p.setFloatValue(0);
		p.cosFloatingPoint();

		QCOMPARE(p.floatValue(), 1.0);
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

		QCOMPARE(p.floatValue(), 123.0f);
	}

	// -123 -> -123.0
	{
		Sim::AfbComponentParam p;

		p.setSignedIntValue(-123);
		p.convertSignedIntToFloat();

		QCOMPARE(p.floatValue(), -123.0f);
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

		QCOMPARE(p.floatValue(), 123.0f);
	}

	// 65535 -> 65535.0
	{
		Sim::AfbComponentParam p;

		p.setWordValue(0xFFFF);
		p.convertWordToFloat();

		QCOMPARE(p.floatValue(), 65535.0f);
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

void SimAfbParamTests::afbComponentInstanceConstruct()
{
	auto afb = std::make_shared<Afb::AfbComponent>();
	afb->setOpCode(12);
	afb->setHasRam(true);
	afb->setCaption("AFB_COMP");
	afb->setMaxInstCount(256);
	afb->setSimulationFunc("SimFunc");
	afb->setVersionOpIndex(10);

	{
		Sim::AfbComponentInstance afbInst{afb, 5};

		Sim::AfbComponentParam p0{0, 0x1122};
		Sim::AfbComponentParam p1{1, 0x3344};

		afbInst.addParam(p0);
		afbInst.addParam(p1);

		afbInst.addParamWord(3, 0x5566);
		afbInst.addParamDword(4, 0x11223344);
		afbInst.addParamFloat(5, 123.0);
		afbInst.addParamDouble(6, 456.0);
		afbInst.addParamSignedInt(7, -1);
		afbInst.addParamSignedInt64(18, -999);

		QCOMPARE(afbInst.paramExists(0), true);
		QCOMPARE(afbInst.paramExists(1), true);
		QCOMPARE(afbInst.paramExists(2), false);
		QCOMPARE(afbInst.paramExists(3), true);
		QCOMPARE(afbInst.paramExists(4), true);
		QCOMPARE(afbInst.paramExists(5), true);
		QCOMPARE(afbInst.paramExists(6), true);
		QCOMPARE(afbInst.paramExists(7), true);
		QCOMPARE(afbInst.paramExists(8), false);
		QCOMPARE(afbInst.paramExists(10), false);	// not does not exist, will appear as versionOpIndex() later
		QCOMPARE(afbInst.paramExists(18), true);
		QCOMPARE(afbInst.paramExists(9999), false);

		// Test get param
		//
		auto verifyFunc = [](auto& afbInst,  quint16 opIndex, bool shouldExist)
		{
			const Sim::AfbComponentParam* param = afbInst.param(opIndex);

			if (shouldExist == true)
			{
				QVERIFY(param != nullptr);
				QCOMPARE(param->opIndex(), opIndex);
			}
			else
			{
				QVERIFY(param == nullptr);
			}
		};

		verifyFunc(afbInst, 0, true);
		verifyFunc(afbInst, 1, true);
		verifyFunc(afbInst, 10, true);		// version should be created implicitly
		verifyFunc(afbInst, 18, true);		// version should be created implicitly

		// --
		//
		afbInst.resetState();
		try
		{
			[[maybe_unused]] const Sim::AfbComponentParam* param = afbInst.param(3);

			QFAIL("Exception was expected but was not thrown");
		}
		catch (Sim::SimException&)
		{
		}
	}
}

void SimAfbParamTests::modelComponent()
{
	auto afb = std::make_shared<Afb::AfbComponent>();
	afb->setOpCode(12);
	afb->setHasRam(true);
	afb->setCaption("AFB_COMP");
	afb->setMaxInstCount(256);
	afb->setSimulationFunc("SimFunc");
	afb->setVersionOpIndex(10);
	bool ok;

	{
		Sim::ModelComponent mc;
		QCOMPARE(mc.isNull(), true);
	}

	{
		Sim::ModelComponent mc{afb};
		QCOMPARE(mc.isNull(), false);

		mc.init();
		QVERIFY(mc.instance(0) != nullptr);
		QVERIFY(mc.instance(255) != nullptr);
		QVERIFY(mc.instance(256) == nullptr);

		Sim::AfbComponentParam param2{2, 0x1122};
		Sim::AfbComponentParam param20{20, 0x3344};

		QString error;

		ok = mc.addParam(256, param2, &error);
		QCOMPARE(ok, false);
		QCOMPARE(error.isEmpty(), false);
		error.clear();

		ok = mc.addParam(10, param2, &error);
		QCOMPARE(ok, true);
		QCOMPARE(error.isEmpty(), true);

		ok = mc.addParam(10, param20, &error);
		QCOMPARE(ok, true);
		QCOMPARE(error.isEmpty(), true);

		// --
		//
		auto instance10 = mc.instance(10);
		QVERIFY(instance10 != nullptr);

		auto pi2 = instance10->param(2);
		QVERIFY(pi2 != nullptr);
		QCOMPARE(pi2->wordValue(), 0x1122);
		QCOMPARE(pi2->opIndex(), 2);

		// Reset
		//
		mc.resetState();

		try
		{
			[[maybe_unused]] auto ppp = instance10->param(2);
		}
		catch (Sim::SimException& s)
		{
			QVERIFY(s.message().startsWith("Param 2 is not found in AFB "));
		}

		return;
	}

}

void SimAfbParamTests::afbComponentSet()
{
	// --
	//
	QFile lmDescritptionFile(":/LM1_SR05.xml");
	if (lmDescritptionFile.open(QIODevice::ReadOnly | QIODevice::Text) == false)
	{
		QFAIL(lmDescritptionFile.errorString().toStdString().data());
		return;
	}

	QByteArray lmdba = lmDescritptionFile.readAll();

	QString errorMessage;

	LmDescription lmd;
	bool ok = lmd.load(lmdba, &errorMessage);
	QCOMPARE(ok, true);
	QVERIFY(errorMessage.isEmpty());
	errorMessage.clear();

	// --
	//
	Sim::AfbComponentSet set;
	ok = set.init(lmd);
	QCOMPARE(ok, true);

	Sim::AfbComponentInstance* afbLogic = set.componentInstance(1, 0);	// Afb Component LOGIC
	QVERIFY(afbLogic);

	Sim::AfbComponentInstance* afbTct = set.componentInstance(3, 22);	// Afb Component TCT
	QVERIFY(afbTct);

	Sim::AfbComponentParam param2{2, 0x1122};
	ok = set.addInstantiatorParam(3, 22, param2, &errorMessage);
	QCOMPARE(ok, true);
	QCOMPARE(errorMessage.isEmpty(), true);

	QVERIFY(afbTct->paramExists(2));

	auto p = afbTct->param(2);
	QVERIFY(p);
	QCOMPARE(p->wordValue(), 0x1122);

	// resetState
	//
	set.resetState();
	QVERIFY(afbTct->paramExists(2) == false);

	// clear
	//
	set.clear();
	QVERIFY(set.m_components.empty());

	return;
}
