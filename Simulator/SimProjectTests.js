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
    sim.connectionsSetEnabled(true);
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

// Test for AFB FLIPFLOP (OpCode 4), Conf 1
// Schema: TEST_FLIPFLOP_V106
//
function testAfbFilpFlopV106Conf1(sim)
{
    // Conf 1, SR
    //    Manual Tests:
    //    Signals: #TEST_FF_V106_T1_SET, #TEST_FF_V106_T1_RESET
    //    Sequence is important!
    //    1. Initial values 0, 0 = 0
    //    2. Set values 1, 0 = 1
    //    3. Set values 0, 0 = 1
    //    4. Set values 0, 1 = 0
    //    5. Set values 1, 1 = 1
    //

    let setSignal = "#TEST_FF_V106_T1_SET";
    let resetSignal = "#TEST_FF_V106_T1_RESET";
    let resultSignal = "#TEST_FF_V106_T1_RESULT";

    //    1. Initial values 0, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    2. Set values 1, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    3. Set values 0, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    4. Set values 0, 1 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    5. Set values 1, 1 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    return;
}


// Test for AFB FLIPFLOP (OpCode 4), Conf 2 RS
// Schema: TEST_FLIPFLOP_V106
//
function testAfbFilpFlopV106Conf2(sim)
{
    // Conf 2, RS
    //    Manual Tests:
    //    Signals: #TEST_FF_V106_T2_SET, #TEST_FF_V106_T2_RESET
    //    Sequence is important!
    //    1. Initial values 0, 0 = 0
    //    2. Set values 1, 0 = 1
    //    3. Set values 0, 0 = 1
    //    4. Set values 0, 1 = 0
    //    5. Set values 1, 1 = 0
    //    6. Set values 1, 0 = 1
    //

    let setSignal = "#TEST_FF_V106_T2_SET";
    let resetSignal = "#TEST_FF_V106_T2_RESET";
    let resultSignal = "#TEST_FF_V106_T2_RESULT";

    //    1. Initial values 0, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    2. Set values 1, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    3. Set values 0, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    4. Set values 0, 1 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    5. Set values 1, 1 = 0
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    6. Set values 1, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    return;
}


// Test for AFB FLIPFLOP (OpCode 4), Conf 3 D ON FRONT
// Schema: TEST_FLIPFLOP_V106
//
function testAfbFilpFlopV106Conf3(sim)
{
    // Conf 3, D ON FRONT
    //    Manual Tests:
    //    Signals: #TEST_FF_V106_T3_D, #TEST_FF_V106_T3_C
    //    1. Initial values 0, 0 = 0
    //    2. Set values 1, 0 = 0
    //    3. Set values 1, 1 = 1
    //    4. Set values 0, 1 = 1
    //    5. Set values 0, 0 = 1
    //    6. Set values 0, 1 = 0
    //    7. Set values 0, 0 = 0
    //
    let setSignal = "#TEST_FF_V106_T3_D";
    let resetSignal = "#TEST_FF_V106_T3_C";
    let resultSignal = "#TEST_FF_V106_T3_RESULT";

    //    1. Initial values 0, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    2. Set values 1, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    3. Set values 1, 1 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    4. Set values 0, 1 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    5. Set values 0, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    6. Set values 0, 1 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    7. Set values 0, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    return;
}


// Test for AFB FLIPFLOP (OpCode 4), Conf 4 T ON FRONT
// Schema: TEST_FLIPFLOP_V106
//
function testAfbFilpFlopV106Conf4(sim)
{
    // Conf 4, T ON FRONT
    //    Manual Tests:
    //    Signal: #TEST_FF_V106_T4_T
    //
    //    1. Initial values 0 = 0
    //    2. Set 1 = 1
    //    3. Set 0 = 1
    //    4. Set 1 = 0
    //    5. Set 0 = 0
    //    6. Set 1 = 1
    //
    let setSignal = "#TEST_FF_V106_T4_T";
    let resultSignal = "#TEST_FF_V106_T4_RESULT";

    //    1. Initial values 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    2. Set 1 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    3. Set 0 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    4. Set 1 = 0
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    5. Set 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    6. Set 1 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    return;
}

// Test for AFB FLIPFLOP (OpCode 4), Conf 5, D ON DECAY
// Schema: TEST_FLIPFLOP_V106
//
function testAfbFilpFlopV106Conf5(sim)
{
    // Conf 5, D ON DECAY
    //    Manual Tests:
    //    Signal: #TEST_FF_V106_T5_D, #TEST_FF_V106_T5_C
    //
    //    1. Initial values 0, 0 = 0
    //    2. Set values 1, 0 = 0
    //    3. Set values 1, 1 = 0
    //    4. Set values 1, 0 = 1
    //    5. Set values 0, 0 = 1
    //    6. Set values 0, 1 = 1
    //    7. Set values 0, 0 = 0
    //
    let setSignal = "#TEST_FF_V106_T5_D";
    let resetSignal = "#TEST_FF_V106_T5_C";
    let resultSignal = "#TEST_FF_V106_T5_RESULT";

    //    1. Initial values 0, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    2. Set values 1, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    3. Set values 1, 1 = 0
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    4. Set values 1, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    5. Set values 0, 0 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    6. Set values 0, 1 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    7. Set values 0, 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.overrideSignalValue(resetSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    return;
}


// Test for AFB FLIPFLOP (OpCode 4), Conf 6 T ON DECAY
// Schema: TEST_FLIPFLOP_V106
//
function testAfbFilpFlopV106Conf6(sim)
{
    // Conf 6, T ON DECAY
    //    Manual Tests:
    //    Signal: #TEST_FF_V106_T6_T
    //
    //    1. Initial values 0 = 0
    //    2. Set 1 = 0
    //    3. Set 0 = 1
    //    4. Set 1 = 1
    //    5. Set 0 = 0
    //    6. Set 1 = 0
    //    7. Set 0 = 1
    //
    let setSignal = "#TEST_FF_V106_T6_T";
    let resultSignal = "#TEST_FF_V106_T6_RESULT";

    //    1. Initial values 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    2. Set 1 = 0
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    3. Set 0 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    4. Set 1 = 1
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

    //    5. Set 0 = 0
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    6. Set 1 = 0
    //
    sim.overrideSignalValue(setSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);

    //    7. Set 0 = 1
    //
    sim.overrideSignalValue(setSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);

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

// Test for AFB MAJ (OpCode 6)
// Schema: TEST_MAJ_V107
//
function testAfbMajV107(sim)
{
    assert(sim.signalValue("#TEST_MAJ_V107_T1_RES1") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T1_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T1_RES3") === 0);

    assert(sim.signalValue("#TEST_MAJ_V107_T2_RES1") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T2_RES2") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T2_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T3_RES1") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T3_RES2") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T3_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T4_RES1") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T4_RES2") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T4_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T5_RES1") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T5_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T5_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T6_RES1") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T6_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T6_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T7_RES1") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T7_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T7_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T8_RES1") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T8_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T8_RES3") === 0);

    // --
    //
    assert(sim.signalValue("#TEST_MAJ_V107_T10_RES1") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T10_RES2") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T10_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T11_RES1") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T11_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T11_RES3") === 1);

    // --
    //
    assert(sim.signalValue("#TEST_MAJ_V107_T20_RES1") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T20_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T20_RES3") === 0);

    assert(sim.signalValue("#TEST_MAJ_V107_T21_RES1") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T21_RES2") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T21_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T22_RES1") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T22_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T22_RES3") === 1);

    assert(sim.signalValue("#TEST_MAJ_V107_T23_RES1") === 1);
    assert(sim.signalValue("#TEST_MAJ_V107_T23_RES2") === 0);
    assert(sim.signalValue("#TEST_MAJ_V107_T23_RES3") === 0);

    return;
}

// Test for AFB SRSST (OpCode 8)
// Schema: TEST_SRSST_V104
//
function testAfbSrsstV104(sim)
{
    //  Test steps:
    //    1. Initial 0, 0 = 0
    //    2. Set 1, 0 = 1
    //    3. Set 1, 1 = 0
    //    4. Set 0, 1 = 0
    //    5. Set 0, 0 = 0
    //
    let simSignal = "#TEST_SRSST_V104_T1_SIM";
    let blockSignal = "#TEST_SRSST_V104_T1_BLOCK";
    let resultSignal = "#TEST_SRSST_V104_T1_RESULT";

    //    1. Initial 0, 0 = 0
    //
    sim.overrideSignalValue(simSignal, 0);
    sim.overrideSignalValue(blockSignal, 0);
    sim.startForMs(5);
    let state = sim.signalState(resultSignal);
    assert(state.value === 0);
    assert(state.simulated === false);
    assert(state.blocked === false);

    //    2. Set 1, 0 = 1
    //
    sim.overrideSignalValue(simSignal, 1);
    sim.overrideSignalValue(blockSignal, 0);
    sim.startForMs(5);
    state = sim.signalState(resultSignal);
    assert(state.value === 1);
    assert(state.simulated === true);
    assert(state.blocked === false);

    //    3. Set 1, 1 = 0
    //
    sim.overrideSignalValue(simSignal, 1);
    sim.overrideSignalValue(blockSignal, 1);
    sim.startForMs(5);
    state = sim.signalState(resultSignal);
    assert(state.value === 0);
    assert(state.simulated === true);
    assert(state.blocked === true);

    //    4. Set 0, 1 = 0
    //
    sim.overrideSignalValue(simSignal, 0);
    sim.overrideSignalValue(blockSignal, 1);
    sim.startForMs(5);
    state = sim.signalState(resultSignal);
    assert(state.value === 0);
    assert(state.simulated === false);
    assert(state.blocked === true);

    //    5. Set 0, 0 = 0
    //
    sim.overrideSignalValue(simSignal, 0);
    sim.overrideSignalValue(blockSignal, 0);
    sim.startForMs(5);
    state = sim.signalState(resultSignal);
    assert(state.value === 0);
    assert(state.simulated === false);
    assert(state.blocked === false);


    //  Test steps:
    //    1. Initial 0, 0 = 1
    //    2. Set 1, 0 = 1
    //    3. Set 1, 1 = 0
    //    4. Set 0, 1 = 0
    //    5. Set 0, 0 = 1
    //
    simSignal = "#TEST_SRSST_V104_T2_SIM";
    blockSignal = "#TEST_SRSST_V104_T2_BLOCK";
    resultSignal = "#TEST_SRSST_V104_T2_RESULT";

    //    1. Initial 0, 0 = 1
    //
    sim.overrideSignalValue(simSignal, 0);
    sim.overrideSignalValue(blockSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);
    state = sim.signalState(resultSignal);
    assert(state.simulated === false);              // Itme has proprty assignflags == false, thus flags must not be assigned
    assert(state.blocked === false);

    //    2. Set 1, 0 = 1
    //
    sim.overrideSignalValue(simSignal, 1);
    sim.overrideSignalValue(blockSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);
    state = sim.signalState(resultSignal);
    assert(state.simulated === false);              // Itme has proprty assignflags == false, thus flags must not be assigned
    assert(state.blocked === false);

    //    3. Set 1, 1 = 0
    //
    sim.overrideSignalValue(simSignal, 1);
    sim.overrideSignalValue(blockSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);
    state = sim.signalState(resultSignal);
    assert(state.simulated === false);              // Itme has proprty assignflags == false, thus flags must not be assigned
    assert(state.blocked === false);

    //    4. Set 0, 1 = 0
    //
    sim.overrideSignalValue(simSignal, 0);
    sim.overrideSignalValue(blockSignal, 1);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 0);
    state = sim.signalState(resultSignal);
    assert(state.simulated === false);              // Itme has proprty assignflags == false, thus flags must not be assigned
    assert(state.blocked === false);

    //    5. Set 0, 0 = 1
    //
    sim.overrideSignalValue(simSignal, 0);
    sim.overrideSignalValue(blockSignal, 0);
    sim.startForMs(5);
    assert(sim.signalValue(resultSignal) === 1);
    state = sim.signalState(resultSignal);
    assert(state.simulated === false);              // Itme has proprty assignflags == false, thus flags must not be assigned
    assert(state.blocked === false);

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


// Test for AFB DAMPER (OpCode 11)
// Schema: TEST_DAMPER
//
function testAfbDamper(sim)
{
    sim.reset();

    sim.startForMs(5);

    assert(sim.signalValue("#TEST_MUX_FP_T2_OV") === 0);
    assert(sim.signalValue("#TEST_MUX_FP_T2_UF") === 0);
    assert(sim.signalValue("#TEST_MUX_FP_T2_ZERO") === 1);
    assert(sim.signalValue("#TEST_MUX_FP_T2_NAN") === 0);
    assert(sim.signalValue("#TEST_MUX_FP_T2_PE") === 0);

    assert(sim.signalValue("#TEST_MUX_SI_T3_ZERO") === 1);
    assert(sim.signalValue("#TEST_MUX_SI_T3_PE") === 0);

    // Start damper from 0 to 1000
    //
    sim.overrideSignalValue("#TEST_DAMPER_FP_T2_SEL", 1);

    // 63.2% - 600 (+-1) cylces - out value 632
    //
    sim.startForMs(600 * 5);

    assert(sim.signalValue("#TEST_MUX_FP_T2_ZERO") === 0);
    assert(sim.signalValue("#TEST_MUX_SI_T3_ZERO") === 0);

    let fpv = sim.signalValue("#TEST_MUX_FP_T2_OUT");
    let siv = sim.signalValue("#TEST_MUX_SI_T3_OUT");

    assert(fpv >= 631.5 && fpv <= 632.5);
    assert(fpv >= 631 && fpv <= 633);

    // 86.5% - 1200 (+-1) cylces - out value 865
    //
    sim.startForMs(600 * 5);

    fpv = sim.signalValue("#TEST_MUX_FP_T2_OUT");
    siv = sim.signalValue("#TEST_MUX_SI_T3_OUT");

    assert(fpv >= 864.5 && fpv <= 865.5);
    assert(fpv >= 664 && fpv <= 8655);

    return;
}

// Test for AFB MEDIAN (OpCode 12)
// Schema: TEST_MEDIAN_V7
//
function testAfbMedianV7(sim)
{
    // median_si -- Test for two valid inputs
    //
    assert(sim.signalValue("#TEST_MED_V7_T1_RMED") === 80);
    assert(sim.signalValue("#TEST_MED_V7_T1_RMAX") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T1_RMIN") === 0);

    // median_si -- Test for one valid inputs
    //
    assert(sim.signalValue("#TEST_MED_V7_T2_RMED") === 100);
    assert(sim.signalValue("#TEST_MED_V7_T2_RMAX") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T2_RMIN") === 0);

    // median_si -- Test for all valid inputs
    //
    assert(sim.signalValue("#TEST_MED_V7_T3_RMED") === 70);
    assert(sim.signalValue("#TEST_MED_V7_T3_RMAX") === 100);
    assert(sim.signalValue("#TEST_MED_V7_T3_RMIN") === 50);


    // median_fp -- Test for two valid inputs
    //
    assert(sim.signalValue("#TEST_MED_V7_T11_RMED") === 80);
    assert(sim.signalValue("#TEST_MED_V7_T11_RMAX") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T11_RMIN") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T11_ROV") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T11_RUF") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T11_RZ") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T11_RNAN") === 0);

    // median_fp -- Test for one valid inputs
    //
    assert(sim.signalValue("#TEST_MED_V7_T12_RMED") === 100);
    assert(sim.signalValue("#TEST_MED_V7_T12_RMAX") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T12_RMIN") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T12_ROV") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T12_RUF") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T12_RZ") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T12_RNAN") === 0);

    // median_fp -- Test for all valid inputs
    //
    assert(sim.signalValue("#TEST_MED_V7_T13_RMED") === 70);
    assert(sim.signalValue("#TEST_MED_V7_T13_RMAX") === 100);
    assert(sim.signalValue("#TEST_MED_V7_T13_RMIN") === 50);
    assert(sim.signalValue("#TEST_MED_V7_T13_ROV") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T13_RUF") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T13_RZ") === 0);
    assert(sim.signalValue("#TEST_MED_V7_T13_RNAN") === 0);

    return;
}



// Test for AFB MATH (OpCode 13)
// Schema: TEST_MATH_FP_1
//
function testAfbMathFp(sim)
{
    assert(sim.signalValue("#TEST_MATH_FP_ADD1") === 7);
    assert(sim.signalValue("#TEST_MATH_FP_ADD2") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD3") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD4") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD5") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_ADD6") === Number.POSITIVE_INFINITY);
    assert(sim.signalValue("#TEST_MATH_FP_ADD7") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_ADD8") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD9") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD10") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_ADD11") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD12") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD13") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_ADD14") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_ADD15") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_SUB1") === -13);
    assert(sim.signalValue("#TEST_MATH_FP_SUB2") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB3") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB4") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB5") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_SUB6") === Number.NEGATIVE_INFINITY);
    assert(sim.signalValue("#TEST_MATH_FP_SUB7") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_SUB8") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB9") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB10") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_SUB11") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB12") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB13") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_SUB14") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_SUB15") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_MIL1") === -30);
    assert(sim.signalValue("#TEST_MATH_FP_MIL2") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL3") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL4") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL5") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_MIL6") === Number.POSITIVE_INFINITY);
    assert(sim.signalValue("#TEST_MATH_FP_MIL7") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_MIL8") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL9") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL10") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_MIL11") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL12") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL13") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_MIL14") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_MIL15") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_DIV1") === 6);
    assert(sim.signalValue("#TEST_MATH_FP_DIV2") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV3") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV4") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV55") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV6") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_DIV7") <= (1.9e-38));  // nearly zero
    assert(sim.signalValue("#TEST_MATH_FP_DIV8") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV9") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_DIV10") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_DIV11") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV12") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_DIV13") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV14") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV15") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV16") === 1);
    assert(sim.signalValue("#TEST_MATH_FP_DIV17") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV18") === 0);

    assert(sim.signalValue("#TEST_MATH_FP_DIV19") === Number.POSITIVE_INFINITY);
    assert(sim.signalValue("#TEST_MATH_FP_DIV20") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV21") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV22") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV23") === 0);
    assert(sim.signalValue("#TEST_MATH_FP_DIV24") === 1);

    return;
}

// Test for AFB MATH (OpCode 13)
// Schema: TEST_MATH_SI_1
//
function testAfbMathSi(sim)
{
    assert(sim.signalValue("#TEST_MATH_SI_ADD1") === 7);
    assert(sim.signalValue("#TEST_MATH_SI_ADD2") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_ADD3") === 0);

    assert(sim.signalValue("#TEST_MATH_SI_ADD7") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_ADD8") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_ADD9") === 1);

    assert(sim.signalValue("#TEST_MATH_SI_ADD10") === 2147483647);
    assert(sim.signalValue("#TEST_MATH_SI_ADD11") === 1);
    assert(sim.signalValue("#TEST_MATH_SI_ADD12") === 0);


    assert(sim.signalValue("#TEST_MATH_SI_SUB1") === -1);
    assert(sim.signalValue("#TEST_MATH_SI_SUB2") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_SUB3") === 0);

    assert(sim.signalValue("#TEST_MATH_SI_SUB7") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_SUB8") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_SUB9") === 1);

    assert(sim.signalValue("#TEST_MATH_SI_SUB10") === 2147483647);
    assert(sim.signalValue("#TEST_MATH_SI_SUB11") === 1);
    assert(sim.signalValue("#TEST_MATH_SI_SUB12") === 0);


    assert(sim.signalValue("#TEST_MATH_SI_MUL1") === -12);
    assert(sim.signalValue("#TEST_MATH_SI_MUL2") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_MUL3") === 0);

    assert(sim.signalValue("#TEST_MATH_SI_MUL7") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_MUL8") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_MUL9") === 1);

    assert(sim.signalValue("#TEST_MATH_SI_MUL10") === 2147483647);
    assert(sim.signalValue("#TEST_MATH_SI_MUL11") === 1);
    assert(sim.signalValue("#TEST_MATH_SI_MUL12") === 0);


    assert(sim.signalValue("#TEST_MATH_SI_DIV1") === -3);
    assert(sim.signalValue("#TEST_MATH_SI_DIV2") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_DIV3") === 0);

    assert(sim.signalValue("#TEST_MATH_SI_DIV7") === -1);
    assert(sim.signalValue("#TEST_MATH_SI_DIV8") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_DIV9") === 1);

    assert(sim.signalValue("#TEST_MATH_SI_DIV10") === 0);
    assert(sim.signalValue("#TEST_MATH_SI_DIV11") === 1);
    assert(sim.signalValue("#TEST_MATH_SI_DIV12") === 0);

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

// Test for AFB MUX (OpCode 21)
// Schema: TEST_MUX
//
function testAfbMux(sim)
{
    sim.reset();

    // switch_si
    //
    sim.overrideSignalValue("#TEST_MUX_SI_T1_SELECTOR", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_SI_T1_RESULT") === 100);

    sim.overrideSignalValue("#TEST_MUX_SI_T1_SELECTOR", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_SI_T1_RESULT") === -200);

    sim.overrideSignalValue("#TEST_MUX_SI_T1_SELECTOR", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_SI_T1_RESULT") === 100);

    // switch_fp
    //
    sim.overrideSignalValue("#TEST_MUX_FP_T1_SELECTOR", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_FP_T1_RESULT") === 100);

    sim.overrideSignalValue("#TEST_MUX_FP_T1_SELECTOR", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_FP_T1_RESULT") === -200);

    sim.overrideSignalValue("#TEST_MUX_FP_T1_SELECTOR", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_FP_T1_RESULT") === 100);

    // bus_switch
    //
    sim.overrideSignalValue("#TEST_MUX_BUS_T1_SELECTOR", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R0") === 1);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R1") === 0);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R2") === 0);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R3") === 1);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R4") === 1);

    sim.overrideSignalValue("#TEST_MUX_BUS_T1_SELECTOR", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R0") === 0);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R1") === 1);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R2") === 1);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R3") === 0);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R4") === 0);

    sim.overrideSignalValue("#TEST_MUX_BUS_T1_SELECTOR", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R0") === 1);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R1") === 0);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R2") === 0);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R3") === 1);
    assert(sim.signalValue("#TEST_MUX_BUS_T1_R4") === 1);

    return;
}


// Test for AFB LIM (OpCode 23)
// Schema: TEST_LIM
//
function testAfbLimiter(sim)
{
    // limc_si
    //
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R1") === 50);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R1MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R1MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T1R2") === -5);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R2MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R2MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T1R3") === -10);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R3MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R3MIN") === 1);

    assert(sim.signalValue("#TEST_LIM_LIMC_T1R4") === -10);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R4MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R4MIN") === 1);

    assert(sim.signalValue("#TEST_LIM_LIMC_T1R5") === 2000);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R5MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R5MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T1R6") === 2000);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R6MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R6MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T1R7") === 2000);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R7MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T1R7MIN") === 0);

    // limc_fp
    //
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R1") === 50);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R2") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R1MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R1MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T2R3") === -5);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R4") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R3MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R3MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T2R5") === -10);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R6") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R5MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R5MIN") === 1);

    assert(sim.signalValue("#TEST_LIM_LIMC_T2R7") === -10);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R8") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R7MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R7MIN") === 1);

    assert(sim.signalValue("#TEST_LIM_LIMC_T2R9") === 2000);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R10") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R9MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R9MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T2R11") === 2000);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R12") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R11MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R11MIN") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T2R13") === 2000);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R14") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R13MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T2R13MIN") === 0);

    // lim_si
    //
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R1") === 50);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R1MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R1MIN") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R2") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T3R3") === 10);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R3MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R3MIN") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R4") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T3R5") === 100);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R5MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R5MIN") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R6") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T3R7") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R7MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R7MIN") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T3R8") === 1);

    // lim_fp
    //
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R1") === 50);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R2") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R1MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R1MIN") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R3") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T4R4") === 10);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R5") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R4MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R4MIN") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R6") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T4R7") === 100);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R8") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R7MAX") === 1);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R7MIN") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R9") === 0);

    assert(sim.signalValue("#TEST_LIM_LIMC_T4R10") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R11") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R10MAX") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R10MIN") === 0);
    assert(sim.signalValue("#TEST_LIM_LIMC_T4R12") === 1);

    return;
}

// Test for AFB POLY (OpCode 25)
// Schema: TEST_POL_V3
//
function testAfbPolyV3(sim)
{
    assert(sim.signalValue("#TEST_POL_V3_T1_RESULT") === 0);
    assert(sim.signalValue("#TEST_POL_V3_T1_ROV") === 0);
    assert(sim.signalValue("#TEST_POL_V3_T1_RUF") === 0);
    assert(sim.signalValue("#TEST_POL_V3_T1_RZERO") === 1);
    assert(sim.signalValue("#TEST_POL_V3_T1_RNAN") === 0);

    assert(isNaN(sim.signalValue("#TEST_POL_V3_T3_RESULT")) === true);
    assert(sim.signalValue("#TEST_POL_V3_T3_ROV") === 0);
    assert(sim.signalValue("#TEST_POL_V3_T3_RUF") === 0);
    assert(sim.signalValue("#TEST_POL_V3_T3_RZERO") === 0);
    assert(sim.signalValue("#TEST_POL_V3_T3_RNAN") === 1);

    // Manual
    // Initial 0   = 0,006228204
    //     Set 100 = 1,033206711        -- 1.033207178
    //     Set 200 = 15,85704089        -- 15.857046127
    //     Set 300 = 87,60783114        -- 87.607826233
    //
    sim.overrideSignalValue("#TEST_POL_V3_T2_X", 0);
    sim.startForMs(5);
    //console.log(sim.signalValue("#TEST_POL_V3_T2_RESULT"));
    assert(sim.signalValue("#TEST_POL_V3_T2_RESULT") >= 0.006228203 && sim.signalValue("#TEST_POL_V3_T2_RESULT") <= 0.006228205);

    sim.overrideSignalValue("#TEST_POL_V3_T2_X", 100);
    sim.startForMs(5);
    //console.log(sim.signalValue("#TEST_POL_V3_T2_RESULT"));
    assert(sim.signalValue("#TEST_POL_V3_T2_RESULT") >= 1.033207178 && sim.signalValue("#TEST_POL_V3_T2_RESULT") <= 1.033207179); //  ???

    sim.overrideSignalValue("#TEST_POL_V3_T2_X", 200);
    sim.startForMs(5);
    //console.log(sim.signalValue("#TEST_POL_V3_T2_RESULT"));
    assert(sim.signalValue("#TEST_POL_V3_T2_RESULT") >= 15.857046127 && sim.signalValue("#TEST_POL_V3_T2_RESULT") <= 15.857046128); //  ???

    sim.overrideSignalValue("#TEST_POL_V3_T2_X", 300);
    sim.startForMs(5);
    //console.log(sim.signalValue("#TEST_POL_V3_T2_RESULT"));
    assert(sim.signalValue("#TEST_POL_V3_T2_RESULT") >= 87.607826232 && sim.signalValue("#TEST_POL_V3_T2_RESULT") <= 87.607826234); //  ???

    return;
}

// Test for AFB MISMATCH (OpCode 27)
// Schema: TEST_MISMATCH_V4
//
function testAfbMismatchV4(sim)
{
    // mismatch_si
    //
    assert(sim.signalValue("#TEST_MISMATCH_V4_T1_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T1_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T1_PE") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T2_R1") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T2_R2") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T2_PE") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T3_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T3_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T3_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T3_PE") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T4_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T4_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T4_R3") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T4_PE") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T5_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T5_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T5_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T5_PE") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T6_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T6_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T6_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T6_R4") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T6_PE") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T7_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T7_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T7_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T7_R4") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T7_PE") === 0);

    sim.overrideSignalValue("#TEST_MISMATCH_V4_TF_PE", 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_TF_VAL") === 0);
    sim.startForMs(5);
    sim.overrideSignalValue("#TEST_MISMATCH_V4_TF_PE", 1);

    // mismatch_fp
    //
    assert(sim.signalValue("#TEST_MISMATCH_V4_T41_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T41_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T41_PE") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T41_OV") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T41_UF") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T41_NAN") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T42_R1") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T42_R2") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T42_PE") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T42_OV") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T42_UF") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T42_NAN") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T43_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T43_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T43_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T43_PE") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T43_OV") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T43_UF") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T43_NAN") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T44_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T44_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T44_R3") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T44_PE") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T44_OV") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T44_UF") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T44_NAN") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T45_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T45_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T45_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T45_PE") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T45_OV") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T45_UF") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T45_NAN") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_R4") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_PE") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_OV") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_UF") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T46_NAN") === 0);

    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_R1") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_R2") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_R3") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_R4") === 1);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_PE") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_OV") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_UF") === 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_T47_NAN") === 0);

    sim.overrideSignalValue("#TEST_MISMATCH_V4_TFL_VAL", 0);
    assert(sim.signalValue("#TEST_MISMATCH_V4_TF_VAL") === 0);
    sim.startForMs(5);
    sim.overrideSignalValue("#TEST_MISMATCH_V4_TF_PE", 1);

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


function test3ChannelDicreteMajority(sim)
{
    // Schema: TEST_3CHANNEL_INPUT
    // Inputs:
    //      #TEST_3CHV_T1_INA
    //      #TEST_3CHV_T1_INB
    //      #TEST_3CHV_T1_INC
    //
    let aInput = "#TEST_3CHV_T1_INA";
    let bInput = "#TEST_3CHV_T1_INB";
    let cInput = "#TEST_3CHV_T1_INC";

    let aResult = "#TEST_3CHV_T1_MAJ_A";
    let bResult = "#TEST_3CHV_T1_MAJ_B";
    let cResult = "#TEST_3CHV_T1_MAJ_C";

    sim.overrideSignalValue(aInput, 0);
    sim.overrideSignalValue(bInput, 0);
    sim.overrideSignalValue(cInput, 0);
    sim.startForMs(10);
    assert(sim.signalValue(aResult) === 0);
    assert(sim.signalValue(bResult) === 0);
    assert(sim.signalValue(cResult) === 0);

    sim.overrideSignalValue(aInput, 1);
    sim.overrideSignalValue(bInput, 0);
    sim.overrideSignalValue(cInput, 0);
    sim.startForMs(10);
    assert(sim.signalValue(aResult) === 0);
    assert(sim.signalValue(bResult) === 0);
    assert(sim.signalValue(cResult) === 0);

    sim.overrideSignalValue(aInput, 0);
    sim.overrideSignalValue(bInput, 1);
    sim.overrideSignalValue(cInput, 0);
    sim.startForMs(10);
    assert(sim.signalValue(aResult) === 0);
    assert(sim.signalValue(bResult) === 0);
    assert(sim.signalValue(cResult) === 0);

    sim.overrideSignalValue(aInput, 0);
    sim.overrideSignalValue(bInput, 0);
    sim.overrideSignalValue(cInput, 1);
    sim.startForMs(10);
    assert(sim.signalValue(aResult) === 0);
    assert(sim.signalValue(bResult) === 0);
    assert(sim.signalValue(cResult) === 0);

    sim.overrideSignalValue(aInput, 1);
    sim.overrideSignalValue(bInput, 1);
    sim.overrideSignalValue(cInput, 0);
    sim.startForMs(10);
    assert(sim.signalValue(aResult) === 1);
    assert(sim.signalValue(bResult) === 1);
    assert(sim.signalValue(cResult) === 1);

    sim.overrideSignalValue(aInput, 1);
    sim.overrideSignalValue(bInput, 0);
    sim.overrideSignalValue(cInput, 1);
    sim.startForMs(10);
    assert(sim.signalValue(aResult) === 1);
    assert(sim.signalValue(bResult) === 1);
    assert(sim.signalValue(cResult) === 1);

    sim.overrideSignalValue(aInput, 0);
    sim.overrideSignalValue(bInput, 1);
    sim.overrideSignalValue(cInput, 1);
    sim.startForMs(10);
    assert(sim.signalValue(aResult) === 1);
    assert(sim.signalValue(bResult) === 1);
    assert(sim.signalValue(cResult) === 1);

    return;
}


function testFlags(sim)
{
    // Schema: TEST_FLAGS
    //
    let state1 = sim.signalState("#TEST_FLAGS_R1");
    assert(state1.valid == 0);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_IN_VALIDITY", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_R1");
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);


    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_IN_VALIDITY", 1);
    sim.overrideSignalValue("#TEST_FLAGS_IN_SIMULATED", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_R1");
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 1);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_IN_VALIDITY", 1);
    sim.overrideSignalValue("#TEST_FLAGS_IN_BLOCKED", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_R1");
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 1);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_IN_VALIDITY", 1);
    sim.overrideSignalValue("#TEST_FLAGS_IN_MISMATCH", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_R1");
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 1);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_IN_VALIDITY", 1);
    sim.overrideSignalValue("#TEST_FLAGS_IN_HIGHLIMIT", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_R1");
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 1);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_IN_VALIDITY", 1);
    sim.overrideSignalValue("#TEST_FLAGS_IN_LOWLIMIT", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_R1");
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 1);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_T2_SIM", 0);
    sim.overrideSignalValue("#TEST_FLAGS_T2_BLOCK", 0);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_T2_RESULT");
    assert(state1.value === 1);
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_T2_SIM", 1);
    sim.overrideSignalValue("#TEST_FLAGS_T2_BLOCK", 0);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_T2_RESULT");
    assert(state1.value === 1);
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 1);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_T2_SIM", 0);
    sim.overrideSignalValue("#TEST_FLAGS_T2_BLOCK", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_T2_RESULT");
    assert(state1.value === 0);
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 1);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_FLAGS_T2_SIM", 1);
    sim.overrideSignalValue("#TEST_FLAGS_T2_BLOCK", 1);
    sim.startForMs(5);
    state1 = sim.signalState("#TEST_FLAGS_T2_RESULT");
    assert(state1.value === 0);
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 1);
    assert(state1.blocked == 1);
    assert(state1.mismatch == 0);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 0);

    state1 = sim.signalState("#TEST_FLAGS_T3_IN1");
    assert(state1.mismatch == 0);

    state1 = sim.signalState("#TEST_FLAGS_T4_IN1");
    assert(state1.mismatch == 1);

    state1 = sim.signalState("#TEST_FLAGS_R5");
    assert(state1.valid == 1);
    assert(state1.stateAvailable == 1);
    assert(state1.simulated == 0);
    assert(state1.blocked == 0);
    assert(state1.mismatch == 1);
    assert(state1.aboveHighLimit == 0);
    assert(state1.belowLowLimit == 1);

    // Test from schema TEST_MISMATCH_V4
    //
    sim.overridesReset();
    sim.overrideSignalValue("#TEST_MISMATCH_V4_TF_IN1", 100);
    sim.startForMs(5);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").mismatch == 1);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN2").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN3").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN4").mismatch == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_MISMATCH_V4_TF_IN2", 100);
    sim.startForMs(5);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN2").mismatch == 1);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN3").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN4").mismatch == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_MISMATCH_V4_TF_IN3", 100);
    sim.startForMs(5);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN2").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN3").mismatch == 1);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN4").mismatch == 0);

    sim.overridesReset();
    sim.overrideSignalValue("#TEST_MISMATCH_V4_TF_IN4", 100);
    sim.startForMs(5);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN2").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN3").mismatch == 0);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN4").mismatch == 1);

    // Test for analog inputs with validity input
    //
    sim.overridesReset();
    sim.overrideSignalValue("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03AVALID", 0);
    sim.overrideSignalValue("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03BVALID", 0);
    sim.startForMs(5);
    assert(sim.signalState("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03A").valid === false);
    assert(sim.signalState("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03B").valid === false);

    sim.overridesReset();
    sim.overrideSignalValue("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03AVALID", 1);
    sim.overrideSignalValue("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03BVALID", 0);
    sim.startForMs(5);
    assert(sim.signalState("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03A").valid === true);
    assert(sim.signalState("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03B").valid === false);

    sim.overridesReset();
    sim.overrideSignalValue("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03AVALID", 0);
    sim.overrideSignalValue("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03BVALID", 1);
    sim.startForMs(5);
    assert(sim.signalState("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03A").valid === false);
    assert(sim.signalState("#SYSTEMID_RACKID_FSCC01_MD03_CTRLIN_IN03B").valid === true);

    return;
}

function testLogicModulePowerOff(sim)
{
    sim.overridesReset();
    sim.reset();

    let lm = sim.logicModule("SYSTEMID_RACKID_FSCC01_MD00");

    sim.startForMs(5);
    assert(lm.powerOff === false);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").valid === true);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").stateAvailable === true);

    lm.powerOff = true;

    sim.startForMs(5);
    assert(lm.powerOff === true);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").valid === false);
    assert(sim.signalState("#TEST_MISMATCH_V4_TF_IN1").stateAvailable === false);

    // All other LMs are working
    //
    assert(sim.logicModule("SYSTEMID_RACKID_FSCC02_MD00").powerOff === false);
    assert(sim.logicModule("SYSTEMID_3CHRACK_CH01_MD00").powerOff === false);
    assert(sim.logicModule("SYSTEMID_3CHRACK_CH02_MD00").powerOff === false);
    assert(sim.logicModule("SYSTEMID_3CHRACK_CH03_MD00").powerOff === false);

    // Check connection state if lm is off
    //
    assert(sim.connection("CONN_CH1_CH2").timeout === false);
    assert(sim.connection("CONN_CH1_CH3").timeout === false);
    assert(sim.connection("CONN_CH2_CH3").timeout === false);

    sim.logicModule("SYSTEMID_3CHRACK_CH02_MD00").powerOff = true;

    // CONN_CH1_CH2
    // CONN_CH1_CH3
    // CONN_CH2_CH3
    //
    sim.startForMs(20);
    assert(sim.connection("CONN_CH1_CH2").timeout === true);
    assert(sim.connection("CONN_CH1_CH3").timeout === false);
    assert(sim.connection("CONN_CH2_CH3").timeout === true);

    sim.logicModule("SYSTEMID_3CHRACK_CH02_MD00").powerOff = false;

    sim.startForMs(10);
    assert(sim.connection("CONN_CH1_CH2").timeout === false);
    assert(sim.connection("CONN_CH1_CH3").timeout === false);
    assert(sim.connection("CONN_CH2_CH3").timeout === false);

    return;
}

function testConnectionEnable(sim)
{
    sim.reset();

    let validitySignal = "#PORT1_TO_PORT1_VALIDITY";
    let conn = sim.connection("PORT1_TO_PORT1");

    // --
    //
    sim.startForMs(10);
    assert(sim.signalValue(validitySignal) === 1);
    assert(conn.connectionID === "PORT1_TO_PORT1");
    assert(conn.enabled === true);
    assert(conn.timeout === false);

    // --
    //
    conn.enabled = false;
    sim.startForMs(20);

    assert(sim.signalValue(validitySignal) === 0);
    assert(conn.enabled === false);
    assert(conn.timeout === true);

    // --
    //
    conn.enabled = true;
    sim.startForMs(20);

    assert(sim.signalValue(validitySignal) === 1);
    assert(conn.enabled === true);
    assert(conn.timeout === false);

    return;
}

function testBusses(sim)
{
    // Test bus composer/extractor
    //
    assert(sim.signalValue("#TEST_BUSSES_BC_RES1") === 1);
    assert(sim.signalValue("#TEST_BUSSES_BC_RES2") === 1);
    assert(sim.signalValue("#TEST_BUSSES_BC_RES3") === 3);
    assert(sim.signalValue("#TEST_BUSSES_BC_RES4") === 10);
    assert(sim.signalValue("#TEST_BUSSES_BC_RES5") === 1);
    assert(sim.signalValue("#TEST_BUSSES_BC_RES6") === 20);

    // Test sending bus to opto connectin
    //
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES1") === 1);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES2") === 1);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES3") === 3);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES4") === 10);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES5") === 1);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES6") === 20);

    // Switch off connection and check that all data in 0's
    //
    let conn = sim.connection("PORT1_TO_PORT1");
    conn.enabled = false;
    sim.startForMs(20);

    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES1") === 0);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES2") === 0);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES3") === 0);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES4") === 0);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES5") === 0);
    assert(sim.signalValue("#TEST_BUSSES_BC_REC_RES6") === 0);

    conn.enabled = true;
    sim.startForMs(20);

    return;
}
