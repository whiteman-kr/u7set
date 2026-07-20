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
var ScriptTags = ["to"];

// To give a custom caption to the the whole script function, declare a
// variable that has the same name as the script file name without path
// and extention and add 'caption' prefix.
// Custom script caption is displayed in TestSuite user interface and
// in reports.
// Example: 'ScriptSample.js' file should have 'captionScriptSample'
// caption variable name.
// Note: This variable is not used in Simulator tests.
//
var captionplugin_to5 = "TO-5 Reports";

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
        log.writeMessage("Дата та час: " + currentDate.toLocaleString(), "START_TIME");
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
        //const currentDate = new Date();
        //log.writeMessage("Finished at: " + currentDate.toLocaleString(), "END_TIME");
    }
}

// init() will be called before each test function is executed.
//
function init(ctrl)
{
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

    //ctrl.overridesReset();                   // Remove all signal overrides.
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
var captionTo5 = "Generate TO-5 report";

function cmpByOutput(comparators, outputAppSignalId)
{
	for (let c = 0; c < comparators.length; c++)
	{
		let cmp = comparators[c];
		
		if (cmp.output.appSignalId === outputAppSignalId)
		{
			return cmp;
		}
	}
	
	return undefined;	
}

function processStaticData(ctrl, setpointMismatch, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, criteria, correct, tolerance, id)
{
	let comparators = ctrl.setpointsByInput(inputAppSignalID);
		
	let cmp = cmpByOutput(comparators, outputAppSignalID);
	if (cmp === undefined)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} was not found!`);
		return;
	}
		
	if (cmp.compare.isConst === false)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} is dynamic!`);
		return;
	}
		
	let setpointValue = cmp.compare.constValue;

	if (Math.abs(setpointValue - criteria) > tolerance)
	{
		setpointMismatch++;
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} expected: ${criteria}`);
		correct = "Ні";
	}
	else
	{
		log.writeMessage(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} = OK `);
		correct = "Так";
	}
		
	let outputValue = ctrl.signalValue(cmp.output.appSignalId);	
	if (isNaN(outputValue) === true)
	{
		outputValue = "???";
		correct = " ";
		log.writeMessage("Output: NAN");	
	}
		
	let record = `${outputAppSignalCaption};${cmp.output.appSignalId};${caseId};${outputValue};${criteria};${setpointValue};${correct};`;
				
	log.writeMessage(record, `TO_TABLE_${id}`);	
	 
} 

function processDynamicData(ctrl, setpointMismatch, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, setpointAppSignalId, criteria, correct, tolerance, id)
{
	let comparators = ctrl.setpointsByInput(inputAppSignalID);
		
	let cmp = cmpByOutput(comparators, outputAppSignalID);
	if (cmp === undefined)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} was not found!`);
		return;
	}
		
	if (cmp.compare.isConst === true)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} is static!`);
		return;
	}
		
	if (cmp.compare.appSignalId !== setpointAppSignalId)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}, wrong  setpointAppSignalId!`);
		return;
	}

	let setpointValue = ctrl.signalValue(setpointAppSignalId);	
	if (isNaN(setpointValue) === true)
	{
		setpointValue = "???";
		log.writeMessage("Setpoint: NAN");	
	}
	else
	{
		if (Math.abs(setpointValue - criteria) > tolerance)
		{
			setpointMismatch++;
			log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} expected: ${criteria}`);
			correct = "Ні";
		}
		else
		{
			log.writeMessage(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} = OK `);
			correct = "Так";
		}
	}
		
	let outputValue = ctrl.signalValue(cmp.output.appSignalId);	
	if (isNaN(outputValue) === true)
	{
		outputValue = "???";
		correct = " ";
		log.writeMessage("Output: NAN");	
	}

		
	let record = `${outputAppSignalCaption};${cmp.output.appSignalId};${caseId};${outputValue};${criteria};${setpointValue};${correct};`;
				
				
	log.writeMessage(record, `TO_TABLE_${id}`);	
	 
} 

// TO5 Report by SchemaID
//
function testTO5BySchemaID(ctrl)
{
	// Read the contents of setpoints file
	//
    let csvFile = "%TEMP%\\PluginTO5.csv";
    
	let result = ctrl.loadTextFile(csvFile);

	if (result.ok === false) 
	{
		assert(false, "Failed to read file " + csvFile + "!");
		return;
	}

	let csvStrings = result.strings;
	log.writeMessage("Lines read: " + csvStrings.length);
	
	const setpointsData = csvStrings.map(row => row.split(";").slice(1));
	setpointsData.shift();
	
	
	const schemaIDList = [];
	for (const row of setpointsData) {
		if (!schemaIDList.includes(row[1 /*SchemaID*/])) {
			schemaIDList.push(row[1 /*SchemaID*/]);
		}
	}
	
	const tolerance = 0.0001;
	
	log.writeMessage(`${schemaIDList.length}`, "SECTION_REPEAT_COUNT(to5)");	// Repeat section for schemaID

	let setpointMismatch = 0;
	let errorReadData = false;
	let currentSchemaID = "";
	for (let i = 0; i < schemaIDList.length; i++)
	{
		for (const data of setpointsData)
		{
			if (schemaIDList[i] !== data[1 /*SchemaID*/])
			{
				continue;
			}
			
			const caseId = data[0 /*CaseID*/];
			const schemaID = data[1 /*SchemaID*/];
			const inputAppSignalID = data[2 /*InputAppSignalID*/];
			const outputAppSignalID = data[5 /*OutputAppSignalID*/];
			const outputAppSignalCaption = data[7 /*OutputAppSignalCaption*/];
			let criteria = data[9 /*Criteria*/];
			const round = data[14 /*Precision*/];
			criteria = math.floor(criteria * 10^round + 0.5) / 10^round;

			if (outputAppSignalID === "")
			{
				errorReadData = true;
				continue;
			}

			let correct = "";
			
			if (currentSchemaID !== schemaIDList[i])
			{
				log.writeMessage(`Алгоритм: ${schemaID}`, `TO_ALG_${i}`);
				currentSchemaID = schemaIDList[i];
			}
			
			if (data[10 /*Type*/] === "S")
			{
				processStaticData(ctrl, setpointMismatch, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, criteria, correct, tolerance, i);
			}
			else if (data[10 /*Type*/] === "D")
			{
				const setpointAppSignalId = data[11 /*SetpointAppSignalID*/]
				processDynamicData(ctrl, setpointMismatch, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, setpointAppSignalId, criteria, correct, tolerance, i);
			}
			else
			{
				// CSV ERROR DATA
			}
		}
	}

	if (errorReadData === true)
	{
		log.writeMessage("Помилка при читанню файлу уставок", "TO_RESULT");
	}
	else
	{
		if (setpointMismatch === 0)
		{
			log.writeMessage("Величини уставок відповідають карті уставок.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Кількість уставок не відповідних до карти уставок: ${setpointMismatch}`, "TO_RESULT");
		}
	
		log.writeMessage("Поточні значення параметрів відповідають стану ПТК.", "TO_RESULT");
	}
}

// TO5 Report by CaseID
//
function testTO5ByCaseID(ctrl)
{
	// Read the contents of setpoints file
	//
    let csvFile = "%TEMP%\\PluginTO5.csv";
    
	let result = ctrl.loadTextFile(csvFile);

	if (result.ok === false) 
	{
		assert(false, "Failed to read file " + csvFile + "!");
		return;
	}

	let csvStrings = result.strings;
	log.writeMessage("Lines read: " + csvStrings.length);
	
	const setpointsData = csvStrings.map(row => row.split(";").slice(1));
	setpointsData.shift();
	
	
	const caseIDList = [];
	for (const row of setpointsData) {
		if (!caseIDList.includes(row[0 /*CaseID*/])) {
			caseIDList.push(row[0 /*CaseID*/]);
		}
	}
	
	const tolerance = 0.0001;
	
	log.writeMessage(`${caseIDList.length}`, "SECTION_REPEAT_COUNT(to5)");	// Repeat section for schemaID

	let setpointMismatch = 0;
	let currentCaseID = "";
	let errorReadData = false;
	for (let i = 0; i < caseIDList.length; i++)
	{
		for (const data of setpointsData)
		{
			if (caseIDList[i] !== data[0 /*CaseID*/])
			{
				continue;
			}
			
			const caseId = data[0 /*CaseID*/];
			const schemaID = data[1 /*SchemaID*/];
			const inputAppSignalID = data[2 /*InputAppSignalID*/];
			const outputAppSignalID = data[5 /*OutputAppSignalID*/];
			const outputAppSignalCaption = data[7 /*OutputAppSignalCaption*/]
			let criteria = data[9 /*Criteria*/];
			const round = data[14 /*Precision*/];
			criteria = math.floor(criteria * 10^round + 0.5) / 10^round;

			let correct = "";

			if (outputAppSignalID === "")
			{
				errorReadData = true;
				continue;
			}
			
			if (currentCaseID !== caseIDList[i])
			{
				log.writeMessage(`Алгоритм: ${caseId}`, `TO_ALG_${i}`);
				currentCaseID = caseIDList[i];
			}
			
			if (data[10 /*Type*/] === "S")
			{
				processStaticData(ctrl, setpointMismatch, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, criteria, correct, tolerance, i);
			}
			else if (data[10 /*Type*/] === "D")
			{
				const setpointAppSignalId = data[11 /*SetpointAppSignalID*/]
				processDynamicData(ctrl, setpointMismatch, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, setpointAppSignalId, criteria, correct, tolerance, i);
			}
			else
			{
				// CSV ERROR DATA
			}
		}
	}
	
	if (errorReadData === true)
	{
		log.writeMessage("Помилка при читанню файлу уставок", "TO_RESULT");
	}
	else
	{
		if (setpointMismatch === 0)
		{
			log.writeMessage("Величини уставок відповідають карті уставок.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Кількість уставок не відповідних до карти уставок: ${setpointMismatch}`, "TO_RESULT");
		}
	
		log.writeMessage("Поточні значення параметрів відповідають стану ПТК.", "TO_RESULT");
	}
}
	
    /*
{
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
    // 3. To use tables in reports, generated text should be divided to
    // different columns using specified separator.
    // By default, a semicolon is used and can be changed in ReportTemplates
    // property. For example, if table has three columns, message text should
    // look like "A;B;C", where A, B and C - text to be displayed in every
    // column. See the example below.
    //
    // 4. Report can contain multiple records with same tag. By default, every
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
}
    */


