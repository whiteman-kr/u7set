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

function isFloatsEqual(f1, f2)
{
    return f1.toFixed(3) === f2.toFixed(3);
}


function test_BUS_VOTER_2_out_of_3(sim)
{
	// Schema AFB_BUS_VOTER
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_VOTER_01") === 1);
	assert(sim.signalValue("#OUT_BUS_VOTER_02") === 1);
	assert(sim.signalValue("#OUT_BUS_VOTER_03") === 1);
	assert(sim.signalValue("#OUT_BUS_VOTER_04") === 0);
	assert(sim.signalValue("#OUT_BUS_VOTER_05") === 0);
	
	//
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_VOTER_02") === 0);
	
}

	
function test_BUS_VOTER_2_out_of_4(sim)
{
	// Schema AFB_BUS_VOTER

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 0);

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP04", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP04", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP04", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP04", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP04", 1);

	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT2_BUS_VOTER_01") === 1);
	assert(sim.signalValue("#OUT2_BUS_VOTER_02") === 1);
	assert(sim.signalValue("#OUT2_BUS_VOTER_03") === 1);
	assert(sim.signalValue("#OUT2_BUS_VOTER_04") === 0);
	assert(sim.signalValue("#OUT2_BUS_VOTER_05") === 1);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP04", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT2_BUS_VOTER_05") === 0);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP04", 1);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT2_BUS_VOTER_04") === 1);
	
}


function test_BUS_VOTER_7_out_of_11(sim)
{
	// Schema AFB_BUS_VOTER_7_11

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 0);

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP04", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP04", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP04", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP04", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP04", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP05", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP05", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP05", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP05", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP05", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP06", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP06", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP06", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP06", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP06", 1);

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP07", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP07", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP07", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP07", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP07", 1);

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP08", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP08", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP08", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP08", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP08", 1);

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP09", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP09", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP09", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP09", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP09", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP10", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP10", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP10", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP10", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP10", 0);

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP11", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP11", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP11", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP11", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP11", 1);

	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT3_BUS_VOTER_01") === 1);
	assert(sim.signalValue("#OUT3_BUS_VOTER_02") === 1);
	assert(sim.signalValue("#OUT3_BUS_VOTER_03") === 0);
	assert(sim.signalValue("#OUT3_BUS_VOTER_04") === 0);
	assert(sim.signalValue("#OUT3_BUS_VOTER_05") === 1);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP10", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP11", 1);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT3_BUS_VOTER_03") === 1);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP04", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT3_BUS_VOTER_01") === 0);

}	