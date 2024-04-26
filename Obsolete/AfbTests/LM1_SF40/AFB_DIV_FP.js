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



function test_AFB_DIV_FP(sim)
{

    sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
    sim.startForMs(5);
    assert(isNaN(sim.signalValue("#OUT_DIV_FP001")) === true);
    assert(sim.signalValue("#OUT_NAN_DIV_FP001") === 1);
    
    sim.overrideSignalValue("#TUN_IN_FP1", 1);
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DIV_FP001") === Number.POSITIVE_INFINITY);
    assert(sim.signalValue("#OUT_DIV_BY_ZERO_DIV_FP001") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", -1);  
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DIV_FP001") === Number.NEGATIVE_INFINITY);
    assert(sim.signalValue("#OUT_DIV_BY_ZERO_DIV_FP001") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", Infinity);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DIV_FP001") === 0);
    assert(sim.signalValue("#OUT_ZERO_DIV_FP001") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", 150,5);
    sim.overrideSignalValue("#TUN_IN_FP2", 5);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_DIV_FP001") === 30,10);

    /*sim.overrideSignalValue("#TUN_IN_FP1", 100,1);
    sim.overrideSignalValue("#TUN_IN_FP2", 10);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_DIV_FP001"));
    assert(sim.signalValue("#OUT_DIV_FP001") === 10.01);*/

    sim.overrideSignalValue("#TUN_IN_FP1", Infinity);
    sim.overrideSignalValue("#TUN_IN_FP2", 9.87);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DIV_FP001") === Number.POSITIVE_INFINITY);

    sim.overrideSignalValue("#TUN_IN_FP1", Infinity);
    sim.overrideSignalValue("#TUN_IN_FP2", Infinity);
	sim.startForMs(5);
    assert(isNaN(sim.signalValue("#OUT_DIV_FP001")) === true);
    assert(sim.signalValue("#OUT_NAN_DIV_FP001") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", NaN);
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
	sim.startForMs(5);
    assert(isNaN(sim.signalValue("#OUT_DIV_FP001")) === true);
    assert(sim.signalValue("#OUT_NAN_DIV_FP001") === 1);
    assert(sim.signalValue("#OUT_DIV_BY_ZERO_DIV_FP001") === 0)

    sim.overrideSignalValue("#TUN_IN_FP1", 200,1);
    sim.overrideSignalValue("#TUN_IN_FP2", 10);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_DIV_FP001"));
    assert((sim.signalValue("#OUT_DIV_FP001")) >= 20.000 && (sim.signalValue("#OUT_DIV_FP001")) <= 20.01);
   

    /*sim.overrideSignalValue("#TUN_IN_FP1", 0);
    sim.overrideSignalValue("#TUN_IN_FP2", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_DIV_FP001") === Infinity);
    assert(sim.signalValue("#OVERFLOW_DIV_FP001") === 0);
    assert(sim.signalValue("#UNDERFLOW_DIV_FP001") === 0);
    assert(sim.signalValue("#OUT_ZERO_DIV_FP001") === 0);
    assert(sim.signalValue("#OUT_NAN_DIV_FP001") === 0);
    assert(sim.signalValue("#OUT_DIV_BY_ZERO_DIV_FP001") === 0);*/


    return;

	
    // Start simulation for N msecs:
    //      sim.startForMs(50);

    // Check signal value:
    //      assert(sim.signalValue("#TEST_NOT_1") === 1);

    // Override signal value:
    //      sim.overrideSignalValue("#TEST_NOT_1", 0);

    // Clear override signal list:
    //      sim.overridesReset();

}