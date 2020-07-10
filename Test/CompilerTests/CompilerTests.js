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
    const NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID_DU = "#SHR0S2P1_STATE.DU";
    const NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID_DU_SOCO = "#SHR0S2P1_STATE.DU.SOCO";

    let lmDesc = sim.scriptLmDescription(LM_QUIPMENT_ID);

    assert(lmDesc.isNull === false);

    let sg = sim.signalParamExt(NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID);

    assert(sg.isNull === false);
    assert(sg.isInput === true);
    assert(sg.isAcquired === false);

	// UAL_BUSSES_3_2_1_01
	//
    let ioAddr = sg.ioBufAddr;

	assert(ioAddr.isValid === true);
    assert(ioAddr.bit === 0);

    assert(lmDesc.isAddrInIoModuleBuf(NOT_ACQUIRED_NOT_USED_MUM_PLACE, ioAddr) === true);

	// UAL_BUSSES_3_2_1_02
	//
    assert(sg.ualAddr.isValid === false);

	// UAL_BUSSES_3_2_1_03
	//
    assert(sg.regBufAddr.isValid === false);

	// UAL_BUSSES_3_2_1_04
	//
    assert(sg.regValueAddr.isValid === false);

    // UAL_BUSSES_3_2_1_05
    //
    assert(sim.signalExists(NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID_DU) === false);
    assert(sim.signalExists(NOT_ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID_DU_SOCO) === false);

    return;
}

function test_UAL_BUSSES_3_2_2(sim)
{
	// Not acquired Used buses tests
	//
	const NOT_ACQUIRED_USED_MUM_PLACE = 4;
	const NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID = "#SHR0S2P4_STATE";
    const NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID_TZ = "#SHR0S2P4_STATE.TZ";
    const NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID_TZ_SPDC = "#SHR0S2P4_STATE.TZ.SPDC";

    let lmDesc = sim.scriptLmDescription(LM_QUIPMENT_ID);

    assert(lmDesc.isNull === false);

    let sg = sim.signalParamExt(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID);

    assert(sg.isNull === false);
    assert(sg.isInput === true);
    assert(sg.isAcquired === false);

	// UAL_BUSSES_3_2_2_01
	//
    let ioAddr = sg.ioBufAddr;

	assert(ioAddr.isValid === true);
    assert(ioAddr.bit === 0);

    assert(lmDesc.isAddrInIoModuleBuf(NOT_ACQUIRED_USED_MUM_PLACE, ioAddr) === true);

	// UAL_BUSSES_3_2_2_02
	//
    let ualAddr = sg.ualAddr;

	assert(ualAddr.isValid === true);
	assert(ualAddr.offset === ioAddr.offset);
    assert(ualAddr.bit === 0);

	// UAL_BUSSES_3_2_2_03
	//
    let regBufAddr = sg.regBufAddr;

	assert(regBufAddr.isValid === false);

	// UAL_BUSSES_3_2_2_04
	//
    let regValueAddr = sg.regValueAddr;

	assert(regValueAddr.isValid === false);

    // UAL_BUSSES_3_2_2_05
    //
    let sgTZ = sim.signalParamExt(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID_TZ);

    assert(sgTZ.isNull === false);
    assert(sgTZ.isInput === true);
    assert(sgTZ.isAcquired === false);

    let sgTZ_SPDC = sim.signalParamExt(NOT_ACQUIRED_USED_INPUT_BUS_SIGNAL_ID_TZ_SPDC);

    assert(sgTZ_SPDC.isNull === false);
    assert(sgTZ_SPDC.isInput === true);
    assert(sgTZ_SPDC.isAcquired === false);

    // UAL_BUSSES_3_2_2_06
    //
    let sgTZ_ualAddr = sgTZ.ualAddr;

    assert(sgTZ_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgTZ_ualAddr.bitAddress < (sg.ualAddr.bitAddress + sg.sizeBit));

    let sgTZ_SPDC_ualAddr = sgTZ.ualAddr;

    assert(sgTZ_SPDC_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgTZ_SPDC_ualAddr.bitAddress < (sg.ualAddr.bitAddress + sg.sizeBit));

    assert(sgTZ_SPDC_ualAddr.bitAddress >= sgTZ_ualAddr.bitAddress &&
           sgTZ_SPDC_ualAddr.bitAddress < (sgTZ_ualAddr.bitAddress + sgTZ.sizeBit));

    // UAL_BUSSES_3_2_2_07
    //
    assert(sgTZ.regBufAddr.isValid === false);
    assert(sgTZ_SPDC.regBufAddr.isValid === false);

    // UAL_BUSSES_3_2_2_08
    //
    assert(sgTZ.regValueAddr.isValid === false);
    assert(sgTZ_SPDC.regValueAddr.isValid === false);

	return;
}

function test_UAL_BUSSES_3_2_3(sim)
{
	// Acquired Not used buses tests
	//
	const ACQUIRED_NOT_USED_MUM_PLACE = 11;
	const ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID = "#SHR0S2P11_STATE";
    const CHILD_BUS_SIGNAL_ID_TB = "#SHR0S2P11_STATE.TB";
    const CHILD_BUS_SIGNAL_ID_TB_STXC = "#SHR0S2P11_STATE.TB.STXC";

    let lmDesc = sim.scriptLmDescription(LM_QUIPMENT_ID);

    assert(lmDesc.isNull === false);

    let sg = sim.signalParamExt(ACQUIRED_NOT_USED_INPUT_BUS_SIGNAL_ID);

    assert(sg.isNull === false);
    assert(sg.isInput === true);
    assert(sg.isAcquired === true);

	// UAL_BUSSES_3_2_3_01
	//
    let ioAddr = sg.ioBufAddr;

	assert(ioAddr.isValid === true);
    assert(ioAddr.bit === 0);
    assert(lmDesc.isAddrInIoModuleBuf(ACQUIRED_NOT_USED_MUM_PLACE, ioAddr) === true);

	// UAL_BUSSES_3_2_3_02
	//
    let ualAddr = sg.ualAddr;

	assert(ualAddr.isValid === true);
	assert(ualAddr.offset === ioAddr.offset);
	assert(ualAddr.bit === 0);

	// UAL_BUSSES_3_2_3_03
	//
    let regBufAddr = sg.regBufAddr;

	assert(regBufAddr.isValid === true);
	assert(regBufAddr.bit === 0);
    assert(lmDesc.isAddrInAcquiredAppDataBuf(regBufAddr) === true);

	// UAL_BUSSES_3_2_3_04
	//
    let regValueAddr = sg.regValueAddr;

	assert(regValueAddr.isValid === true);
	assert(regValueAddr.bit === 0);
    assert(regValueAddr.offset === (regBufAddr.offset - lmDesc.appDataStartAddr.offset));

    // UAL_BUSSES_3_2_3_05
    //

    // UAL_BUSSES_3_2_3_06
    //
    let sgTB = sim.signalParamExt(CHILD_BUS_SIGNAL_ID_TB);

    assert(sgTB.isNull === false);
    assert(sgTB.isInput === true);
    assert(sgTB.isAcquired === true);

    let sgTB_STXC = sim.signalParamExt(CHILD_BUS_SIGNAL_ID_TB_STXC);

    assert(sgTB_STXC.isNull === false);
    assert(sgTB_STXC.isInput === true);
    assert(sgTB_STXC.isAcquired === true);

    // UAL_BUSSES_3_2_3_07
    //
    let sgTB_ualAddr = sgTB.ualAddr;

    assert(sgTB_ualAddr.isValid === true);

    assert(sgTB_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgTB_ualAddr.bitAddress < sg.ualAddr.bitAddress + sg.sizeBit);

    let sgTB_STXC_ualAddr = sgTB_STXC.ualAddr;

    assert(sgTB_STXC_ualAddr.isValid === true);

    assert(sgTB_STXC_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgTB_STXC_ualAddr.bitAddress < sg.ualAddr.bitAddress + sg.sizeBit);

    assert(sgTB_STXC_ualAddr.bitAddress >= sgTB.ualAddr.bitAddress &&
           sgTB_STXC_ualAddr.bitAddress < sgTB.ualAddr.bitAddress + sgTB.sizeBit);

    // UAL_BUSSES_3_2_3_08
    //
    let sgTB_regBufAddr = sgTB.regBufAddr;

    assert(sgTB_regBufAddr.isValid === true);

    assert(sgTB_regBufAddr.bitAddress >= sg.regBufAddr.bitAddress &&
           sgTB_regBufAddr.bitAddress < sg.regBufAddr.bitAddress + sg.sizeBit);

    let sgTB_STXC_regBufAddr = sgTB_STXC.regBufAddr;

    assert(sgTB_STXC_regBufAddr.isValid === true);

    assert(sgTB_STXC_regBufAddr.bitAddress >= sg.regBufAddr.bitAddress &&
           sgTB_STXC_regBufAddr.bitAddress < sg.regBufAddr.bitAddress + sg.sizeBit);

    assert(sgTB_STXC_regBufAddr.bitAddress >= sgTB.regBufAddr.bitAddress &&
           sgTB_STXC_regBufAddr.bitAddress < sgTB.regBufAddr.bitAddress + sgTB.sizeBit);

    // UAL_BUSSES_3_2_3_09
    //
    let sgTB_regValueAddr = sgTB.regValueAddr;

    assert(sgTB_regValueAddr.isValid === true);

    assert(sgTB_regValueAddr.bitAddress >= sg.regValueAddr.bitAddress &&
           sgTB_regValueAddr.bitAddress < sg.regValueAddr.bitAddress + sg.sizeBit);

    let sgTB_STXC_regValueAddr = sgTB_STXC.regValueAddr;

    assert(sgTB_STXC_regValueAddr.isValid === true);

    assert(sgTB_STXC_regValueAddr.bitAddress >= sg.regValueAddr.bitAddress &&
           sgTB_STXC_regValueAddr.bitAddress < sg.regValueAddr.bitAddress + sg.sizeBit);

    assert(sgTB_STXC_regValueAddr.bitAddress >= sgTB.regValueAddr.bitAddress &&
           sgTB_STXC_regValueAddr.bitAddress < sgTB.regValueAddr.bitAddress + sgTB.sizeBit);

    return;
}

function test_UAL_BUSSES_3_2_4(sim)
{
	// Acquired Used buses tests
	//
	const ACQUIRED_USED_MUM_PLACE = 14;
	const ACQUIRED_USED_INPUT_BUS_SIGNAL_ID = "#SHR0S2P14_STATE";
    const CHILD_BUS_SIGNAL_ID_SAR = "#SHR0S2P14_STATE.SAR";
    const CHILD_BUS_SIGNAL_ID_SAR_SRCC = "#SHR0S2P14_STATE.SAR.SRCC";

    let lmDesc = sim.scriptLmDescription(LM_QUIPMENT_ID);

    assert(lmDesc.isNull === false);

    let sg = sim.signalParamExt(ACQUIRED_USED_INPUT_BUS_SIGNAL_ID)

    assert(sg.isNull === false);
    assert(sg.isInput === true);
    assert(sg.isAcquired === true);

	// UAL_BUSSES_3_2_4_01
	//
    let ioAddr = sg.ioBufAddr;

	assert(ioAddr.isValid === true);
    assert(ioAddr.bit === 0);
    assert(lmDesc.isAddrInIoModuleBuf(ACQUIRED_USED_MUM_PLACE, ioAddr) === true);

	// UAL_BUSSES_3_2_4_02
	//
    let ualAddr = sg.ualAddr;

	assert(ualAddr.isValid === true);
	assert(ualAddr.offset === ioAddr.offset);
	assert(ualAddr.bit === 0);

	// UAL_BUSSES_3_2_4_03
	//
    let regBufAddr = sg.regBufAddr;

	assert(regBufAddr.isValid === true);
	assert(regBufAddr.bit === 0);
    assert(lmDesc.isAddrInAcquiredAppDataBuf(regBufAddr) === true);

	// UAL_BUSSES_3_2_4_04
	//
    let regValueAddr = sg.regValueAddr;

	assert(regValueAddr.isValid === true);
	assert(regValueAddr.bit === 0);
    assert(regValueAddr.offset === (regBufAddr.offset - lmDesc.appDataStartAddr.offset));

	// UAL_BUSSES_3_2_4_05
	//
	// Copy code should be generated!

    // UAL_BUSSES_3_2_4_06
	//
    let sgSAR = sim.signalParamExt(CHILD_BUS_SIGNAL_ID_SAR);

    assert(sgSAR.isNull === false);
    assert(sgSAR.isInput === true);
    assert(sgSAR.isAcquired === true);

    let sgSAR_SRCC = sim.signalParamExt(CHILD_BUS_SIGNAL_ID_SAR_SRCC);

    assert(sgSAR_SRCC.isNull === false);
    assert(sgSAR_SRCC.isInput === true);
    assert(sgSAR_SRCC.isAcquired === true);

    // UAL_BUSSES_3_2_4_07
    //
    let sgSAR_ualAddr = sgSAR.ualAddr;

    assert(sgSAR_ualAddr.isValid === true);

    assert(sgSAR_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgSAR_ualAddr.bitAddress < sg.ualAddr.bitAddress + sg.sizeBit);

    let sgTB_SAR_SRCC_ualAddr = sgSAR_SRCC.ualAddr;

    assert(sgTB_SAR_SRCC_ualAddr.isValid === true);

    assert(sgTB_SAR_SRCC_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgTB_SAR_SRCC_ualAddr.bitAddress < sg.ualAddr.bitAddress + sg.sizeBit);

    assert(sgTB_SAR_SRCC_ualAddr.bitAddress >= sgSAR.ualAddr.bitAddress &&
           sgTB_SAR_SRCC_ualAddr.bitAddress < sgSAR.ualAddr.bitAddress + sgSAR.sizeBit);

    // UAL_BUSSES_3_2_4_08
    //
    let sgSAR_regBufAddr = sgSAR.regBufAddr;

    assert(sgSAR_regBufAddr.isValid === true);

    assert(sgSAR_regBufAddr.bitAddress >= sg.regBufAddr.bitAddress &&
           sgSAR_regBufAddr.bitAddress < sg.regBufAddr.bitAddress + sg.sizeBit);

    let sgSAR_SRCC_regBufAddr = sgSAR_SRCC.regBufAddr;

    assert(sgSAR_SRCC_regBufAddr.isValid === true);

    assert(sgSAR_SRCC_regBufAddr.bitAddress >= sg.regBufAddr.bitAddress &&
           sgSAR_SRCC_regBufAddr.bitAddress < sg.regBufAddr.bitAddress + sg.sizeBit);

    assert(sgSAR_SRCC_regBufAddr.bitAddress >= sgSAR.regBufAddr.bitAddress &&
           sgSAR_SRCC_regBufAddr.bitAddress < sgSAR.regBufAddr.bitAddress + sgSAR.sizeBit);

    // UAL_BUSSES_3_2_3_09
    //
    let sgSAR_regValueAddr = sgSAR.regValueAddr;

    assert(sgSAR_regValueAddr.isValid === true);

    assert(sgSAR_regValueAddr.bitAddress >= sg.regValueAddr.bitAddress &&
           sgSAR_regValueAddr.bitAddress < sg.regValueAddr.bitAddress + sg.sizeBit);

    let sgSAR_SRCC_regValueAddr = sgSAR_SRCC.regValueAddr;

    assert(sgSAR_SRCC_regValueAddr.isValid === true);

    assert(sgSAR_SRCC_regValueAddr.bitAddress >= sg.regValueAddr.bitAddress &&
           sgSAR_SRCC_regValueAddr.bitAddress < sg.regValueAddr.bitAddress + sg.sizeBit);

    assert(sgSAR_SRCC_regValueAddr.bitAddress >= sgSAR.regValueAddr.bitAddress &&
           sgSAR_SRCC_regValueAddr.bitAddress < sgSAR.regValueAddr.bitAddress + sgSAR.sizeBit);

	return;
}

function test_UAL_BUSSES_3_2_5(sim)
{
	// Acquired Used buses tests
	//
	const ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID = "#BUS16_ACQUIRED";
    const ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT6 = "#BUS16_ACQUIRED.bit6";
    const ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT15 = "#BUS16_ACQUIRED.bit15";

    let lmDesc = sim.scriptLmDescription(LM_QUIPMENT_ID);

    assert(lmDesc.isNull === false);

    let sg = sim.signalParamExt(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID)

    assert(sg.isNull === false);
    assert(sg.isInternal === true);
    assert(sg.isAcquired === true);

	// UAL_BUSSES_3_2_5_01
	//
    let ioAddr = sg.ioBufAddr;

	assert(ioAddr.isValid === false);

	// UAL_BUSSES_3_2_5_02
	//
    let ualAddr = sg.ualAddr;

	assert(ualAddr.isValid === true);
    assert(ualAddr.bit === 0);
    assert(lmDesc.isAddrInAcquiredAppDataBuf(ualAddr) === true);

	// UAL_BUSSES_3_2_5_03
	//
    let regBufAddr = sg.regBufAddr;

	assert(regBufAddr.isValid === true);
	assert(ualAddr.offset === regBufAddr.offset);
	assert(regBufAddr.bit === 0);

	// UAL_BUSSES_3_2_5_04
	//
    let regValueAddr = sg.regValueAddr;

	assert(regValueAddr.isValid === true);
	assert(regValueAddr.bit === 0);
    assert(regValueAddr.offset === (regBufAddr.offset - lmDesc.appDataStartAddr.offset));

    // UAL_BUSSES_3_2_5_05
    //
    let sgBit6 = sim.signalParamExt(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT6);

    assert(sgBit6.isNull === false);
    assert(sgBit6.isInternal === true);
    assert(sgBit6.isAcquired === true);

    let sgBit15 = sim.signalParamExt(ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT15);

    assert(sgBit15.isNull === false);
    assert(sgBit15.isInternal === true);
    assert(sgBit15.isAcquired === true);

    // UAL_BUSSES_3_2_5_06
    //
    let sgBit6_ualAddr = sgBit6.ualAddr;

    assert(sgBit6_ualAddr.isValid === true);

    assert(sgBit6_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgBit6_ualAddr.bitAddress < sg.ualAddr.bitAddress + sg.sizeBit);

    let sgBit15_ualAddr = sgBit15.ualAddr;

    assert(sgBit15_ualAddr.isValid === true);

    assert(sgBit15_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgBit15_ualAddr.bitAddress < (sg.ualAddr.bitAddress + sg.sizeBit));

    // UAL_BUSSES_3_2_5_07
    //
    let sgBit6_regBufAddr = sgBit6.regBufAddr;

    assert(sgBit6_regBufAddr.isValid === true);

    assert(sgBit6_regBufAddr.bitAddress >= sg.regBufAddr.bitAddress &&
           sgBit6_regBufAddr.bitAddress < (sg.regBufAddr.bitAddress + sg.sizeBit));

    let sgBit15_regBufAddr = sgBit15.regBufAddr;

    assert(sgBit15_regBufAddr.isValid === true);

    assert(sgBit15_regBufAddr.bitAddress >= sg.regBufAddr.bitAddress &&
           sgBit15_regBufAddr.bitAddress < (sg.regBufAddr.bitAddress + sg.sizeBit));

	return;
}

function test_UAL_BUSSES_3_2_6(sim)
{
    // Acquired Used buses tests
    //
    const NON_ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID = "#BUS16_NON_ACQUIRED";
    const NON_ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT0 = "#BUS16_NON_ACQUIRED.bit0";
    const NON_ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT13 = "#BUS16_NON_ACQUIRED.bit13";

    let lmDesc = sim.scriptLmDescription(LM_QUIPMENT_ID);

    assert(lmDesc.isNull === false);

    let sg = sim.signalParamExt(NON_ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID)

    assert(sg.isNull === false);
    assert(sg.isInternal === true);
    assert(sg.isAcquired === false);

    // UAL_BUSSES_3_2_6_01
    //
    let ioAddr = sg.ioBufAddr;

    assert(ioAddr.isValid === false);

    // UAL_BUSSES_3_2_6_02
    //
    let ualAddr = sg.ualAddr;

    assert(ualAddr.isValid === true);
    assert(ualAddr.bit === 0);
    assert(lmDesc.isAddrAfterAcquiredAppDataBuf(ualAddr) === true);

    // UAL_BUSSES_3_2_6_03
    //
    assert(sg.regBufAddr.isValid === false);

    // UAL_BUSSES_3_2_6_04
    //
    assert(sg.regValueAddr.isValid === false);

    // UAL_BUSSES_3_2_6_05
    //
    let sgBit0 = sim.signalParamExt(NON_ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT0);

    assert(sgBit0.isNull === false);
    assert(sgBit0.isInternal === true);
    assert(sgBit0.isAcquired === false);

    let sgBit13 = sim.signalParamExt(NON_ACQUIRED_USED_INTERNAL_BUS_SIGNAL_ID_BIT13);

    assert(sgBit13.isNull === false);
    assert(sgBit13.isInternal === true);
    assert(sgBit13.isAcquired === false);

    // UAL_BUSSES_3_2_6_06
    //
    let sgBit0_ualAddr = sgBit0.ualAddr;

    assert(sgBit0_ualAddr.isValid === true);

    assert(sgBit0_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgBit0_ualAddr.bitAddress < sg.ualAddr.bitAddress + sg.sizeBit);

    let sgBit13_ualAddr = sgBit13.ualAddr;

    assert(sgBit13_ualAddr.isValid === true);

    assert(sgBit13_ualAddr.bitAddress >= sg.ualAddr.bitAddress &&
           sgBit13_ualAddr.bitAddress < (sg.ualAddr.bitAddress + sg.sizeBit));

    // UAL_BUSSES_3_2_6_07
    //
    assert(sgBit0.regBufAddr.isValid === false);
    assert(sgBit13.regBufAddr.isValid === false);

    // UAL_BUSSES_3_2_6_08
    //
    assert(sgBit0.regValueAddr.isValid === false);
    assert(sgBit13.regValueAddr.isValid === false);

    return;
}

