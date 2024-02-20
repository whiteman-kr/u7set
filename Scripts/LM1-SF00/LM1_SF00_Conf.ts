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

	mV_Type_L = 36,
	mV_Type_M = 37,
	mV_Raw_m1200_p1200 = 38,
}

enum OutputMode {
	Plus0_Plus5_V = 0,
	Plus4_Plus20_mA = 1,
	Minus10_Plus10_V = 2,
	Plus0_Plus5_mA = 3,
	Plus0_Plus20_mA = 4,
	Plus0_Plus24_mA = 5,
};

enum UnitsConvertorErrorCode {
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
	calcHash64(dataString: string): any;

	jsSetDescriptionFields(descriptionVersion: number, description: string): void;
	jsAddDescription(channel: number, description: string): void;
	jsSetUniqueID(LMNumber: number, uniqueID: number): void;

	writeLog(message: string): void;
	buildNumber(): number;

	checkMacForUnique(m1: number, m2: number, m3: number): boolean;
}

interface IssueLogger {
	writeMessage(message: string): void;
	writeWarning(message: string): void;
	writeError(message: string): void;

	errINT1001(message: string): void;

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

	jsLanControllerType(index: number): number;
	jsLanControllerPlace(index: number): number;

	FlashMemory_ConfigFramePayload: number;
	FlashMemory_ConfigFrameCount: number;
	FlashMemory_ConfigUartId: number;
	Memory_TxDiagDataSize: number;
	OptoInterface_OptoPortCount: number;
	Lan_ControllerCount: number;
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

interface LanConfig {
	flags: number;
	ip: number;
	port: number;
	serviceIP: number;
	servicePort: number;
	wordsCount: number;
	dataID: number;
}

enum LanControllerType {
	Unknown = 0,
	Tuning = 1,
	AppData = 2,
	DiagData = 4,
	AppAndDiagData = 6,
	TuningAndAppAndDiagData = 7
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
//let configScriptVersion: number = 42;		// DiagDataSize is written for i/o module frame for LM8_SR10, LM1_SR03 and LM1_SR04
//let configScriptVersion: number = 43;		// Tuning LAN configuration is placed in LAN2 and LAN3 for LM1_SR04 LAN 
//let configScriptVersion: number = 44;		// Tuning LAN configuration is placed in LAN1 or LAN2/LAN3 depending on LAN description
//let configScriptVersion: number = 45; 	// Added mV_Type_L, mV_Type_M and mV_Raw_m1200_p1200 sensor types
//let configScriptVersion: number = 46;		// MAC address is checkind for uniqueness, LAN values are set to 0 if LAN is switched off
//let configScriptVersion: number = 47;		// LAN configuration is dynamically generated
let configScriptVersion: number = 48;		// Using DiagLANDataSize calculated by RPCT

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
		return generate_lm(builder, root, module, confFirmware, log, signalSet, subsystemStorage, opticModuleStorage, logicModuleDescription);
	}

	return false;
}

// Generate configuration for module LM
//
//
function generate_lm(builder: Builder, root: ScriptDeviceObject, module: ScriptDeviceModule, confFirmware: ModuleFirmware, log: IssueLogger,
	signalSet: SignalSet, subsystemStorage: SubsystemStorage, opticModuleStorage: OptoModuleStorage, logicModuleDescription: LogicModule) {

	let checkProperties: string[] = ["SubsystemID", "LMNumber", "AppLANDataSize", "DiagLANDataSize", "TuningLANDataUID", "AppLANDataUID", "DiagLANDataUID"];
	for (let cp: number = 0; cp < checkProperties.length; cp++) {
		if (module.propertyValue(checkProperties[cp]) == undefined) {
			log.errCFG3000(checkProperties[cp], module.equipmentId);
			return false;
		}
	}

	const MODULEID_LM1_SF00: number = 0x1100;
	const MODULEID_LM1_SF01: number = 0x1101;

	const MODULEID_LM1_SR01: number = 0x11A0;
	const MODULEID_LM1_SR02: number = 0x11A1;
	const MODULEID_LM1_SR03: number = 0x11A2;
	const MODULEID_LM1_SR04: number = 0x11B0;

	const MODULEID_LM1_SR20: number = 0x11A3;
	const MODULEID_LM1_SR05: number = 0x11B2;
	const MODULEID_LM8_SR10: number = 0x11D0;
	const MODULEID_LM11_SR90: number = 0x1190;

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

	const appWordsCount: number = module.propertyInt("AppLANDataSize");
	const diagWordsCount: number = module.propertyInt("DiagLANDataSize");

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
	confFirmware.writeLog("ModuleID = " + moduleId.toString(16) + "h\r\n");
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

	if (module.parent().isChassis() === false) {
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

		if ((diagWordsIoCount & 1) != 0) {
			diagWordsIoCount++;	// Align to words
		}

		if (moduleId != MODULEID_LM1_SF00 &&
			moduleId != MODULEID_LM1_SF01 &&
			moduleId != MODULEID_LM1_SR01 &&
			moduleId != MODULEID_LM1_SR02 &&
			moduleId != MODULEID_LM1_SR03 && 
			moduleId != MODULEID_LM1_SR04) {

			// I/o module diag data size are written in new LMs
			//
			ptr = 1006;
			if (setData16(confFirmware, log, LMNumber, ioModule.equipmentId, frame, ptr, "DiagDataSize", diagWordsIoCount) == false) {
				return false;
			}
			confFirmware.writeLog("    [" + frame + ":" + ptr + "] DiagDataSize = " + diagWordsIoCount + "\r\n");
		}
	}

	let lanConfigFrame: number = frameIOConfig + ioModulesMaxCount;

	// Create LANs configuration
	//
	confFirmware.writeLog("Writing LAN configuration.\r\n");

	const maxLanControllerCount: number = 3;

	let lanControllerCount: number = logicModuleDescription.Lan_ControllerCount;

	if (lanControllerCount < 1 || lanControllerCount > maxLanControllerCount) {
		log.writeError(module.equipmentId + ": wrong LAN controllers count (" + lanControllerCount + "), expected 1.." + maxLanControllerCount);
		return false;
	}

	let appAndDiagChannel: number = 0;

	for (let i: number = 0; i < lanControllerCount; i++) {

		let lanPlace: number = logicModuleDescription.jsLanControllerPlace(i);

		if (lanPlace < 1 || lanPlace > maxLanControllerCount)
		{
			log.writeError(module.equipmentId + ": wrong LAN controller place in LM description (" + lanPlace + "), expected 1.." + maxLanControllerCount);
			return false;
		}

		let lanType: LanControllerType = logicModuleDescription.jsLanControllerType(i);

		let ethernetcontrollerId: string = "_ETHERNET0" + lanPlace;

		let lanFrame: number = lanConfigFrame +  (lanPlace - 1);

		confFirmware.writeLog("    Ethernet Controller " + module.equipmentId + ethernetcontrollerId + "\r\n");

		let tuningLan: LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: 716,
			dataID: 0
		};

		let emptyLan: LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: 0,
			dataID: 0
		};

		let appLan: LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: appWordsCount,
			dataID: 0
		};

		let diagLan: LanConfig = {
			flags: 0,
			ip: 0,
			port: 0,
			serviceIP: 0,
			servicePort: 0,
			wordsCount: diagWordsCount,
			dataID: 0
		};

		if (appAndDiagChannel == 0) {
			// Set default values for first App and Diag channel

			appLan.serviceIP = 0xc0a80bfe;	//	192.168.11.254
			appLan.servicePort = 13322;

			diagLan.serviceIP = 0xc0a815fe;	//	192.168.21.254
			diagLan.servicePort = 13352;
		}
		

		if (lanType == LanControllerType.Tuning) {

			if (fillLanServiceData(confFirmware, SoftwareType.TuningService, root, module, ethernetcontrollerId, tuningLan, log) == false) {
				return false;
			}

			if (generate_LANConfiguration_v0(confFirmware, lanFrame, module, ethernetcontrollerId, tuningLan, emptyLan, log) == false)	//Channel is not used
			{
				return false;
			}
		}

		if (lanType == LanControllerType.AppAndDiagData) {

			if (fillLanServiceData(confFirmware, SoftwareType.AppDataService, root, module, ethernetcontrollerId, appLan, log) == false) {
				return false;
			}

			if (fillLanServiceData(confFirmware, SoftwareType.DiagDataService, root, module, ethernetcontrollerId, diagLan, log) == false) {
				return false;
			}

			if (generate_LANConfiguration_v0(confFirmware, lanFrame, module, ethernetcontrollerId, appLan, diagLan, log) == false) {
				return false;
			}

			appAndDiagChannel++;
		}

		if (lanType == LanControllerType.TuningAndAppAndDiagData) {

			if (fillLanServiceData(confFirmware, SoftwareType.TuningService, root, module, ethernetcontrollerId, tuningLan, log) == false) {
				return false;
			}
			
			if (fillLanServiceData(confFirmware, SoftwareType.AppDataService, root, module, ethernetcontrollerId, appLan, log) == false) {
				return false;
			}

			if (fillLanServiceData(confFirmware, SoftwareType.DiagDataService, root, module, ethernetcontrollerId, diagLan, log) == false) {
				return false;
			}

			let lans: LanConfig[] = [];
			lans.push(appLan);
			lans.push(diagLan);
			lans.push(tuningLan);

			if (generate_LANConfiguration_v1(confFirmware, lanFrame, module, ethernetcontrollerId, lans, log) == false) {
				return false;
			}

			appAndDiagChannel++;
		}
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

function fillLanServiceData(
	confFirmware: ModuleFirmware,
	softwareType: SoftwareType,
	root: ScriptDeviceObject,
	module: ScriptDeviceModule,
	ethernetcontrollerId: string,
	lan: LanConfig,
	log: IssueLogger): boolean {

	// Build prefix

	let controllerPrefix: string;
	let servicePrefix: string;
	let overridePrefix: string;

	switch (softwareType) {
		case SoftwareType.TuningService:
			controllerPrefix = "Tuning";
			servicePrefix = "TuningData";
			overridePrefix = "Tuning";
			break;
		case SoftwareType.AppDataService:
			controllerPrefix = "AppData";
			servicePrefix = "AppDataReceiving";
			overridePrefix = "App";
			break;
		case SoftwareType.DiagDataService:
			controllerPrefix = "DiagData";
			servicePrefix = "DiagDataReceiving";
			overridePrefix = "Diag";
			break;
		default:
			log.writeError("fillLanServiceData: wrong software type");
			return false;
	}

	// Get ethernet controller

	let ethernetControllerObject: ScriptDeviceObject = module.childByEquipmentId(module.equipmentId + ethernetcontrollerId);
	if (ethernetControllerObject == null || ethernetControllerObject.isController() == false) {
		log.errCFG3004(module.equipmentId + ethernetcontrollerId, module.equipmentId);
		return false;
	}

	let ethernetController: ScriptDeviceController = ethernetControllerObject.toController();

	let checkControllerProperties: string[] = [controllerPrefix + "ServiceID", controllerPrefix + "Enable", controllerPrefix + "IP", controllerPrefix + "Port", "Override" + overridePrefix + "DataWordCount"];
	for (let cp: number = 0; cp < checkControllerProperties.length; cp++) {
		if (ethernetController.propertyValue(checkControllerProperties[cp]) == undefined) {
			log.errCFG3000(checkControllerProperties[cp], ethernetController.equipmentId);
			return false;
		}
	}

	// Get data from services

	let serviceID: string = ethernetController.propertyString(controllerPrefix + "ServiceID");

	if (ethernetController.propertyBool(controllerPrefix + "Enable") == true) {

		// If Enable == true, take IP from service or default if service is not found

		lan.ip = ethernetController.propertyIP(controllerPrefix + "IP");
		lan.port = ethernetController.propertyInt(controllerPrefix + "Port");

		let serviceObject: ScriptDeviceObject = root.childByEquipmentId(serviceID);	// This can be software or controller
		let serviceSoftware: ScriptDeviceSoftware = null;							// This will be software

		if (serviceObject != null)
		{
			if (serviceObject.isController() == true)
			{
				let parentObject: ScriptDeviceObject = serviceObject.parent();

				if (parentObject != null && parentObject.isSoftware() == true)
				{
					serviceSoftware = parentObject.toSoftware();
				}
			}
			else
			{
				if (serviceObject.isSoftware() == true)
				{
					serviceSoftware = serviceObject.toSoftware();
				}
			}
		}

		if (serviceObject == null || serviceSoftware == null) {

			//Service was not found

			if (lan.serviceIP != 0 && lan.servicePort != 0) {
				log.wrnCFG3018(controllerPrefix + "DataService", ipToString(lan.serviceIP), lan.servicePort, ethernetController.equipmentId);
			}
			else {
				log.wrnCFG3008(serviceID, module.equipmentId);
			}
		}
		else {
			// Check software type
			//

			if (serviceSoftware.softwareType != softwareType) {
				log.errCFG3017(ethernetController.equipmentId, "Type", serviceSoftware.equipmentId);
				return false;
			}

			// Take address from service

			let checkServiceProperties: string[] = [servicePrefix + "IP", servicePrefix + "Port"];
			for (let cp: number = 0; cp < checkServiceProperties.length; cp++) {
				if (serviceObject.propertyValue(checkServiceProperties[cp]) == undefined) {
					log.errCFG3000(checkServiceProperties[cp], serviceObject.equipmentId);
					return false;
				}
			}

			lan.serviceIP = serviceObject.propertyIP(servicePrefix + "IP");
			lan.servicePort = serviceObject.propertyInt(servicePrefix + "Port");
		}

		lan.dataID = module.propertyValue(overridePrefix + "LANDataUID");
		if (lan.dataID == undefined) {
			log.errCFG3000(overridePrefix + "LANDataUID", module.equipmentId);
			return false;
		}

		let overrideTuningWordsCount: number = ethernetController.propertyInt("Override" + overridePrefix + "DataWordCount");
		if (overrideTuningWordsCount != -1) {
			lan.wordsCount = overrideTuningWordsCount;
			lan.dataID = 0;
		}
	}
	else {
		// If Enable == false, set service ID is 0 even

		lan.dataID = 0;
		lan.wordsCount = 0;
		lan.serviceIP = 0;
		lan.servicePort = 0;
	}


	return true;
}

function generate_LANConfiguration_v0(confFirmware: ModuleFirmware, frame: number, module: ScriptDeviceModule, ethernetControllerId: string, lan1: LanConfig, lan2: LanConfig, log: IssueLogger): boolean {

	let lan: LanConfig[] = [];

	lan.push(lan1);
	lan.push(lan2);

	let ptr: number = 0;

	let controllerEquipmentID: string = module.equipmentId + ethernetControllerId;
	let LMNumber: number = module.propertyInt("LMNumber");

	let m1: number = 0;
	let m2: number = 0;
	let m3: number = 0;

	if (lan1.ip == 0 && lan2.ip == 0 && lan1.serviceIP == 0 && lan2.serviceIP == 0) {
		// mac is empty
		//
	}
	else {
		//mac
		//
		let hashName: string = "S";
		for (let i: number = 0; i < lan.length; i++) {
			hashName += lan[i].ip;
		}
		hashName += controllerEquipmentID;
		for (let i: number = 0; i < lan.length; i++) {
			hashName += lan[i].serviceIP;
		}

		let hashList: any = confFirmware.calcHash64(hashName);
		if (hashList.length != 2) {
			log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
			return false;
		}

		let h: number = (hashList[0] + hashList[1]);
		m1 = 0x4200;
		m2 = h & 0x7fff;
		m3 = (h >> 16) & 0x7fff;

		if (confFirmware.checkMacForUnique(m1, m2, m3) == false) {
			log.errINT1001("MAC address " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + " of " + controllerEquipmentID + " is not unique!");
		}
	}

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

	for (let i: number = 0; i < lan.length; i++) {

		// ip

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " IP = " + ipToString(lan[i].ip) + "\r\n");

		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " IP", lan[i].ip) == false) {
			return false;
		}
		ptr += 4;

		// port

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Port = " + lan[i].port + "\r\n");

		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Port", lan[i].port) == false) {
			return false;
		}
		ptr += 2;
	}

	if (lan.length == 1) {
		//	If only one LAN is used - skip LAN 2 data
		ptr += 4;

		ptr += 2;
	}

	for (let i: number = 0; i < lan.length; i++) {

		// ServiceIP

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Service IP = " + ipToString(lan[i].serviceIP) + "\r\n");

		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Service IP", lan[i].serviceIP) == false) {
			return false;
		}
		ptr += 4;

		// ServicePort

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Service Port = " + lan[i].servicePort + "\r\n");

		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Service Port = ", lan[i].servicePort) == false) {
			return false;
		}
		ptr += 2;

		// WordsCount

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " Words Count = " + lan[i].wordsCount + "\r\n");

		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " Words Count = ", lan[i].wordsCount) == false) {
			return false;
		}
		ptr += 2;
	}

	if (lan.length == 1) {
		//	If only one LAN is used - skip LAN 2 data
		ptr += 4;

		ptr += 2;

		ptr += 2;
	}

	for (let i: number = 0; i < lan.length; i++) {

		// DUID

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN " + (i + 1) + " DUID = " + lan[i].dataID + "\r\n");

		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "LAN " + (i + 1) + " DUID", lan[i].dataID) == false) {
			return false;
		}
		ptr += 4;
	}

	if (lan.length == 1) {
		//	If only one LAN is used - skip LAN 2 data
		ptr += 4;
	}

	return true;
}

function generate_LANConfiguration_v1(confFirmware: ModuleFirmware, frame: number, module: ScriptDeviceModule, ethernetControllerId: string,
	lan: LanConfig[], log: IssueLogger): boolean {
	let ptr: number = 0;

	let controllerEquipmentID: string = module.equipmentId + ethernetControllerId;
	let LMNumber: number = module.propertyInt("LMNumber");

	// Version
	//
	const version: number = 1;

	confFirmware.writeLog("    [" + frame + ":" + ptr + "] : LAN Configuration format version = " + version + "\r\n");
	if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "Version", version) == false) {
		return false;
	}
	ptr += 2;

	// MAC address
	//

	let m1: number = 0;
	let m2: number = 0;
	let m3: number = 0;

	let macIsEmpty: boolean = true;

	for (let i: number = 0; i < lan.length; i++) {
		if (lan[i].ip != 0 || lan[i].serviceIP != 0) {
			macIsEmpty = false;
			break;
		}
	}

	if (macIsEmpty == true) {
		// mac is empty
		//
	}
	else {
		// mac
		//
		let hashName: string = "S";
		for (let i: number = 0; i < lan.length; i++) {
			hashName += lan[i].ip;
		}
		hashName += controllerEquipmentID;
		for (let i: number = 0; i < lan.length; i++) {
			hashName += lan[i].serviceIP;
		}

		let hashList: any = confFirmware.calcHash64(hashName);
		if (hashList.length != 2) {
			log.writeError("Hash is not 2 32-bitwords in function generate_LANConfiguration!");
			return false;
		}

		let h: number = (hashList[0] + hashList[1]);
		m1 = 0x4200;
		m2 = h & 0x7fff;
		m3 = (h >> 16) & 0x7fff;

		if (confFirmware.checkMacForUnique(m1, m2, m3) == false) {
			log.errINT1001("MAC address " + m1.toString(16) + ":" + m2.toString(16) + ":" + m3.toString(16) + " of " + controllerEquipmentID + " is not unique!");
		}
	}

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

	for (let i: number = 0; i < lan.length; i++) {

		// WordOfFlags 

		let flags: number = 0;

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Flags = " + flags + "\r\n");
		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Flags", flags) == false) {
			return false;
		}
		ptr += 2;

		// IP

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " IP = " + ipToString(lan[i].ip) + "\r\n");

		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " IP", lan[i].ip) == false) {
			return false;
		}
		ptr += 4;

		// Port

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Port = " + lan[i].port + "\r\n");

		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Port", lan[i].port) == false) {
			return false;
		}
		ptr += 2;

		// ServiceIP

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Service IP = " + ipToString(lan[i].serviceIP) + "\r\n");

		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Service IP", lan[i].serviceIP) == false) {
			return false;
		}
		ptr += 4;

		// ServicePort

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " Service Port = " + lan[i].servicePort + "\r\n");

		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " Service Port", lan[i].servicePort) == false) {
			return false;
		}
		ptr += 2;

		// WordsCount

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " words count = " + lan[i].wordsCount + "\r\n");

		if (setData16(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " words count", lan[i].wordsCount) == false) {
			return false;
		}
		ptr += 2;

		// DUID

		confFirmware.writeLog("    [" + frame + ":" + ptr + "] : SUBN " + (i + 1) + " DUID = " + lan[i].dataID + "\r\n");

		if (setData32(confFirmware, log, LMNumber, controllerEquipmentID, frame, ptr, "SUBN " + (i + 1) + " DUID", lan[i].dataID) == false) {
			return false;
		}
		ptr += 4;

		ptr += 4;	// Reserved

	}
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
		"; [" + frame + ":" + (ptr + 6) + "] ModuleId = " + moduleId.toString(16) + "h\r\n");

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
