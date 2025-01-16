// Strict mode part
//

"use strict";

let FamilyLMID: number = 0x1100;
let FamilyVDUID: number = 0x1C00;

let UartID: number = 0;

let LMNumberCount: number = 0;
let VDUNumberCount: number = 0;

//let configScriptVersion = 1;		// first logged version
//let configScriptVersion = 2;		// TuningDataSize in LM port has been changed to 716 (1432 / 2)
//let configScriptVersion = 3;		// AIM and AOM signal are now found not by place but by identifier, findSignalByPlace is not used.
//let configScriptVersion = 4;		// AIM filteringTime calculation algorithm has been changed
//let configScriptVersion = 5;		// LM-1 properties SubsysID and Channel have been renamed to SubsystemID and SubsystemChannel
//let configScriptVersion = 6;		// SubsystemChannel renamed to LMNumber
//let configScriptVersion = 7;		// MAC address calculation changed
//let configScriptVersion = 8;		// IP address of LAN controller is written even service is not specified
//let configScriptVersion = 9;		// IP address and port of LAN controller can't be zero - an error is reported
//let configScriptVersion = 10;		// AIM signals parameters algorithm has been changed
//let configScriptVersion = 11;		// If software for LM ethernet controller is not found, default values are used
//let configScriptVersion = 12;		// DiagDataSize changed to TxDiagDataSize
//let configScriptVersion = 13;		// AppDataSize changed to AppLANDataSize
//let configScriptVersion = 14;		// connectionCaption=>connectionID; strID=>equipmentID
//let configScriptVersion = 15;		// added text description fields for every value
//let configScriptVersion = 16;		// added dynamic custom module family
//let configScriptVersion = 17;		// LMNumber limit is 12
//let configScriptVersion = 18;		// i/o modules scripts moved to ConfiurationScript in presets
//let configScriptVersion = 19;		// i/o modules family checking has been removed
//let configScriptVersion = 21;		// DataUID for LM modules is taken from linked port
//let configScriptVersion = 22;		// added builder parameter to interrupt build
//let configScriptVersion = 23;		// added OverrideApp(Diag)DataWordCount properties processing
//let configScriptVersion = 25;		// buildThread has been replaced by builder, added buildNo
//let configScriptVersion = 26;		// added UniqueID computing
//let configScriptVersion = 27;		// First script that supports subsystems filtering
//let configScriptVersion : number = 28;	// Code is written using TypeScript
//let configScriptVersion: number = 29;		// Added module place checking
//let configScriptVersion: number = 30;		// ModuleID for LM is placed in .mct file
//let configScriptVersion: number = 31;		// Add LmDescriptionVersion to Storage Format frame
//let configScriptVersion: number = 32;		// Removed structure ModuleFirmwareCollection
//let configScriptVersion: number = 33;		// Changes in  ModuleFirmware functions, uartID added
//let configScriptVersion: number = 34;		// Changes in LmNumberCount calculation
//let configScriptVersion: number = 35;		// Add Software type checking
//let configScriptVersion: number = 36;		// Changes in App/DiagDataService processing
//let configScriptVersion: number = 37;		// Add setDataFloat function
//let configScriptVersion: number = 38;		// If TuningEnable/AppDataEnable/DiagDataEnable flag is false, IP address is zero
//let configScriptVersion: number = 39;		// Description is added for LmNumberCount and UniqueID
//let configScriptVersion: number = 40;		// Let is used instead of var
//let configScriptVersion: number = 41;		// ScriptDeviceObject is used
//let configScriptVersion: number = 42;		// DiagDataSize is written for i/o module frame for LM8_SR10, LM1_SR03 and LM1_SR04
//let configScriptVersion: number = 43;		// Tuning LAN configuration is placed in LAN2 and LAN3 for LM1_SR04 LAN 
//let configScriptVersion: number = 44;		// Tuning LAN configuration is placed in LAN1 or LAN2/LAN3 depending on LAN description
//let configScriptVersion: number = 45; 	// Added mV_Type_L, mV_Type_M and mV_Raw_m1200_p1200 sensor types
//let configScriptVersion: number = 46;		// MAC address is checkind for uniqueness, LAN values are set to 0 if LAN is switched off
//let configScriptVersion: number = 47;		// LAN configuration is dynamically generated
//let configScriptVersion: number = 48;		// Using DiagLANDataSize calculated by RPCT
let configScriptVersion: number = 49;		// Added VDU configuration

//

function main(builder: ConfigStruct.Builder,
	root: ConfigStruct.ScriptDeviceObject,
	logicModules: ConfigStruct.ScriptDeviceModule[],
	confFirmware: ConfigStruct.ModuleFirmware,
	log: ConfigStruct.IssueLogger,
	signalSet: ConfigStruct.SignalSet,
	subsystemStorage: ConfigStruct.SubsystemStorage,
	opticModuleStorage: ConfigStruct.OptoModuleStorage,
	logicModuleDescription: ConfigStruct.LogicModule): boolean
{
	if (logicModules.length != 0)
	{
		let subSysID: string = logicModules[0].propertyString("SubsystemID");
		log.writeMessage("Subsystem " + subSysID + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
	}

	for (let i: number = 0; i < logicModules.length; i++)
	{
		if (logicModules[i].moduleFamily == FamilyLMID)
		{
			LMNumberCount++;
		}

		if (logicModules[i].moduleFamily == FamilyVDUID)
		{
			VDUNumberCount++;
		}
	}

	for (let i: number = 0; i < logicModules.length; i++)
	{

		if (logicModules[i].moduleFamily === FamilyLMID)
		{

			let result: boolean = generate_lm(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
			if (result == false)
			{
				return false;
			}
		}

		if (logicModules[i].moduleFamily === FamilyVDUID)
		{
			let result: boolean =  generate_vdu(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
			if (result == false)
			{
				return false;
			}
		}


		if (builder.jsIsInterruptRequested() == true)
		{
			return true;
		}
	}

	return true;
}

