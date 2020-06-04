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


//
// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(sim)
{
    console.log(sim.buildPath);

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

// Test for AFB Not (OpCode 1)
// Schema: TEST_NOT
//
function testAfbNot(sim)
{
    assert(sim.signalValue("#TEST_NOT_1") === 1);
    assert(sim.signalValue("#TEST_NOT_2") === 0);
    assert(sim.signalValue("#TEST_NOT_3") === 0);

    // Test loopback 0101010...
    //
    let lastValue = sim.signalValue("#TEST_NOT_4");  // start value
    for (let i = 0; i < 5; i++)
    {
        sim.startForMs(5);

        let currentValue = sim.signalValue("#TEST_NOT_4");
        if  (lastValue === 0)
        {
            assert(currentValue === 1);
        }
        else
        {
            assert(currentValue === 0);
        }

        lastValue = currentValue;
    }

    return;
}


// Test for AFB Not (OpCode 2)
// Schema: TEST_NOT
//
function testAfbLogic(sim)
{
    // Test OR
    //
    assert(sim.signalValue("#TEST_LOGIC_OR1") === 1);
    assert(sim.signalValue("#TEST_LOGIC_OR2") === 0);
    assert(sim.signalValue("#TEST_LOGIC_OR3") === 1);
    assert(sim.signalValue("#TEST_LOGIC_OR4") === 1);

    // Test OR for Bus
    //
    assert(sim.signalValue("#TEST_LOGIC_V207_T90_R1") === 1);
    assert(sim.signalValue("#TEST_LOGIC_V207_T90_R2") === 1);
    assert(sim.signalValue("#TEST_LOGIC_V207_T90_R3") === 1);
    assert(sim.signalValue("#TEST_LOGIC_V207_T90_R4") === 1);
    assert(sim.signalValue("#TEST_LOGIC_V207_T90_R5") === 0);

    // Test AND
    //
    assert(sim.signalValue("#TEST_LOGIC_AND1") === 0);
    assert(sim.signalValue("#TEST_LOGIC_AND2") === 0);
    assert(sim.signalValue("#TEST_LOGIC_AND3") === 0);
    assert(sim.signalValue("#TEST_LOGIC_AND4") === 1);

    // Test AND for Bus
    //
    assert(sim.signalValue("#TEST_LOGIC_V207_T91_R1") === 0);
    assert(sim.signalValue("#TEST_LOGIC_V207_T91_R2") === 0);
    assert(sim.signalValue("#TEST_LOGIC_V207_T91_R3") === 1);
    assert(sim.signalValue("#TEST_LOGIC_V207_T91_R4") === 0);
    assert(sim.signalValue("#TEST_LOGIC_V207_T91_R5") === 1);

    // Test XOR
    //
    assert(sim.signalValue("#TEST_LOGIC_XOR1") === 0);
    assert(sim.signalValue("#TEST_LOGIC_XOR2") === 0);
    assert(sim.signalValue("#TEST_LOGIC_XOR3") === 1);
    assert(sim.signalValue("#TEST_LOGIC_XOR4") === 1);
    assert(sim.signalValue("#TEST_LOGIC_XOR5") === 0);

    // Test XOR for Bus
    //
    assert(sim.signalValue("#TEST_LOGIC_V207_T92_R1") === 1);
    assert(sim.signalValue("#TEST_LOGIC_V207_T92_R2") === 0);
    assert(sim.signalValue("#TEST_LOGIC_V207_T92_R3") === 0);
    assert(sim.signalValue("#TEST_LOGIC_V207_T92_R4") === 1);
    assert(sim.signalValue("#TEST_LOGIC_V207_T92_R5") === 0);

    return;
}


// Test for AFB TCT (OpCode 3), conf 1
// Schema: TEST_TCT_V209_CONF1_ON
//
function testAfbTctV209Conf1(sim)
{
    // AFB tct_on,
    // Label TEST_TCT_V209_CONF1_ON_8400
    // time 200ms
    // in - #TEST_TCT_CONF1_TMAN_IN
    // out - #TEST_TCT_CONF1_TMAN_R

    // 0. Initially initiator must be in 0
    // 1. set initiator to 1
    // 1.R. result must become 1 in 200ms
    // 2. reset initiator to 0, result must become 0 immediately
    // 2.R. result must become 1 in 200ms

    let workcycle = 5;
    let timerCounter = 200 / workcycle;
    let initiatorSignalId = "#TEST_TCT_CONF1_TMAN_IN";
    let resultSignalId = "#TEST_TCT_CONF1_TMAN_R";

    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 0
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);

        let current = sim.signalValue(resultSignalId)
        assert(current === 0);
    }

    sim.startForMs(5);
    let current = sim.signalValue(resultSignalId)
    assert(current === 1);

    for (let i = 0; i < 10; i++)    // give extra time and check that result stayed in 1
    {
        sim.startForMs(5);
        let current = sim.signalValue(resultSignalId)
        assert(current === 1);
    }

    // 2.
    //
    sim.overrideSignalValue(initiatorSignalId, 0);

    for (let i = 0; i < 10; i++)    // give extra time and check that result stayed in 0
    {
        sim.startForMs(5);
        let current = sim.signalValue(resultSignalId)
        assert(current === 0);
    }

    sim.overridesReset();

    // 0. Initially initiator must be in 0
    // 1. set initiator to 1 for 195ms,
    // 2. reset initiator to 0,
    // R. result must stay in 0 all the time, and check some extra time
    //

    // 1.
    //
    sim.overrideSignalValue(initiatorSignalId, 1);
    for (let i = 0; i < timerCounter - 1; i++)    // give extra time and check that result stayed in 1
    {
        sim.startForMs(5);
        let current = sim.signalValue(resultSignalId)
        assert(current === 0);
    }

    // 2.
    //
    sim.overrideSignalValue(initiatorSignalId, 0);

    for (let i = 0; i < 10; i++)    // give extra time and check that result stayed in 0
    {
        sim.startForMs(5);
        let current = sim.signalValue(resultSignalId)
        assert(current === 0);
    }

    return;
}

// Test for AFB TCT (OpCode 3), conf 2
// Schema: TEST_TCT_V209_CONF2_OFF
//
function testAfbTctV209Conf2(sim)
{
    // AFB tct_off,
    // Label TEST_TCT_V209_CONF2_OFF_8420
    // time 200ms
    // in - #TEST_TCT_CONF2_TMAN_IN
    // out - #TEST_TCT_CONF2_TMAN_R

    // 0. Initially initiator must be in 0
    // 1. set initiator to 1, Result must became 1 immediately!
    // 1.1. Wait some time
    // 2. reset initiator to 0
    // R. Check that result will will become 0 in 200ms
    //
    let workcycle = 5;
    let time = 200; // ms
    let timerCounter = time / workcycle;
    let initiatorSignalId = "#TEST_TCT_CONF2_TMAN_IN";
    let resultSignalId = "#TEST_TCT_CONF2_TMAN_R";

    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 0
    //
    for (let i = 0; i < 10; i++)
    {
        sim.startForMs(5);

        let current = sim.signalValue(resultSignalId)
        assert(current === 1);
    }

    sim.overrideSignalValue(initiatorSignalId, 0);

    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);

        let current = sim.signalValue(resultSignalId)
        assert(current === 1);
    }

    sim.startForMs(5);
    let current = sim.signalValue(resultSignalId)
    assert(current === 0);

    sim.overridesReset();

    // 1. set initiator to 1, Result must became 1 immediately!
    // 2. set initiator to 0101, CHECK Result == 1 all the time
    // 3. set initiator to 0
    // R. Check that result will will become 0 in 200ms

    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 0);
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(100);
    assert(sim.signalValue(resultSignalId) === 1);

    // 2.
    //
    sim.overrideSignalValue(initiatorSignalId, 0);
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);

        let current = sim.signalValue(resultSignalId)
        assert(current === 1);
    }

    sim.startForMs(5);
    current = sim.signalValue(resultSignalId)
    assert(current === 0);

    return;
}

// Test for AFB TCT (OpCode 3), conf 3
// Schema: TEST_TCT_V209_CONF3_VIBR
//
function testAfbTctV209Conf3(sim)
{
    // AFB tct_off,
    // Label TEST_TCT_V209_CONF3_VIBR_8421
    // time 200ms
    // in - #TEST_TCT_CONF3_TMAN_IN
    // out - #TEST_TCT_CONF3_TMAN_R

    // 0. Initially initiator must be in 0
    // 1. set initiator to 1, Result must became 1 immediately!
    // R. Check result 1 for 200ms, and then fall to 0
    //
    let workcycle = 5;
    let time = 200; // ms
    let timerCounter = time / workcycle;
    let initiatorSignalId = "#TEST_TCT_CONF3_TMAN_IN";
    let resultSignalId = "#TEST_TCT_CONF3_TMAN_R";

    // 0.
    //
    assert(sim.signalValue(initiatorSignalId) === 0);
    assert(sim.signalValue(resultSignalId) === 0);

    // 1
    //
    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 1
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 1);
    }

    for (let i = 0; i < 10; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 0);
    }

    sim.overridesReset();
    sim.startForMs(5);

    // 0. Initially initiator must be in 0
    // 1. set initiator to 101, Result must became 1 immediately!
    // R. Check result 1 (after the first 1 on initial) for 200ms, and then fall to 0
    //

    // 0.
    //
    assert(sim.signalValue(initiatorSignalId) === 0);
    assert(sim.signalValue(resultSignalId) === 0);

    // 1.
    //
    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 0);
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 0
    //
    for (let i = 0; i < 160 / workcycle; i++)   // 160  because two startForMs(20) already made 40ms
    {
        sim.startForMs(workcycle);
        assert(sim.signalValue(resultSignalId) === 1);
    }

    for (let i = 0; i < 10; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 0);
    }

    return;
}

// Test for AFB TCT (OpCode 3), conf 4
// Schema: TEST_TCT_V209_CONF4_FILTER
//
function testAfbTctV209Conf4(sim)
{
    // AFB tct_filter,
    // Label TEST_TCT_CONF4_FILTER_4009
    // time 200ms
    // in - #TEST_TCT_CONF4_TMAN_IN
    // out - #TEST_TCT_CONF4_TMAN_R

    // 0. Initially initiator must be in 0
    // 1. set initiator to 1
    // R. Check result 0 for 200ms, and then rise to 1
    // -. Some time, check result still in 1
    // 2. Set initiator to 0
    // R. Check result 1 for 200ms, and then fall to 0
    //
    let workcycle = 5;
    let time = 200; // ms
    let timerCounter = time / workcycle;
    let initiatorSignalId = "#TEST_TCT_CONF4_TMAN_IN";
    let resultSignalId = "#TEST_TCT_CONF4_TMAN_R";

    // 0.
    //
    assert(sim.signalValue(initiatorSignalId) === 0);
    assert(sim.signalValue(resultSignalId) === 0);

    // 1
    //
    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 0
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 0);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 1);

    // -. Some time
    //
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);  // Check result still in 1

    // 2. Set initiator to 0
    //
    sim.overrideSignalValue(initiatorSignalId, 0);

    // 200ms result must be in 1
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 1);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 0);

    // 0. Initially initiator must be in 0
    // 1. set initiator to 101
    // R. Check result 0 for 200ms, and then rise to 1
    // -. Some time, check result still in 1
    // 2. Set initiator to 010
    // R. Check result 1 for 200ms, and then fall to 0
    //

    // 0. Initially initiator must be in 0
    //
    sim.overridesReset();
    sim.startForMs(5);

    assert(sim.signalValue(initiatorSignalId) === 0);
    assert(sim.signalValue(resultSignalId) === 0);

    // 1
    //
    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 0);

    sim.overrideSignalValue(initiatorSignalId, 0);
    sim.startForMs(50);
    assert(sim.signalValue(resultSignalId) === 0);

    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 0
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 0);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 1);

    // -. Some time
    //
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);  // Check result still in 1

    // 2. Set initiator to 010
    //
    sim.overrideSignalValue(initiatorSignalId, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 0);

    // 200ms result must be in 1
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 1);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 0);

    return;
}


// Test for AFB TCT (OpCode 3), conf 5
// Schema: TEST_TCT_V209_CONF5_RVIBR
//
function testAfbTctV209Conf5(sim)
{
    // AFB tctc_rsv,
    // Label TEST_TCT_V209_CONF5_RVIBR_8458
    // time 200ms
    // in - #TEST_TCT_CONF5_TMAN_IN
    // out - #TEST_TCT_CONF5_TMAN_R

    // 0. Initially initiator must be in 0
    // 1. set initiator to 1
    // R. Check result 1 for 200ms, and then fall to 0
    // -. Some time, check result still in 0
    //
    let workcycle = 5;
    let time = 200; // ms
    let timerCounter = time / workcycle;
    let initiatorSignalId = "#TEST_TCT_CONF5_TMAN_IN";
    let resultSignalId = "#TEST_TCT_CONF5_TMAN_R";

    // 0.
    //
    assert(sim.signalValue(initiatorSignalId) === 0);
    assert(sim.signalValue(resultSignalId) === 0);

    // 1. set initiator to 1
    //
    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 1
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 1);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 0);

    // -. Some time
    //
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 0);  // Check result still in 0


    // 0. Initially initiator must be in 0
    // 1. set initiator to 101
    // R. Check result 1 for 200ms SINCE LAST 1, and then fall to 0
    // -. Some time, check result still in 0
    //

    // 0.
    //
    sim.overridesReset();
    sim.startForMs(5);

    assert(sim.signalValue(initiatorSignalId) === 0);
    assert(sim.signalValue(resultSignalId) === 0);

    // 1. set initiator to 1
    //
    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(10);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 0);
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.overrideSignalValue(initiatorSignalId, 1);

    // 200ms result must be in 1
    //
    for (let i = 0; i < timerCounter; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 1);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 0);

    // -. Some time
    //
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 0);  // Check result still in 0

    return;
}


// Test for AFB TCT (OpCode 3), conf 6
// Schema: TEST_TCT_V209_CONF6_RCFILTER
//
function testAfbTctV209Conf6(sim)
{
    // AFB tctc_rcfilter,
    // Label TEST_TCT_V209_CONF6_RCFILTER_8469
    // time 200ms
    // in - #TEST_TCT_CONF6_TMAN_IN
    // out - #TEST_TCT_CONF6_TMAN_R

    // 0. Initially initiator must be in 0
    // 1. set initiator to 1, wait for 40ms
    // 2. set initiator to 0, wait for 20ms
    // 3. set initiator to 1
    // R. Check result 0 for 240ms (from 1. or 180 from 3.), and then rise to 1   [40 - 20 = 20 (60ms)] + [180ms]

    // -. Some time, check result still in 0
    //
    let workcycle = 5;
    let time = 200; // ms
    let timerCounter = time / workcycle;
    let initiatorSignalId = "#TEST_TCT_CONF6_TMAN_IN";
    let resultSignalId = "#TEST_TCT_CONF6_TMAN_R";

    // 0.
    //
    assert(sim.signalValue(initiatorSignalId) === 0);
    assert(sim.signalValue(resultSignalId) === 0);

    // 1. set initiator to 1, wait for 40ms
    //
    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(40);
    assert(sim.signalValue(resultSignalId) === 0);

    // 2. set initiator to 1, wait for 20ms
    //
    sim.overrideSignalValue(initiatorSignalId, 0);
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 0);

    // 3. set initiator to 1, wait for 40ms
    //
    sim.overrideSignalValue(initiatorSignalId, 1);

    // 180ms result must be in 0
    //
    for (let i = 0; i < 180 / workcycle; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 0);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 1);

    sim.startForMs(200);
    assert(sim.signalValue(resultSignalId) === 1);

    // 0. Initially initiator and result are in 0
    // 1. set initiator to 0, wait for 40ms
    // 2. set initiator to 1, wait for 20ms
    // 3. set initiator to 0
    // R. Check result 1 for 240ms (from 1. or 180 from 3.), and then rise to 0  [40 - 20 = 20 (60ms)] + [180ms]
    //

    // 0. Initially initiator and result are in 0
    //
    assert(sim.signalValue(initiatorSignalId) === 1);
    assert(sim.signalValue(resultSignalId) === 1);

    // 1. set initiator to 0, wait for 40ms
    //
    sim.overrideSignalValue(initiatorSignalId, 0);
    sim.startForMs(40);
    assert(sim.signalValue(resultSignalId) === 1);


    // 2. set initiator to 1, wait for 20ms
    //
    sim.overrideSignalValue(initiatorSignalId, 1);
    sim.startForMs(20);
    assert(sim.signalValue(resultSignalId) === 1);

    // 3. set initiator to 0
    //
    sim.overrideSignalValue(initiatorSignalId, 0);

    // 180ms result must be in 0
    //
    for (let i = 0; i < 180 / workcycle; i++)
    {
        sim.startForMs(5);
        assert(sim.signalValue(resultSignalId) === 1);
    }

    sim.startForMs(5);
    assert(sim.signalValue(resultSignalId) === 0);

    sim.startForMs(200);
    assert(sim.signalValue(resultSignalId) === 0);

    return;
}

// Test for AFB CTUP (OpCode 5)
// Schema: TEST_CTUD_1
//
function testAfbCtud(sim)
{
    // AFB cnt_up, cnt_dn
    //
    let counterSignal = "#TEST_CTUD_T1_CNT";        // 0, 1, 2, 3, 4, 5, ....
    let initiatorSignal = "#TEST_CTUD_T1_CNT1";     // 0, 1, 0, 1, 0, 1, ....
    let resultUpSignal = "#TEST_CTUD_T1_RES1";        // 1, 1, 2, 2, 3, 3, ....
    let resultDnSignal = "#TEST_CTUD_T2_RES1";        // 1, 1, 2, 2, 3, 3, ....

    sim.reset();

    for (let i = 0; i < 10; i++)
    {
        sim.startForMs(5);

        assert(sim.signalValue(counterSignal) === i + 1);
        assert(sim.signalValue(initiatorSignal) === (i + 1) % 2);

        assert(sim.signalValue(resultUpSignal) === Math.floor(i / 2) + 1);
        assert(sim.signalValue(resultDnSignal) === Math.floor((i + 1) / 2) * -1);
    }

    // Test reset signal
    //
    sim.overrideSignalValue("#TEST_CTUD_T1_RESET", 1);
    sim.startForMs(5);

    assert(sim.signalValue(resultUpSignal) === 0);
    assert(sim.signalValue(resultDnSignal) === 0);

    return;
}


// Test for AFB BDEC (OpCode 9), conf 1
// Schema: TEST_BDEC_CONF1
//
function testAfbBDecConf1(sim)
{
    // Test 0 to dec
    //
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R1") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R2") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R3") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R4") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R5") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R6") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R7") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R8") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R9") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R10") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R11") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R12") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R13") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R14") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R15") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R16") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R17") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R18") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R19") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R20") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R21") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R22") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R23") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R24") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R25") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R26") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R27") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R28") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R29") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R30") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R31") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T1R32") === 0);

    // Test 31 to dec
    //
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R1") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R2") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R3") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R4") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R5") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R6") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R7") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R8") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R9") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R10") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R11") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R12") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R13") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R14") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R15") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R16") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R17") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R18") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R19") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R20") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R21") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R22") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R23") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R24") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R25") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R26") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R27") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R28") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R29") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R30") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R31") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T2R32") === 1);

    // Test 1 to dec
    //
    assert(sim.signalValue("#TEST_BDEC_CONF1_T3R1") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T3R2") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T3R3") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T3R4") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF1_T3R5") === 0);

    return;
}

// Test for AFB BCOD (OpCode 8), conf 1
// Schema: TEST_BCOD_CONF1
//
function testAfbBCodConf1(sim)
{
    // Test 0s to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T1R") === 0);
    assert(sim.signalValue("#TEST_BCOD_T1A") === 0);

    // Test 1000... to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T2R") === 0);
    assert(sim.signalValue("#TEST_BCOD_T2A") === 1);

    // Test 0000...1 to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T3R") === 31);
    assert(sim.signalValue("#TEST_BCOD_T3A") === 1);

    // Test 0000...in20-1...0000 to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T4R") === 19);
    assert(sim.signalValue("#TEST_BCOD_T4A") === 1);

    // Test 0000...in20-1100100001 to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T5R") === 19);
    assert(sim.signalValue("#TEST_BCOD_T5A") === 1);

    // Test 0s28ins to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T6R") === 0);
    assert(sim.signalValue("#TEST_BCOD_T6A") === 0);

    // Test 0000001 to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T7R") === 6);
    assert(sim.signalValue("#TEST_BCOD_T7A") === 1);

    // Test 0 to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T8R") === 0);
    assert(sim.signalValue("#TEST_BCOD_T8A") === 0);

    // Test 1 to cod
    //
    assert(sim.signalValue("#TEST_BCOD_T9R") === 0);
    assert(sim.signalValue("#TEST_BCOD_T9A") === 1);

    return;
}

// Test for AFB BCOD (OpCode 8), conf 2
// Schema: TEST_BCOD_CONF2
//
function testAfbBCodConf2(sim)
{
    // Test 0s to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T1R") === 0);

    // Test 1s to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T2R") === -1);

    // Test 0000000....1 to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T3R") === -2147483648);

    // Test 111000000... to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T4R") === 7);

    // Test 10000000000000000000000000000010 to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T5R") === 1073741825);

    // Test 00000000010000000000000000000000 to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T6R") === 512);

    // Test 000000000000000000011 to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T7R") === 1572864);

    // Test 0 to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T8R") === 0);

    // Test 1 to cod_num
    //
    assert(sim.signalValue("#TEST_BCOD_CONF2_T9R") === 1);

    return;
}


// Test for AFB BDEC (OpCode 9), conf 2
// Schema: TEST_BDEC_CONF2
//
function testAfbBDecConf2(sim)
{
    // Test 0 to dec_num
    //
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R1") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R2") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R3") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R4") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R5") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R6") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R7") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R8") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R9") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R10") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R11") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R12") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R13") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R14") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R15") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R16") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R17") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R18") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R19") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R20") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R21") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R22") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R23") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R24") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R25") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R26") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R27") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R28") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R29") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R30") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R31") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T1R32") === 0);

    // Test -1 to dec_num
    //
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R1") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R2") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R3") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R4") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R5") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R6") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R7") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R8") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R9") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R10") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R11") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R12") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R13") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R14") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R15") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R16") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R17") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R18") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R19") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R20") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R21") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R22") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R23") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R24") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R25") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R26") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R27") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R28") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R29") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R30") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R31") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T2R32") === 1);

    // Test 10 to dec_num
    //
    assert(sim.signalValue("#TEST_BDEC_CONF2_T3R1") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T3R2") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T3R3") === 0);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T3R4") === 1);
    assert(sim.signalValue("#TEST_BDEC_CONF2_T3R5") === 0);

    return;
 }

// Test for AFB BCOMP (OpCode 10)
// Schema: TEST_BCOMP_FP_1
//
function testAfbBCompFp(sim)
{
    // cmpc_fp_ls
    //
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T1R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T1RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T2R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T2RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T3R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T3RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T4R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T4RNAN") === 0);

    //  Test steps:
    //    1. Initial value 0, expected result 1
    //    2. Set input to 150, expected result 0
    //    3. Set input to 99, expected result 1
    //    4. Set input to 104, expected result 1
    //    5. Set input to 105, expected result 0
    //
    sim.overrideSignalValue("#TEST_BCOMP_FP_1_LESS_T5IN", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_FP_1_LESS_T5IN", 150);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_FP_1_LESS_T5IN", 99);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_FP_1_LESS_T5IN", 104);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_FP_1_LESS_T5IN", 105);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_LESS_T5RNAN") === 0);


    // cmpc_fp_gr
    //
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T1R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T1RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T2R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T2RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T3R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T3RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T4R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T4RNAN") === 0);

    // Test steps:
    //    1. Initial value 0, expected result 0
    //    2. Set input to 150, expected result 1
    //    3. Set input to 96, expected result 1
    //    4. Set input to 95, expected result 0
    //    5. Set input to 101, expected result 1
    //
    sim.overrideSignalValue("#TEST_BCOMP_FP_1_GR_T5IN", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_FP_1_GR_T5IN", 150);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5RNAN") === 0);


    sim.overrideSignalValue("#TEST_BCOMP_FP_1_GR_T5IN", 96);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_FP_1_GR_T5IN", 95);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_FP_1_GR_T5IN", 101);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_GR_T5RNAN") === 0);

    // cmpc_fp_eq
    //
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T1R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T1RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T2R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T2RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T3R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T3RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T4R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T4RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T5R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T5RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T6R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T6RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T7R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T7RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T8R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T8RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T9R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T9RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T10R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T10RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T11R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T11RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T12R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T12RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T13R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T13RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T14R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_EQ_T14RNAN") === 0);

    // cmpc_fp_ne
    //
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT1R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT1RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT2R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT2RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT3R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT3RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT4R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT4RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT5RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT6R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT6RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT7R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT7RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT8R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT8RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT9R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT9RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT10R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT10RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT11R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT11RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT12R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT12RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT13R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT13RNAN") === 0);

    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT14R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_FP_1_NEQT14RNAN") === 0);

    return;
}


// Test for AFB BCOMP (OpCode 10)
// Schema: TEST_BCOMP_SI_1
//
function testAfbBCompSi(sim)
{
    // cmpc_si_ls
    //
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T1R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T2R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T3R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T4R") === 1);

    //  Test steps:
    //    1. Initial value 0, expected result 1
    //    2. Set input to 150, expected result 0
    //    3. Set input to 99, expected result 1
    //    4. Set input to 104, expected result 1
    //    5. Set input to 105, expected result 0
    //
    sim.overrideSignalValue("#TEST_BCOMP_SI_1_LESS_T5IN", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T5R") === 1);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_LESS_T5IN", 150);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T5R") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_LESS_T5IN", 99);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T5R") === 1);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_LESS_T5IN", 104);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T5R") === 1);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_LESS_T5IN", 105);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_LESS_T5R") === 0);

    // cmpc_si_gr
    //
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T1R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T2R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T3R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T4R") === 0);

    // Test steps:
    //    1. Initial value 0, expected result 0
    //    2. Set input to 150, expected result 1
    //    3. Set input to 96, expected result 1
    //    4. Set input to 95, expected result 0
    //    5. Set input to 101, expected result 1
    //
    sim.overrideSignalValue("#TEST_BCOMP_SI_1_GR_T5IN", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T5R") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_GR_T5IN", 150);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T5R") === 1);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_GR_T5IN", 96);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T5R") === 1);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_GR_T5IN", 95);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T5R") === 0);

    sim.overrideSignalValue("#TEST_BCOMP_SI_1_GR_T5IN", 101);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_GR_T5R") === 1);

    // cmpc_si_eq
    //
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T1R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T2R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T3R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T4R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T5R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T6R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T7R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T8R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T9R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T10R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T11R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T12R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T13R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_EQ_T14R") === 0);

    // cmpc_si_ne
    //
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT1R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT2R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT3R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT4R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT5R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT6R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT7R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT8R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT9R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT10R") === 1);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT11R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT12R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT13R") === 0);
    assert(sim.signalValue("#TEST_BCOMP_SI_1_NEQT14R") === 1);

    return;
}


// Test for AFB FUNC (OpCode 16)
// Schema: TEST_FUNC_V3
//
function testAfbFuncV3(sim)
{
    // 9 -> math_sqrt_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T1_RESULT") === 3);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T1_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T1_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T1_RNAN") === 0);

    // 0 -> math_sqrt_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T2_RESULT") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T2_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T2_RZ") === 1);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T2_RNAN") === 0);

    // inf -> math_sqrt_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T3_RESULT") === Number.POSITIVE_INFINITY);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T3_ROV") === 1);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T3_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T3_RNAN") === 0);

    // -inf -> math_sqrt_fp
    //
    assert(isNaN(sim.signalValue("#TEST_FUNC_V3_C1_T4_RESULT")));
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T4_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T4_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T4_RNAN") === 1);

    // -9 -> math_sqrt_fp
    //
    assert(isNaN(sim.signalValue("#TEST_FUNC_V3_C1_T5_RESULT")));
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T5_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T5_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C1_T5_RNAN") === 1);

    // 9 -> math_abs_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C2_T1_RESULT") === 9);

    // -9 -> math_abs_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C2_T2_RESULT") === 9);

    // NaN -> math_abs_fp
    //
    assert(isNaN(sim.signalValue("#TEST_FUNC_V3_C2_T3_RESULT")));

    // inf -> math_abs_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C2_T4_RESULT") === Number.POSITIVE_INFINITY);

    // -inf -> math_abs_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C2_T5_RESULT") === Number.POSITIVE_INFINITY);

    // 0 -> math_abs_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C2_T6_RESULT") === 0);

    // 10 -> math_inv_fp
    //
    assert(Math.abs(sim.signalValue("#TEST_FUNC_V3_C7_T1_RESULT") - 0.1000) < 0.000001);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T1_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T1_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T1_RNAN") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T1_RDBZ") === 0);

    // nan -> math_inv_fp
    //
    assert(isNaN(sim.signalValue("#TEST_FUNC_V3_C7_T2_RESULT")));
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T2_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T2_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T2_RNAN") === 1);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T2_RDBZ") === 0);

    // inf -> math_inv_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T3_RESULT") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T3_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T3_RZ") === 1);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T3_RNAN") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T3_RDBZ") === 0);

    // 0 -> math_inv_fp
    //
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T4_RESULT") === Number.POSITIVE_INFINITY);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T4_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T4_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T4_RNAN") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T4_RDBZ") === 1);

    // -10 -> math_inv_fp
    //
    assert(Math.abs(sim.signalValue("#TEST_FUNC_V3_C7_T5_RESULT") + 0.1000) < 0.000001);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T5_ROV") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T5_RZ") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T5_RNAN") === 0);
    assert(sim.signalValue("#TEST_FUNC_V3_C7_T5_RDBZ") === 0);

    return;
}


// Test for AFB DPCOMP (OpCode 20)
// Schema: TEST_DPCOMP_FP_1
//
function testAfbDpCompCompFp1(sim)
{
    // cmp_fp_ls
    //
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T1R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T1ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T1RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T1RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T2R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T2ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T2RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T2RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T3R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T3ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T3RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T3RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T4R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T4ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T4RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T4RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T6R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T6ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T6RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T6RNAN") === 1)

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T7R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T7ROV") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T7RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T7RNAN") === 0)

    // Test steps:
    //    1. Initial value 0, expected result 1
    //    2. Set input to 150, expected result 0
    //    3. Set input to 99, expected result 1
    //    4. Set input to 104, expected result 1
    //    5. Set input to 105, expected result 0
    //
    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_LESS_T5IN", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_LESS_T5IN", 150);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_LESS_T5IN", 99);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_LESS_T5IN", 104);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_LESS_T5IN", 105);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_LESS_T5RNAN") === 0);

    // cmp_fp_gr
    //
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T1R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T1ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T1RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T1RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T2R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T2ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T2RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T2RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T3R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T3RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T4R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T4RNAN") === 0);

    // Test steps:
    //    1. Initial value 0, expected result 0
    //    2. Set input to 150, expected result 1
    //    3. Set input to 96, expected result 1
    //    4. Set input to 95, expected result 0
    //    5. Set input to 101, expected result 1
    //
    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_GR_T5IN", 0);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_GR_T5IN", 150);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_GR_T5IN", 96);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_GR_T5IN", 95);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5RNAN") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_FP_1_GR_T5IN", 101);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_GR_T5RNAN") === 0);

    // cmp_fp_eq
    //
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T1R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T1ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T1RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T1RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T2R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T2ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T2RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T2RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T3R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T3ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T3RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T3RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T4R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T4ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T4RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T4RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T5R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T5ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T5RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T5RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T6R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T6ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T6RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T6RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T8R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T8ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T8RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T8RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T9R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T9ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T9RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T9RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T10R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T10ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T10RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T10RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T11R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T11ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T11RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T11RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T12R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T12ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T12RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T12RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T13R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T13ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T13RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T13RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T14R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T14ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T14RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_EQ_T14RNAN") === 0);

    // cmp_fp_ne
    //
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT1R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT1ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT1RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT1RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT2R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT2ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT2RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT2RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT3R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT3ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT3RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT3RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT4R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT4ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT4RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT4RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT5ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT5RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT5RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT6R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT6ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT6RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT6RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT7R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT7ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT7RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT7RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT8R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT8ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT8RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT8RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT9R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT9ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT9RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT9RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT10R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT10ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT10RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT10RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT11R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT11ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT11RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT11RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT12R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT12ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT12RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT12RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT13RNAN") === 0);

    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT14R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT14ROV") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT14RUF") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_FP_1_NEQT14RNAN") === 0);

    return;
}



// Test for AFB DPCOMP (OpCode 20)
// Schema: TEST_DPCOMP_SI_1
//
function testAfbDpCompCompSi1(sim)
{
    // cmp_si_ls
    //
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T1R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T2R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T3R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T4R") === 1);

    // Test steps:
    //    1. Initial value 0, expected result 1
    //    2. Set input to 150, expected result 0
    //    3. Set input to 99, expected result 1
    //    4. Set input to 104, expected result 1
    //    5. Set input to 105, expected result 0
    //
    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_LESS_T5IN", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T5R") === 1);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_LESS_T5IN", 150);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T5R") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_LESS_T5IN", 99);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T5R") === 1);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_LESS_T5IN", 104);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T5R") === 1);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_LESS_T5IN", 105);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_LESS_T5R") === 0);

    // cmp_si_gr
    //
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T1R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T2R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T3R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T4R") === 0);

    // Test steps:
    //    1. Initial value 0, expected result 0
    //    2. Set input to 150, expected result 1
    //    3. Set input to 96, expected result 1
    //    4. Set input to 95, expected result 0
    //    5. Set input to 101, expected result 1
    //
    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_GR_T5IN", 0);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T5R") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_GR_T5IN", 150);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T5R") === 1);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_GR_T5IN", 96);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T5R") === 1);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_GR_T5IN", 95);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T5R") === 0);

    sim.overrideSignalValue("#TEST_DPCOMP_SI_1_GR_T5IN", 101);
    sim.startForMs(150);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_GR_T5R") === 1);

    // cmp_si_eq
    //
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T1R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T2R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T3R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T4R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T5R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T6R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T7R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T8R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T9R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T10R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T11R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T12R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T13R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_EQ_T14R") === 0);

    // cmp_si_ne
    //
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT1R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT2R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT3R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT4R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT5R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT6R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT7R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT8R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT9R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT10R") === 1);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT11R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT12R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT13R") === 0);
    assert(sim.signalValue("#TEST_DPCOMP_SI_1_NEQT14R") === 1);

    return;
}



// Test for AFB PULSE_GET (OpCode 30)
// Schema: TEST_PULSE_GET_V0
//
function testAfbPulseGenV0(sim)
{
    // High 250
    // Low 500
    //
    let workcycle = 5;
    let high = 250 / workcycle;
    let low = 500 / workcycle;
    let signalId = "#TEST_PULSEGEN_V0_T1_RESULT";

    // Find the end of 0 phase
    //
    let last = sim.signalValue(signalId)
    let foundStart1 = false;

    for (let i = 0; i < (high + low + 1); i++)
    {
        sim.startForMs(5);

        let current = sim.signalValue(signalId)
        if (last === 0 && current === 1)
        {
            foundStart1 = true;
            break;
        }

        last = current;
    }

    assert(foundStart1);

    // Now LogicModule started 1 phase, and it already took 5 ms
    //
    for (let i = 0; i < high - 1; i++)
    {
        sim.startForMs(5);

        let current = sim.signalValue(signalId)
        assert(current === 1);
    }

    // Now LogicModule should start 0 phase
    //
    for (let i = 0; i < low; i++)
    {
        sim.startForMs(5);

        let current = sim.signalValue(signalId)
        assert(current === 0);
    }

    // Check that after 0 phase the result switched back to 1
    //
    sim.startForMs(5);

    let current = sim.signalValue(signalId)
    assert(current === 1);

    return;
}


