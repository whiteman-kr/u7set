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

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    console.log(sim.buildPath);

    sim.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
    sim.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv

    sim.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules

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

	