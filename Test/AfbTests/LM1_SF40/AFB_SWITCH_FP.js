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



function test_AFB_SWITCH_FP(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("TUN_IN_FP1", 2.6);
    sim.overrideSignalValue("TUN_IN_FP2", 1.0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_SWITCH_FP1") === 2.6);
    
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("TUN_IN_FP1", 999.99);
    sim.overrideSignalValue("TUN_IN_FP2", 777.77);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_SWITCH_FP1") === 999.99);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("TUN_IN_FP1", 0);
    sim.overrideSignalValue("TUN_IN_FP2", 0.99);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_SWITCH_FP1") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("TUN_IN_FP1", 1.26);
    sim.overrideSignalValue("TUN_IN_FP2", 19.87);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_SWITCH_FP1") === 1.26);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("TUN_IN_FP1", 98.6);
    sim.overrideSignalValue("TUN_IN_FP2", 156.5);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_SWITCH_FP1") === 156.5);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("TUN_IN_FP1", 1.5);
    sim.overrideSignalValue("TUN_IN_FP2", 2.5);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_SWITCH_FP1") === 2.5);

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

