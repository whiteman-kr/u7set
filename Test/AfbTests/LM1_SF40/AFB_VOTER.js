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



function test_AFB_VOTER_2_3(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER003") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS003") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST003") === 0);  

    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER003") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS003") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST003") === 1);
    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER003") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS003") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST003") === 1);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER003") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS003") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST003") === 0);
 
    return;
}

function test_AFB_VOTER_2_4(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER004") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS004") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST004") === 0); 

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER004") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS004") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST004") === 1);
    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER004") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS004") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST004") === 1); 

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER004") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS004") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST004") === 1); 

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 1);
    sim.overrideSignalValue("#TUN_DSCR4", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER004") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS004") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST004") === 1); 

    return;
}

function test_AFB_VOTER_2_7(sim)
{
	
    sim.overrideSignalValue("#TUN_DSCR1", 0);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER007") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS007") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST007") === 0);
    
    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 0);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER007") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS007") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST007") === 1);
    

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER007") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS007") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST007") === 1);


    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 0);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER007") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS007") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST007") === 1);

    sim.overrideSignalValue("#TUN_DSCR1", 1);
    sim.overrideSignalValue("#TUN_DSCR2", 1);
    sim.overrideSignalValue("#TUN_DSCR3", 0);
    sim.overrideSignalValue("#TUN_DSCR4", 0);
    sim.overrideSignalValue("#TUN_DSCR5", 0);
    sim.overrideSignalValue("#TUN_DSCR6", 1);
    sim.overrideSignalValue("#TUN_DSCR7", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_VOTER007") === 1);
    assert(sim.signalValue("#OUT_VOTER_ERR_MS007") === 0);
    assert(sim.signalValue("#OUT_VOTER_ERR_ST007") === 1);




	
	
    // Start simulation for N msecs:
    //      sim.startForMs(50);

    // Check signal value:
    //      assert(sim.signalValue("#TEST_NOT_1") === 1);

    // Override signal value:
    //      sim.overrideSignalValue("#TEST_NOT_1", 0);

    // Clear override signal list:
    //      sim.overridesReset();

