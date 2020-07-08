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

const LM_QUIPMENT_ID = "SYSTEMID_RACKID_CH02_MD00";

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    console.log(sim.buildPath);

    assert(sim.logicModuleExists(LM_QUIPMENT_ID) === true);

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

function test_UAL_BUSSES_3_2_1(sim)
{
	// Not acquired Not used buses tests
	//
	const NOT_ACQUIRED_NOT_USED_MUM_PLACE = 1;
	const NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID = "#SHR0S2P1_STATE";

    assert(sim.signalExists(NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID) === true);

    let sg = sim.signalParamExt(NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID);

    assert(sg.isAcquired === false);

	// UAL_BUSSES_3_2_1_01
	//
    let ioAddr = sg.ioBufAddr;

	assert(ioAddr.isValid === true);
    assert(ioAddr.bit === 0);

    let lmDesc = sim.scriptLmDescription(LM_QUIPMENT_ID);

    assert(lmDesc.isAddrInIoModuleBuf(NOT_ACQUIRED_NOT_USED_MUM_PLACE, ioAddr) === true);

    assert(lmDesc.isNull !== true);

	// UAL_BUSSES_3_2_1_02
	//
    let ualAddr = sg.ualAddr;

	assert(ualAddr.isValid === false);

	// UAL_BUSSES_3_2_1_03
	//
    let regBufAddr = sg.regBufAddr;

	assert(regBufAddr.isValid === false);

	// UAL_BUSSES_3_2_1_04
	//
    let regValueAddr = sg.regValueAddr;

	assert(regValueAddr.isValid === false);

	return;
}
/*
function test_UAL_BUSSES_3_2_2(sim)
{
	// Not acquired Used buses tests
	//
	const NOT_ACQUIRED_USED_MUM_PLACE = 4;
	const NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID = "#SHR0S2P4_STATE";

	assert(sim.isSignalExists(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID) === true);
	assert(sim.signalIsAcquired(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID) === false);

	// UAL_BUSSES_3_2_2_01
	//
	let ioAddr = sim.signalIoAddr(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(ioAddr.isValid === true);
	assert(sim.addrInIoModuleBuf(LM_QUIPMENT_ID, NOT_ACQUIRED_USED_MUM_PLACE, ioAddr) === true);
	assert(ioAddr.bit === 0);

	// UAL_BUSSES_3_2_2_02
	//
	let ualAddr = sim.signalUalAddr(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(ualAddr.isValid === true);
	assert(ualAddr.offset === ioAddr.offset);
	assert(ualAddr.bit === 0);

	// UAL_BUSSES_3_2_2_03
	//
	let regBufAddr = sim.signalRegBufAddr(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(regBufAddr.isValid === false);

	// UAL_BUSSES_3_2_2_04
	//
	let regValueAddr = sim.signalRegValueAddr(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(regValueAddr.isValid === false);

	return;
}

function test_UAL_BUSSES_3_2_3(sim)
{
	// Acquired Not used buses tests
	//
	const ACQUIRED_NOT_USED_MUM_PLACE = 11;
	const ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID = "#SHR0S2P11_STATE";

	assert(sim.isSignalExists(ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID) === true);
	assert(sim.signalIsAcquired(ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID) === true);

	// UAL_BUSSES_3_2_3_01
	//
	let ioAddr = sim.signalIoAddr(ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID);

	assert(ioAddr.isValid === true);
	assert(sim.addrInIoModuleBuf(LM_QUIPMENT_ID, ACQUIRED_NOT_USED_MUM_PLACE, ioAddr) === true);
	assert(ioAddr.bit === 0);

	// UAL_BUSSES_3_2_3_02
	//
	let ualAddr = sim.signalUalAddr(ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID);

	assert(ualAddr.isValid === true);
	assert(ualAddr.offset === ioAddr.offset);
	assert(ualAddr.bit === 0);

	// UAL_BUSSES_3_2_3_03
	//
	let regBufAddr = sim.signalRegBufAddr(ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID);

	assert(regBufAddr.isValid === true);
	assert(regBufAddr.bit === 0);
	assert(sim.addrInRegBuf(LM_QUIPMENT_ID, regBufAddr) === true);

	// UAL_BUSSES_3_2_3_04
	//
	let regValueAddr = sim.signalRegValueAddr(ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID);

	assert(regValueAddr.isValid === true);
	assert(regValueAddr.bit === 0);
	assert(regValueAddr.offset === (regBufAddr.offset - sim.regBufStartAddr(LM_QUIPMENT_ID)));

	// UAL_BUSSES_3_2_3_04
	//


	return;
}

function test_UAL_BUSSES_3_2_4(sim)
{
	// Acquired Used buses tests
	//
	const ACQUIRED_USED_MUM_PLACE = 14;
	const ACQUIRED_USED_INPUT_BUS_SIGNAL_ID = "#SHR0S2P14_STATE";

	assert(sim.isSignalExists(ACQUIRED_USED_INPUT_BUS_SIGNAL_ID) === true);
	assert(sim.signalIsAcquired(ACQUIRED_USED_INPUT_BUS_SIGNAL_ID) === true);

	// UAL_BUSSES_3_2_4_01
	//
	let ioAddr = sim.signalIoAddr(ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(ioAddr.isValid === true);
	assert(sim.addrInIoModuleBuf(LM_QUIPMENT_ID, ACQUIRED_USED_MUM_PLACE, ioAddr) === true);
	assert(ioAddr.bit === 0);

	// UAL_BUSSES_3_2_4_02
	//
	let ualAddr = sim.signalUalAddr(ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(ualAddr.isValid === true);
	assert(ualAddr.offset === ioAddr.offset);
	assert(ualAddr.bit === 0);

	// UAL_BUSSES_3_2_4_03
	//
	let regBufAddr = sim.signalRegBufAddr(ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(regBufAddr.isValid === true);
	assert(regBufAddr.bit === 0);
	assert(sim.addrInRegBuf(LM_QUIPMENT_ID, regBufAddr) === true);

	// UAL_BUSSES_3_2_4_04
	//
	let regValueAddr = sim.signalRegValueAddr(ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

	assert(regValueAddr.isValid === true);
	assert(regValueAddr.bit === 0);
	assert(regValueAddr.offset === (regBufAddr.offset - sim.regBufStartAddr(LM_QUIPMENT_ID)));

	// UAL_BUSSES_3_2_4_05
	//
	// Copy code should be generated!

	// UAL_BUSSES_3_2_4_05
	//
	const ACQUIRED_USED_INPUT_BUS_NESTED1_SIGNAL_ID = "#SHR0S2P14_STATE.DU";

	assert(sim.isSignalExists(ACQUIRED_USED_INPUT_BUS_NESTED1_SIGNAL_ID) === true);

	let ualAddr1 = sim.signalUalAddr(ACQUIRED_USED_INPUT_BUS_NESTED1_SIGNAL_ID);

	assert(ualAddr1.isValid === true);

	let regBufAddr1 = sim.signalRegBufAddr(ACQUIRED_USED_INPUT_BUS_NESTED1_SIGNAL_ID);

	assert(regBufAddr1.isValid === true);

	let regValueAddr1 = sim.signalRegValueAddr(ACQUIRED_USED_INPUT_BUS_NESTED1_SIGNAL_ID);

	assert(regValueAddr1.isValid === true);

	//

	const ACQUIRED_USED_INPUT_BUS_NESTED2_SIGNAL_ID = "#SHR0S2P14_STATE.SAR.SSCC";

	assert(sim.isSignalExists(ACQUIRED_USED_INPUT_BUS_NESTED2_SIGNAL_ID) === true);

	let ualAddr2 = sim.signalUalAddr(ACQUIRED_USED_INPUT_BUS_NESTED2_SIGNAL_ID);

	assert(ualAddr2.isValid === true);

	let regBufAddr2 = sim.signalRegBufAddr(ACQUIRED_USED_INPUT_BUS_NESTED2_SIGNAL_ID);

	assert(regBufAddr2.isValid === true);

	let regValueAddr2 = sim.signalRegValueAddr(ACQUIRED_USED_INPUT_BUS_NESTED2_SIGNAL_ID);

	assert(regValueAddr2.isValid === true);

	return;
}

function test_UAL_BUSSES_3_2_5(sim)
{
	// Acquired Used buses tests
	//
	const ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID = "#BUS16_ACQUIRED";

	assert(sim.isSignalExists(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID) === true);
	assert(sim.signalIsAcquired(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID) === true);

	// UAL_BUSSES_3_2_5_01
	//
	let ioAddr = sim.signalIoAddr(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID);

	assert(ioAddr.isValid === false);

	// UAL_BUSSES_3_2_5_02
	//
	let ualAddr = sim.signalUalAddr(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID);

	assert(ualAddr.isValid === true);
	assert(sim.addrInRegBuf(LM_QUIPMENT_ID, ualAddr) === true);
	assert(ualAddr.bit === 0);

	// UAL_BUSSES_3_2_5_03
	//
	let regBufAddr = sim.signalRegBufAddr(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID);

	assert(regBufAddr.isValid === true);
	assert(ualAddr.offset === regBufAddr.offset);
	assert(regBufAddr.bit === 0);

	// UAL_BUSSES_3_2_5_04
	//
	let regValueAddr = sim.signalRegValueAddr(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID);

	assert(regValueAddr.isValid === true);
	assert(regValueAddr.bit === 0);
	assert(regValueAddr.offset === (regBufAddr.offset - sim.regBufStartAddr(LM_QUIPMENT_ID)));

	return;
}


*/
