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

	//Test
	//Schema AFB_BUS_SWITCH element tests


function test_BUS_SWITCH_ANALOG_0_1(sim)
{
//	sim.startForMs(5);
	
//	let f1 = 23.456788998437294;
	
//	log.writeText("f1 = " + f1);
//	log.writeText("f1.toFixed(4) = " + f1.toFixed(4));
	
	sim.overrideSignalValue("#TUN_ANALOG_BUS_01_INP02", 25.4);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_02_INP02", -111.7);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_03_INP02", -25.5);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_04_INP02", 12000);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_05_INP02", 0.5);
	
	sim.overrideSignalValue("#TUN_ANALOG_BUS_01_INP03", -1);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_02_INP03", 33.5);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_03_INP03", -20000);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_04_INP03", -1500);
	sim.overrideSignalValue("#TUN_ANALOG_BUS_05_INP03", -88.5);
	
	sim.overrideSignalValue("#TUN_DSCR1", 0);

	sim.startForMs(5);
	
	assert(isFloatsEqual(sim.signalValue("#OUT_BUS_SWITCH_01"), 25.4) === true);
	assert(isFloatsEqual(sim.signalValue("#OUT_BUS_SWITCH_02"), -111.7) === true);
	assert(sim.signalValue("#OUT_BUS_SWITCH_03") === -25.5);
	assert(sim.signalValue("#OUT_BUS_SWITCH_04") === 12000);
	assert(sim.signalValue("#OUT_BUS_SWITCH_05") === 0.5);
	
	
	sim.overrideSignalValue("#TUN_DSCR1", 1);

	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SWITCH_01") === -1);
	assert(sim.signalValue("#OUT_BUS_SWITCH_02") === 33.5);
	assert(sim.signalValue("#OUT_BUS_SWITCH_03") === -20000);
	assert(sim.signalValue("#OUT_BUS_SWITCH_04") === -1500);
	assert(sim.signalValue("#OUT_BUS_SWITCH_05") === -88.5);
	
}

function test_BUS_SWITCH_DSCR_0_1(sim)
{
	
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
	
	
	sim.overrideSignalValue("#TUN_DSCR1", 0);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_01") === 1);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_02") === 0);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_03") === 0);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_04") === 1);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_05") === 0);
	
	
	sim.overrideSignalValue("#TUN_DSCR1", 1);
	
	sim.startForMs(5);
	
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_01") === 1);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_02") === 1);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_03") === 1);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_04") === 0);
	assert(sim.signalValue("#OUT_BUS_SWITCH_D_05") === 0);
	
}


function test_BUS_SWITCH_MIX_0_1(sim)
{
	
	sim.overrideSignalValue("#TUN_IN_FP1", -0.7);
	sim.overrideSignalValue("#TUN_IN_SI1", -20000);
	sim.overrideSignalValue("#TUN_UAL_BUS_IN1", 1);
	sim.overrideSignalValue("#TUN_UAL_BUS_IN2", 0);
	
	sim.overrideSignalValue("#TUN_IN_FP2", 99.9);
	sim.overrideSignalValue("#TUN_IN_SI2", -80000);
	sim.overrideSignalValue("#TUN_UAL_BUS_IN3", 0);
	sim.overrideSignalValue("#TUN_UAL_BUS_IN4", 1);
	

	sim.overrideSignalValue("#TUN_DSCR1", 0);
	
	sim.startForMs(5);
	
	assert(isFloatsEqual(sim.signalValue("#OUT2_BUS_SWITCH_01"), -0.7) === true);
	assert(sim.signalValue("#OUT2_BUS_SWITCH_02") === -20000);
	assert(sim.signalValue("#OUT2_BUS001") === 1);
	assert(sim.signalValue("#OUT2_BUS002") === 0);


	sim.overrideSignalValue("#TUN_DSCR1", 1);

	sim.startForMs(5);

	assert(isFloatsEqual(sim.signalValue("#OUT2_BUS_SWITCH_01"), 99.9) === true);
	assert(sim.signalValue("#OUT2_BUS_SWITCH_02") === -80000);
	assert(sim.signalValue("#OUT2_BUS001") === 0);
	assert(sim.signalValue("#OUT2_BUS002") === 1);

	return;
}	


