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

    sim.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
    sim.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv

    sim.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules

    return;
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
    sim.reset();                        // Reset module, requires 5 ms run for actual reset
    sim.overridesReset();               // Remove all signal overrides
    sim.connectionsSetEnabled(true);    // Enable all connections
    sim.startForMs(5);                  // For applying overridesReset(), and for actual reset all modules

    return;
}

	//Test
	//Schema AFB_BUS_AND element tests


function test_AND_BUS_5_Signal_0_0(sim)
{

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 0);
	
	sim.startForMs(5);
	assert(sim.signalValue("#OUT_BUS_2AND_01") === 0);
	assert(sim.signalValue("#OUT_BUS_2AND_02") === 0);
	assert(sim.signalValue("#OUT_BUS_2AND_03") === 0);
	assert(sim.signalValue("#OUT_BUS_2AND_04") === 0);
	assert(sim.signalValue("#OUT_BUS_2AND_05") === 0);
	
}


function test_AND_BUS_5_Signal_1_1(sim)
{
		
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 1);
	
	sim.startForMs(5);
	assert(sim.signalValue("#OUT_BUS_2AND_01") === 1);
	assert(sim.signalValue("#OUT_BUS_2AND_02") === 1);
	assert(sim.signalValue("#OUT_BUS_2AND_03") === 1);
	assert(sim.signalValue("#OUT_BUS_2AND_04") === 1);
	assert(sim.signalValue("#OUT_BUS_2AND_05") === 1);
	
}


function test_AND_BUS_5_Signal_1_0_1(sim)
{	

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 0);
	
	sim.startForMs(5);
	assert(sim.signalValue("#OUT_BUS_2AND_01") === 1);
	assert(sim.signalValue("#OUT_BUS_2AND_02") === 0);
	assert(sim.signalValue("#OUT_BUS_2AND_03") === 0);
	assert(sim.signalValue("#OUT_BUS_2AND_04") === 1);
	assert(sim.signalValue("#OUT_BUS_2AND_05") === 0);
	
	
    return;
}






