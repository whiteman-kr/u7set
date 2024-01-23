'use strict'

// If Project Property 'Run Simulator Tests on Build' is set to 'true',
// then following constant 'SkipOnBuild' can control whether tests will be executed
// on the build from this particular file.
// Note, if SkipOnBuild is 'true' then manual tests run is still possible.
// 'true': Do not execute tests on build
// 'false': Execute tests ob build (note project property 'Run Simulator Tests on Build')
//
var SkipOnBuild = false;

// Using script tags, project developer can create multiple instances of TestSuite preset that process different set scripts.
// ScriptTags variable is a string array that contains a set of tags. Script tags are also specified
// in ScriptTags property of TestSuite preset. Test scripts are processed by TestSuite if both test and preset script tags match,
// or if no tags are specified at all.
// Note: GlobalScript.js file is always processed by every TestSuite.

var ScriptTags = ["simtest"];

// To give a custom caption to the the whole script function, declare a variable that has the same name as
// the script file name without path and extention and add 'caption' prefix.
// Custom script caption is displayed in TestSuite user interface and in reports.
// Example: 'ScriptSample.js' file should have 'captionScriptSample' caption variable name.
//
//var captionScriptSample = "Script Sample";

// Optional allow<FileName> function is used to give permission for running tests in current script.
// To use current script running permission:
// 1. Uncomment the next function example and replace SCRIPT_FILENAME_NO_EXT fragment with script filename,
// for example "allowTestScript01" will be used for TestScript01.js script.
// 2. Write the code that returns true if running script is allowed, otherwise returns false
/*
function allow<SCRIPT_FILENAME_NO_EXT>(ctrl)
{
    // Script execution is allowed when #SCRIPT1_PERMISSION_CH1, #SCRIPT1_PERMISSION_CH2 and #SCRIPT1_PERMISSION_CH3 values are set to 1.
    //
    const enabledSignals = ["#SCRIPT1_PERMISSION_CH1", "#SCRIPT1_PERMISSION_CH2", "#SCRIPT1_PERMISSION_CH3"];
    for (let i = 0; i < enabledSignals.length; i++) 
    {
        let state = ctrl.signalState(enabledSignals[i]);
        if (state.stateAvailable === false || state.valid === false || state.value === 0) 
        {
            return false;
        }
    }
    return true;
}
*/

// initTestCase() - will be called before the first test function is executed.
//
function initTestCase(ctrl)
{
    log.writeMessage(ctrl.projectName);

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


// To give a custom name to the test function, declare a variable that has the same name as test function name but replace 'test' prefix to 'caption' prefix.
// Custom caption is displayed in TestSuite user interface and in reports.
// Example 1: 'testWaterPressure' function should have 'captionWaterPressure' caption variable name.
// Example 2: 'test1' function should have 'caption1' caption variable name.
//
var caption1 = "Sample Test 1"; // Test 1 caption

// Test 1
//

function test1(ctrl)
{
    /*
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

    // Log messages are used in generated reports. To add log message to the report:
    // 1. Edit ReportTemplates property of TestSuite preset in RPCT and specify same tags for Text, Table objects etc (Tag attribute).
    // 2. Specify tag parameter in the log writing function (for example, "report_tab_tag1" or "report_text_tag1"). 
    // 3. To use tables in reports, generated text should be divided to different columns using specified separator.
    //    By default, a semicolon is used and can be changed in ReportTemplates property. For example, if table has three
    //    columns, message text should look like "A;B;C", where A, B and C - text to be displayed in every column See the example below.
    // 4. Report can contain multiple records with same tag. By default, every message is added to the report. To display message only once,
    //    "$FIRST(tag)" and "$LAST(tag) tags descriptions are used in report templates. These descriptions specify to display only first
    //    or only last appearance of the tagged message. The example is testing start and end time.

    // Write message to console/test log for generating reports:
    //
    log.writeMessage("Column 1 Value;Column 2 Value;Column 3 Value", "report_tab_tag1");    // Table message
    
    log.writeWarning("Text", "report_text_tag1");   // Text message
    log.writeError("Text", "report_text_tag2");     // Text message
    */

    return;
}

var caption2 = "Sample Test 2"; // Test 1 caption
function test2(ctrl)
{
    /*
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
    */

    return;
}
