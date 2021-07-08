// Non-strict mode part
//

enum DeviceObjectType {
	Root = 0,
	System,
	Rack,
	Chassis,
	Module,
	Controller,
	Workstation,
	Software,
	AppSignal
}

enum SoftwareType {
	Monitor = 9000,
	ConfigurationService = 9001,
	AppDataService = 9002,
	ArchiveService = 9003,
	TuningService = 9004,
	DiagDataService = 9005,
	TuningClient = 9006,
	Metrology = 9007,
	ServiceControlManager = 9008,
}


enum ElectricUnit {
	NoUnit = 0,
	mA = 1,
	mV = 2,
	Ohm = 3,
	V = 4,
}

enum SensorType {
	NoSensor = 0,

	Ohm_Pt50_W1391 = 1,
	Ohm_Pt100_W1391 = 2,
	Ohm_Pt50_W1385 = 3,
	Ohm_Pt100_W1385 = 4,

	Ohm_Cu_50_W1428 = 5,
	Ohm_Cu_100_W1428 = 6,
	Ohm_Cu_50_W1426 = 7,
	Ohm_Cu_100_W1426 = 8,

	Ohm_Pt21 = 9,
	Ohm_Cu23 = 10,

	mV_K_TXA = 11,
	mV_L_TXK = 12,
	mV_N_THH = 13,

	//

	mV_Type_B = 14,
	mV_Type_E = 15,
	mV_Type_J = 16,
	mV_Type_K = 17,
	mV_Type_N = 18,
	mV_Type_R = 19,
	mV_Type_S = 20,
	mV_Type_T = 21,

	mV_Raw_Mul_8 = 22,
	mV_Raw_Mul_32 = 23,

	Ohm_Ni50_W1617 = 24,
	Ohm_Ni100_W1617 = 25,

	V_0_5 = 26,
	V_m10_p10 = 27,

	Ohm_Pt_a_391 = 28,
	Ohm_Pt_a_385 = 29,
	Ohm_Cu_a_428 = 30,
	Ohm_Cu_a_426 = 31,
	Ohm_Ni_a_617 = 32,

	Ohm_Raw = 33,

}

enum OutputMode {
	Plus0_Plus5_V = 0,
	Plus4_Plus20_mA = 1,
	Minus10_Plus10_V = 2,
	Plus0_Plus5_mA = 3,
	Plus0_Plus20_mA = 4,
	Plus0_Plus24_mA = 5,
};

enum UnitsConvertorErrorCode{
	ErrorGeneric = 1,
	LowLimitOutOfRange = 2,
	HighLimitOutOfRange = 3,
}

interface Builder {
	jsIsInterruptRequested(): boolean;
}

interface ScriptDeviceObject {
	
	equipmentId: string;
	caption: string;
	uuid: string;
	deviceType: DeviceObjectType;
	place: number;
	childrenCount: number;

	parent(): ScriptDeviceObject;
	child(index: number): ScriptDeviceObject;
	childByEquipmentId(equipmentId: string): ScriptDeviceObject;	

	toRack(): ScriptDeviceRack;
	toChassis(): ScriptDeviceChassis;
	toModule(): ScriptDeviceModule;
	toController(): ScriptDeviceController;
	toAppSignal(): ScriptDeviceAppSignal;
	toWorkstation(): ScriptDeviceWorkstation;
	toSoftware(): ScriptDeviceSoftware;

	isRack(): boolean;
	isChassis(): boolean;
	isModule(): boolean;
	isController(): boolean;
	isAppSignal(): boolean;
	isWorkstation(): boolean;
	isSoftware(): boolean;	

	propertyValue(name: string): any;
	propertyInt(name: string): number;
	propertyBool(name: string): boolean;
	propertyString(name: string): string;
	propertyIP(name: string): number;	
}

interface ScriptDeviceRack extends ScriptDeviceObject {
}

interface ScriptDeviceChassis extends ScriptDeviceObject {
}

interface ScriptDeviceModule extends ScriptDeviceObject {
	moduleFamily: number;
	moduleVersion: number;
}

interface ScriptDeviceController extends ScriptDeviceObject {
}

interface ScriptDeviceAppSignal extends ScriptDeviceObject {
}

interface ScriptDeviceWorkstation extends ScriptDeviceObject {
}

interface ScriptDeviceSoftware extends ScriptDeviceObject {
	softwareType: SoftwareType;
}

interface JsVariantList {
	jsSize(): number;
	jsAt(index: number): number;
}

interface ModuleFirmware {

	setData8(frameIndex: number, offset: number, data: number): boolean;
	setData16(frameIndex: number, offset: number, data: number): boolean;
	setData32(frameIndex: number, offset: number, data: number): boolean;
	setDataFloat(frameIndex: number, offset: number, data: number): boolean;

	data8(frameIndex: number, offset: number): number;
	data16(frameIndex: number, offset: number): number;
	data32(frameIndex: number, offset: number): number;
	dataFloat(frameIndex: number, offset: number): number;

	storeCrc64(frameIndex: number, start: number, count: number, offset: number): string;
	storeHash64(frameIndex: number, offset: number, dataString: string): string;

	calcCrc32(frameIndex: number, start: number, count: number): number;
	calcHash64(dataString: string): JsVariantList;

	jsSetDescriptionFields(descriptionVersion: number, description: string): void;
	jsAddDescription(channel: number, description: string): void;
	jsSetUniqueID(LMNumber: number, uniqueID: number): void;

	writeLog(message: string): void;
	buildNumber(): number;

}

interface IssueLogger {
	writeMessage(message: string): void;
	writeWarning(message: string): void;
	writeError(message: string): void;

	errCFG3000(propertyName: string, equipmentID: string): void;
	errCFG3001(subSysID: string, module: string): void;
	errCFG3002(name: string, value: number, min: number, max: number, module: string): void;
	errCFG3003(LMNumber: number, module: string): void;
	errCFG3004(controllerID: string, module: string): void;
	wrnCFG3008(softwareID: string, module: string): void;
	errCFG3011(addressProperty: string, address: number, controller: string): void;
	errCFG3012(portProperty: string, port: number, controller: string): void;
	errCFG3017(objectID: string, propertyName: string, softwareID: string): void;
	wrnCFG3018(propertyName: string, ip: string, port: number, controller: string): void;
	errCFG3042(moduleId: string, moduleUuid: string): void;		// Title: Module %1 should be installed in chassis.

}

interface SignalSet {
}

interface SubsystemStorage {
	ssKey(subSysID: string): number;
}

interface OptoPort {
	connectionID(): string;
	equipmentID(): string;
	linkedPortID(): string;

	isLinked(): boolean;

	portID(): number;

	txStartAddress(): number;
	txDataID(): number;

	txDataSizeW(): number;
	rxDataSizeW(): number;
}

interface OptoModuleStorage {
	jsGetOptoPort(controllerEquipmentID: string): OptoPort;
}

interface LogicModule {
	descriptionNumber(): number;
	jsConfigurationStringFile(): string;

	FlashMemory_ConfigFramePayload: number;
	FlashMemory_ConfigFrameCount: number;
	FlashMemory_ConfigUartId: number;
	Memory_TxDiagDataSize: number;
	OptoInterface_OptoPortCount: number;
}

interface UnitsConvertor {
	physicalToElectric(value: number, electricLowLimit: number, electricHighLimit: number, unitID: number, sensorType: number): number;
	electricToPhysical(value: number, electricLowLimit: number, electricHighLimit: number, unitID: number, sensorType: number): number;
}

function runConfigScript(configScript: string,
	confFirmware: ModuleFirmware,
	ioModule: ScriptDeviceObject,
	LMNumber: number,
	frame: number,
	log: IssueLogger,
	signalSet: SignalSet,
	opticModuleStorage: OptoModuleStorage): boolean {
	//let funcStr = "(function (confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage){log.writeMessage(\"Hello\"); return true; })";
	//
	let funcStr = "(" + configScript + ")";
	let funcVar = eval(funcStr);
	if (funcVar(confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false) {
		return false;
	}

	return true;
}

// Strict mode part
//

"use strict";

let FamilyLMID: number = 0x1100;

let UartID: number = 0;

let LMNumberCount: number = 0;

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
let configScriptVersion: number = 42;		// DiagDataSize is written for i/o module frame

//

function main(builder: Builder, root: ScriptDeviceObject, logicModules: ScriptDeviceModule[], confFirmware: ModuleFirmware,
	log: IssueLogger, signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule): boolean {

	if (logicModules.length != 0) {
		let subSysID: string = logicModules[0].propertyString("SubsystemID");
		log.writeMessage("Subsystem " + subSysID + ", configuration script: " + logicModuleDescription.jsConfigurationStringFile() + ", version: " + configScriptVersion + ", logic modules count: " + logicModules.length);
	}

	for (let i: number = 0; i < logicModules.length; i++) {

		if (logicModules[i].moduleFamily == FamilyLMID) {
			LMNumberCount++;
		}
	}

	for (let i: number = 0; i < logicModules.length; i++) {

		if (logicModules[i].moduleFamily != FamilyLMID) {
			continue;
		}

		let result: boolean = module_lm_1(builder, root, logicModules[i], confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
		if (result == false) {
			return false;
		}

		if (builder.jsIsInterruptRequested() == true) {
			return true;
		}
	}

	return true;
}

function setData8(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	if (channel != -1 && equpmentID.length > 0) {
		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "8;" + caption + ";0x" + data.toString(16));
	}

	if (confFirmware.setData8(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData8");
		return false;
	}
	return true;
}

function setData16(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	if (channel != -1 && equpmentID.length > 0) {
		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "16;" + caption + ";0x" + data.toString(16));
	}

	if (confFirmware.setData16(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData16");
		return false;
	}
	return true;
}

function setData32(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	if (channel != -1 && equpmentID.length > 0) {
		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";0x" + data.toString(16));
	}

	if (confFirmware.setData32(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setData32");
		return false;
	}
	return true;
}

function setDataFloat(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: number): boolean {
	if (channel != -1 && equpmentID.length > 0) {
		confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";0;" + "32;" + caption + ";" + data);
	}

	if (confFirmware.setDataFloat(frameIndex, offset, data) == false) {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function setDataFloat");
		return false;
	}
	return true;
}

function storeCrc64(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, start: number, count: number, offset: number): string {
	let result: string = confFirmware.storeCrc64(frameIndex, start, count, offset);

	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + "CRC64;0x" + result);

	if (result == "") {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeCrc64");
	}
	return result;
}

function storeHash64(confFirmware: ModuleFirmware, log: IssueLogger, channel: number, equpmentID: string, frameIndex: number, offset: number, caption: string, data: string): string {
	let result: string = confFirmware.storeHash64(frameIndex, offset, data);

	confFirmware.jsAddDescription(channel, equpmentID + ";" + frameIndex + ";" + offset + ";" + "0;" + "64;" + caption + ";0x" + result);

	if (result == "") {
		log.writeError("Frame = " + frameIndex + ", Offset = " + offset + ", frameIndex or offset are out of range in function storeHash64");
	}
	return result;
}

function ipToString(ip: number): string {
	let ip0: number = (ip >> 24) & 0xff;
	let ip1: number = (ip >> 16) & 0xff;
	let ip2: number = (ip >> 8) & 0xff;
	let ip3: number = (ip) & 0xff;
	let result: string = ip0 + "." + ip1 + "." + ip2 + "." + ip3;
	return result;
}

function truncate_to_int(x: number): number {
	if (x > 0) {
		return Math.floor(x);
	}
	else {
		return Math.ceil(x);
	}
}

function valToADC(val: number, lowLimit: number, highLimit: number, lowADC: number, highADC: number): number {
	if ((highLimit - lowLimit) == 0) {
		return 0;		// to exclude division by zero
	}

	let res: number = (highADC - lowADC) * (val - lowLimit) / (highLimit - lowLimit) + lowADC;

	return Math.round(res);
}

function module_lm_1(builder: Builder, root: ScriptDeviceObject, module: ScriptDeviceModule, confFirmware: ModuleFirmware, log: IssueLogger,
	signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule): boolean {

	if (module.moduleFamily == FamilyLMID) {
		let place: number = module.place;

		if (place != 0) {
			log.errCFG3002("Place", place, 0, 0, module.equipmentId);
			return false;
		}

		// Generate Configuration
		//
		return generate_lm_1_rev3(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
	}

	return false;
}

// Generate configuration for module LM-1
//
//
function generate_lm_1_rev3(builder: Builder, root: ScriptDeviceObject, module: ScriptDeviceModule, confFirmware: ModuleFirmware, log: IssueLogger,
	signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {

	let checkProperties: string[] = ["SubsystemID", "LMNumber", "AppLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID"];
	for (let cp: number = 0; cp < checkProperties.length; cp++) {
		if (module.propertyValue(checkProperties[cp]) == undefined) {
			log.errCFG3000(checkProperties[cp], module.equipmentId);
			return false;
		}
	}

	// Variables
	//
	let subSysID: string = module.propertyString("SubsystemID");
	let LMNumber: number = module.propertyInt("LMNumber");
	let moduleId: number = module.moduleFamily + module.moduleVersion;

	// Constants
	//
	let frameSize: number = logicModuleDescription.FlashMemory_ConfigFramePayload;
	let frameCount: number = logicModuleDescription.FlashMemory_ConfigFrameCount;

	if (frameSize < 1016) {
		log.errCFG3002("FlashMemory/ConfigFrameSize", frameSize, 1016, 65535, module.equipmentId);
		return false;
	}

	if (frameCount < 78 /*2 + 19  frames * 4 channels*/) {
		log.errCFG3002("FlashMemory/ConfigFrameCount", frameCount, 78, 65535, module.equipmentId);
		return false;
	}

	let uartId: number = 0x0102;

	let appWordsCount: number = module.propertyInt("AppLANDataSize");
	let diagWordsCount: number = logicModuleDescription.Memory_TxDiagDataSize;

	let ssKeyValue: number = subsystemStorage.ssKey(subSysID);
	if (ssKeyValue == -1) {
		log.errCFG3001(subSysID, module.equipmentId);
		return false;
	}

	let maxLMNumber: number = 12;               // Can be changed!
	let configStartFrames: number = 2;
	let configFrameCount: number = 19;          // number of frames in each configuration
	let ioModulesMaxCount: number = 14;

	if (LMNumber < 1 || LMNumber > maxLMNumber) {
		log.errCFG3002("System/LMNumber", LMNumber, 1, maxLMNumber, module.equipmentId);
		return false;
	}

	let descriptionVersion = 1;

	confFirmware.jsSetDescriptionFields(descriptionVersion, "EquipmentID;Frame;Offset;BitNo;Size;Caption;Value");

	confFirmware.writeLog("---\r\n");
	confFirmware.writeLog("Module: LM-1\r\n");
	confFirmware.writeLog("EquipmentID = " + module.equipmentId + "\r\n");
	confFirmware.writeLog("Subsystem ID = " + subSysID + "\r\n");
	confFirmware.writeLog("Key value = " + ssKeyValue + "\r\n");
	confFirmware.writeLog("ModuleID = " + moduleId + "\r\n");
	confFirmware.writeLog("UartID = " + uartId + "\r\n");
	confFirmware.writeLog("Frame size = " + frameSize + "\r\n");
	confFirmware.writeLog("LMNumber = " + LMNumber + "\r\n");
	confFirmware.writeLog("LMDescriptionNumber = " + logicModuleDescription.descriptionNumber() + "\r\n");

	// Configuration storage format
	//
	let frameStorageConfig: number = 1;
	let ptr: number = 0;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Marker", 0xca70) == false)     //CFG_Marker
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Marker = 0xca70" + "\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "Version", 0x0001) == false)     //CFG_Version
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] CFG_Version = 0x0001" + "\r\n");
	ptr += 2;


	let ssKey: number = ssKeyValue << 6;             //0000SSKEYY000000b
	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "SubsystemKey", ssKey) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] ssKey = " + ssKey + "\r\n");
	ptr += 2;

	let buildNo: number = confFirmware.buildNumber();
	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "BuildNo", buildNo) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] BuildNo = " + buildNo + "\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMDescriptionNumber", logicModuleDescription.descriptionNumber()) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMDescriptionNumber = " + logicModuleDescription.descriptionNumber() + "\r\n");
	ptr += 2;

	// reserved
	ptr += 4;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, ptr, "LMNumberCount", LMNumberCount) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + ptr + "] LMNumberCount = " + LMNumberCount + "\r\n");
	ptr += 2;

	let configIndexOffset: number = ptr + (LMNumber - 1) * (2/*offset*/ + 4/*reserved*/);
	let configFrame: number = configStartFrames + configFrameCount * (LMNumber - 1);

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameStorageConfig, configIndexOffset, "ConfigStartFrame", configFrame) == false) {
		return false;
	}
	confFirmware.writeLog("    [" + frameStorageConfig + ":" + configIndexOffset + "] configFrame = " + configFrame + "\r\n");

	// Service information
	//
	confFirmware.writeLog("Writing service information.\r\n");

	let frameServiceConfig: number = configFrame;
	ptr = 0;
	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "ServiceVersion", 0x0001) == false)   //CFG_Ch_Vers
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] CFG_Ch_Vers = 0x0001\r\n");
	ptr += 2;

	if (setData16(confFirmware, log, LMNumber, module.equipmentId, frameServiceConfig, ptr, "UartID", uartId) == false)  //CFG_Ch_Dtype == UARTID?
	{
		return false;
	}
	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] uartId = " + uartId + "\r\n");
	ptr += 2;

	//Hash (UniqueID) will be counted later, write zero for future replacement

	confFirmware.writeLog("    [" + frameServiceConfig + ":" + ptr + "] UniqueID = 0\r\n");
	ptr += 8;

	// I/O Modules configuration
	//
	confFirmware.writeLog("Writing I/O modules configuration.\r\n");

	let frameIOConfig: number = configFrame + 1;

	if (module.parent().isChassis() === false)
	{
		log.errCFG3042(module.equipmentId, module.uuid)
	}

	let parent: ScriptDeviceChassis = module.parent().toChassis();

	for (let i: number = 0; i < parent.childrenCount; i++) {
		if (builder.jsIsInterruptRequested() == true) {
			return true;
		}

		if (parent.child(i).isModule() == false) {
			continue;
		}

		let ioModule: ScriptDeviceModule = parent.child(i).toModule();

		if (ioModule.moduleFamily == FamilyLMID) {
			continue;
		}

		let ioPlace: number = ioModule.place;
		if (ioPlace < 1 || ioPlace > ioModulesMaxCount) {
			log.errCFG3002("Place", ioPlace, 1, ioModulesMaxCount, ioModule.equipmentId);
			return false;
		}

		let ioEquipmentID: string = ioModule.equipmentId;

		let checkProperties: string[] = ["ConfigurationScript"];
		for (let cp: number = 0; cp < checkProperties.length; cp++) {
			if (ioModule.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], ioEquipmentID);
				return false;
			}
		}

		let ioModuleFamily: number = ioModule.moduleFamily;

		let frame: number = frameIOConfig + ioPlace - 1;

		confFirmware.writeLog("Generating configuration for " + ioModule.caption + ": " + ioEquipmentID + " Place: " + ioModule.place + " Frame: " + frame + "\r\n");

		let configScript: string = ioModule.propertyString("ConfigurationScript");
		if (configScript.length != 0) {
			if (runConfigScript(configScript, confFirmware, ioModule, LMNumber, frame, log, signalSet, opticModuleStorage) == false) {
				return false;
			}
		}

		let diagWordsIoCount: number = ioModule.propertyInt("TxDiagDataSize");
		if (diagWordsIoCount == null) {
			log.errCFG3000("TxDiagDataSize", ioEquipmentID);
			return false;
		}

		if ((diagWordsIoCount & 1) != 0)
		{
			diagWordsIoCount++;	// Align to word
		}

		diagWordsCount += diagWordsIoCount;

		// I/o module diag data size
		//
		ptr = 1006;
		if (setData16(confFirmware, log, LMNumber, ioModule.equipmentId, frame, ptr, "DiagDataSize", diagWordsIoCount) == false)
		{
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr + "] DiagDataSize = " + diagWordsIoCount + "\r\n");
	
	}

	let lanConfigFrame: number = frameIOConfig + ioModulesMaxCount;

	// Create LANs configuration
	//
	confFirmware.writeLog("Writing LAN configuration.\r\n");

	let lanFrame: number = lanConfigFrame;

	// Tuning
	//
	let ethernetcontrollerId: string = "_ETHERNET01";
	let ethernetControllerObject: ScriptDeviceObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerId);
	if (ethernetControllerObject == null || ethernetControllerObject.isController() == false) {
		log.errCFG3004(module.equipmentId + ethernetcontrollerId, module.equipmentId);
		return false;
	}

	let ethernetController: ScriptDeviceController = ethernetControllerObject.toController();

	let checkTuningProperties: string[] = ["TuningServiceID", "TuningEnable", "TuningIP", "TuningPort", "OverrideTuningDataWordCount"];
	for (let cp: number = 0; cp < checkTuningProperties.length; cp++) {
		if (ethernetController.propertyValue(checkTuningProperties[cp]) == undefined) {
			log.errCFG3000(checkTuningProperties[cp], ethernetController.equipmentId);
			return false;
		}
	}
	confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");

	// Controller
	//
	let tuningWordsCount: number = 716;

	let tuningIP: number = 0;
	let tuningPort: number = 0;

	// Service
	//
	let tuningServiceIP: number = 0;
	let tuningServicePort: number = 0;

	let serviceID: string = ethernetController.propertyString("TuningServiceID");

	if (ethernetController.propertyBool("TuningEnable") == true) {

		tuningIP = ethernetController.propertyIP("TuningIP");
		tuningPort = ethernetController.propertyInt("TuningPort");

		let serviceObject: ScriptDeviceObject = root.childByEquipmentId(serviceID);
		if (serviceObject == null || serviceObject.isSoftware() == false) {
			log.wrnCFG3008(serviceID, module.equipmentId);
		}
		else {
			// Check software type
			//
			let service: ScriptDeviceSoftware = serviceObject.toSoftware();
			
			if (service.softwareType != SoftwareType.TuningService) {
				log.errCFG3017(ethernetController.equipmentId, "Type", service.equipmentId);
				return false;
			}

			//

			let checkTuningProperties: string[] = ["TuningDataIP", "TuningDataPort"];
			for (let cp: number = 0; cp < checkTuningProperties.length; cp++) {
				if (service.propertyValue(checkTuningProperties[cp]) == undefined) {
					log.errCFG3000(checkTuningProperties[cp], service.equipmentId);
					return false;
				}
			}

			tuningServiceIP = service.propertyIP("TuningDataIP");
			tuningServicePort = service.propertyInt("TuningDataPort");
		}
	}

	let controllerTuningWordsCount: number = tuningWordsCount;

	let tuningDataID: number = module.propertyValue("TuningLANDataUID");

	let overrideTuningWordsCount: number = ethernetController.propertyInt("OverrideTuningDataWordCount");
	if (overrideTuningWordsCount != -1) {
		controllerTuningWordsCount = overrideTuningWordsCount;
		tuningDataID = 0;
	}

	if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController,
		controllerTuningWordsCount, tuningIP, tuningPort, tuningServiceIP, tuningServicePort, tuningDataID,
		0, 0, 0, 0, 0, 0) == false)	//Subnet2 is not used
	{
		return false;
	}
	lanFrame++;

	// REG / DIAG
	//
	for (let i: number = 0; i < 2; i++) {

		ethernetcontrollerId = "_ETHERNET0" + (i + 2);

		ethernetControllerObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerId);
		if (ethernetControllerObject == null || ethernetControllerObject.isController() == false) {
			log.errCFG3004(module.equipmentId + ethernetcontrollerId, module.equipmentId);
			return false;
		}
	
		ethernetController = ethernetControllerObject.toController();

		let checkProperties: string[] = ["AppDataServiceID", "AppDataEnable", "AppDataIP", "AppDataPort",
			"DiagDataServiceID", "DiagDataEnable", "DiagDataIP", "DiagDataPort",
			"OverrideAppDataWordCount", "OverrideDiagDataWordCount"];
		for (let cp: number = 0; cp < checkProperties.length; cp++) {
			if (ethernetController.propertyValue(checkProperties[cp]) == undefined) {
				log.errCFG3000(checkProperties[cp], ethernetController.equipmentId);
				return false;
			}
		}
		confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");

		let servicesName: string[] = ["App", "Diag"];

		let ip: number[] = [0, 0];
		let port: number[] = [0, 0];

		let serviceIP: number[] = [0, 0];
		let servicePort: number[] = [0, 0];

		for (let s: number = 0; s < 2; s++) {

			// Service
			let serviceID: string = ethernetController.propertyString(servicesName[s] + "DataServiceID");

			if (ethernetController.propertyBool(servicesName[s] + "DataEnable") == true) {

				ip[s] = ethernetController.propertyIP(servicesName[s] + "DataIP");
				port[s] = ethernetController.propertyInt(servicesName[s] + "DataPort");

				let serviceObject: ScriptDeviceObject = root.childByEquipmentId(serviceID);
				if (serviceObject == null || serviceObject.isSoftware() == false) {

					log.wrnCFG3008(serviceID, module.equipmentId);

					if (i == 0)	// in Ethernet port 1, if service was not found, use default IP addresses
					{
						if (s == 0)	// this is App
						{
							serviceIP[s] = 0xc0a80bfe;	//	192.168.11.254
							servicePort[s] = 13322;
						}

						if (s == 1)	// this is Diag
						{
							serviceIP[s] = 0xc0a815fe;	//	192.168.21.254
							servicePort[s] = 13352;
						}

						if (serviceIP[s] != 0 && servicePort[s] != 0) {
							log.wrnCFG3018(servicesName[s] + "DataService", ipToString(serviceIP[s]), servicePort[s], ethernetController.equipmentId);
						}
					}

				}
				else {

					// Check software type
					//
					let service: ScriptDeviceSoftware = serviceObject.toSoftware();

					if ((s == 0 && service.softwareType != SoftwareType.AppDataService) ||
						(s == 1 && service.softwareType != SoftwareType.DiagDataService)) {
						log.errCFG3017(ethernetController.equipmentId, "Type", service.equipmentId);
						return false;
					}

					let checkProperties: string[] = ["DataReceivingIP", "DataReceivingPort"];
					for (let cp: number = 0; cp < checkProperties.length; cp++) {
						if (service.propertyValue(servicesName[s] + checkProperties[cp]) == undefined) {
							log.errCFG3000(servicesName[s] + checkProperties[cp], service.equipmentId);
							return false;
						}
					}

					serviceIP[s] = service.propertyIP(servicesName[s] + "DataReceivingIP");
					servicePort[s] = service.propertyInt(servicesName[s] + "DataReceivingPort");
				}
			}
		}

		let regDataID: number = module.propertyValue("AppLANDataUID");
		let diagDataID: number = module.propertyValue("DiagLANDataUID");

		let controllerAppWordsCount: number = appWordsCount;

		let overrideRegWordsCount: number = ethernetController.propertyInt("OverrideAppDataWordCount");
		if (overrideRegWordsCount != -1) {
			controllerAppWordsCount = overrideRegWordsCount;
			regDataID = 0;
		}

		let controllerDiagWordsCount: number = diagWordsCount;

		let overrideDiagWordsCount: number = ethernetController.propertyInt("OverrideDiagDataWordCount");
		if (overrideDiagWordsCount != -1) {
			controllerDiagWordsCount = overrideDiagWordsCount;
			diagDataID = 0;
		}

		if (generate_LANConfiguration(confFirmware, log, lanFrame, module, ethernetController,
			controllerAppWordsCount, ip[0], port[0], serviceIP[0], servicePort[0], regDataID,
			controllerDiagWordsCount, ip[1], port[1], serviceIP[1], servicePort[1], diagDataID) == false) {
			return false;
		}
		lanFrame++;
	}

	// Create TX/RX configuration
	//
	confFirmware.writeLog("Writing TxRx(Opto) configuration.\r\n");

	let txRxConfigFrame: number = lanConfigFrame + 3;

	if (generate_lmTxRxOptoConfiguration(confFirmware, log, txRxConfigFrame, module, LMNumber, opticModuleStorage, logicModuleDescription) == false) {
		return false;
	}

	// create UniqueID
	//
	let startFrame: number = configStartFrames + configFrameCount * (LMNumber - 1);

	let uniqueID: number = 0;

	for (let i: number = 0; i < configFrameCount; i++) {
		let crc: number = confFirmware.calcCrc32(startFrame + i, 0, frameSize);

		uniqueID ^= crc;
	}

	confFirmware.jsSetUniqueID(LMNumber, uniqueID);

	return true;
}

function generate_txRxIoConfig(confFirmware: ModuleFirmware, equipmentID: string, LMNumber: number, frame: number, offset: number, log: IssueLogger,
	flags: number, configFrames: number, dataFrames: number, moduleId: number): boolean {
	// TxRx Block's configuration structure
	//
	let ptr: number = offset;

	confFirmware.writeLog("    TxRxConfig: [" + frame + ":" + ptr + "] Flags = " + flags +
		"; [" + frame + ":" + (ptr + 2) + "] ConfigFrames = " + configFrames +
		"; [" + frame + ":" + (ptr + 4) + "] DataFrames = " + dataFrames +
		"; [" + frame + ":" + (ptr + 6) + "] ModuleId = " + moduleId + "\r\n");

	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "TxRxFlags", flags) == false)        // Flags word
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Configuration words quantity", configFrames) == false) // Configuration words quantity
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "Data words quantity", dataFrames) == false)   // Data words quantity
	{
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, equipmentID, frame, ptr, "ModuleID", moduleId) == false)         // Tx ID
	{
		return false;
	}
	ptr += 2;

	return true;
}

function generate_LANConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: ScriptDeviceModule, ethernetController: ScriptDeviceController,
	regWordsCount: number, regIP: number, regPort: number, regServiceIP: number, regServicePort: number, regDataID: number,
	diagWordsCount: number, diagIP: number, diagPort: number, diagServiceIP: number, diagServicePort: number, diagDataID: number): boolean {
	let ptr: number = 0;

	let moduleEquipmentID: string = module.equipmentId;
	let LMNumber: number = module.propertyInt("LMNumber");
	let controllerEquipmentID: string = ethernetController.equipmentId;

	//mac
	//
	let hashName: string = "S" + regIP + diagIP + moduleEquipmentID + regServiceIP + diagServiceIP;
	let hashList: JsVariantList = confFirmware.calcHash64(hashName);
	let size: number = hashList.jsSize();
	if (size != 2) {
		log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
		return false;
	}

	let h0: number = hashList.jsAt(0);
	let h1: number = hashList.jsAt(1);

	let m1: number = 0x4200;
	let m2: number = h0 & 0x7fff;
	let m3: number = (h0 >> 16) & 0x7fff;

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : MAC address of LM = " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + "\r\n");
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC1", m1) == false) {
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC2", m2) == false) {
		return false;
	}
	ptr += 2;
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "MAC3", m3) == false) {
		return false;
	}
	ptr += 2;

	// regIP

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : IP 1 = " + ipToString(regIP) + "\r\n");

	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "IP 1", regIP) == false) {
		return false;
	}
	ptr += 4;

	// regPort

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 1 = " + regPort + "\r\n");

	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1", regPort) == false) {
		return false;
	}
	ptr += 2;

	// diagIP

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : IP 2 = " + ipToString(diagIP) + "\r\n");

	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "IP 2", diagIP) == false) {
		return false;
	}
	ptr += 4;

	// diagPort

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 2 = " + diagPort + "\r\n");

	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2", regPort) == false) {
		return false;
	}
	ptr += 2;

	// regServiceIP

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service IP 1 = " + ipToString(regServiceIP) + "\r\n");

	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service IP 1", regServiceIP) == false) {
		return false;
	}
	ptr += 4;

	// regServicePort

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service Port 1 = " + regServicePort + "\r\n");

	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service Port 1", regServicePort) == false) {
		return false;
	}
	ptr += 2;

	// regWordsCount

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 1 words count = " + regWordsCount + "\r\n");

	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1 words count", regWordsCount) == false) {
		return false;
	}
	ptr += 2;

	// diagServiceIP

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service IP 2 = " + ipToString(diagServiceIP) + "\r\n");

	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service IP 2", diagServiceIP) == false) {
		return false;
	}
	ptr += 4;

	// diagServicePort

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Service Port 2 = " + diagServicePort + "\r\n");

	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Service Port 2", diagServicePort) == false) {
		return false;
	}
	ptr += 2;

	// diagWordsCount

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 2 words count = " + diagWordsCount + "\r\n");

	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2 words count", diagWordsCount) == false) {
		return false;
	}
	ptr += 2;

	// appDUID

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 1 DUID = " + regDataID + "\r\n");

	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 1 DUID", regDataID) == false) {
		return false;
	}
	ptr += 4;

	// diagDUID

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : Port 2 DUID = " + diagDataID + "\r\n");

	if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Port 2 DUID", diagDataID) == false) {
		return false;
	}
	ptr += 4;

	return true;
}

// function returns the amount of transmitting words
//
function generate_lmTxRxOptoConfiguration(confFirmware: ModuleFirmware, log: IssueLogger, frame: number, module: ScriptDeviceModule, LMNumber: number, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {
	if (module.propertyValue("EquipmentID") == undefined) {
		log.errCFG3000("EquipmentID", "Class_Module");
		return false;
	}

	let portCount: number = logicModuleDescription.OptoInterface_OptoPortCount;

	let txWordsCount: number = 0;

	for (let p: number = 0; p < portCount; p++) {
		let controllerID: string = module.equipmentId + "_OPTOPORT0";
		controllerID = controllerID + (p + 1);

		let controllerObject: ScriptDeviceObject = module.childByEquipmentId(controllerID);
		if (controllerObject == null || controllerObject.isController() == false) {
			log.errCFG3004(controllerID, module.equipmentId);
			return false;
		}

		let controller: ScriptDeviceController = controllerObject.toController();

		let optoPort: OptoPort = opticModuleStorage.jsGetOptoPort(controller.equipmentId);
		if (optoPort == null) {
			continue;
		}

		if (optoPort.connectionID() == "" && optoPort.txDataSizeW() == 0 && optoPort.rxDataSizeW() == 0) {
			continue;
		}

		confFirmware.writeLog("    OptoPort " + controller.equipmentId + ": connection ID = " + optoPort.equipmentID() +
			" (" + optoPort.connectionID() + ")\r\n");

		let ptr: number = 0 + p * 2;

		let value: number = optoPort.txStartAddress();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX startAddress for TxRx Block (Opto) " + (p + 1), value) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX startAddress for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

		ptr = 5 * 2 + p * 2;
		value = optoPort.txDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");
		txWordsCount += value;

		ptr = 10 * 2 + p * 2;
		value = optoPort.portID();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TX id for TxRx Block (Opto) " + (p + 1), value) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TX id for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

		ptr = 15 * 2 + p * 2;
		value = optoPort.rxDataSizeW();
		if (setData16(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "RX data words quantity for TxRx Block (Opto) " + (p + 1), value) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr + "]: RX data words quantity for TxRx Block (Opto) " + (p + 1) + " = " + value + "\r\n");

		let dataUID: number = 0;
		if (optoPort.isLinked() == true) {
			let linkedPort: string = optoPort.linkedPortID();
			let linkedOptoPort: OptoPort = opticModuleStorage.jsGetOptoPort(linkedPort);
			if (linkedOptoPort != null) {
				dataUID = linkedOptoPort.txDataID();
			}
		}

		ptr = 20 * 2 + p * 4;
		if (setData32(confFirmware, log, LMNumber, controller.equipmentId, frame, ptr, "TxRx Block (Opto) Data UID " + (p + 1), dataUID) == false) {
			return false;
		}
		confFirmware.writeLog("    [" + frame + ":" + ptr + "]: TxRx Block (Opto) Data UID " + (p + 1) + " = " + dataUID + "\r\n");
	} // p

	return true;
}
