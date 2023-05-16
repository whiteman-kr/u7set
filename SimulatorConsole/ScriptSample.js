'use strict'

// Use these constants as param for ReadRam*/WriteRam* functions of Simulator
//
const RamReadAccess = 1;        
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
function initTestCase(ctrl)
{
    if (isSimulator == true)
    {
        // Performing Simulator-specific initialization 
        //
        log.writeMessage("Test mode: Simulator");

        console.log(ctrl.buildPath);

        ctrl.unlockTimer = true;             // Unlock simulation timer binding to PC's time. This param can significantly increase simulation speed but it depends on underlying hardware and project size.
        ctrl.appDataTrasmittion = false;     // Allow or disable LogicModules' Application Data transmittion to AppDataSrv
    
        ctrl.startForMs(5);                  // Run simulation for 5 ms, it warms up all modules
    }
    else
    {
        if (isTestSuite == true)
        {
            // Performing TestSuite-specific initialization 
            //
            log.writeMessage("Test mode: TestSuite");
        }
    }


    return;
}

// cleanupTestCase() - will be called after the last test function was executed.
//
function cleanupTestCase(ctrl)
{
}

// init() - will be called before each test function is executed.
//
function init(ctrl)
{
}

// cleanup() - will be called after every test function.
//
function cleanup(ctrl)
{
    if (isSimulator == true)
    {
        // Performing Simulator-specific cleanup 
        //
        ctrl.reset();                        // Reset module, requires 5 ms run for actual reset
        ctrl.overridesReset();               // Remove all signal overrides
        ctrl.connectionsSetEnabled(true);    // Enable all connections
        ctrl.startForMs(5);                  // For applying overridesReset(), and for actual reset all modules
    }

    return;
}

// Test 1
//
function test1(ctrl)
{
    // Start simulation for N msecs (for Simulator) or wait N msecs (for TestSuite):
    //      ctrl.startForMs(50);
    //      or
    //      ctrl.waitForMs(5000);

    // Check signal value:
    //      assert(ctrl.signalValue("#TEST_NOT_1") === 1);

    // Override signal value:
    //      ctrl.overrideSignalValue("#TEST_NOT_1", 0);


    //if (isTestSuite == true)
    //{
        // Wait for all signals override complete
        //        assert(ctrl.waitForSignalOverrides(5000));

        // Wait signal to get specified value with specified timeout (5000 milliseconds)
        //        assert(ctrl.expectSignalValue("#OUTPUT1", 1, 5000) === true);
    //}

    // Get AppSignalState structure for the signal:
    //      let state = ctrl.signalState("#OUTPUT1");
    
    // Get signal value as double value:
    //      let doubleValue = ctrl.signalValue("#OUTPUT1");

    // Check if signal exists:
    //      let exists = ctrl.signalExists("#OUTPUT1");
    
    // Get AppSignalParam structure for the signal:
    //      let param = signalParam("OUTPUT1");

    // Clear override signal list:
    //      ctrl.overridesReset();

    // Write message to console:
    
    //      log.writeMessage("Log Message");
    //      log.writeMessage("Log Message With Tag", 1);
    
    //      log.writeWarning("Log Warning");
    //      log.writeWarning("Log Warning With Tag", 2);
    
    //      log.writeError("Log Error");
    //      log.writeError("Log Error With Tag", 3);
    
    //      log.writeText("Log Text"); 
    //      log.writeText("Log Text With Tag", 4); 

    return;
}
