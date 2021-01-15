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



function test_AFB_DAMP_SI(sim)
{
    
    sim.overrideSignalValue("#TUN_IN_SI1", 0);
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_IN_SI2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DAMP_SI1") === 0);
    assert(sim.signalValue("#OUT_ZERO_DAMP_SI") === 0);
    assert(sim.signalValue("#OUT_PARAM_ERR_DAMP_SI1") === 0);
 
    sim.overrideSignalValue("#TUN_IN_SI1", 5);
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_IN_SI2", 5);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DAMP_SI1") === 5);
    assert(sim.signalValue("#OUT_ZERO_DAMP_SI") === 0);
    assert(sim.signalValue("#OUT_PARAM_ERR_DAMP_SI1") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 4);
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_IN_SI2", 4);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DAMP_SI1") === 4);
    assert(sim.signalValue("#OUT_ZERO_DAMP_SI") === 0);
    assert(sim.signalValue("#OUT_PARAM_ERR_DAMP_SI1") === 1);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 5);
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_IN_SI2", 5);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DAMP_SI1") === 5);
    assert(sim.signalValue("#OUT_ZERO_DAMP_SI") === 0);
    assert(sim.signalValue("#OUT_PARAM_ERR_DAMP_SI1") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 256);
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_IN_SI2", 5);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DAMP_SI1") === 256);
    assert(sim.signalValue("#OUT_ZERO_DAMP_SI") === 0);
    assert(sim.signalValue("#OUT_PARAM_ERR_DAMP_SI1") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 256);
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_IN_SI2", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_DAMP_SI1") === 256);
    assert(sim.signalValue("#OUT_ZERO_DAMP_SI") === 0);
    assert(sim.signalValue("#OUT_PARAM_ERR_DAMP_SI1") === 0);


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

