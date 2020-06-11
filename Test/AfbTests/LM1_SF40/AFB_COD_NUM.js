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

function test_AFB_COD_NUM(sim)
{
    sim.overrideSignalValue("#TUN_DSCR1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI001") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI001") === 1);
 
    return;
}

function test_AFB_2COD_NUM(sim)
{
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI002") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI002") === 1);
 
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI002") === 2);
  

    return;
}

function test_AFB_4COD(sim)
{
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI004") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI004") === 1);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI004") === 4);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI004") === 8);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI004") === 3);

    return;
}
function test_AFB_9COD(sim)
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
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI009") === 0);
	
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI009") === 1);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI009") === 2);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 0);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI009") === 4);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI009") === 480);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 1);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI009") === 511);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
    sim.overrideSignalValue("#TUN_DSCR8", 1);
    sim.overrideSignalValue("#TUN_DSCR9", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI009") === 170);

    return;
}
function test_AFB_16COD(sim)
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
    assert(sim.signalValue("#OUT_COD_NUM_SI0016") === 0);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
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
    assert(sim.signalValue("#OUT_COD_NUM_SI0016") === 1);

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
    assert(sim.signalValue("#OUT_COD_NUM_SI0016") === 65535);

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
    assert(sim.signalValue("#OUT_COD_NUM_SI0016") === 255);

    
    return;
}

function test_AFB_32COD(sim)
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
    sim.overrideSignalValue("#TUN_DSCR17", 0);
    sim.overrideSignalValue("#TUN_DSCR18", 0);
    sim.overrideSignalValue("#TUN_DSCR19", 0);
    sim.overrideSignalValue("#TUN_DSCR20", 0);
    sim.overrideSignalValue("#TUN_DSCR21", 0);
    sim.overrideSignalValue("#TUN_DSCR22", 0);
    sim.overrideSignalValue("#TUN_DSCR23", 0);
    sim.overrideSignalValue("#TUN_DSCR24", 0);
    sim.overrideSignalValue("#TUN_DSCR25", 0);
    sim.overrideSignalValue("#TUN_DSCR26", 0);
    sim.overrideSignalValue("#TUN_DSCR27", 0);
    sim.overrideSignalValue("#TUN_DSCR28", 0);
    sim.overrideSignalValue("#TUN_DSCR29", 0);
    sim.overrideSignalValue("#TUN_DSCR30", 0);
    sim.overrideSignalValue("#TUN_DSCR31", 0);
    sim.overrideSignalValue("#TUN_DSCR32", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI_0032") === 0);

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
    sim.overrideSignalValue("#TUN_DSCR17", 1);
    sim.overrideSignalValue("#TUN_DSCR18", 1);
    sim.overrideSignalValue("#TUN_DSCR19", 1);
    sim.overrideSignalValue("#TUN_DSCR20", 1);
    sim.overrideSignalValue("#TUN_DSCR21", 1);
    sim.overrideSignalValue("#TUN_DSCR22", 1);
    sim.overrideSignalValue("#TUN_DSCR23", 1);
    sim.overrideSignalValue("#TUN_DSCR24", 1);
    sim.overrideSignalValue("#TUN_DSCR25", 1);
    sim.overrideSignalValue("#TUN_DSCR26", 1);
    sim.overrideSignalValue("#TUN_DSCR27", 1);
    sim.overrideSignalValue("#TUN_DSCR28", 1);
    sim.overrideSignalValue("#TUN_DSCR29", 1);
    sim.overrideSignalValue("#TUN_DSCR30", 1);
    sim.overrideSignalValue("#TUN_DSCR31", 1);
    sim.overrideSignalValue("#TUN_DSCR32", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI_0032") === -1);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
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
    sim.overrideSignalValue("#TUN_DSCR17", 0);
    sim.overrideSignalValue("#TUN_DSCR18", 0);
    sim.overrideSignalValue("#TUN_DSCR19", 0);
    sim.overrideSignalValue("#TUN_DSCR20", 0);
    sim.overrideSignalValue("#TUN_DSCR21", 0);
    sim.overrideSignalValue("#TUN_DSCR22", 0);
    sim.overrideSignalValue("#TUN_DSCR23", 0);
    sim.overrideSignalValue("#TUN_DSCR24", 0);
    sim.overrideSignalValue("#TUN_DSCR25", 0);
    sim.overrideSignalValue("#TUN_DSCR26", 0);
    sim.overrideSignalValue("#TUN_DSCR27", 0);
    sim.overrideSignalValue("#TUN_DSCR28", 0);
    sim.overrideSignalValue("#TUN_DSCR29", 0);
    sim.overrideSignalValue("#TUN_DSCR30", 0);
    sim.overrideSignalValue("#TUN_DSCR31", 0);
    sim.overrideSignalValue("#TUN_DSCR32", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI_0032") === -2147483647);

    sim.overrideSignalValue("#TUN_DSCR1", 0);
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
    sim.overrideSignalValue("#TUN_DSCR17", 1);
    sim.overrideSignalValue("#TUN_DSCR18", 1);
    sim.overrideSignalValue("#TUN_DSCR19", 1);
    sim.overrideSignalValue("#TUN_DSCR20", 1);
    sim.overrideSignalValue("#TUN_DSCR21", 1);
    sim.overrideSignalValue("#TUN_DSCR22", 1);
    sim.overrideSignalValue("#TUN_DSCR23", 1);
    sim.overrideSignalValue("#TUN_DSCR24", 1);
    sim.overrideSignalValue("#TUN_DSCR25", 1);
    sim.overrideSignalValue("#TUN_DSCR26", 1);
    sim.overrideSignalValue("#TUN_DSCR27", 1);
    sim.overrideSignalValue("#TUN_DSCR28", 1);
    sim.overrideSignalValue("#TUN_DSCR29", 1);
    sim.overrideSignalValue("#TUN_DSCR30", 1);
    sim.overrideSignalValue("#TUN_DSCR31", 1);
    sim.overrideSignalValue("#TUN_DSCR32", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_COD_NUM_SI_0032") === 2147483646);

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

