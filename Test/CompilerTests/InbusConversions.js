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

// Test 1
//
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
	assert(sim.signalValue("#LM1_RES01") === 70000);
	
	//

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 65535);
	sim.startForMs(5);

	assert(sim.signalValue("#LM1_SI32_CONV01") === 65535);
	assert(sim.signalValue("#LM1_RES01") === 131070);
	
	//

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 65536);
	sim.startForMs(5);

	assert(sim.signalValue("#LM1_SI32_CONV01") === 0);
	assert(sim.signalValue("#LM1_RES01") === 0);
	
	//

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", 65537);
	sim.startForMs(5);

	assert(sim.signalValue("#LM1_SI32_CONV01") === 1);
	assert(sim.signalValue("#LM1_RES01") === 2);

	//

	sim.overrideSignalValue("#LM1_TUN_SINT32_01", -1);
	sim.startForMs(5);

	assert(sim.signalValue("#LM1_SI32_CONV01") === 65535);
	assert(sim.signalValue("#LM1_RES01") === 131070);

    return;
}
