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



function test_AFB_TCONV_FP(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_FP1", 0.1);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
	assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 0);
    
    sim.overrideSignalValue("#TUN_IN_FP1", 0.9);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
    assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 0);

    sim.overrideSignalValue("#TUN_IN_FP1", 1);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
    assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 1);
    
    sim.overrideSignalValue("#TUN_IN_FP1", 1.49);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
    assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 1);
    
    sim.overrideSignalValue("#TUN_IN_FP1", 1.5);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
    assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", 1.75);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
    assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", 1.99);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
    assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 1);

    sim.overrideSignalValue("#TUN_IN_FP1", 2.01);
    sim.startForMs(5);
    console.log(sim.signalValue("#OUT_TCONV_FP_SI1"));
    assert(sim.signalValue("#OUT_TCONV_FP_SI1") === 2);


    

    
    
    

    
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

