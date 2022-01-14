Global Script for Monitor and TuningClient applications can be written using TypeScript language.

You will need a copy of Node.js as an environment to run the package. 
Use a dependency manager like npm to download TypeScript into your project.

For more information see https://www.npmjs.com/package/typescript.

Also it is recommended to use Visual Studio Code for writing scripts.

RPCT TypeScript library contains following components:

App.ts - contains application control interfaces;
E.ts - contains enums used in scripting;
Log.ts - containg interface for accessing log file;
Signal.ts - contains interfaces for signal controllers, parameters and states;
VFrame30.ts - contains interfaces for schema items;
GlobalScript_Sample.ts - a sample script file;
tsconfig.json - TypeScript compiler configuration file.

To develop global script code, perform the following operations:

1. Place App.ts, E.ts, Log.ts, Signal.ts and VFrame30.ts, tsconfig.json into working directory;

2. Create GlobalScript.ts file in this directory and add following lines (or take them from sample):

	"use strict";

	// Global objects declarations
	declare var view: VFrame30.ScriptSchemaView;
	declare var signals: Signal.AppSignalController;
	declare var tuning: Signal.TuningController;
	declare var log: Log.LogController;

3. Write global functions in this file;

4. Compile the project using tsc command. This creates GlobalScript.js file in out directory. This file
should be placed in GlobalScript property of Monitor or TuningClient.