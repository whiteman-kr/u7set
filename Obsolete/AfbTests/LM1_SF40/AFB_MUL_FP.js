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



function test_AFB_MUL_FP(sim)
{
	
    sim.overrideSignalValue("#TUN_IN_FP1", 2);
    sim.overrideSignalValue("#TUN_IN_FP2", 2);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_MUL_FP001") === 4);
    
    
    sim.overrideSignalValue("#TUN_IN_FP1", 25.5);
    sim.overrideSignalValue("#TUN_IN_FP2", 2.6);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_MUL_FP001") === 66.29999542236328);
    
    sim.overrideSignalValue("#TUN_IN_FP1", -45);
    sim.overrideSignalValue("#TUN_IN_FP2", 12);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_MUL_FP001") === -540);
    
    sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_MUL_FP001") === 0);
    assert(sim.signalValue("#OUT_ZERO_MUL_FP001") === 1);
    

    sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", -1.1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_MUL_FP001") === 0);
    assert(sim.signalValue("#OUT_ZERO_MUL_FP001") === 1);
    
    sim.overrideSignalValue("#TUN_IN_FP1", 0.05);
    sim.overrideSignalValue("#TUN_IN_FP2", 0.148);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_MUL_FP001"));
    assert(sim.signalValue("#OUT_MUL_FP001") === 0.007400000002235174);
 

    /*sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", Infinity);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_MUL_FP001") === NaN);*/
	
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

