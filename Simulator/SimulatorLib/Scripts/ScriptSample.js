"use strict";

// If Project Property 'Run Simulator Tests on Build' is set to 'true',
// then following constant 'SkipOnBuild' can control whether tests will
// be executed  on the build from this particular file.
// Note, if SkipOnBuild is 'true' then manual tests run is still possible.
// 'true': Do not execute tests on build.
// 'false': Execute tests ob build (note project property
// 'Run Simulator Tests on Build').
//
var SkipOnBuild = false;

// Using script tags, project developer can create multiple instances of
// TestSuite preset that process different set scripts.
// ScriptTags variable is a string array that contains a set of tags.
// Script tags are also specified in ScriptTags property of TestSuite preset.
// Test scripts are processed by TestSuite if both test and preset script
// tags match, or if no tags are specified at all.
// Note: GlobalScript.js file is always processed by every TestSuite.
//
var ScriptTags = ["simtest"];

// To give a custom caption to the the whole script function, declare a
// variable that has the same name as the script file name without path
// and extention and add 'caption' prefix.
// Custom script caption is displayed in TestSuite user interface and
// in reports.
// Example: 'ScriptSample.js' file should have 'captionScriptSample'
// caption variable name.
// Note: This variable is not used in Simulator tests.
//
//var captionScriptSample = "Script Sample";

// Optional allow<FileName> function is used to give permission for running
// tests in current script.
// To use current script running permission:
// 1. Uncomment the next function example and replace SCRIPT_FILENAME_NO_EXT
// fragment with script filename,
// for example "allowTestScript01" will be used for TestScript01.js script.
// 2. Write the code that returns true if running script is allowed, 
// otherwise returns false.
// Note: This variable is not used in Simulator tests.
//
/*
function allowscript(ctrl)
{
    // Script execution is allowed when #SCRIPT1_PERMISSION_CH1,
    // #SCRIPT1_PERMISSION_CH2 and #SCRIPT1_PERMISSION_CH3
    // values are set to 1.
    //
    const enabledSignals = ["#SCRIPT1_PERMISSION_CH1", "#SCRIPT1_PERMISSION_CH2", "#SCRIPT1_PERMISSION_CH3"];

    for (let i = 0; i < enabledSignals.length; i++)
    {
        const state = ctrl.signalState(enabledSignals[i]);
        if (state.stateAvailable === false ||
            state.valid === false ||
            state.value === 0)
        {
            return false;
        }
    }
    return true;
}
*/

// initTestCase() will be called before the first test function is executed.
//
function initTestCase(ctrl)
{
    log.writeMessage(ctrl.projectName);

    if (isSimulator === true)
    {
        // Unlock simulation timer binding to PC's time. This parameter can
        // significantly increase simulation speed but it depends on
        // underlying hardware and project size.
        //
        ctrl.unlockTimer = true;

        // Allow or disable LogicModules' Application Data transmitting to AppDataSrv.
        //
        ctrl.appDataTrasmittion = false;

        // Run simulation for 5 ms, it warms up all "modules".
        //
        ctrl.startForMs(5);
    }

    if (isTestSuite === true)
    {
        // Add initialization for TestSuite here, will be called once
        // before all tests.

        // Write testing start time to the log, START_TIME is a report template tag
        //
        const currentDate = new Date();
        log.writeMessage("Started at: " + currentDate.toLocaleString(), "START_TIME");
    }
}

// cleanupTestCase() will be called after the last test function was executed.
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

// init() will be called before each test function is executed.
//
function init(ctrl)
{
    // Add additional code, for example, to acqure the alert before the test.

    /*
    const tolerance = 0;
    const ON = 1;
    const OFF = 0;
    const timeoutMs = 5000;

    if (ctrl.signalValue("#ALERTED_1") === ON)
    {
        // Set the Reset signal to "ON"
        //
        ctrl.overrideSignalValue("#ALERT_RESET_1", ON);

        // Wait while the Alert signal gets OFF value. If it does not for 5000 ms, throw an error.
        //
        if (ctrl.expectSignalValue("#ALERTED_1", timeoutMs, OFF, tolerance) === false)
        {
            assert("Failed to reset the alert!");
        }

        // Set the Reset signal to OFF
        //
        ctrl.overrideSignalValue("#ALERT_RESET_1", OFF);
    }
    */
}

// cleanup() will be called after every test function.
//
function cleanup(ctrl)
{
    if (isSimulator === true)
    {
        ctrl.reset(); // Reset module, requires 5 ms run for actual reset.
        ctrl.connectionsSetEnabled(true);    // Enable all connections.
    }

    if (isTestSuite === true)
    {
        // Add additional code, for example add some messages to the report.
    }

    log.writeMessage("Test function complete.");

    ctrl.overridesReset();                   // Remove all signal overrides.
}

// To give a custom name to the test function, declare a variable that has
// the same name as test function name but replace 'test' prefix to 'caption'
// prefix.
// Custom caption is displayed in TestSuite user interface and in reports.
// Example 1: 'testWaterPressure' function should have 'captionWaterPressure'
// caption variable name.
// Example 2: 'test1' function should have 'caption1' caption variable name.
// Note: This variable is not used in Simulator tests.
//
var caption1 = "Sample Test"; // Test 1 caption

// Generic test example
//
function test1(ctrl)
{
    /*
    // Start simulation for N msecs:
    //
    if (isSimulator === true)
    {
        ctrl.startForMs(50);
    }

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

    // Log messages are used by the TestSuite for generated reports.
    // To add log message to the report:
    //
    // 1. Edit ReportTemplates property of TestSuite preset in RPCT and
    // specify same tags for Text, Table objects etc (Tag attribute).
    //
    // 2. Specify tag parameter in the log writing function (for example,
    // "report_tab_tag1" or "report_text_tag1").
    //
    // 3. Tags can contain $(REPEATINDEX) macro (e.g. TO_ALG_$(REPEATINDEX)). This macro is used together with SECTION_REPEAT_COUNT(<sectionTag>) macro.
    // The SECTION_REPEAT_COUNT value sets the number of iterations to process the single section with different iterated tags.
    // Example: SECTION_REPEAT_COUNT(<sectionTag>) = 2, then the following tags will be iterated:
    // TO_ALG_$(REPEATINDEX) => TO_ALG_0, TO_ALG_1
    // This is used to split section into parts
    //
    // 4. To use tables in reports, generated text should be divided to
    // different columns using specified separator.
    // By default, a semicolon is used and can be changed in ReportTemplates
    // property. For example, if table has three columns, message text should
    // look like "A;B;C", where A, B and C - text to be displayed in every
    // column. See the example below.
    //
    // 5. Report can contain multiple records with same tag. By default, every
    // message is added to the report. To display message only once,
    // "$FIRST(tag)" and "$LAST(tag) tags descriptions are used in report
    // templates. These descriptions specify to display only first
    // or only last appearance of the tagged message. The example is testing
    // start and end time.

    // Write message to log for generating reports by the TestSuite:
    //
    if (isTestSuite === true)
    {
        // Table message
        log.writeMessage("Column 1 Value;Column 2 Value;Column 3 Value", "report_tab_tag1");

        log.writeWarning("Text", "report_text_tag1");   // Text message
        log.writeError("Text", "report_text_tag2");     // Text message
    }
    */
}

var caption2 = "Observer Test"; // Test 2 caption

// Observer using example:
//
function test2(ctrl)
{
    /*
    // Create observer.
    //
    const observer = ctrl.createObserver();

    // Add expectation which will be initial condition.
    //
    const initiatorId = observer.addEqualExpectation("#INPUT", 1);
    observer.setInitiator(initiatorId);

    // Add expectations for measure time.
    //
    observer.addEqualExpectation("#OUTPUT1", 1); // Wait the signal to become 1.
    observer.addEqualExpectation("#OUTPUT2", 0); // Wait the signal to become 0.

    // Observer connects to AppDataService for signal state retrieval,
    // start of measurements.
    //
    observer.start();

    // Set initial signal.
    //
    ctrl.overrideSignalValue("#INPUT", 1);

    // Wait for satisfying all three expectations.
    // returns true if all expectations were fulfilled.
    //
    const waitResult = observer.wait(5000);
    assert(waitResult);

    // Expected signal #OUTPUT1 to become 1 after 50 ms.
    //
    assert(observer.elapsedMs("#OUTPUT1") === 50);

    // Expected signal #OUTPUT2 to become 0 after 100 ms.
    //
    assert(observer.elapsedMs("#OUTPUT2") === 100);
    */
}
