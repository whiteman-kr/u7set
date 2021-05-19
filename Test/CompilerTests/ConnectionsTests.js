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
// UAL_CONN_4_1* - Opto connections general tests
//
// ---------------------------------------------------------------------------------------------------------

function test_UAL_CONN_4_1_01(sim)
{
	conn01_port1.isTxSignalExist("#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP");
	conn01_port2.isRxSignalExist("#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP");
	
	conn01_port1.isTxSignalExist("#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK");
	conn01_port2.isRxSignalExist("#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK");

	conn01_port1.isTxSignalExist("#BUS32_S01");
	conn01_port2.isRxSignalExist("#BUS32_S01");
}

function test_UAL_CONN_4_1_02(sim)
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

function test_UAL_CONN_4_1_03(sim)
{
	// Signals of modules SYSTEMID_RACK01_FSCC02_MD00 (LM) and SYSTEMID_RACK01_FSCC02_MD14 (OCM) are not added to AppSignals
	// Opto validity signals should be created automatically
	//
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD00_PI_OPTOPORT01VALID") === true);
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD00_PI_OPTOPORT02VALID") === true);
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD00_PI_OPTOPORT03VALID") === true);
	
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT01VALID") === true);
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT02VALID") === true);
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT03VALID") === true);
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT04VALID") === true);
	assert(sim.signalExists("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT05VALID") === true);
}

function test_UAL_CONN_4_1_04(sim)
{
	{
		sim.overrideSignalValue("#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP", 36.0);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#LM2_LM1_TEMP") === 36.0);
		assert(sim.signalValue("#LM2_LM1_TEMP2") === 36.0);
		assert(sim.signalValue("#LM2_LM1_TEMP3") === 36.0);
		assert(sim.signalValue("#LM2_LM1_TEMP4") === 36.0);
	
		sim.overrideSignalValue("#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP", 15.0);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#LM2_LM1_TEMP") === 15.0);
		assert(sim.signalValue("#LM2_LM1_TEMP2") === 15.0);
		assert(sim.signalValue("#LM2_LM1_TEMP3") === 15.0);
		assert(sim.signalValue("#LM2_LM1_TEMP4") === 15.0);
	}
	
	{
		sim.overrideSignalValue("#LM1_DS01", 1);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#LM1_DS01") === 1);
	
		sim.overrideSignalValue("#LM1_DS01", 0);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#LM1_DS01") === 0);
		
	}
	
	{
		sim.overrideSignalValue("#LM1_BUS_00", 1);
		sim.overrideSignalValue("#LM1_BUS_01", 0);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#OUT_BUS32_B00") === 1);
		assert(sim.signalValue("#OUT_BUS32_B11") === 1);
		assert(sim.signalValue("#OUT_BUS32_B16") === 1);
		assert(sim.signalValue("#OUT_BUS32_B31") === 1);
		
		assert(sim.signalValue("#OUT_BUS32_B03") === 0);
		assert(sim.signalValue("#OUT_BUS32_B15") === 0);
		assert(sim.signalValue("#OUT_BUS32_B18") === 0);
		
		sim.overrideSignalValue("#LM1_BUS_00", 0);
		sim.overrideSignalValue("#LM1_BUS_01", 1);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#OUT_BUS32_B00") === 0);
		assert(sim.signalValue("#OUT_BUS32_B11") === 0);
		assert(sim.signalValue("#OUT_BUS32_B16") === 0);
		assert(sim.signalValue("#OUT_BUS32_B31") === 0);
		
		assert(sim.signalValue("#OUT_BUS32_B03") === 1);
		assert(sim.signalValue("#OUT_BUS32_B15") === 1);
		assert(sim.signalValue("#OUT_BUS32_B18") === 1);
	}
	
	sim.overridesReset();
}

// ---------------------------------------------------------------------------------------------------------
//
// UAL_CONN_4_2* - Opto transmitters tests
//
// ---------------------------------------------------------------------------------------------------------

function test_UAL_CONN_4_2_01(sim)
{
	assert(conn01_port1.isTxSignalExist("#LM1_DS03") === true);
	assert(conn01_port2.isRxSignalExist("#LM1_DS03") === true);
	
	assert(conn01_port1.isTxSignalExist("#LM1_DS02") === true);
	assert(conn01_port2.isRxSignalExist("#LM1_DS02") === true);
	
	sim.overrideSignalValue("#LM1_DS02", 0);
	
	sim.startForMs(10);
	
	assert(sim.signalValue("#LM2_DS02") === 0);
	assert(sim.signalValue("#LM2_DS03") === 0);


	sim.overrideSignalValue("#LM1_DS03", 1);
	
	sim.startForMs(10);
	
	assert(sim.signalValue("#LM2_DS02") === 1);
	assert(sim.signalValue("#LM2_DS03") === 1);
}

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
		let rxSignal1 = conn01_port2.rxSignalInfo("#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP");
		
		let ualRxSignal1 = sim.signalParamExt("#LM2_LM1_TEMP");
		let ualRxSignal2 = sim.signalParamExt("#LM2_LM1_TEMP2");
		let ualRxSignal3 = sim.signalParamExt("#LM2_LM1_TEMP3");
		let ualRxSignal4 = sim.signalParamExt("#LM2_LM1_TEMP4");
		
		assert(rxSignal1.absAddr == ualRxSignal1.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal2.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal3.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal4.ualAddr);
	}
	
	{
		let rxSignal1 = conn01_port2.rxSignalInfo("#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK");
		
		let ualRxSignal1 = sim.signalParamExt("#LM2_LM1_BLINK");
		let ualRxSignal2 = sim.signalParamExt("#LM2_LM1_BLINK2");
		let ualRxSignal3 = sim.signalParamExt("#LM2_LM1_BLINK3");
		let ualRxSignal4 = sim.signalParamExt("#LM2_LM1_BLINK4");
		
		assert(rxSignal1.absAddr == ualRxSignal1.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal2.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal3.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal4.ualAddr);
	}
	
	{
		let rxSignal1 = conn01_port2.rxSignalInfo("#BUS32_S01");
		
		let ualRxSignal1 = sim.signalParamExt("#LM2_BUS32_S01");
		let ualRxSignal2 = sim.signalParamExt("#LM2_BUS32_S01_2");
		let ualRxSignal3 = sim.signalParamExt("#LM2_BUS32_S01_3");
		let ualRxSignal4 = sim.signalParamExt("#LM2_BUS32_S01_4");
		
		assert(rxSignal1.absAddr == ualRxSignal1.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal2.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal3.ualAddr);
		assert(rxSignal1.absAddr == ualRxSignal4.ualAddr);
	}
}

function test_UAL_CONN_4_3_03(sim)
{
	let rxValiditySignalAbsAddr = conn01_port2.rxValiditySignalAbsAddr;
	
	let ualRxValSignal1 = sim.signalParamExt("#LM2_CONN_01_VALIDITY");
	let ualRxValSignal2 = sim.signalParamExt("#LM2_CONN_01_VALIDITY2");
	let ualRxValSignal3 = sim.signalParamExt("#LM2_CONN_01_VALIDITY3");
	let ualRxValSignal4 = sim.signalParamExt("#LM2_CONN_01_VALIDITY4");
	
	let ualAutoRxValSignal = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT04VALID");		// auto created validity signal
												 
	assert(ualAutoRxValSignal != null);

	assert(rxValiditySignalAbsAddr == ualAutoRxValSignal.ualAddr);
	assert(rxValiditySignalAbsAddr == ualRxValSignal1.ualAddr);
	assert(rxValiditySignalAbsAddr == ualRxValSignal2.ualAddr);
	assert(rxValiditySignalAbsAddr == ualRxValSignal3.ualAddr);
	assert(rxValiditySignalAbsAddr== ualRxValSignal4.ualAddr);
}


