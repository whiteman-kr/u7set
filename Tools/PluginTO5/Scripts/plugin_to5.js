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

function processStaticData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, criteria, precision, correct, tolerance, id)
{
	let setpointMismatch = 0;
	let outputMismatch = 0;

	let comparators = ctrl.setpointsByInput(inputAppSignalID);
		
	let cmp = cmpByOutput(comparators, outputAppSignalID);
	if (cmp === undefined)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} was not found!`);
		return [0,0];
	}
		
	if (cmp.compare.isConst === false)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} is dynamic!`);
		return [0,0];
	}
	
	// Check the static setpoint value matching
	//
	let setpointValue = Number(cmp.compare.constValue);
	if (isNaN(setpointValue) === true)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} has wrong format!`);
		return [0,0];				
	}

	setpointValue = Number(setpointValue.toPrecision(precision));	
	if (Math.abs(setpointValue - criteria) > tolerance)
	{
		log.writeWarning(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} expected: ${criteria}`);
		correct = "Ні";
		setpointMismatch++;
	}
	else
	{
		log.writeMessage(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} = OK `);
		correct = "Так";
	}
				
	// Print the output value
	//
	let outputValue = ctrl.signalValue(cmp.output.appSignalId);	
	if (isNaN(outputValue) === true)
	{
		outputValue = "???";
		outputMismatch++;
	}
		
	let record = `${outputAppSignalCaption};${cmp.output.appSignalId};${caseId};${outputValue};${criteria};${setpointValue};${correct};`;
				
	log.writeMessage(record, `TO_TABLE_${id}`);	
	
	return [setpointMismatch, outputMismatch];
} 

function processDynamicData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, setpointAppSignalId, criteria, precision, correct, tolerance, id)
{
	let setpointMismatch = 0;
	let outputMismatch = 0;

	let comparators = ctrl.setpointsByInput(inputAppSignalID);
		
	let cmp = cmpByOutput(comparators, outputAppSignalID);
	if (cmp === undefined)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} was not found!`);
		return [0,0];
	}
		
	if (cmp.compare.isConst === true)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID} is static!`);
		return [0,0];
	}
		
	if (cmp.compare.appSignalId !== setpointAppSignalId)
	{
		log.writeError(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}, wrong  setpointAppSignalId!`);
		return [0,0];
	}

	// Check the dynamic setpoint value matching
	//
	let setpointValue = ctrl.signalValue(setpointAppSignalId);
	if (isNaN(setpointValue) === true)
	{
		setpointValue = "???";
		correct = "???";
		setpointMismatch++;
	}
	else
	{
		setpointValue = Number(setpointValue.toPrecision(precision));	
		if (Math.abs(setpointValue - criteria) > tolerance)
		{
			log.writeWarning(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} expected: ${criteria}`);
			correct = "Ні";
			setpointMismatch++;
		}
		else
		{
			log.writeMessage(`Setpoint ${inputAppSignalID} -> ${outputAppSignalID}: ${setpointValue} = OK `);
			correct = "Так";
		}
	}
		
	// Print the output value
	//
	let outputValue = ctrl.signalValue(cmp.output.appSignalId);	
	if (isNaN(outputValue) === true)
	{
		outputValue = "???";
		outputMismatch++;
	}

		
	let record = `${outputAppSignalCaption};${cmp.output.appSignalId};${caseId};${outputValue};${criteria};${setpointValue};${correct};`;
				
	log.writeMessage(record, `TO_TABLE_${id}`);	
	
	return [setpointMismatch, outputMismatch];
	 
} 

function parseCsvLine(line, delimiter = ";") 
{
    const result = [];
    let field = "";
    let inQuotes = false;

    for (let i = 0; i < line.length; i++) {
        const c = line[i];

        if (c === '"') {
            // Escaped quote ("")
            if (inQuotes && line[i + 1] === '"') {
                field += '"';
                i++; // Skip the second quote
            } else {
                // Enter or leave quoted field
                inQuotes = !inQuotes;
            }
        } else if (c === delimiter && !inQuotes) {
            // End of current field
            result.push(field);
            field = "";
        } else {
            field += c;
        }
    }

    // Add the last field
    result.push(field);

    return result;
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
	
	let setpointsData = csvStrings.map(row => parseCsvLine(row).slice(1));
	setpointsData.shift();
	
	const schemaIDList = [];
	for (const row of setpointsData) {
		if (!schemaIDList.includes(row[2 /*SchemaID*/])) {
			schemaIDList.push(row[2 /*SchemaID*/]);
		}
	}
	
	const tolerance = 0.000001;
	
	log.writeMessage(`${schemaIDList.length}`, "SECTION_REPEAT_COUNT(to5)");	// Repeat section for schemaID

	let setpointMismatch = 0;
	let outputMismatch = 0;
	let errorReadData = false;
	let currentSchemaID = "";
	let index = 0;
	for (let i = 0; i < schemaIDList.length; i++)
	{
		for (const data of setpointsData)
		{
			if (schemaIDList[i] !== data[2 /*SchemaID*/])
			{
				continue;
			}
			
			const caseId = data[1 /*CaseID*/];
			const schemaID = data[2 /*SchemaID*/];
			const inputAppSignalID = data[3 /*InputAppSignalID*/];
			const outputAppSignalID = data[6 /*OutputAppSignalID*/];
			const outputAppSignalCaption = data[8 /*OutputAppSignalCaption*/];
			let criteria = Number(data[10 /*Criteria*/]);
			const precision = data[15 /*Precision*/];
			criteria = Number(criteria.toPrecision(precision));

			if (outputAppSignalID === "")
			{
				log.writeError(`CSV outputAppSignalID is empty, index : ${index}`);
				errorReadData = true;
				continue;
			}

			let correct = "";
			
			if (currentSchemaID !== schemaIDList[i])
			{
				log.writeMessage(`Схема: ${schemaID}`, `TO_ALG_${i}`);
				currentSchemaID = schemaIDList[i];
			}
			
			if (data[11 /*Type*/] === "S")
			{
				const [setpointMismatch0, outputMismatch0] = processStaticData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, criteria, precision, correct, tolerance, i);
				setpointMismatch += setpointMismatch0;
				outputMismatch += outputMismatch0;
			}
			else if (data[11 /*Type*/] === "D")
			{
				const setpointAppSignalId = data[12 /*SetpointAppSignalID*/]
				const [setpointMismatch0, outputMismatch0] = processDynamicData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, setpointAppSignalId, criteria, precision, correct, tolerance, i);
				setpointMismatch += setpointMismatch0;
				outputMismatch += outputMismatch0;
			}
			else
			{
				// CSV ERROR DATA
				log.writeError(`CSV setpoint type unknown, index : ${index}`);
				errorReadData = true;
			}
		}
	}

	if (errorReadData === true)
	{
		log.writeMessage("Помилка при читанні файлу уставок.", "TO_RESULT");
	}
	else
	{
		if (setpointMismatch === 0)
		{
			log.writeMessage("Усі величини уставок відповідають карті уставок.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Уставок, що не відповідають карті уставок: ${setpointMismatch}.`, "TO_RESULT");
		}
		
		if (outputMismatch === 0)
		{
			log.writeMessage("Поточні значення спрацювання уставок отримано.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Помилок отримання значень спрацювання уставок: ${outputMismatch}.`, "TO_RESULT");
		}
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
	
	let setpointsData = csvStrings.map(row => parseCsvLine(row).slice(1));
	setpointsData.shift();
	
	const caseIDList = [];
	for (const row of setpointsData) {
		if (!caseIDList.includes(row[1 /*CaseID*/])) {
			caseIDList.push(row[1 /*CaseID*/]);
		}
	}
	
	const tolerance = 0.000001;
	
	log.writeMessage(`${caseIDList.length}`, "SECTION_REPEAT_COUNT(to5)");	// Repeat section for schemaID

	let setpointMismatch = 0;
	let outputMismatch = 0;
	let currentCaseID = "";
	let errorReadData = false;
	let index = 0;
	for (let i = 0; i < caseIDList.length; i++)
	{
		for (const data of setpointsData)
		{
			if (caseIDList[i] !== data[1 /*CaseID*/])
			{
				continue;
			}
			
			index++;
			
			const caseId = data[1 /*CaseID*/];
			const schemaID = data[2 /*SchemaID*/];
			const inputAppSignalID = data[3 /*InputAppSignalID*/];
			const outputAppSignalID = data[6 /*OutputAppSignalID*/];
			const outputAppSignalCaption = data[8 /*OutputAppSignalCaption*/]
			let criteria = Number(data[10 /*Criteria*/]);
			const precision = data[15 /*Precision*/];
			criteria = Number(criteria.toPrecision(precision));

			let correct = "";

			if (outputAppSignalID === "")
			{
				log.writeError(`CSV outputAppSignalID is empty, index : ${index}`);
				errorReadData = true;
				continue;
			}
			
			if (currentCaseID !== caseIDList[i])
			{
				log.writeMessage(`Шафа: ${caseId}`, `TO_ALG_${i}`);
				currentCaseID = caseIDList[i];
			}
			
			if (data[11 /*Type*/] === "S")
			{
				const [setpointMismatch0, outputMismatch0] = processStaticData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, criteria, precision, correct, tolerance, i);
				setpointMismatch += setpointMismatch0;
				outputMismatch += outputMismatch0;
			}
			else if (data[11 /*Type*/] === "D")
			{
				const setpointAppSignalId = data[12 /*SetpointAppSignalID*/]
				const [setpointMismatch0, outputMismatch0] = processDynamicData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, setpointAppSignalId, criteria, precision, correct, tolerance, i);
				setpointMismatch += setpointMismatch0;
				outputMismatch += outputMismatch0;
			}
			else
			{
				// CSV ERROR DATA
				log.writeError(`CSV setpoint type unknown, index : ${index}`);
				errorReadData = true;
			}
		}
	}

	log.writeMessage(`CSV total indexes : ${index}`);
	
	if (errorReadData === true)
	{
		log.writeMessage("Помилка при читанні файлу уставок.", "TO_RESULT");
	}
	else
	{
		if (setpointMismatch === 0)
		{
			log.writeMessage("Усі величини уставок відповідають карті уставок.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Уставок, що не відповідають карті уставок: ${setpointMismatch}.`, "TO_RESULT");
		}
		
		if (outputMismatch === 0)
		{
			log.writeMessage("Поточні значення спрацювання уставок отримано.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Помилок отримання значень спрацювання уставок: ${outputMismatch}.`, "TO_RESULT");
		}
	}
}


// TO5 Report by Algorithm
//
function testTO5ByAlgorithm(ctrl)
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
	
	let setpointsData = csvStrings.map(row => parseCsvLine(row).slice(1));
	setpointsData.shift();
	
	const algorithmList = [];
	for (const row of setpointsData) {
		if (!algorithmList.includes(row[0 /*Algorithm*/])) {
			algorithmList.push(row[0 /*Algorithm*/]);
		}
	}
	
	const tolerance = 0.000001;
	
	log.writeMessage(`${algorithmList.length}`, "SECTION_REPEAT_COUNT(to5)");	// Repeat section for schemaID

	let setpointMismatch = 0;
	let outputMismatch = 0;
	let currentAlgorithm = "";
	let errorReadData = false;
	let index = 0;
	for (let i = 0; i < algorithmList.length; i++)
	{
		for (const data of setpointsData)
		{
			if (algorithmList[i] !== data[0 /*Algorithm*/])
			{
				continue;
			}

			index++;

			const algorithm = data[0 /*Algorithm*/];
			const caseId = data[1 /*CaseID*/];
			const schemaID = data[2 /*SchemaID*/];
			const inputAppSignalID = data[3 /*InputAppSignalID*/];
			const outputAppSignalID = data[6 /*OutputAppSignalID*/];
			const outputAppSignalCaption = data[8 /*OutputAppSignalCaption*/]
			let criteria = Number(data[10 /*Criteria*/]);
			const precision = data[15 /*Precision*/];
			criteria = Number(criteria.toPrecision(precision));

			let correct = "";

			if (outputAppSignalID === "")
			{
				log.writeError(`CSV outputAppSignalID is empty, index : ${index}`);
				errorReadData = true;
				continue;
			}
			
			if (currentAlgorithm !== algorithmList[i])
			{
				log.writeMessage(`Алгоритм: ${algorithm}`, `TO_ALG_${i}`);
				currentAlgorithm = algorithmList[i];
			}
			
			if (data[11 /*Type*/] === "S")
			{
				const [setpointMismatch0, outputMismatch0] = processStaticData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, criteria, precision, correct, tolerance, i);
				setpointMismatch += setpointMismatch0;
				outputMismatch += outputMismatch0;
			}
			else if (data[11 /*Type*/] === "D")
			{
				const setpointAppSignalId = data[12 /*SetpointAppSignalID*/]
				const [setpointMismatch0, outputMismatch0] = processDynamicData(ctrl, caseId, inputAppSignalID, outputAppSignalID, outputAppSignalCaption, setpointAppSignalId, criteria, precision, correct, tolerance, i);
				setpointMismatch += setpointMismatch0;
				outputMismatch += outputMismatch0;
			}
			else
			{
				// CSV ERROR DATA
				log.writeError(`CSV setpoint type unknown, index : ${index}`);
				errorReadData = true;
			}
		}
	}
	
	log.writeMessage(`CSV total indexes : ${index}`);
	
	if (errorReadData === true)
	{
		log.writeMessage("Помилка при читанні файлу уставок.", "TO_RESULT");
	}
	else
	{
		if (setpointMismatch === 0)
		{
			log.writeMessage("Усі величини уставок відповідають карті уставок.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Уставок, що не відповідають карті уставок: ${setpointMismatch}.`, "TO_RESULT");
		}
		
		if (outputMismatch === 0)
		{
			log.writeMessage("Поточні значення спрацювання уставок отримано.", "TO_RESULT");
		}
		else
		{
			log.writeMessage(`Помилок отримання значень спрацювання уставок: ${outputMismatch}.`, "TO_RESULT");
		}
	}
}

