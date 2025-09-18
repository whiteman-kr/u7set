'use strict'

function assert(condition, message)
{
	if (!condition)
	{
		message = message || "Assertion failed";
		throw new Error(message);
	}
}


//
// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
	console.log(sim.buildPath);

	sim.unlockTimer = false;       // Unlocks simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
	sim.enabledLanComm = true;     // Allows or disables LogicModules' Application Data transmittion to AppDataSrv
	
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
	sim.reset();
	sim.overridesReset();
	sim.connectionsSetEnabled(true);
	sim.startForMs(5);      // For applying overridesReset(), and for actual reset all modules

	return;
}

// Start Tuning
//
function testStartTuning(sim)
{
	let lm = sim.logicModule("SYSTEMID_CLIENTTEST_CH11_MD00");
	assert(lm != undefined);

	lm.armingKey = true;
	lm.tuningKey = true;

	// Get ScriptLogicModule object for LM
	//
	lm = sim.logicModule("SYSTEMID_CLIENTTEST_CH12_MD00");
	assert(lm != undefined);

	lm.armingKey = true;
	lm.tuningKey = true;

	// TestSuite tests
	//
	lm = sim.logicModule("SYSTEMID_CLIENTTEST_CH10_MD00");
	assert(lm != undefined);

	lm.armingKey = true;
	lm.tuningKey = true;

	// Start simulation for 300 seconds
	//
	sim.startForMs(300 * 1000);	

	return;
}


