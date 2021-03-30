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



function test_AFB_DEC_NUM(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC_NUM001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC_NUM001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 2);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC_NUM001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC_NUM001") === 1);
    
    return;
}

function test_AFB_DEC_NUM_2(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_2DEC_NUM002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_2DEC_NUM002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_2DEC_NUM002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 155);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_2DEC_NUM002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -128);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_2DEC_NUM002") === 0);

    return;
}

function test_AFB_DEC_NUM_5(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM005") === 0);

	sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM005") === 0);

	sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM002") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM003") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM004") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM005") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 2);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM002") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 3);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM002") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 8);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM004") === 1);
    assert(sim.signalValue("#OUT_5DEC_NUM005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 16);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_5DEC_NUM005") === 1);

    return;
}
    


function test_AFB_DEC_NUM_11(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_11DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM011") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_11DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_11DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM011") === 0); 

    sim.overrideSignalValue("#TUN_IN_SI1", 12);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_11DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM003") === 1);
    assert(sim.signalValue("#OUT_11DEC_NUM004") === 1);
    assert(sim.signalValue("#OUT_11DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM011") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 1024);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_11DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_11DEC_NUM011") === 1);

    return;
}

function test_AFB_DEC_NUM_32(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM011") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM012") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM013") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM014") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM015") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM016") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM017") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM018") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM019") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM020") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM021") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM022") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM023") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM024") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM025") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM026") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM027") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM028") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM029") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM030") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM031") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC_NUM001") === 1);
    assert(sim.signalValue("#OUT_32DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM011") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM012") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM013") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM014") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM015") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM016") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM017") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM018") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM019") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM020") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM021") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM022") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM023") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM024") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM025") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM026") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM027") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM028") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM029") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM030") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM031") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM032") === 0);
    

    sim.overrideSignalValue("#TUN_IN_SI1", 8);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM004") === 1);
    assert(sim.signalValue("#OUT_32DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM011") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM012") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM013") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM014") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM015") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM016") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM017") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM018") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM019") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM020") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM021") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM022") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM023") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM024") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM025") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM026") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM027") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM028") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM029") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM030") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM031") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 32);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM006") === 1);
    assert(sim.signalValue("#OUT_32DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM008") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM011") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM012") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM013") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM014") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM015") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM016") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM017") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM018") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM019") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM020") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM021") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM022") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM023") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM024") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM025") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM026") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM027") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM028") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM029") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM030") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM031") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 128);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC_NUM001") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM002") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM003") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM004") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM005") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM006") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM007") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM008") === 1);
    assert(sim.signalValue("#OUT_32DEC_NUM009") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM010") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM011") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM012") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM013") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM014") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM015") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM016") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM017") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM018") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM019") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM020") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM021") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM022") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM023") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM024") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM025") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM026") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM027") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM028") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM029") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM030") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM031") === 0);
    assert(sim.signalValue("#OUT_32DEC_NUM032") === 0);
    
    
    
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

