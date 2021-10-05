'use strict'

const RamReadAccess = 1;        // Use these constants as param for ReadRam*/WriteRam* functions
const RamWriteAccess = 2;
const RamReadWriteAccess = 3;

function assert(condition, message)
{
    if (!condition)
    {
        message = message || "Assertion failed";
        throw new Error(message);
    }
}

var lm1;
var lm1Description;

const SI32_CONST = 1000;
const SI32_VAR_SIGNAL_ID = "#LM1_TUN_SINT32";

var si32ConstBusUalAddr;
var si32VarBusUalAddr;

const FP32_CONST = 1234.5;
const FP32_VAR_SIGNAL_ID = "#LM1_TUN_FP32";

var fp32ConstBusUalAddr;
var fp32VarBusUalAddr;

var devUtils;

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    console.log(sim.buildPath);

    sim.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
    sim.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv

    sim.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules
	
	devUtils = sim.devUtils();
    assert(devUtils !== null);
	
	let lm1ID = "SYSTEMID_RACK01_FSCC01_MD00";
	
	lm1 = sim.logicModule(lm1ID);
    assert(lm1 !== null);
	
	lm1Description = sim.scriptLmDescription(lm1ID);
    assert(lm1Description !== null);
	
	//
	
    let si32ConstBusSignal = sim.signalParamExt("#LM1_CONST_SI32_INBUS_CONV");
    assert(si32ConstBusSignal !== null);
    si32ConstBusUalAddr = si32ConstBusSignal.ualAddr;

	//
	
    let si32VarBusSignal = sim.signalParamExt("#LM1_VAR_SI32_INBUS_CONV");
    assert(si32VarBusSignal !== null);
    si32VarBusUalAddr = si32VarBusSignal.ualAddr;

    //

    let fp32ConstBusSignal = sim.signalParamExt("#LM1_CONST_FP32_INBUS_CONV");
    assert(fp32ConstBusSignal !== null);
    fp32ConstBusUalAddr = fp32ConstBusSignal.ualAddr;

    //

    let fp32VarBusSignal = sim.signalParamExt("#LM1_VAR_FP32_INBUS_CONV");
    assert(fp32VarBusSignal !== null);
    fp32VarBusUalAddr = fp32VarBusSignal.ualAddr;

    return;
}

// cleanupTestCase() - will be called after the last test function was executed.
//
function cleanupTestCase(sim)
{
}

// init() - will be called before each test function is executed.
//
function init(sim)
{
}

// cleanup() - will be called after every test function.
//
function cleanup(sim)
{
    sim.reset();                        // Reset module, requires 5 ms run for actual reset
    sim.overridesReset();               // Remove all signal overrides
    sim.connectionsSetEnabled(true);    // Enable all connections
    sim.startForMs(5);                  // For applying overridesReset(), and for actual reset all modules

    return;
}

function test_conversion_SI32_SI32_NS(sim)
{
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 12, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === SI32_CONST);
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_NS") === SI32_CONST);

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI32_NS";
	
    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 12, 0);
	
	let v = 123456;

    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -32945879;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
}

function test_conversion_SI32_FP32_NS(sim)
{
	// check const conversion
	
    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 0, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === SI32_CONST);
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_NS") === SI32_CONST);

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_FP32_NS";
	
    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 0, 0);
	
	let v = -34567;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = 1223344;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, -999912345);
	sim.startForMs(5);
    assert(lm1.readRamFloat(addr, RamReadWriteAccess) === -999912320);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === -999912320);		// not precise value
}

function test_conversion_SI32_SI16_NS(sim)
{
	// check const conversion
	
    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 8, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === SI32_CONST);
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_NS") === SI32_CONST);			

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI16_NS";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 8, 0);
	
	let v = -31500;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = 27100;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v);	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, 56000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === -9536);	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === -9536);
}

function test_conversion_SI32_UI16_NS(sim)
{
	// check const conversion
	
    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 20, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === SI32_CONST);	
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_NS") === SI32_CONST);		

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_UI16_NS";
	
    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 20, 0);
	
	let v = 1400;

    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === v);	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, 67000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 1464);	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 1464);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, -7000);					// underflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 58536);	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 58536);
}

function test_conversion_SI32_SI32_SC(sim)
{
	// inbus = 2 * x + 20
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 16, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === SI32_CONST * 2 + 20);	
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_SC") === SI32_CONST);
	
	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI32_SC";
	
    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 16, 0);
	
	let v = 12000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v * 2 + 20);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -3000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v * 2 + 20);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
}

function test_conversion_SI32_FP32_SC(sim)
{
	// inbus = 0.5 * x - 5
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 4, 0);

	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === SI32_CONST * 0.5 - 5);	
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_SC") === SI32_CONST);
	
	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_FP32_SC";
	
    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 4, 0);
	
	let v = 11000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v * 0.5 - 5);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -3100;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v * 0.5 - 5);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = 85458247;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === 42729120);		// not precise value
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 85458248);			// not precise value
}

function test_conversion_SI32_SI16_SC(sim)
{
	// inbus = 0.25 * x + 5
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 10, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === SI32_CONST * 0.25 + 5);	
    assert(sim.signalValue("#LM1_CONST_SI32_SI16_SC") === SI32_CONST);
	
	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI16_SC";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 10, 0);
	
	let v = 11000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v * 0.25 + 5);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -3100;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v * 0.25 + 5);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, 111);
	sim.startForMs(5);
    assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === 32);	// truncated value of 32.75
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 108);					// not precise value
}

function test_conversion_SI32_UI16_SC(sim)
{
	// inbus = 1.1 * x + 10
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 22	, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === (SI32_CONST * 1.1 + 10) - 1);	// not precise value
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_SC") === SI32_CONST - 1);						// not precise value
	
	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_UI16_SC";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 22, 0);
	
	let v = 55000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 60508);		// not precise value
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v - 2);					// not precise value
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, 3);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 13);			// truncated value of 13.3
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 2);						// not precise value
}

function test_conversion_SI32_SI32_NS_BO(sim)
{
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 14, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(SI32_CONST));
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_NS_BO") === SI32_CONST);

	// check variable conversion
	
    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI32_NS_BO";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 14, 0);
	
	let v = 87654321;

    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -1234567;

    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
}

function test_conversion_SI32_FP32_NS_BO(sim)
{
	// check const conversion
	
    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 2, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === devUtils.reverseFloat(SI32_CONST));
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_NS_BO") === SI32_CONST);

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_FP32_NS_BO";
	
    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 2, 0);
	
	let v = -345670;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === devUtils.reverseFloat(v));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = 1220000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === devUtils.reverseFloat(v));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, -999912345);
	sim.startForMs(5);
    assert(lm1.readRamFloat(addr, RamReadWriteAccess) === devUtils.reverseFloat(-999912320));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === -999912320);		// not precise value
}

function test_conversion_SI32_SI16_NS_BO(sim)
{
	// check const conversion
	
    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 9, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(SI32_CONST));
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_NS_BO") === SI32_CONST);			

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI16_NS_BO";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 9, 0);
	
	let v = -1500;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = 17100;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v));	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, 56000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(-9536));	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === -9536);
}

function test_conversion_SI32_UI16_NS_BO(sim)
{
	// check const conversion
	
    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 21, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(SI32_CONST));	
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_NS") === SI32_CONST);		

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_UI16_NS_BO";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 21, 0);
	
	let v = 54000;

    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(v));	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, 67000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(1464));	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 1464);
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, -7000);					// underflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(58536));	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 58536);
}

function test_conversion_SI32_SI32_SC_BO(sim)
{
	// inbus = -1 * x + 0
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 18, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(SI32_CONST * (-1)));	
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_SC") === SI32_CONST);

	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI32_SC_BO";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 18, 0);
	
	let v = 12000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v * (-1)));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -3000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v * (-1)));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
}

function test_conversion_SI32_FP32_SC_BO(sim)
{
	// inbus = -2 * x - 20
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 6, 0);

	sim.startForMs(5);
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_SC_BO") === SI32_CONST);
	
	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_FP32_SC_BO";
	
    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 6, 0);
	
	let v = 11019;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -3133;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = 2147483616;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 2147483647);			// not precise value
}

function test_conversion_SI32_SI16_SC_BO(sim)
{
	// inbus = 1 * x - 2
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 11, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(SI32_CONST * 1 - 2));	
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_SC_BO") === SI32_CONST);
	
	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_SI16_SC_BO";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 11, 0);
	
	let v = 11111;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v * 1 - 2));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = -3122;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v * 1 - 2));
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
	
	v = 0;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v * 1 - 2));	
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === v);
}

function test_conversion_SI32_UI16_SC_BO(sim)
{
	// inbus = 0.333333333333 * x + 0
	
	let K = 0.333333333333;
	
	// check const conversion

    let addr = sim.createRamAddress(si32ConstBusUalAddr.offset + 23	, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(SI32_CONST * K));
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_SC_BO") === SI32_CONST - 1);						// not precise value
	
	// check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_SI32_UI16_SC_BO";

    addr = sim.createRamAddress(si32VarBusUalAddr.offset + 23, 0);
	
	let v = 44000;
	
    sim.overrideSignalValue(SI32_VAR_SIGNAL_ID, v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(14665));		// not precise value, precise = 14666
    assert(sim.signalValue(VAR_OUT_SIGNAL_ID) === 43995);												// not precise value
}

// ---------------------------------------------------------------------------------------------------------------------------

function isFloatsEqual(f1, f2)
{
    return f1.toFixed(3) === f2.toFixed(3);
}

function test_conversion_FP32_FP32_NS(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 0, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(lm1.readRamFloat(addr, RamReadWriteAccess), FP32_CONST));
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_FP32_NS"), FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_FP32_NS";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 0, 0);

    let v = 3546.9;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(lm1.readRamFloat(addr, RamReadWriteAccess), v));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));

    v = -329.923;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(lm1.readRamFloat(addr, RamReadWriteAccess), v));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));
}

function test_conversion_FP32_FP32_NS_BO(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 2, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(devUtils.reverseFloat(lm1.readRamFloat(addr, RamReadWriteAccess)), FP32_CONST));
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_FP32_NS_BO"), FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_FP32_NS_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 2, 0);

    let v = 35.1;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(devUtils.reverseFloat(lm1.readRamFloat(addr, RamReadWriteAccess)), v));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));

    v = -29.9;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(devUtils.reverseFloat(lm1.readRamFloat(addr, RamReadWriteAccess)), v));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));
}

function test_conversion_FP32_FP32_SC(sim)
{
    // inbus = 2 * x + 100

    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 4, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(lm1.readRamFloat(addr, RamReadWriteAccess), FP32_CONST * 2.0 + 100.0));
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_FP32_SC"), FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_FP32_SC";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 4, 0);

    let v = 546.9;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(lm1.readRamFloat(addr, RamReadWriteAccess), v * 2.0 + 100.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));

    v = -309.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(lm1.readRamFloat(addr, RamReadWriteAccess), v * 2.0 + 100.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));
}

function test_conversion_FP32_FP32_SC_BO(sim)
{
    // inbus = -1.5 * x - 100

    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 6, 0);

    sim.startForMs(5);
    assert(devUtils.reverseFloat(lm1.readRamFloat(addr, RamReadWriteAccess)) === FP32_CONST * (-1.5) - 100.0);
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_FP32_SC_BO"), FP32_CONST));      // not fully precise equality

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_FP32_SC_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 6, 0);

    let v = 500.5;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(devUtils.reverseFloat(lm1.readRamFloat(addr, RamReadWriteAccess)), v * (-1.5) - 100.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));

    v = -1300.4;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(devUtils.reverseFloat(lm1.readRamFloat(addr, RamReadWriteAccess)), v * (-1.5) - 100.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));
}

function test_conversion_FP32_SI16_NS(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 8, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt16(addr, RamReadWriteAccess).toString()) + 0.5, FP32_CONST));
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI16_NS") + 0.5, FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI16_NS";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 8, 0);

    let v = 1122.7;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt16(addr, RamReadWriteAccess).toString()), 1123.0));     // round value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 1123.0));                                          // round value

    v = -3322.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt16(addr, RamReadWriteAccess).toString()), -3322.0));    // truncated value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), -3322.0));                                         // truncated value
}

function test_conversion_FP32_SI16_NS_BO(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 9, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt16(lm1.readRamSignedInt16(addr, RamReadWriteAccess)).toString()) + 0.5, FP32_CONST));
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI16_NS_BO") + 0.5, FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI16_NS_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 9, 0);

    let v = 1122.7;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt16(lm1.readRamSignedInt16(addr, RamReadWriteAccess)).toString()), 1123.0));     // round value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 1123.0));                                          // round value

    v = -3322.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt16(lm1.readRamSignedInt16(addr, RamReadWriteAccess)).toString()), -3322.0));    // truncated value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), -3322.0));                                         // truncated value
}

function test_conversion_FP32_SI16_SC(sim)
{
    // inbus = 0.5 * x + 50
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 10, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt16(addr, RamReadWriteAccess).toString()), parseFloat("667.0")));  // truncated value of 667.25
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI16_SC") + 0.5, FP32_CONST));        // not precise value

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI16_SC";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 10, 0);

    let v = 2000.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt16(addr, RamReadWriteAccess).toString()), 1050.0));     // truncated value of 1050.1
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 2000.0));                                          // not precise value

    v = -3000.4;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt16(addr, RamReadWriteAccess).toString()), -1450.0));    // truncated value of -1550.2
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), -3000));                                           // not precise value
}

function test_conversion_FP32_SI16_SC_BO(sim)
{
    // inbus = 0.2 * x - 30
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 11, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt16(lm1.readRamSignedInt16(addr, RamReadWriteAccess)).toString()),
                         parseFloat("217.0")));  // round value of 216.9
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI16_SC_BO"), FP32_CONST + 0.5));        // not precise value

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI16_SC_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 11, 0);

    let v = -5000.0;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt16(lm1.readRamSignedInt16(addr, RamReadWriteAccess)).toString()),
                         v * 0.2 - 30));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), v));
}

function test_conversion_FP32_SI32_NS(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 12, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt(addr, RamReadWriteAccess).toString()) + 0.5, FP32_CONST));   // not precise value
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI32_NS") + 0.5, FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI32_NS";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 12, 0);

    let v = 1102.7;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt(addr, RamReadWriteAccess).toString()), 1103.0));     // round value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 1103.0));                                          // round value

    v = -4300.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt(addr, RamReadWriteAccess).toString()), -4300.0));    // truncated value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), -4300.0));                                         // truncated value
}

function test_conversion_FP32_SI32_NS_BO(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 14, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt32(lm1.readRamSignedInt(addr, RamReadWriteAccess)).toString()) + 0.5, FP32_CONST));   // not precise value
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI32_NS_BO") + 0.5, FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI32_NS_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 14, 0);

    let v = 1102.7;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt32(lm1.readRamSignedInt(addr, RamReadWriteAccess)).toString()), 1103.0));     // round value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 1103.0));                                          // round value

    v = -4300.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt32(lm1.readRamSignedInt(addr, RamReadWriteAccess)).toString()), -4300.0));    // truncated value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), -4300.0));                                         // truncated value
}

function test_conversion_FP32_SI32_SC(sim)
{
    // inbus = 0.4 * x + 0
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 16, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt(addr, RamReadWriteAccess).toString()), parseFloat("494.0")));  // round value 493.8
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI32_SC"), FP32_CONST + 0.5));        // not precise value

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI32_SC";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 16, 0);

    let v = 2000.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt(addr, RamReadWriteAccess).toString()), 800.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 2000.0));                                          // not precise value

    v = -4000.4;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamSignedInt(addr, RamReadWriteAccess).toString()), -1600.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), -4000));                                           // not precise value
}

function test_conversion_FP32_SI32_SC_BO(sim)
{
    // inbus = 0.8 * x + 0
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 18, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt32(lm1.readRamSignedInt(addr, RamReadWriteAccess)).toString()),
                         parseFloat("988.0")));  // round value 987.6
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_SI32_SC_BO"), FP32_CONST + 0.5));        // not precise value

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_SI32_SC_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 18, 0);

    let v = 2000.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt32(lm1.readRamSignedInt(addr, RamReadWriteAccess)).toString()), 1600.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 2000.0));                                          // not precise value

    v = -4000.4;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseSignedInt32(lm1.readRamSignedInt(addr, RamReadWriteAccess)).toString()), -3200.0));
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), -4000));                                           // not precise value
}

function test_conversion_FP32_UI16_NS(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 20, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess).toString()) + 0.5, FP32_CONST));
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_UI16_NS") + 0.5, FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_UI16_NS";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 20, 0);

    let v = 57000.7;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess).toString()), 57001.0));     // round value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 57001.0));                                         // round value
}

function test_conversion_FP32_UI16_NS_BO(sim)
{
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 21, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseUnsignedInt16(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess)).toString()) + 0.5,
                         FP32_CONST));
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_UI16_NS_BO") + 0.5, FP32_CONST));

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_UI16_NS_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 21, 0);

    let v = 47000.7;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseUnsignedInt16(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess)).toString()),
                         47001.0));     // round value
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 47001.0));                                         // round value
}

function test_conversion_FP32_UI16_SC(sim)
{
    // inbus = 0.5 * x + 10
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 22, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess).toString()), parseFloat("627.0")));  // truncated value of 627.25
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_UI16_SC") + 0.5, FP32_CONST));        // not precise value

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_UI16_SC";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 22, 0);

    let v = 2000.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess).toString()), 1010.0));     // truncated value of 1010.1
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 2000.0));                                          // not precise value
}

function test_conversion_FP32_UI16_SC_BO(sim)
{
    // inbus = 0.5 * x + 10
    // check const conversion

    let addr = sim.createRamAddress(fp32ConstBusUalAddr.offset + 23, 0);

    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseUnsignedInt16(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess)).toString()),
                         parseFloat("627.0")));  // truncated value of 627.25
    assert(isFloatsEqual(sim.signalValue("#LM1_CONST_FP32_UI16_SC_BO") + 0.5, FP32_CONST));        // not precise value

    // check variable conversion

    const VAR_OUT_SIGNAL_ID = "#LM1_VAR_FP32_UI16_SC_BO";

    addr = sim.createRamAddress(fp32VarBusUalAddr.offset + 23, 0);

    let v = 2000.2;

    sim.overrideSignalValue(FP32_VAR_SIGNAL_ID, v);
    sim.startForMs(5);
    assert(isFloatsEqual(parseFloat(devUtils.reverseUnsignedInt16(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess)).toString()),
                         1010.0));     // truncated value of 1010.1
    assert(isFloatsEqual(sim.signalValue(VAR_OUT_SIGNAL_ID), 2000.0));                                          // not precise value
}

// ---------------------------------------------------------------------------------------------------------------------------

function test_SI32_TO_UI16_INBUS_CONVERSION(sim)
{
    // Set filler1 to non-zero value
    //
    sim.overrideSignalValue("#LM1_TUN_SINT32_FILLER", 33);

    // Set values of input Signed Int 32 signal
    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_01", 0);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV01") === 0);
    assert(sim.signalValue("#LM1_RES01") === 0);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_01", 35000);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV01") === 35000);
    assert(sim.signalValue("#LM1_RES01") === 35000 * 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_01", 65535);			// Highest UInt16 value
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV01") === 65535);
    assert(sim.signalValue("#LM1_RES01") === 65535 * 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_01", 65536);			// UInt16 overflow!
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV01") === 0);
    assert(sim.signalValue("#LM1_RES01") === 0);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_01", 65537);			// UInt16 overflow!
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV01") === 1);
    assert(sim.signalValue("#LM1_RES01") === 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_01", -1);				// UInt16 underflow!
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV01") === 65535);
    assert(sim.signalValue("#LM1_RES01") === 65535 * 2);

    return;
}

function test_SI32_TO_SI16_INBUS_CONVERSION(sim)
{
    // Set filler2 to non-zero value
    //
    sim.overrideSignalValue("#LM1_TUN_SINT32_FILLER", 77);

    // Set values of input Signed Int 32 signal
    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_02", 0);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV02") === 0);
    assert(sim.signalValue("#LM1_RES02") === 0);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_02", 4321);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV02") === 4321);
    assert(sim.signalValue("#LM1_RES02") === 4321 * 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_02", 32767);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV02") === 32767);			// SInt16 highest positive value
    assert(sim.signalValue("#LM1_RES02") === 32767 * 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_02", 32768);			// SInt16 overflow!
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV02") === -32768);
    assert(sim.signalValue("#LM1_RES02") === -32768 * 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_02", -456);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV02") === -456);
    assert(sim.signalValue("#LM1_RES02") === -456 * 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_02", -32768);			// SInt16 lowest negative value
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV02") === -32768);
    assert(sim.signalValue("#LM1_RES02") === -32768 * 2);

    //

    sim.overrideSignalValue("#LM1_TUN_SINT32_02", -32769);			// SInt16 underflow!
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_SI32_CONV02") === 32767);
    assert(sim.signalValue("#LM1_RES02") === 32767 * 2);

    return;
}

function test_AUTO_SIGNAL_FROMBUS_CONVERSION(sim)
{
    // Set filler1, filler2 to non-zero value
    //
    sim.overrideSignalValue("#LM1_TUN_SINT32_FILLER", 200002);

    // Set values of input Unsigned Int 32 signal
    //
    sim.overrideSignalValue("#LM1_TUN_SINT32_01", 321);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_RES03") === 321 - 10);
    assert(sim.signalValue("#LM1_RES04") === 321 + 10);

    assert(sim.signalValue("#LM1_FILLER_RES01") === 200002);

    // Set values of input Signed Int 32 signal
    //
    sim.overrideSignalValue("#LM1_TUN_SINT32_02", -765);
    sim.startForMs(5);

    assert(sim.signalValue("#LM1_RES05") === -765 - (-11));
    assert(sim.signalValue("#LM1_RES06") === -765 + (-11));

    assert(sim.signalValue("#LM1_FILLER_RES02") === 200002);

    return;
}
