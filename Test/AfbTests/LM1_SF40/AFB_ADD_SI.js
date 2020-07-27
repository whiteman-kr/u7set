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

    // Warm up all modules
    //
    sim.startForMs(5);
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
    sim.overridesReset();
    sim.startForMs(5);      // For applying overridesReset()
    return;
}



function test_AFB_ADD_SI(sim)
{
	
    sim.overrideSignalValue("#TUN_IN_SI1", 0);
    sim.overrideSignalValue("#TUN_IN_SI2", 0);
    sim.startForMs(5);
    //console.log(sim.signalValue("#OUT_ADD_SI001"));
    //console.log(sim.signalValue("#OUT_ZERO_ADD_SI001"));
    assert(sim.signalValue("#OUT_ADD_SI001") === 0);
    assert(sim.signalValue("#OUT_ZERO_ADD_SI001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
    sim.overrideSignalValue("#TUN_IN_SI2", 1);
    sim.startForMs(5);
    //console.log(sim.signalValue("#OUT_ADD_SI001"));
    assert(sim.signalValue("#OUT_ADD_SI001") === 2);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
    sim.overrideSignalValue("#TUN_IN_SI2", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_ADD_SI001") === 2);


    sim.overrideSignalValue("#TUN_IN_SI1", 1200);
    sim.overrideSignalValue("#TUN_IN_SI2", 6880);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_ADD_SI001") === 8080);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
    sim.overrideSignalValue("#TUN_IN_SI2", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_ADD_SI001") === 0);
    assert(sim.signalValue("#OUT_ZERO_ADD_SI001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 1999999999);
    sim.overrideSignalValue("#TUN_IN_SI2", 999999990);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_ADD_SI001") === 2147483647);
    assert(sim.signalValue("#OVERFLOW_ADD_SI001") === 1);
    

    return;
}



	
	
    // Start simulation for N msecs:
    //      sim.startForMs(50);

    // Check signal value:
    //      assert(sim.signalValue("#TEST_NOT_1") === 1);

    // Override signal value:
    //      sim.overrideSignalValue("#TEST_NOT_1", 0);

    // Clear override signal list:
    //      sim.overridesReset();

