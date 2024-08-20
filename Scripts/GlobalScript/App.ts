"use strict";

module App {
	export interface ScriptMonitorApplication {
		
		// Properties
		//
		equipmentID: string;

		// Archive and shapshot functions
		//
		showArchive(signalsList: Array<string>, startTime: Date, endTime: Date, timeType: E.ArchiveTimeType): void;

		showSnapshot(signalsList: Array<string>): void;
		showSnapshotByMask(masks: Array<string>): void;
		showSnapshotByTag(tags: Array<string>): void;

		// Application control functions
		//
		setVisibleSchemaTree(visible: boolean): boolean;
		toggleSchemaTree(): void;
		setVisibleTabBar(visible: boolean): void;
		setVisibleToolBar(visible: boolean): void;
		setVisibleStatusBar(visible: boolean): void;
		setVisibleMenu(visible: boolean): void;
		setFullScreen(fullScreen: boolean): void;

		start(program: string, args: string, workDir?: string): boolean;
	}

	export interface ScriptTuningClientApplication {

		// Properties
		//
		equipmentID: string;
	}

	export interface ScriptTestSuiteApplication {

		// Properties
		//
		equipmentID: string;
	}
}