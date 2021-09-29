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

var constBusSignal;
var constBusUalAddr;

var varBusSignal;
var varBusUalAddr;

var devUtils;

const SI32_CONST = 1000;

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    console.log(sim.buildPath);

    sim.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
    sim.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv

    sim.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules
	
	devUtils = sim.devUtils();
	assert(devUtils != null);
	
	let lm1ID = "SYSTEMID_RACK01_FSCC01_MD00";
	
	lm1 = sim.logicModule(lm1ID);
	assert(lm1 != null);
	
	lm1Description = sim.scriptLmDescription(lm1ID);
	assert(lm1Description != null);
	
	//
	
	constBusSignal = sim.signalParamExt("#LM1_CONST_SI32_INBUS_CONV");
	assert(constBusSignal != null);
	constBusUalAddr = constBusSignal.ualAddr;

	//
	
	varBusSignal = sim.signalParamExt("#LM1_VAR_SI32_INBUS_CONV");
	assert(varBusSignal != null);
	varBusUalAddr = varBusSignal.ualAddr;

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

	let addr = sim.createRamAddress(constBusUalAddr.offset + 12, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === SI32_CONST);
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_NS") === SI32_CONST);

	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 12, 0);
	
	let v = 123456;

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v);
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_NS") === v);
	
	v = -32945879;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v);
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_NS") === v);
}

function test_conversion_SI32_FP32_NS(sim)
{
	// check const conversion
	
	let addr = sim.createRamAddress(constBusUalAddr.offset + 0, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === SI32_CONST);
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_NS") === SI32_CONST);

	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 0, 0);
	
	let v = -34567;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS") === v);
	
	v = 1223344;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS") === v);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -999912345);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) == -999912320);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS") === -999912320);		// not precise value
}

function test_conversion_SI32_SI16_NS(sim)
{
	// check const conversion
	
	let addr = sim.createRamAddress(constBusUalAddr.offset + 8, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === SI32_CONST);
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_NS") === SI32_CONST);			

	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 8, 0);
	
	let v = -31500;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v);
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS") === v);
	
	v = 27100;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v);	
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS") === v);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 56000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === -9536);	
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS") === -9536);
}

function test_conversion_SI32_UI16_NS(sim)
{
	// check const conversion
	
	let addr = sim.createRamAddress(constBusUalAddr.offset + 20, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === SI32_CONST);	
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_NS") === SI32_CONST);		

	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 20, 0);
	
	let v = 1400;

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === v);	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS") === v);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 67000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 1464);	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS") === 1464);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -7000);					// underflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 58536);	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS") === 58536);
}

function test_conversion_SI32_SI32_SC(sim)
{
	// inbus = 2 * x + 20
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 16, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === SI32_CONST * 2 + 20);	
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_SC") === SI32_CONST);
	
	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 16, 0);
	
	let v = 12000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v * 2 + 20);
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_SC") === v);
	
	v = -3000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === v * 2 + 20);
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_SC") === v);
}

function test_conversion_SI32_FP32_SC(sim)
{
	// inbus = 0.5 * x - 5
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 4, 0);

	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === SI32_CONST * 0.5 - 5);	
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_SC") === SI32_CONST);
	
	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 4, 0);
	
	let v = 11000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v * 0.5 - 5);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_SC") === v);
	
	v = -3100;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === v * 0.5 - 5);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_SC") === v);
	
	v = 85458247;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === 42729120);		// not precise value
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_SC") === 85458248);			// not precise value
}

function test_conversion_SI32_SI16_SC(sim)
{
	// inbus = 0.25 * x + 5
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 10, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === SI32_CONST * 0.25 + 5);	
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_SC") === SI32_CONST);
	
	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 10, 0);
	
	let v = 11000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v * 0.25 + 5);
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_SC") === v);
	
	v = -3100;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === v * 0.25 + 5);
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_SC") === v);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 111);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === 32);			// truncated value of 32.75
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_SC") === 108);					// not precise value
}

function test_conversion_SI32_UI16_SC(sim)
{
	// inbus = 1.1 * x + 10
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 22	, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === (SI32_CONST * 1.1 + 10) - 1);	// not precise value
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_SC") === SI32_CONST - 1);						// not precise value
	
	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 22, 0);
	
	let v = 55000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 60508);		// not precise value
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_SC") === v - 2);					// not precise value
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 3);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === 13);			// truncated value of 13.3
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_SC") === 2);						// not precise value
}


function test_conversion_SI32_SI32_NS_BO(sim)
{
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 14, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(SI32_CONST));
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_NS_BO") === SI32_CONST);

	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 14, 0);
	
	let v = 87654321;

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v));
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_NS_BO") === v);
	
	v = -1234567;

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v));
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_NS_BO") === v);
}

function test_conversion_SI32_FP32_NS_BO(sim)
{
	// check const conversion
	
	let addr = sim.createRamAddress(constBusUalAddr.offset + 2, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === devUtils.reverseFloat(SI32_CONST));
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_NS_BO") === SI32_CONST);

	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 2, 0);
	
	let v = -345670;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === devUtils.reverseFloat(v));
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS_BO") === v);
	
	v = 1220000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === devUtils.reverseFloat(v));
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS_BO") === v);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -999912345);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) == devUtils.reverseFloat(-999912320));
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS_BO") === -999912320);		// not precise value 
}

function test_conversion_SI32_SI16_NS_BO(sim)
{
	// check const conversion
	
	let addr = sim.createRamAddress(constBusUalAddr.offset + 9, 0);
	
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(SI32_CONST));
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_NS_BO") === SI32_CONST);			

	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 9, 0);
	
	let v = -1500;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v));
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS_BO") === v);
	
	v = 17100;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v));	
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS_BO") === v);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 56000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(-9536));	
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS_BO") === -9536);
}

function test_conversion_SI32_UI16_NS_BO(sim)
{
	// check const conversion
	
	let addr = sim.createRamAddress(constBusUalAddr.offset + 21, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(SI32_CONST));	
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_NS") === SI32_CONST);		

	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 21, 0);
	
	let v = 54000;

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(v));	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS_BO") === v);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 67000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(1464));	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS_BO") === 1464);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -7000);					// underflow
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(58536));	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS_BO") === 58536);
}

function test_conversion_SI32_SI32_SC_BO(sim)
{
	// inbus = -1 * x + 0
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 18, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(SI32_CONST * (-1)));	
	assert(sim.signalValue("#LM1_CONST_SI32_SI32_SC") === SI32_CONST);

	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 18, 0);
	
	let v = 12000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v * (-1)));
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_SC_BO") === v);
	
	v = -3000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === devUtils.reverseSignedInt32(v * (-1)));
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_SC_BO") === v);
}

function test_conversion_SI32_FP32_SC_BO(sim)
{
	// inbus = -2 * x - 20
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 6, 0);

	sim.startForMs(5);
	assert(sim.signalValue("#LM1_CONST_SI32_FP32_SC_BO") === SI32_CONST);
	
	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 6, 0);
	
	let v = 11019;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_SC_BO") === v);
	
	v = -3133;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_SC_BO") === v);
	
	v = 2147483616;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_SC_BO") === 2147483647);			// not precise value
}

function test_conversion_SI32_SI16_SC_BO(sim)
{
	// inbus = 1 * x - 2
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 11, 0);

	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(SI32_CONST * 1 - 2));	
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_SC_BO") === SI32_CONST);
	
	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 11, 0);
	
	let v = 11111;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v * 1 - 2));
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_SC_BO") === v);
	
	v = -3122;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v * 1 - 2));
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_SC_BO") === v);
	
	v = 0;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt16(addr, RamReadWriteAccess) === devUtils.reverseSignedInt16(v * 1 - 2));	
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_SC_BO") === v);		
}

function test_conversion_SI32_UI16_SC_BO(sim)
{
	// inbus = 0.333333333333 * x + 0
	
	let K = 0.333333333333;
	
	// check const conversion

	let addr = sim.createRamAddress(constBusUalAddr.offset + 23	, 0);

	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(SI32_CONST * K));
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_SC_BO") === SI32_CONST - 1);						// not precise value
	
	// check variable conversion

	addr = sim.createRamAddress(varBusUalAddr.offset + 23, 0);
	
	let v = 44000;
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", v);
	sim.startForMs(5);
	assert(lm1.readRamUnsignedInt16(addr, RamReadWriteAccess) === devUtils.reverseUnsignedInt16(14665));		// not precise value, precise = 14666
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_SC_BO") === 43995);												// not precise value
}


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

