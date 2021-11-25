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


function test_BUS_SIMLOCK_block_1_sim_1_in_1(sim)
{
	// Schema AFB_BUS_SIMLOCK
	
	// set all INPUTS to 1
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 1);

	// set all SIMs to 1
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 1);
	
	// set all BLOCKSs to 1
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 1);
	
	sim.startForMs(5);
	
	// check OUTPUTS
	//
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 0);
	
	// set all INPUTS to different values
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 1);
	
	sim.startForMs(5);

	// check OUTPUTS
	//
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 0);
	
	// set all SIMs to different values
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 1);
	
	sim.startForMs(5);
	
	// check OUTPUTS
	//
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 0);
	
	// reset all BLOCKs to 0
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 0);
	
	// reset all SIMs to 0 here
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 0);
	
	sim.startForMs(5);
	
	// now sim == 0, block == 0, what should be on output ???
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 1);
	
}

function test_BUS_SIMLOCK_block_0_sim_0_in_1(sim)
{
	// Schema AFB_BUS_SIMLOCK

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 1);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 0);

}

function test_BUS_SIMLOCK_block_0_sim_1_in_0(sim)
{
	// Schema AFB_BUS_SIMLOCK

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 1);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 1);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 1);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 1);
	
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 1);
	
}

function test_BUS_SIMLOCK_mix(sim)
{
	// Schema AFB_BUS_SIMLOCK

	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP01", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP01", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP02", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP02", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 0);
	
	sim.overrideSignalValue("#TUN_DSCR_BUS_01_INP03", 1);
	sim.overrideSignalValue("#TUN_DSCR_BUS_02_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_03_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_04_INP03", 0);
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP03", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_01") === 0);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_02") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_03") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_04") === 1);
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 0);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP01", 1);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 1);
	
	//
	sim.overrideSignalValue("#TUN_DSCR_BUS_05_INP02", 1);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SIMLOCK_05") === 1);	
	
}	