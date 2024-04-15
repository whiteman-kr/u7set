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



function test_AFB_TCT_OFF(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_TIME1", 1000);
    sim.startForMs(5);
	assert(sim.signalValue("#OUT_TCT_OFF") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_TIME1", 1000);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_TCT_OFF") === 1);


    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_TIME1", 5000);
    sim.startForMs(5);
	assert(sim.signalValue("#OUT_TCT_OFF") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_TIME1", 5000);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_TCT_OFF") === 1);


    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_TIME1", 10000);
    sim.startForMs(5);
	assert(sim.signalValue("#OUT_TCT_OFF") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_TIME1", 10000);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_TCT_OFF") === 1);
    

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_TIME1", 1000000);
    sim.startForMs(5);
	assert(sim.signalValue("#OUT_TCT_OFF") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_TIME1", 100000);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_TCT_OFF") === 1);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_TIME1", 0);
    sim.startForMs(5);
	assert(sim.signalValue("#PARAM_ERR_TCT_OFF") === 1);
	

    return;
    
}
function test_AFB_TCT_ON(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_TIME2", 1000);
    sim.startForMs(5);
	assert(sim.signalValue("#OUT_TCT_OFF") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_TIME2", 1000);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_TCT_ON") === 1);
    
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

