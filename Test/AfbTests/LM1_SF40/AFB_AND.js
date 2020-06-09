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

// Test 1
//
function test_AFB_2AND(sim)
{
	
	sim.overrideSignalValue("#TUN_DSCR1", 0);
	sim.overrideSignalValue("#TUN_DSCR2", 0);
	sim.startForMs(5);
	assert(sim.signalValue("#2AND_OUT") === 0);
	
	sim.overrideSignalValue("#TUN_DSCR1", 1);
	sim.overrideSignalValue("#TUN_DSCR2", 0);
	sim.startForMs(5);
	assert(sim.signalValue("#2AND_OUT") === 0);
	
	sim.overrideSignalValue("#TUN_DSCR1", 0);
	sim.overrideSignalValue("#TUN_DSCR2", 1);
	sim.startForMs(5);
	assert(sim.signalValue("#2AND_OUT") === 0);
	
	sim.overrideSignalValue("#TUN_DSCR1", 1);
	sim.overrideSignalValue("#TUN_DSCR2", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#2AND_OUT") === 1);
    return;
}

function test_AFB_7AND(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.startForMs(5);
	assert(sim.signalValue("#7AND_OUT") === 0);
    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#7AND_OUT") === 0);
    
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#7AND_OUT") === 0);
    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#7AND_OUT") === 0);
    
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#7AND_OUT") === 0);
    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#7AND_OUT") === 1);
    return;
}	
function test_AFB_11AND(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
    sim.overrideSignalValue("#TUN_DSCR10", 0);
    sim.overrideSignalValue("#TUN_DSCR11", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#11AND_OUT") === 0);
    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
    sim.overrideSignalValue("#TUN_DSCR10", 0);
    sim.overrideSignalValue("#TUN_DSCR11", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#11AND_OUT") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#11AND_OUT") === 0);

    
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
    sim.overrideSignalValue("#TUN_DSCR10", 0);
    sim.overrideSignalValue("#TUN_DSCR11", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#11AND_OUT") === 0);

    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
    sim.overrideSignalValue("#TUN_DSCR10", 0);
    sim.overrideSignalValue("#TUN_DSCR11", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#11AND_OUT") === 0);

    
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#11AND_OUT") === 0);

    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#11AND_OUT") === 1);

    return;
}
function test_AFB_16AND(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
    sim.overrideSignalValue("#TUN_DSCR10", 0);
    sim.overrideSignalValue("#TUN_DSCR11", 0);
    sim.overrideSignalValue("#TUN_DSCR12", 0);
    sim.overrideSignalValue("#TUN_DSCR13", 0);
    sim.overrideSignalValue("#TUN_DSCR14", 0);
    sim.overrideSignalValue("#TUN_DSCR15", 0);
    sim.overrideSignalValue("#TUN_DSCR16", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#16AND_OUT") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
    sim.overrideSignalValue("#TUN_DSCR10", 0);
    sim.overrideSignalValue("#TUN_DSCR11", 0);
    sim.overrideSignalValue("#TUN_DSCR12", 0);
    sim.overrideSignalValue("#TUN_DSCR13", 0);
    sim.overrideSignalValue("#TUN_DSCR14", 0);
    sim.overrideSignalValue("#TUN_DSCR15", 0);
    sim.overrideSignalValue("#TUN_DSCR16", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#16AND_OUT") === 0); 
    
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 1);
    sim.overrideSignalValue("#TUN_DSCR12", 1);
    sim.overrideSignalValue("#TUN_DSCR13", 1);
    sim.overrideSignalValue("#TUN_DSCR14", 1);
    sim.overrideSignalValue("#TUN_DSCR15", 1);
    sim.overrideSignalValue("#TUN_DSCR16", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#16AND_OUT") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 1);
    sim.overrideSignalValue("#TUN_DSCR12", 1);
    sim.overrideSignalValue("#TUN_DSCR13", 1);
    sim.overrideSignalValue("#TUN_DSCR14", 1);
    sim.overrideSignalValue("#TUN_DSCR15", 1);
    sim.overrideSignalValue("#TUN_DSCR16", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#16AND_OUT") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 0);
    sim.overrideSignalValue("#TUN_DSCR12", 1);
    sim.overrideSignalValue("#TUN_DSCR13", 0);
    sim.overrideSignalValue("#TUN_DSCR14", 1);
    sim.overrideSignalValue("#TUN_DSCR15", 0);
    sim.overrideSignalValue("#TUN_DSCR16", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#16AND_OUT") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 0);
    sim.overrideSignalValue("#TUN_DSCR12", 1);
    sim.overrideSignalValue("#TUN_DSCR13", 0);
    sim.overrideSignalValue("#TUN_DSCR14", 1);
    sim.overrideSignalValue("#TUN_DSCR15", 0);
    sim.overrideSignalValue("#TUN_DSCR16", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#16AND_OUT") === 0); 

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
    sim.overrideSignalValue("#TUN_DSCR10", 1);
    sim.overrideSignalValue("#TUN_DSCR11", 1);
    sim.overrideSignalValue("#TUN_DSCR12", 1);
    sim.overrideSignalValue("#TUN_DSCR13", 1);
    sim.overrideSignalValue("#TUN_DSCR14", 1);
    sim.overrideSignalValue("#TUN_DSCR15", 1);
    sim.overrideSignalValue("#TUN_DSCR16", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#16AND_OUT") === 1);

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

