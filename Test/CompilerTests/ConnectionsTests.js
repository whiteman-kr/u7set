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

const CONN_01 = "CONN_01";

var conn01;
var conn01_port1;
var conn01_port2;

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    console.log(sim.buildPath);

    sim.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
    sim.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv

    sim.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules
	
	conn01 = sim.connection(CONN_01);
	
	assert(conn01 != null);
	
	conn01_port1 = conn01.port1Info;
	
	assert(conn01_port1 != null)
	assert(conn01_port1.equipmentID == "SYSTEMID_RACK01_FSCC01_MD00_OPTOPORT01");
	
	conn01_port2 = conn01.port2Info;

	assert(conn01_port2 != null)
	assert(conn01_port2.equipmentID == "SYSTEMID_RACK01_FSCC02_MD14_OPTOPORT04");

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

//function test_1(sim)
//{

    // Start simulation for N msecs:
    //      sim.startForMs(50);

    // Check signal value:
    //      assert(sim.signalValue("#TEST_NOT_1") === 1);

    // Override signal value:
    //      sim.overrideSignalValue("#TEST_NOT_1", 0);

    // Clear override signal list:
    //      sim.overridesReset();

    // Write message to console:
    //      log.writeText("Log Text");
    //      log.writeWarning("Log Text");

	//return;
//}

// ---------------------------------------------------------------------------------------------------------
//
// UAL_CONN_4_3* - Opto receivers tests
//
// ---------------------------------------------------------------------------------------------------------

function test_UAL_CONN_4_3_01(sim)
{
	let rxPort = conn01_port2;

	{
		let analogSignalID = "#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP";
		
		let txSignal = conn01_port1.txSignalInfo(analogSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(analogSignalID);
		
		assert(rxSignal.absAddr.offset > rxPort.rxBufferAbsAddr && rxSignal.absAddr.offset < (rxPort.rxBufferAbsAddr + rxPort.rxDataSizeW));
		
		let ualRxSignal = sim.signalParamExt("#LM2_LM1_TEMP");
		
		assert(ualRxSignal.ualAddr === rxSignal.absAddr);
	}
	
	{
		let discreteSignalID = "#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK";
		
		let txSignal = conn01_port1.txSignalInfo(discreteSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(discreteSignalID);

		assert(rxSignal.absAddr.offset > rxPort.rxBufferAbsAddr && rxSignal.absAddr.offset < (rxPort.rxBufferAbsAddr + rxPort.rxDataSizeW));
		
		let ualRxSignal = sim.signalParamExt("#LM2_LM1_BLINK");
		
		assert(ualRxSignal.ualAddr === rxSignal.absAddr);
	}
	
	{
		let busSignalID = "#BUS32_S01";
		
		let txSignal = conn01_port1.txSignalInfo(busSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(busSignalID);

		assert(rxSignal.absAddr.offset > rxPort.rxBufferAbsAddr && rxSignal.absAddr.offset < (rxPort.rxBufferAbsAddr + rxPort.rxDataSizeW));
		
		let ualRxSignal = sim.signalParamExt("#LM2_BUS32_S01");
		
		assert(ualRxSignal.ualAddr === rxSignal.absAddr);
	}
}

function test_UAL_CONN_4_3_02(sim)
{
	{
		let analogSignalID = "#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP";
		
		let txSignal = conn01_port1.txSignalInfo(analogSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(analogSignalID);
		
		assert(rxSignal.addrInBuf === txSignal.addrInBuf);
	}

	{
		let discreteSignalID = "#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK";
		
		let txSignal = conn01_port1.txSignalInfo(discreteSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(discreteSignalID);

		assert(rxSignal.addrInBuf === txSignal.addrInBuf);
	}
	
	{
		let busSignalID = "#BUS32_S01";
		
		let txSignal = conn01_port1.txSignalInfo(busSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(busSignalID);

		assert(rxSignal.addrInBuf === txSignal.addrInBuf);
	}
	
}

