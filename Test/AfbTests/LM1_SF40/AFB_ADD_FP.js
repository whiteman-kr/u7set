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



function test_AFB_ADD_FP(sim)
{
    sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", -1);
    //console.log(sim.signalValue("#OUT_ADD_FP001"));
    //console.log(sim.signalValue("#OUT_ZERO_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === 0);
    assert(sim.signalValue("#OUT_ZERO_ADD_FP001") === 1);
    assert(sim.signalValue("#UNDERFLOW_ADD_FP001") === 0);

    
    sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    console.log(sim.signalValue("#OUT_ZERO_ADD_FP001"));
    console.log(sim.signalValue("#UNDERFLOW_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === 0);
    assert(sim.signalValue("#OUT_ZERO_ADD_FP001") === 1);
    assert(sim.signalValue("#UNDERFLOW_ADD_FP001") === 0);


   sim.overrideSignalValue("#TUN_IN_FP1", 1200.25);
    sim.overrideSignalValue("#TUN_IN_FP2", 6880.6);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    //assert(sim.signalValue("#OUT_ADD_FP001") === 8080.85);

    sim.overrideSignalValue("#TUN_IN_FP1", -458);
    sim.overrideSignalValue("#TUN_IN_FP2", 6880.6);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    //assert(sim.signalValue("#OUT_ADD_FP001") === 6422.6);


    sim.overrideSignalValue("#TUN_IN_FP1", NaN);
    sim.overrideSignalValue("#TUN_IN_FP2", NaN);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === NaN);
    assert(sim.signalValue("#OUT_NAN_ADD_FP001") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", NaN);
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === NaN);
    assert(sim.signalValue("#OUT_NAN_ADD_FP001") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", NaN);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === NaN);
    assert(sim.signalValue("#OUT_NAN_ADD_FP001") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", 1200);
    sim.overrideSignalValue("#TUN_IN_FP2", Infinity);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === Infinity);

    sim.overrideSignalValue("#TUN_IN_FP1", Infinity);
    sim.overrideSignalValue("#TUN_IN_FP2", Infinity);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === Infinity);

    sim.overrideSignalValue("#TUN_IN_FP1", Infinity);
    sim.overrideSignalValue("#TUN_IN_FP2", 125.8);
    console.log(sim.signalValue("#OUT_ADD_FP001"));
    assert(sim.signalValue("#OUT_ADD_FP001") === Infinity);
    

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

