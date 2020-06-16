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



function test_AFB_DEC(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC001") === 1);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC001") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 3);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC001") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 16);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_1DEC001") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 100);
	sim.startForMs(5);
	assert(sim.signalValue("#OUT_1DEC001") === 0);
    
    return;
}

function test_AFB_2DEC(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC001") === 1);
    assert(sim.signalValue("#OUT_2DEC002") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC001") === 0);
    assert(sim.signalValue("#OUT_2DEC002") === 1);
    
    sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC001") === 0);
    assert(sim.signalValue("#OUT_2DEC002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 2);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC001") === 0);
    assert(sim.signalValue("#OUT_2DEC002") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 16);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_2DEC001") === 0);
    assert(sim.signalValue("#OUT_2DEC002") === 0);
    
    sim.overrideSignalValue("#TUN_IN_SI1", 128);
	sim.startForMs(5);
	assert(sim.signalValue("#OUT_2DEC001") === 0);
    assert(sim.signalValue("#OUT_2DEC002") === 0);
    return;
}


function test_AFB_5DEC(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 1);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 0);
    assert(sim.signalValue("#OUT_5DEC004") === 0);
    assert(sim.signalValue("#OUT_5DEC005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 0);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 0);
    assert(sim.signalValue("#OUT_5DEC004") === 0);
    assert(sim.signalValue("#OUT_5DEC005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 2);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 0);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 1);
    assert(sim.signalValue("#OUT_5DEC004") === 0);
    assert(sim.signalValue("#OUT_5DEC005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 3);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 0);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 0);
    assert(sim.signalValue("#OUT_5DEC004") === 1);
    assert(sim.signalValue("#OUT_5DEC005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 4);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 0);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 0);
    assert(sim.signalValue("#OUT_5DEC004") === 0);
    assert(sim.signalValue("#OUT_5DEC005") === 1);

    
    sim.overrideSignalValue("#TUN_IN_SI1", 10);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 0);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 0);
    assert(sim.signalValue("#OUT_5DEC004") === 0);
    assert(sim.signalValue("#OUT_5DEC005") === 0);


   sim.overrideSignalValue("#TUN_IN_SI1", 128);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 0);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 0);
    assert(sim.signalValue("#OUT_5DEC004") === 0);
    assert(sim.signalValue("#OUT_5DEC005") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 65536);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_5DEC001") === 0);
    assert(sim.signalValue("#OUT_5DEC002") === 0);
    assert(sim.signalValue("#OUT_5DEC003") === 0);
    assert(sim.signalValue("#OUT_5DEC004") === 0);
    assert(sim.signalValue("#OUT_5DEC005") === 0);
    return;
}

function test_AFB_16DEC(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_16DEC001") === 1);
    assert(sim.signalValue("#OUT_16DEC002") === 0);
    assert(sim.signalValue("#OUT_16DEC003") === 0);
    assert(sim.signalValue("#OUT_16DEC004") === 0);
    assert(sim.signalValue("#OUT_16DEC005") === 0);
    assert(sim.signalValue("#OUT_16DEC006") === 0);
    assert(sim.signalValue("#OUT_16DEC007") === 0);
    assert(sim.signalValue("#OUT_16DEC008") === 0);
    assert(sim.signalValue("#OUT_16DEC009") === 0);
    assert(sim.signalValue("#OUT_16DEC010") === 0);
    assert(sim.signalValue("#OUT_16DEC011") === 0);
    assert(sim.signalValue("#OUT_16DEC012") === 0);
    assert(sim.signalValue("#OUT_16DEC013") === 0);
    assert(sim.signalValue("#OUT_16DEC014") === 0);
    assert(sim.signalValue("#OUT_16DEC015") === 0);
    assert(sim.signalValue("#OUT_16DEC016") === 0);

	sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_16DEC001") === 0);
    assert(sim.signalValue("#OUT_16DEC002") === 0);
    assert(sim.signalValue("#OUT_16DEC003") === 0);
    assert(sim.signalValue("#OUT_16DEC004") === 0);
    assert(sim.signalValue("#OUT_16DEC005") === 0);
    assert(sim.signalValue("#OUT_16DEC006") === 0);
    assert(sim.signalValue("#OUT_16DEC007") === 0);
    assert(sim.signalValue("#OUT_16DEC008") === 0);
    assert(sim.signalValue("#OUT_16DEC009") === 0);
    assert(sim.signalValue("#OUT_16DEC010") === 0);
    assert(sim.signalValue("#OUT_16DEC011") === 0);
    assert(sim.signalValue("#OUT_16DEC012") === 0);
    assert(sim.signalValue("#OUT_16DEC013") === 0);
    assert(sim.signalValue("#OUT_16DEC014") === 0);
    assert(sim.signalValue("#OUT_16DEC015") === 0);
    assert(sim.signalValue("#OUT_16DEC016") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_16DEC001") === 0);
    assert(sim.signalValue("#OUT_16DEC002") === 1);
    assert(sim.signalValue("#OUT_16DEC003") === 0);
    assert(sim.signalValue("#OUT_16DEC004") === 0);
    assert(sim.signalValue("#OUT_16DEC005") === 0);
    assert(sim.signalValue("#OUT_16DEC006") === 0);
    assert(sim.signalValue("#OUT_16DEC007") === 0);
    assert(sim.signalValue("#OUT_16DEC008") === 0);
    assert(sim.signalValue("#OUT_16DEC009") === 0);
    assert(sim.signalValue("#OUT_16DEC010") === 0);
    assert(sim.signalValue("#OUT_16DEC011") === 0);
    assert(sim.signalValue("#OUT_16DEC012") === 0);
    assert(sim.signalValue("#OUT_16DEC013") === 0);
    assert(sim.signalValue("#OUT_16DEC014") === 0);
    assert(sim.signalValue("#OUT_16DEC015") === 0);
    assert(sim.signalValue("#OUT_16DEC016") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 15);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_16DEC001") === 0);
    assert(sim.signalValue("#OUT_16DEC002") === 0);
    assert(sim.signalValue("#OUT_16DEC003") === 0);
    assert(sim.signalValue("#OUT_16DEC004") === 0);
    assert(sim.signalValue("#OUT_16DEC005") === 0);
    assert(sim.signalValue("#OUT_16DEC006") === 0);
    assert(sim.signalValue("#OUT_16DEC007") === 0);
    assert(sim.signalValue("#OUT_16DEC008") === 0);
    assert(sim.signalValue("#OUT_16DEC009") === 0);
    assert(sim.signalValue("#OUT_16DEC010") === 0);
    assert(sim.signalValue("#OUT_16DEC011") === 0);
    assert(sim.signalValue("#OUT_16DEC012") === 0);
    assert(sim.signalValue("#OUT_16DEC013") === 0);
    assert(sim.signalValue("#OUT_16DEC014") === 0);
    assert(sim.signalValue("#OUT_16DEC015") === 0);
    assert(sim.signalValue("#OUT_16DEC016") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 64);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_16DEC001") === 0);
    assert(sim.signalValue("#OUT_16DEC002") === 0);
    assert(sim.signalValue("#OUT_16DEC003") === 0);
    assert(sim.signalValue("#OUT_16DEC004") === 0);
    assert(sim.signalValue("#OUT_16DEC005") === 0);
    assert(sim.signalValue("#OUT_16DEC006") === 0);
    assert(sim.signalValue("#OUT_16DEC007") === 0);
    assert(sim.signalValue("#OUT_16DEC008") === 0);
    assert(sim.signalValue("#OUT_16DEC009") === 0);
    assert(sim.signalValue("#OUT_16DEC010") === 0);
    assert(sim.signalValue("#OUT_16DEC011") === 0);
    assert(sim.signalValue("#OUT_16DEC012") === 0);
    assert(sim.signalValue("#OUT_16DEC013") === 0);
    assert(sim.signalValue("#OUT_16DEC014") === 0);
    assert(sim.signalValue("#OUT_16DEC015") === 0);
    assert(sim.signalValue("#OUT_16DEC016") === 0);
    return;
}

function test_AFB_32DEC(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_SI1", 0);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 1);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 1);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -1);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    
    sim.overrideSignalValue("#TUN_IN_SI1", 2);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 1);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 7);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 1);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 12);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 1);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 16);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 1);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 27);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 1);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -100);
	sim.startForMs(5);
    assert(sim.signalValue("#OUT_32DEC001") === 0);
    assert(sim.signalValue("#OUT_32DEC002") === 0);
    assert(sim.signalValue("#OUT_32DEC003") === 0);
    assert(sim.signalValue("#OUT_32DEC004") === 0);
    assert(sim.signalValue("#OUT_32DEC005") === 0);
    assert(sim.signalValue("#OUT_32DEC006") === 0);
    assert(sim.signalValue("#OUT_32DEC007") === 0);
    assert(sim.signalValue("#OUT_32DEC008") === 0);
    assert(sim.signalValue("#OUT_32DEC009") === 0);
    assert(sim.signalValue("#OUT_32DEC010") === 0);
    assert(sim.signalValue("#OUT_32DEC011") === 0);
    assert(sim.signalValue("#OUT_32DEC012") === 0);
    assert(sim.signalValue("#OUT_32DEC013") === 0);
    assert(sim.signalValue("#OUT_32DEC014") === 0);
    assert(sim.signalValue("#OUT_32DEC015") === 0);
    assert(sim.signalValue("#OUT_32DEC016") === 0);
    assert(sim.signalValue("#OUT_32DEC017") === 0);
    assert(sim.signalValue("#OUT_32DEC018") === 0);
    assert(sim.signalValue("#OUT_32DEC019") === 0);
    assert(sim.signalValue("#OUT_32DEC020") === 0);
    assert(sim.signalValue("#OUT_32DEC021") === 0);
    assert(sim.signalValue("#OUT_32DEC022") === 0);
    assert(sim.signalValue("#OUT_32DEC023") === 0);
    assert(sim.signalValue("#OUT_32DEC024") === 0);
    assert(sim.signalValue("#OUT_32DEC025") === 0);
    assert(sim.signalValue("#OUT_32DEC026") === 0);
    assert(sim.signalValue("#OUT_32DEC027") === 0);
    assert(sim.signalValue("#OUT_32DEC028") === 0);
    assert(sim.signalValue("#OUT_32DEC029") === 0);
    assert(sim.signalValue("#OUT_32DEC030") === 0);
    assert(sim.signalValue("#OUT_32DEC031") === 0);
    assert(sim.signalValue("#OUT_32DEC032") === 0);


    
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

