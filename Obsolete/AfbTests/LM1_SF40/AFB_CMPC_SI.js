'use strict';

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



function test_AFB_CMPC_SI_EQ_0(sim)
{
    //const int SETPOINT = 0   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 100 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme
    sim.overrideSignalValue("#TUN_IN_SI1", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -78);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -51);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -50);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -49);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 50);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 51);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ001") === 1);
    return;
}
function test_AFB_CMPC_SI_EQ_1(sim)
{
    //const int SETPOINT = 24   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 100 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme
    sim.overrideSignalValue("#TUN_IN_SI2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", -100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", -99);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", -26);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 76);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", 77);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", 9999);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", -26);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ002") === 1);
    return;
}

function test_AFB_CMPC_SI_EQ_2(sim)
{
    //const int SETPOINT = -24   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 100 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme
    sim.overrideSignalValue("#TUN_IN_SI3", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", -78);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 0);

    sim.overrideSignalValue("#TUN_IN_SI3", -74);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", -49);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", 51);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 0);

    sim.overrideSignalValue("#TUN_IN_SI3", -74);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", -75);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 0);

    sim.overrideSignalValue("#TUN_IN_SI3", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_EQ003") === 1);
    return;
}

function test_AFB_CMPC_SI_GR_0(sim)
{
    //const int SETPOINT = 0   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 21 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme
    sim.overrideSignalValue("#TUN_IN_SI1", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 75);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 50);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -1);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -10);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -20);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -21);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -22);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", -100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 0);

    sim.overrideSignalValue("#TUN_IN_SI1", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR001") === 0);

    return;
}

function test_AFB_CMPC_SI_GR_1(sim) 
{
    //const int SETPOINT = 11   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 21 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme
    sim.overrideSignalValue("#TUN_IN_SI2", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 75);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 50);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 11);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", -1);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", -10);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", -11);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", -20);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", -21);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", -22);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", 11);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 0);

    sim.overrideSignalValue("#TUN_IN_SI2", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);
    sim.overrideSignalValue("#TUN_IN_SI2", 9999);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR002") === 1);

    return;
}

function test_AFB_CMPC_SI_GR_2(sim) 
{
    //const int SETPOINT = -11   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 21 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme
    sim.overrideSignalValue("#TUN_IN_SI2", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 75);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 50);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 11);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", -31);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1); 

    sim.overrideSignalValue("#TUN_IN_SI2", -32);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1); 

    sim.overrideSignalValue("#TUN_IN_SI2", -11);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1); 

    sim.overrideSignalValue("#TUN_IN_SI2", 11);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI2", 9999);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_GR003") === 1);

    return;
}

function test_AFB_CMPC_SI_LS_0(sim) 
{
    //const int SETPOINT = 0   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 122 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme

    sim.overrideSignalValue("#TUN_IN_SI1", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 75);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 50);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -1);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);
    
    sim.overrideSignalValue("#TUN_IN_SI1", -10);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -20);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);

    sim.overrideSignalValue("#TUN_IN_SI1", -100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS001") === 1);
    return;

}
function test_AFB_CMPC_SI_LS_1(sim) 
{
sim.overrideSignalValue("#TUN_IN_SI2", 100);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", 75);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", 50);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", 25);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", 1);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", 0);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", -1);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", -10);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", -20);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

sim.overrideSignalValue("#TUN_IN_SI2", -100);
sim.startForMs(5);
assert(sim.signalValue("#OUT_CMPC_SI_LS002") === 1);

return;
}

function test_AFB_CMPC_SI_LS_2(sim) 
{
    //const int SETPOINT = 0   // Added a constant with the value of SETPOINT, the same value is set on the scheme
    //const int DEAD_BAND = 21 // Added a constant with the value of DEAD_BAND, the same value is set on the scheme

    sim.overrideSignalValue("#TUN_IN_SI3", 100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", 75);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", 50);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", 25);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", 1);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", 0);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", -1);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);
    
    sim.overrideSignalValue("#TUN_IN_SI3", -10);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", -20);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);

    sim.overrideSignalValue("#TUN_IN_SI3", -100);
    sim.startForMs(5);
    assert(sim.signalValue("#OUT_CMPC_SI_LS003") === 1);
    return;
}



    // Start simulation for N msecs:
    //      sim.startForMs(50);

    // Check signal value:
    //      assert(sim.signalValue("#TEST_NOT_1") === 1);

    // Override signal value:
    //      sim.overrideSignalValue("#TEST_NOT_1", 0);

    // Clear override signal list:
    //      sim.overridesReset();

