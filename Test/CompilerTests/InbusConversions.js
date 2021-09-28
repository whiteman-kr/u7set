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

const lm1ID = "SYSTEMID_RACK01_FSCC01_MD00";
var lm1;
var lm1Description;

var constBusSignal;
var constBusUalAddr;

var varBusSignal;
var varBusUalAddr;

const SI32_CONST = 1000;

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    console.log(sim.buildPath);

    sim.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
    sim.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv

    sim.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules
	
	lm1 = sim.logicModule(lm1ID);
	assert(lm1 != null);
	
	lm1Description = sim.scriptLmDescription(lm1ID);
	assert(lm1Description != null);
	
	//
	
	constBusSignal = sim.signalParamExt("#LM1_SI32_INBUS_CONV_02");
	assert(constBusSignal != null);
	constBusUalAddr = constBusSignal.ualAddr;

	//
	
	varBusSignal = sim.signalParamExt("#LM1_SI32_INBUS_CONV_01");
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

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 123456);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === 123456);
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_NS") === 123456);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -32945879);
	sim.startForMs(5);
	assert(lm1.readRamSignedInt(addr, RamReadWriteAccess) === -32945879);
	assert(sim.signalValue("#LM1_VAR_SI32_SI32_NS") === -32945879);
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
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -34567);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === -34567);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS") === -34567);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 129381029);
	sim.startForMs(5);
	assert(lm1.readRamFloat(addr, RamReadWriteAccess) === 129381032);
	assert(sim.signalValue("#LM1_VAR_SI32_FP32_NS") === 129381032);
	
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
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === SI32_CONST);
	assert(sim.signalValue("#LM1_CONST_SI32_SI16_NS") === SI32_CONST);			

	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 8, 0);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -31500);
	sim.startForMs(5);
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === 0x84F4);
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS") === -31500);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 27100);
	sim.startForMs(5);
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === 27100);	
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS") === 27100);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 56000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === 0xDAC0);	
	assert(sim.signalValue("#LM1_VAR_SI32_SI16_NS") === -9536);
}

function test_conversion_SI32_UI16_NS(sim)
{
	// check const conversion
	
	let addr = sim.createRamAddress(constBusUalAddr.offset + 20, 0);

	sim.startForMs(5);
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === SI32_CONST);	
	assert(sim.signalValue("#LM1_CONST_SI32_UI16_NS") === SI32_CONST);		

	// check variable conversion
	
	addr = sim.createRamAddress(varBusUalAddr.offset + 20, 0);

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 1400);
	sim.startForMs(5);
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === 1400);	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS") === 1400);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 67000);					// overflow
	sim.startForMs(5);
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === 1464);	
	assert(sim.signalValue("#LM1_VAR_SI32_UI16_NS") === 1464);
	
	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -7000);					// underflow
	sim.startForMs(5);
	assert(lm1.readRamWord(addr, RamReadWriteAccess) === 58536);	
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




/*
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

*/	

