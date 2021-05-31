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

const lm1ID = "SYSTEMID_RACK01_FSCC01_MD00";
var lm1;
var lm1Description;


const lm2ID = "SYSTEMID_RACK01_FSCC02_MD00";
var lm2;
var lm2Description;

var conn01;
var conn01_port1;
var conn01_port2;

var conn02;
var conn02_port1;
var conn02_port2;

var conn03;
var conn03_port1;
var conn03_port2;

var conn04;
var conn04_port1;
var conn04_port2;

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    
	log.writeText("Log Text");
	
    sim.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
    sim.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv

    sim.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules
	
	//
	
	lm1 = sim.logicModule(lm1ID);
	assert(lm1 != null);
	
	lm1Description = sim.scriptLmDescription(lm1ID);
	assert(lm1Description != null);
	
	//
	
	lm2 = sim.logicModule(lm2ID);
	assert(lm2 != null);
	
	lm2Description = sim.scriptLmDescription(lm2ID);
	assert(lm2Description != null);

	//
	
	conn01 = sim.connection("CONN_01");
	
	assert(conn01 != null);
	
	conn01_port1 = conn01.port1Info;
	
	assert(conn01_port1 != null)
	assert(conn01_port1.equipmentID === "SYSTEMID_RACK01_FSCC01_MD00_OPTOPORT01");
	
	conn01_port2 = conn01.port2Info;

	assert(conn01_port2 != null)
	assert(conn01_port2.equipmentID === "SYSTEMID_RACK01_FSCC02_MD14_OPTOPORT04");
	
	//
	
	conn02 = sim.connection("CONN_02");
	
	assert(conn02 != null);
	
	conn02_port1 = conn02.port1Info;
	
	assert(conn02_port1 != null)
	assert(conn02_port1.equipmentID === "SYSTEMID_RACK01_FSCC01_MD11_OPTOPORT02");
	
	conn02_port2 = conn02.port2Info;

	assert(conn02_port2 != null)
	assert(conn02_port2.equipmentID === "SYSTEMID_RACK01_FSCC02_MD14_OPTOPORT03");
	
	//
	
	conn03 = sim.connection("CONN_03");
	
	assert(conn03 != null);
	
	conn03_port1 = conn03.port1Info;
	
	assert(conn03_port1 != null)
	assert(conn03_port1.equipmentID === "SYSTEMID_RACK01_FSCC01_MD00_OPTOPORT02");
	
	conn03_port2 = conn03.port2Info;

	assert(conn03_port2 != null)
	assert(conn03_port2.equipmentID === "SYSTEMID_RACK01_FSCC02_MD00_OPTOPORT02");

	//
	
	conn04 = sim.connection("CONN_04");
	
	assert(conn04 != null);
	
	conn04_port1 = conn04.port1Info;
	
	assert(conn04_port1 != null)
	assert(conn04_port1.equipmentID === "SYSTEMID_RACK01_FSCC01_MD11_OPTOPORT01");
	
	conn04_port2 = conn04.port2Info;

	assert(conn04_port2 != null)
	assert(conn04_port2.equipmentID === "SYSTEMID_RACK01_FSCC02_MD14_OPTOPORT05");
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
	let sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD00_PI_OPTOPORT01VALID");
	assert(sg != null);									// check - signal is exists
	assert(sg.regBufAddr.isValid === true);				// checks - signal is placed in reg buf
	assert(sg.regValueAddr.isValid === true);			//
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD00_PI_OPTOPORT02VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	

	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD00_PI_OPTOPORT03VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	//
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT01VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	

	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT02VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT03VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT04VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT05VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	// Signals of modules SYSTEMID_RACK01_FSCC01_MD00 (LM) and SYSTEMID_RACK01_FSCC01_MD11 (OCM) are added to AppSignals
	// Opto validity signals should be also exists
	//
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD00_PI_OPTOPORT01VALID");
	assert(sg != null);								
	assert(sg.regBufAddr.isValid === true);				
	assert(sg.regValueAddr.isValid === true);
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD00_PI_OPTOPORT02VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	

	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD00_PI_OPTOPORT03VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	//
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT01VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	

	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT02VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	let sg2 = sim.signalParamExt("#LM1_CONN_02_VALIDITY");			// same as #SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT02VALID
	assert(sg2 != null);
	assert(sg2.regBufAddr.isValid === true);
	assert(sg2.regValueAddr.isValid === true);	
	
	assert(sg.regBufAddr === sg2.regBufAddr);
	assert(sg.regValueAddr === sg2.regValueAddr);
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT03VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT04VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
	
	sg = sim.signalParamExt("#SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT05VALID");
	assert(sg != null);
	assert(sg.regBufAddr.isValid === true);
	assert(sg.regValueAddr.isValid === true);	
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
		
		assert(sim.signalValue("#LM2_DS01") === 1);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#LM1_DS01_RETURNED") === 1);
	
		sim.overrideSignalValue("#LM1_DS01", 0);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#LM2_DS01") === 0);
		
		sim.startForMs(10);
		
		assert(sim.signalValue("#LM1_DS01_RETURNED") === 0);
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

function test_UAL_CONN_4_1_05(sim)
{
	sim.overridesReset();

	sim.connectionsSetEnabled(true);
	sim.startForMs(10);
	
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC01_MD00_PI_OPTOPORT01VALID") === 1);
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT04VALID") === 1);
	
	conn01.enabled = false;
	sim.startForMs(20);
			
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC01_MD00_PI_OPTOPORT01VALID") === 0);
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT04VALID") === 0);
	
	sim.connectionsSetEnabled(true);
	sim.startForMs(10);
							
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC01_MD00_PI_OPTOPORT01VALID") === 1);
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT04VALID") === 1);
	
	conn02.enabled = false;
	sim.startForMs(20);
	
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT02VALID") === 0);
	assert(sim.signalValue("#LM1_CONN_02_VALIDITY") === 0);									// same as prev signal
	
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT03VALID") === 0);
	
	sim.connectionsSetEnabled(true);
	sim.startForMs(10);

	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC01_MD11_PI_OPTOPORT02VALID") === 1);
	assert(sim.signalValue("#LM1_CONN_02_VALIDITY") === 1);
	
	assert(sim.signalValue("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT03VALID") === 1);
}

function test_UAL_CONN_4_1_06(sim)
{
	// conn01_port1 is not receive any signal from conn01_port2
	// only dataID should be receive
	//
	assert(conn01_port1.rxDataSizeW === 2);
	assert(conn01_port2.txDataSizeW === 2);
	
	sim.connectionsSetEnabled(true);
	sim.startForMs(15);
	
	let receivedDataID = lm1.readRamDword(conn01_port1.rxBufAbsAddr, RamReadAccess);
	
	assert(receivedDataID === conn01_port1.rxDataID);
	assert(receivedDataID === conn01_port2.txDataID);
	
	let transmittedDataID = lm2.readRamDword(conn01_port2.txBufAbsAddr, RamWriteAccess);
	
	assert(receivedDataID === transmittedDataID);
	
	assert(transmittedDataID === conn01_port1.rxDataID);
	assert(transmittedDataID === conn01_port2.txDataID);
}

function test_UAL_CONN_4_1_07(sim)
{
	assert(conn01.dataIDControlDisabled === false);

	sim.connectionsSetEnabled(true);
	sim.startForMs(15);

	// LM1
	
	let receivedDataID = lm1.readRamDword(conn01_port1.rxBufAbsAddr, RamReadAccess);
	
	assert(receivedDataID != 0);
	assert(receivedDataID === conn01_port1.rxDataID);
	assert(receivedDataID === conn01_port2.txDataID);
	
	transmittedDataID = lm1.readRamDword(conn01_port1.txBufAbsAddr, RamWriteAccess);
	
	assert(transmittedDataID != 0);
	assert(transmittedDataID === conn01_port1.txDataID);
	assert(transmittedDataID === conn01_port2.rxDataID);
	
	// LM2
	
	receivedDataID = lm2.readRamDword(conn01_port2.rxBufAbsAddr, RamReadAccess);
	
	assert(receivedDataID != 0);
	assert(receivedDataID === conn01_port2.rxDataID);
	assert(receivedDataID === conn01_port1.txDataID);
	
	let transmittedDataID = lm2.readRamDword(conn01_port2.txBufAbsAddr, RamWriteAccess);
	
	assert(transmittedDataID != 0);
	assert(transmittedDataID === conn01_port1.rxDataID);
	assert(transmittedDataID === conn01_port2.txDataID);
}

function test_UAL_CONN_4_1_08(sim)
{
	assert(conn02.dataIDControlDisabled === true);

	sim.connectionsSetEnabled(true);
	sim.startForMs(15);

	// LM1
	
	let receivedDataID = lm1.readRamDword(conn02_port1.rxBufAbsAddr, RamReadAccess);
	
	assert(receivedDataID === 0);
	assert(receivedDataID === conn02_port1.rxDataID);
	assert(receivedDataID === conn02_port2.txDataID);
	
	transmittedDataID = lm1.readRamDword(conn02_port1.txBufAbsAddr, RamWriteAccess);
	
	assert(transmittedDataID === 0);
	assert(transmittedDataID === conn02_port1.txDataID);
	assert(transmittedDataID === conn02_port2.rxDataID);
	
	// LM2
	
	receivedDataID = lm2.readRamDword(conn02_port2.rxBufAbsAddr, RamReadAccess);
	
	assert(receivedDataID === 0);
	assert(receivedDataID === conn02_port2.rxDataID);
	assert(receivedDataID === conn02_port1.txDataID);
	
	let transmittedDataID = lm2.readRamDword(conn02_port2.txBufAbsAddr, RamWriteAccess);
	
	assert(transmittedDataID === 0);
	assert(transmittedDataID === conn02_port1.rxDataID);
	assert(transmittedDataID === conn02_port2.txDataID);
}

function test_UAL_CONN_4_1_09(sim)
{
	assert(conn02.manualSettingsEnabled === true);
	
	//
	
	let optoModuleStartAddr = lm1Description.ioModuleBufStartAddr(11);
	assert(conn02_port1.txBufAbsAddr.offset === optoModuleStartAddr.offset + conn02_port1.manualTxStartAddr);
	
	//
	
	optoModuleStartAddr = lm2Description.ioModuleBufStartAddr(14);
	assert(conn02_port2.txBufAbsAddr.offset === optoModuleStartAddr.offset + conn02_port2.manualTxStartAddr);
}

function test_UAL_CONN_4_1_10(sim)
{
	assert(conn02.manualSettingsEnabled === true);
	
	//
	
	assert(conn02_port1.txDataSizeW === conn02_port1.manualTxWordsQuantity);
	assert(conn02_port1.txDataSizeW === conn02_port2.rxDataSizeW);
	assert(conn02_port2.rxDataSizeW === conn02_port2.manualRxWordsQuantity);
	
	//
	
	assert(conn02_port2.txDataSizeW === conn02_port2.manualTxWordsQuantity);
	assert(conn02_port2.txDataSizeW === conn02_port1.rxDataSizeW);
	assert(conn02_port1.rxDataSizeW === conn02_port1.manualRxWordsQuantity);
}

function test_UAL_CONN_4_1_11(sim)
{
	let validitySignal = sim.signalParamExt("#SYSTEMID_RACK01_FSCC02_MD14_PI_OPTOPORT04VALID");
	assert(validitySignal != null);
	
	let optoSignal = sim.signalParamExt("#LM2_LM1_TEMP");
	assert(optoSignal != null);
	assert(validitySignal.regValueAddr === optoSignal.regValidityAddr);
	
	// Signal #LM2_LM1_TEMP4 is NOT  acquired
	
	optoSignal = sim.signalParamExt("#LM2_LM1_TEMP4");
	assert(optoSignal != null);
	assert(optoSignal.regValueAddr.isValid === false);
	assert(optoSignal.regValidityAddr.isValid === false);

	optoSignal = sim.signalParamExt("#LM2_DS01");
	assert(optoSignal != null);
	assert(validitySignal.regValueAddr === optoSignal.regValidityAddr);
}

function test_UAL_CONN_4_1_12(sim)
{
	// case: auto port after manual port
	//
	// conn02_port2 is port 3 of OCM (place 14) with manual settings
	//
	assert(conn02.manualSettingsEnabled === true);

	// conn01_port2 is port 4 of OCM (place 14) with auto settings
	//
	assert(conn01.manualSettingsEnabled === false);
	
	let optoModuleStartAddr = lm2Description.ioModuleBufStartAddr(14);
	assert(conn01_port2.txBufAbsAddr.offset === optoModuleStartAddr.offset + conn02_port2.manualTxStartAddr + conn02_port2.manualTxWordsQuantity);
	assert(conn01_port2.txBufAbsAddr.offset === conn02_port2.txBufAbsAddr.offset + conn02_port2.txDataSizeW);
	
	// case: first auto port
	//
	// conn04_port1 port 1 of OCM (place 11) with auto settings
	//
	assert(conn04.manualSettingsEnabled === false);
	
	optoModuleStartAddr = lm1Description.ioModuleBufStartAddr(11);
	assert(conn04_port1.txBufAbsAddr.offset === optoModuleStartAddr.offset);
	
	// case: auto port after auto port
	//
	// conn04_port2 is port 5 of OCM (place 14) with auto settings
	//
	assert(conn04.manualSettingsEnabled === false);
	
	assert(conn04_port2.txBufAbsAddr.offset === conn01_port2.txBufAbsAddr.offset + conn01_port2.txDataSizeW);
}

function test_UAL_CONN_4_1_13(sim)
{
	// conn02_port2 is port 3 of OCM (place 14) with manual settings
	//
	assert(conn02.manualSettingsEnabled === true);
	
	let optoModuleStartAddr = lm2Description.ioModuleBufStartAddr(14);
	assert(conn02_port2.txBufAbsAddr.offset === optoModuleStartAddr.offset + conn02_port2.manualTxStartAddr);
}

function test_UAL_CONN_4_1_14(sim)
{
	let optoModuleStartAddr = lm2Description.ioModuleBufStartAddr(14);
	
	assert(conn02_port2.rxBufAbsAddr.offset === optoModuleStartAddr.offset + 163);			// start of receive buffers should be moved from beginnig of opto module memory
	assert(conn02_port2.rxDataSizeW === conn02_port2.manualRxWordsQuantity);
	assert(conn02_port2.rxDataSizeW == conn02_port1.txDataSizeW);	
	
	assert(conn01_port2.rxBufAbsAddr.offset === conn02_port2.rxBufAbsAddr.offset + conn02_port2.rxDataSizeW);
	assert(conn01_port2.rxDataSizeW == conn01_port1.txDataSizeW);
	
	assert(conn04_port2.rxBufAbsAddr.offset === conn01_port2.rxBufAbsAddr.offset + conn01_port2.rxDataSizeW);
	assert(conn04_port2.rxDataSizeW == conn04_port1.txDataSizeW);
}

function test_UAL_CONN_4_1_15(sim)
{
	let optoInterfaceOffset = lm1Description.optoInterfaceDataOffset;
	assert(optoInterfaceOffset != null);
	
	//
	
	assert(conn01.manualSettingsEnabled === false);
	
	assert(conn01_port1.txBufAbsAddr.offset === optoInterfaceOffset);
	assert(conn01_port1.rxBufAbsAddr.offset === optoInterfaceOffset);
}

function test_UAL_CONN_4_1_16(sim)
{
	let optoInterfaceOffset = lm1Description.optoInterfaceDataOffset;
	assert(optoInterfaceOffset != null);
	
	let optoPortDataSize = lm1Description.optoPortDataSize;
	assert(optoPortDataSize != null);
	
	//
	
	assert(conn03.manualSettingsEnabled === true);
	
	assert(conn03_port1.txBufAbsAddr.offset === optoInterfaceOffset + 1 * optoPortDataSize + conn03_port1.manualTxStartAddr);
	assert(conn03_port1.rxBufAbsAddr.offset === optoInterfaceOffset + 1 * optoPortDataSize);
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

function test_UAL_CONN_4_2_02(sim)
{
	let txBufAbsAddr = conn01_port1.txBufAbsAddr.offset;
	let txDataSizeW = conn01_port1.txDataSizeW;
	
	let sg = conn01_port1.txSignalInfo("#LM1_DS02");
	assert(sg != null);
	
	assert(sg.absAddr.offset > txBufAbsAddr && sg.absAddr.offset < (txBufAbsAddr + txDataSizeW));
	
	sg = conn01_port1.txSignalInfo("#LM1_DS03");
	assert(sg != null);
	assert(sg.absAddr.offset > txBufAbsAddr && sg.absAddr.offset < (txBufAbsAddr + txDataSizeW));
	
	sg = conn01_port1.txSignalInfo("#SYSTEMID_RACK01_FSCC01_MD00_PI_TEMP");
	assert(sg != null);
	assert(sg.absAddr.offset > txBufAbsAddr && sg.absAddr.offset < (txBufAbsAddr + txDataSizeW));

	sg = conn01_port1.txSignalInfo("#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK");
	assert(sg != null);
	assert(sg.absAddr.offset > txBufAbsAddr && sg.absAddr.offset < (txBufAbsAddr + txDataSizeW));

	sg = conn01_port1.txSignalInfo("#BUS32_S01");
	assert(sg != null);
	assert(sg.absAddr.offset > txBufAbsAddr && sg.absAddr.offset < (txBufAbsAddr + txDataSizeW));
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
		
		assert(rxSignal.absAddr.offset > rxPort.rxBufAbsAddr.offset && rxSignal.absAddr.offset < (rxPort.rxBufAbsAddr.offset + rxPort.rxDataSizeW));
		
		let ualRxSignal = sim.signalParamExt("#LM2_LM1_TEMP");
		
		assert(ualRxSignal.ualAddr === rxSignal.absAddr);
	}
	
	{
		let discreteSignalID = "#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK";
		
		let txSignal = conn01_port1.txSignalInfo(discreteSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(discreteSignalID);

		assert(rxSignal.absAddr.offset > rxPort.rxBufAbsAddr.offset && rxSignal.absAddr.offset < (rxPort.rxBufAbsAddr.offset + rxPort.rxDataSizeW));
		
		let ualRxSignal = sim.signalParamExt("#LM2_LM1_BLINK");
		
		assert(ualRxSignal.ualAddr === rxSignal.absAddr);
	}
	
	{
		let busSignalID = "#BUS32_S01";
		
		let txSignal = conn01_port1.txSignalInfo(busSignalID);
		let rxSignal = conn01_port2.rxSignalInfo(busSignalID);

		assert(rxSignal.absAddr.offset > rxPort.rxBufAbsAddr.offset && rxSignal.absAddr.offset < (rxPort.rxBufAbsAddr.offset + rxPort.rxDataSizeW));
		
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
		
		assert(rxSignal1.absAddr === ualRxSignal1.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal2.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal3.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal4.ualAddr);
	}
	
	{
		let rxSignal1 = conn01_port2.rxSignalInfo("#SYSTEMID_RACK01_FSCC01_MD00_PI_BLINK");
		
		let ualRxSignal1 = sim.signalParamExt("#LM2_LM1_BLINK");
		let ualRxSignal2 = sim.signalParamExt("#LM2_LM1_BLINK2");
		let ualRxSignal3 = sim.signalParamExt("#LM2_LM1_BLINK3");
		let ualRxSignal4 = sim.signalParamExt("#LM2_LM1_BLINK4");
		
		assert(rxSignal1.absAddr === ualRxSignal1.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal2.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal3.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal4.ualAddr);
	}
	
	{
		let rxSignal1 = conn01_port2.rxSignalInfo("#BUS32_S01");
		
		let ualRxSignal1 = sim.signalParamExt("#LM2_BUS32_S01");
		let ualRxSignal2 = sim.signalParamExt("#LM2_BUS32_S01_2");
		let ualRxSignal3 = sim.signalParamExt("#LM2_BUS32_S01_3");
		let ualRxSignal4 = sim.signalParamExt("#LM2_BUS32_S01_4");
		
		assert(rxSignal1.absAddr === ualRxSignal1.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal2.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal3.ualAddr);
		assert(rxSignal1.absAddr === ualRxSignal4.ualAddr);
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

	assert(rxValiditySignalAbsAddr === ualAutoRxValSignal.ualAddr);
	assert(rxValiditySignalAbsAddr === ualRxValSignal1.ualAddr);
	assert(rxValiditySignalAbsAddr === ualRxValSignal2.ualAddr);
	assert(rxValiditySignalAbsAddr === ualRxValSignal3.ualAddr);
	assert(rxValiditySignalAbsAddr=== ualRxValSignal4.ualAddr);
}


