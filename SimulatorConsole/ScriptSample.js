'use strict'

// If Project Property 'Run Simulator Tests on Build' is set to 'true',
// then following constant 'SkipOnBuild' can control whether tests will be executed
// on the build from this particular file.
// Note, if SkipOnBuild is 'true' then manual tests run is still possible.
// 'true': Do not execute tests on build
// 'false': Execute tests ob build (note project property 'Run Simulator Tests on Build')
//
var SkipOnBuild = false;

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(ctrl)
{
    log.writeMessage(ctrl.buildPath);

    if (isSimulator === true)
    {
        // Unlock simulation timer binding to PC's time. This param can significantly increase
        // simulation speed but it depends on underlying hardware and project size.
        //
        ctrl.unlockTimer = true;

        // Allow or disable LogicModules' Application Data transmittion to AppDataSrv.
        //
        ctrl.appDataTrasmittion = false;

        // Run simulation for 5 ms, it warms up all "modules".
        //
        ctrl.startForMs(5);
    }

    if (isTestSuite === true)
    {
        // Add initialization for TestSuite here, will be called once befor all tests.
        //

        // Write testing start time to the log, START_TIME is a report template tag
        //
        const currentDate = new Date();
        log.writeMessage("Started at: " + currentDate.toLocaleString(), "START_TIME");
    }

    return;
}

// cleanupTestCase() - will be called after the last test function was executed.
//
function cleanupTestCase(ctrl)
{
    if (isTestSuite === true)
    {
        // Write testing start time to the log, END_TIME is a report template tag
        //
        const currentDate = new Date();
        log.writeMessage("Finished at: " + currentDate.toLocaleString(), "END_TIME");
    }
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
    if (isSimulator === true)
    {
        ctrl.reset();                        // Reset module, requires 5 ms run for actual reset.
        ctrl.connectionsSetEnabled(true);    // Enable all connections.
    }

    if (isTestSuite === true)
    {
    }

    ctrl.overridesReset();                   // Remove all signal overrides.

    return;
}

// Test 1
//
function test1(ctrl)
{
    /*
    try
    {
        // Simulator test example:
        //

        // Start simulation for N msecs:
        //
        ctrl.startForMs(50);

        // Check signal value:
        //
        assert(ctrl.signalValue("#TEST_NOT_1") === 1);

        // Override signal value:
        //
        ctrl.overrideSignalValue("#TEST_NOT_1", 0);

        // Clear overriden signal list:
        //
        ctrl.overridesReset();

        // Write message to console/test log:
        //
        log.writeMessage("Text");
        log.writeWarning("Text");
        log.writeError("Text");

        // Write message to the log with ALG_RESULT tag. This message will be used to generate a report.
        // Report template is described in the TestSuite preset. The table has ALG_RESULT tag, two columns,
        // column separator is ';'.
        //
        if (isTestSuite === true)
        {
            log.writeMessage("test1;OK", "ALG_RESULT");
        }
    }

    catch (e)
    {
        if (isTestSuite === true)
        {
            log.writeError("test1;FAILED", "ALG_RESULT");
        }
        throw e;
    }
    */

    return;
}

function test2(ctrl)
{
    /*
    try
    {
        // Observer using example:
        //

        // Create observer.
        //
        let observer = ctrl.createObserver();

        // Add expectation which will be initial condition.
        //
        let initiatorId = observer.addEqualExpectation("#INPUT", 1);
        observer.setInitiator(initiatorId);

        // Add expectations for measure time.
        //
        observer.addEqualExpectation("#OUTPUT1", 1); // Wait this signal to become 1.
        observer.addEqualExpectation("#OUTPUT2", 0); // Wait this signal to become 0.

        // Observer connects to AppDataService for signal state retrieval, start of measurements.
        //
        observer.start();

        // Set initial signal.
        //
        ctrl.overrideSignalValue("#INPUT", 1);

        // Wait for satisfying all three expectations.
        // returns true if all expectations were fulfilled.
        //
        let waitResult = observer.wait(5000);
        assert(waitResult);

        // Expected signal #OUTPUT1 to become 1 after 50 ms.
        //
        assert(observer.elapsedMs("#OUTPUT1") === 50);

        // Expected signal #OUTPUT2 to become 0 after 100 ms.
        //
        assert(observer.elapsedMs("#OUTPUT2") === 100);

        // Write message to the log with ALG_RESULT tag. This message will be used to generate a report.
        // Report template is described in the TestSuite preset. The table has ALG_RESULT tag, two columns,
        // column separator is ';'.
        //
        if (isTestSuite === true)
        {
            log.writeMessage("test2;OK", "ALG_RESULT");
        }
    }
    catch (e)
    {
        if (isTestSuite === true)
        {
            log.writeError("test2;FAILED", "ALG_RESULT");
        }
        throw e;
    }
    */

    return;
}
